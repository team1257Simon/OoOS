#include "sched/scheduler.hpp"
#include "isr_table.hpp"
#include "arch/arch_amd64.h"
using enum priority_val;
extern "C" 
{
	extern std::atomic<bool> task_change_flag;
	extern std::atomic<bool> task_lock_flag;
	extern std::atomic<int8_t> task_signal_code;
	extern task_t kproc;
	extern void direct_reentry(task_t*) attribute(noreturn);
}
scheduler scheduler::__instance;
bool scheduler::__has_init;
scheduler::scheduler() = default;
bool scheduler::has_init() noexcept { return __has_init; }
bool scheduler::init_instance() noexcept { return has_init() || (__has_init	= __instance.init()); }
scheduler& scheduler::get() noexcept { return __instance; }
void scheduler::register_task(task_t* task) { __queues[task->task_ctl.prio_base].push(task); __total_tasks++; }
bool scheduler::__set_untimed_wait(task_t* task)
{
	try 
	{
		__non_timed_sleepers.push_back(task); 
		task->task_ctl.block			= true;
		task->task_ctl.can_interrupt	= true;
		return true;
	}
	catch(std::exception& e) { panic(e.what()); }
	return false;
}
bool scheduler::interrupt_wait(task_t* waiting)
{
	task_wait_queue::const_iterator i 			= __sleepers.find(waiting);
	if(i != __sleepers.end() && (*i)->task_ctl.can_interrupt)
		return __sleepers.interrupt_wait(i);
	else if(std::vector<task_t*>::iterator i 	= __non_timed_sleepers.find(waiting); i != __non_timed_sleepers.end()) 
	{
		task_t* task							= *i;
		task->task_ctl.block					= false;
		__non_timed_sleepers.erase(i);
		return true;
	}
	return false;
}
bool scheduler::__set_wait_time(task_t* task, unsigned int time, bool can_interrupt)
{
	task->task_ctl.block			= true;
	task->task_ctl.can_interrupt	= can_interrupt;
	unsigned int total				= __sleepers.cumulative_remaining_ticks();
	if(time < total)
	{
		unsigned int cumulative 	= 0;
		for(task_wait_queue::const_iterator i	= __sleepers.current(); i != __sleepers.end(); i++)
		{
			unsigned int cwait 		= (*i)->task_ctl.wait_ticks_delta;
			if(cwait + cumulative > time) {
				task->task_ctl.wait_ticks_delta		= time - cumulative;
				return __sleepers.insert(i, task)	!= __sleepers.end();
			}
			cumulative 				+= cwait;
		}
	}
	task->task_ctl.wait_ticks_delta	= time - total;
	__sleepers.push(task);
	return true;
}
void scheduler::__do_task_change(task_t* cur, task_t* next)
{
	next->quantum_rem	= next->quantum_val;
	cur->next			= next;
	task_change_flag.store(true);
	uint64_t ts			= __timestamp_stopwatch.get();
	next->run_split		= ts;
	cur->run_time		+= (ts - cur->run_split);
}
bool scheduler::unregister_task(task_t* task) 
{
	bool result			= false;
	if(task->task_ctl.prio_base	== PVSYS)
	{
		task_pl_queue::const_iterator i			= __queues[PVSYS].find(task, true); 
		if(i != __queues[PVSYS].end()) result	= __queues[PVSYS].erase(i) != 0;
		asm volatile("mfence" ::: "memory");
	}
	for(priority_val pv	= task->task_ctl.prio_base; pv <= PVEXTRA; ++pv)
	{
		task_pl_queue::const_iterator i		= __queues[pv].find(task, true); 
		if(i != __queues[pv].end())	result	= __queues[pv].erase(i) != 0;
		asm volatile("mfence" ::: "memory");
	}
	__sync_synchronize();
	if(__total_tasks) __total_tasks--;
	if(__unlikely(!__total_tasks)) set_gs_base(std::addressof(kproc));
	return result;
}
bool scheduler::unregister_task_tree(task_t* task)
{
	if(!task->num_child_procs || !task->child_procs) return unregister_task(task);
	bool result		= true;
	for(size_t i = 0UZ; i < task->num_child_procs; i++)
		result		&= unregister_task(task->child_procs[i]);
	return result && unregister_task(task);
}
task_t* scheduler::select_next()
{
	if(__unlikely(!__total_tasks)) return nullptr;
	task_t* target		= nullptr;
	for(priority_val pv	= PVSYS; pv >= PVLOW; --pv)
	{
		task_pl_queue& queue			= __queues[pv];
		if(!queue.empty())
		{
			task_pl_queue::iterator i	= queue.current(), j;
			task_t* result;
			do {
				if(queue.at_end()) queue.restart();
				fence();
				result					= queue.pop();
				fence();
				j						= queue.current();
			} while(result->task_ctl.block && i != j);
			if(result->task_ctl.block) continue;
			result->task_ctl.skips		= 0UC;
			if(result->task_ctl.prio_base != pv && pv != PVSYS)
			{ 
				__queues[pv].unpop();
				__queues[pv].transfer_next(__queues[pv - 1SC]);
				fence();
			}
			target						= result;
			for(priority_val qv	= pv - 1SC; qv >= PVLOW; --qv) 
			{ 
				queue.on_skipped(); 
				if(queue.skip_flag()) 
				{ 
					priority_val rv						= qv + 1SC;
					queue.transfer_next(__queues[rv]);
					__queues[rv].back()->task_ctl.skips	= 0UC;
				}
			}
			break;
		}
	}
	return target;
}
bool scheduler::set_wait_untimed(task_t* task)
{
	if(!task->task_ctl.block)
		return __set_untimed_wait(task);
	return false;
}
bool scheduler::set_wait_timed(task_t* task, unsigned int time, bool can_interrupt)
{
	if(!task->task_ctl.block)
		return __set_wait_time(task, static_cast<uint32_t>(__deferred_actions.compute_ticks(time)), can_interrupt);
	return false;
}
void scheduler::on_tick()
{
	if(__deferred_actions) __deferred_actions.tick();
	if(__unlikely(!__total_tasks)) return;
	if(task_lock_flag) return;
	__sleepers.tick_wait();
	if(!__sleepers.at_end())
	{ 
		task_t* front_sleeper		= __sleepers.next();
		while(front_sleeper && front_sleeper->task_ctl.wait_ticks_delta	== 0)
		{
			task_t* wakee			= __sleepers.pop();
			wakee->task_ctl.block	= false;
			front_sleeper			= __sleepers.next();
		} 
	}
	task_t* cur					= get_task_base();
	if(cur->quantum_rem) cur->quantum_rem--;
	if(cur->quantum_rem	== 0 || cur->task_ctl.block)
	{
		if(task_t* next			= select_next())
			__do_task_change(cur, next);
		else cur->quantum_rem	= cur->quantum_val;
	}
}
__nointerrupts bool scheduler::init() noexcept
{
	try
	{
		__sleepers.reserve(16UZ);
		__non_timed_sleepers.reserve(16UZ);
		for(prio_level_task_queues::iterator i	= __queues.begin(); i != __queues.end(); i++) i->reserve(16);
		__deferred_actions.create_if_empty(4UZ);
	}
	catch(std::exception& e) { panic(e.what()); return false; }
	task_change_flag.store(false);
	uint32_t timer_frequency		= cpuid(0x15U, 0).ecx;
	if(!timer_frequency)
		timer_frequency				= cpuid(0x16U, 0).ecx;
	__cycle_divisor					= timer_frequency;
	__tick_rate						= magnitude(timer_frequency);
	__deferred_actions.ticks_per_ms	= (1.0L / static_cast<long double>(__cycle_divisor)) * __tick_rate;
	interrupt_table::add_irq_handler(0, std::move([this]() -> void
	{
		if(__running)
		{
			__tick_cycles		+= __tick_rate;
			if(__tick_cycles >= __cycle_divisor) on_tick();
			__tick_cycles		= __tick_cycles % __cycle_divisor;
		}
	}));
	interrupt_table::add_interrupt_callback([this](byte idx, qword) -> void
	{
		if(idx < 0x20UC)
		{
			task_t* task_base	= get_task_base();
			uint64_t fmagic		= task_base->frame_ptr.deref<uint64_t>();
			if(fmagic			!= uframe_magic) return;
			task_signal_code	= exception_signals[idx];
		}
	});
	__total_tasks 					= 0UZ;
	__timestamp_stopwatch.start();
	return true;
}
task_t* scheduler::yield()
{
	task_t* cur			= current_active_task();
	cur->quantum_rem	= 0US;
	task_t* next		= select_next();
	if(!next) next		= cur; 
	else asm volatile("swapgs; wrgsbase %0; swapgs" :: "r"(next) : "memory"); 
	next->quantum_rem	= next->quantum_val;
	return next;
}
task_t* scheduler::fallthrough_yield()
{
	if(!__total_tasks) return nullptr;
	task_t* next				= select_next();
	if(next) next->quantum_rem	= next->quantum_val;
	return next;
}
void scheduler::add_worker_task(task_t* w)
{
	__instance.register_task(w);
	task_t* cur = get_task_base();
	if(cur == std::addressof(kproc))
		asm volatile("swapgs; wrgsbase %0; swapgs" :: "r"(w) : "memory");
}
void scheduler::remove_worker_task(task_t* w)
{
	task_t* cur			= get_task_base();
	if(cur == w) {
		task_t* next	= __instance.fallthrough_yield();
		if(next && next != cur) __instance.__do_task_change(cur, next);
	}
	__instance.unregister_task(w);
}