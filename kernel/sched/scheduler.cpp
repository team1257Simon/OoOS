#include <arch/arch_amd64.h>
#include <sched/scheduler.hpp>
#include <isr_table.hpp>
using enum priority_val;
extern "C"
{
	extern std::atomic<bool> task_change_flag;
	extern std::atomic<bool> task_lock_flag;
	extern std::atomic<int8_t> task_signal_code;
	extern task_t kproc;
	extern void direct_reentry(kthread_ptr) attribute(noreturn);
}
typedef task_wait_queue::const_iterator waiterator;
typedef std::vector<kthread_ptr>::iterator ntwaiterator;
typedef task_pl_queue::const_iterator task_citerator;
typedef task_pl_queue::iterator task_iterator;
scheduler scheduler::__instance;
bool scheduler::__has_init;
scheduler::scheduler() = default;
void write_task_base(task_t const& next) { asm volatile("swapgs; wrgsbase %0; swapgs" :: "r"(std::addressof(next)) : "memory"); }
bool scheduler::has_init() noexcept { return __has_init; }
bool scheduler::init_instance() noexcept { return has_init() || (__has_init	= __instance.init()); }
scheduler& scheduler::get() noexcept { return __instance; }
void scheduler::add_worker_task(kthread_ptr const& w) { __instance.register_task(w); }
void scheduler::register_task(kthread_ptr const& task) {
	__queues[task->task_ctl.prio_base].push(task);
	__total_tasks++;
}
static kthread_ptr kthread_of(task_t* task_base)
{
	uint64_t fmagic	= task_base->frame_ptr.deref<uint64_t>();
	if(fmagic == uframe_magic) return kthread_ptr(task_base, task_base->frame_ptr.deref<uframe_tag>().translate(task_base->thread_ptr));
	return kthread_ptr(task_base, task_base->thread_ptr);
}
bool scheduler::__set_untimed_wait(kthread_ptr& task)
{
	try
	{
		__non_timed_sleepers.push_back(task);
		task.set_blocking(true);
		return true;
	}
	catch(std::exception& e) { panic(e.what()); }
	return false;
}
bool scheduler::interrupt_wait(kthread_ptr& waiting)
{
	waiterator i	= __sleepers.find(waiting);
	if(i			!= __sleepers.end()) return __sleepers.interrupt_wait(i);
	ntwaiterator j 	= __non_timed_sleepers.find(waiting);
	if(j			!= __non_timed_sleepers.end())
	{
		j->clear_blocking();
		__non_timed_sleepers.erase(j);
		return true;
	}
	return false;
}
bool scheduler::interrupt_wait(task_t* task)
{
	bool result			= false;
	for(waiterator i	= __sleepers.begin(); i != __sleepers.end(); i++)
	{
		if(i->task_ptr	!= task) continue;
		__sleepers.interrupt_wait(i);
		result			= true;
	}
	for(ntwaiterator i	= __non_timed_sleepers.begin(); i != __non_timed_sleepers.end(); i++)
	{
		if(i->task_ptr	!= task) continue;
		i->clear_blocking();
		__non_timed_sleepers.erase(i);
		result			= true;
	}
	return result;
}
bool scheduler::__set_wait_time(kthread_ptr& task, clock_t time, bool can_interrupt)
{
	task.set_blocking(can_interrupt);
	clock_t total				= __sleepers.cumulative_remaining_ticks();
	if(time < total)
	{
		clock_t cumulative		= 0UL;
		for(waiterator i		= __sleepers.current(); i != __sleepers.end(); i++)
		{
			clock_t cwait 		= i->get_wait_delta();
			if(cwait + cumulative > time)
			{
				task.set_wait_delta(static_cast<clock_t>(time - cumulative));
				waiterator res	= __sleepers.insert(i, task);
				return res		!= __sleepers.end();
			}
			cumulative 			+= cwait;
		}
	}
	task.set_wait_delta(static_cast<clock_t>(time - total));
	__sleepers.push(task);
	return true;
}
void scheduler::__do_task_change(kthread_ptr& cur, kthread_ptr& next)
{
	if(__unlikely(cur == next)) return;
	next->quantum_rem	= next->quantum_val;
	cur->next			= next.task_ptr;
	if(next.thread_ptr) next.activate();
	task_change_flag.store(true);
	uint64_t ts			= __timestamp_stopwatch.get();
	next->run_split		= ts;
	cur->run_time		+= (ts - cur->run_split);
}
bool scheduler::unregister_task(task_t* task)
{
	size_t result			= 0UZ;
	for(priority_val pv 	= task->task_ctl.prio_base; pv <= PVSYS; ++pv)
	{
		task_citerator i	= __queues[pv].find_like(task, true);
		if(i != __queues[pv].end())
			result			+= __queues[pv].erase(i) ? 1UZ : 0UZ;
	}
	fence();
	if(__total_tasks && result <= __total_tasks) __total_tasks -= result;
	return result != 0UZ;
}
bool scheduler::unregister_task(kthread_ptr const& task)
{
	bool result				= false;
	for(priority_val pv		= task->task_ctl.prio_base; pv <= PVSYS ; ++pv)
	{
		task_citerator i	= __queues[pv].find(task, true);
		if(i != __queues[pv].end()) {
			result			= __queues[pv].erase(i) != 0;
			break;
		}
	}
	fence();
	if(__total_tasks && result) __total_tasks--;
	return result;
}
kthread_ptr scheduler::select_next()
{
	if(__unlikely(!__total_tasks)) return kthread_ptr(nullptr, nullptr);
	kthread_ptr target(nullptr, nullptr);
	for(priority_val pv	= PVSYS; pv >= PVLOW; --pv)
	{
		task_pl_queue& queue	= __queues[pv];
		if(!queue.empty())
		{
			task_citerator i	= queue.current(), j;
			kthread_ptr result;
			do {
				if(queue.at_end()) queue.restart();
				fence();
				result			= queue.pop();
				fence();
				j				= queue.current();
			} while(result.is_blocking() && i != j);
			if(result.is_blocking()) continue;
			result->task_ctl.skips	= 0UC;
			if(result->task_ctl.prio_base != pv && pv != PVSYS)
			{
				__queues[pv].unpop();
				__queues[pv].transfer_next(__queues[pv - 1SC]);
				fence();
			}
			target					= result;
			for(priority_val qv		= pv - 1SC; qv >= PVLOW; --qv)
			{
				queue.on_skipped();
				if(queue.skip_flag())
				{
					priority_val rv		= qv + 1SC;
					queue.transfer_next(__queues[rv]);
					kthread_ptr& b		= __queues[rv].back();
					b->task_ctl.skips	= 0UC;
				}
			}
			break;
		}
	}
	return target;
}
bool scheduler::set_wait_untimed(kthread_ptr& task)
{
	if(!task.is_blocking())
		return __set_untimed_wait(task);
	return false;
}
bool scheduler::set_wait_timed(kthread_ptr& task, clock_t time, bool can_interrupt)
{
	if(!task.is_blocking())
		return __set_wait_time(task, static_cast<clock_t>(__deferred_actions.compute_ticks(time)), can_interrupt);
	return false;
}
void scheduler::retrothread(task_t* task, thread_t* thread)
{
	kthread_ptr srch(task, nullptr);
	for(priority_val pv		= task->task_ctl.prio_base; pv <= PVSYS; ++pv)
		if(task_iterator i	= __queues[pv].find(srch); i != __queues[pv].end())
			i->thread_ptr	= thread;
}
void scheduler::on_tick()
{
	if(__deferred_actions) __deferred_actions.tick();
	if(__unlikely(!__total_tasks)) return;
	if(task_lock_flag) return;
	__sleepers.tick_wait();
	if(!__sleepers.at_end())
	{
		kthread_ptr& front_sleeper	= __sleepers.next();
		while(front_sleeper && !front_sleeper.get_wait_delta())
		{
			kthread_ptr wakee		= __sleepers.pop();
			wakee.clear_blocking();
			front_sleeper			= __sleepers.next();
		}
	}
	task_t* cptr				= get_task_base();
	kthread_ptr cur				= kthread_of(cptr);
	if(cur->quantum_rem) cur->quantum_rem--;
	if(cur->quantum_rem	== 0 || cur.is_blocking())
	{
		if(kthread_ptr next		= select_next())
			__do_task_change(cur, next);
		else cur->quantum_rem	= cur->quantum_val;
	}
}
__nointerrupts bool scheduler::init() noexcept
{
	typedef prio_level_task_queues::iterator meta_iterator;
	try
	{
		__sleepers.reserve(16UZ);
		__non_timed_sleepers.reserve(16UZ);
		for(meta_iterator i			= __queues.begin(); i != __queues.end(); i++) i->reserve(16);
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
			uint64_t fmagic		= get_task_base()->frame_ptr.deref<uint64_t>();
			if(fmagic			!= uframe_magic) return;
			task_signal_code	= exception_signals[idx];
		}
	});
	__total_tasks 				= 0UZ;
	__timestamp_stopwatch.start();
	return true;
}
kthread_ptr scheduler::yield()
{
	task_t* cptr		= current_active_task();
	kthread_ptr cur		= kthread_of(cptr);
	cur->quantum_rem	= 0US;
	kthread_ptr next	= select_next();
	if(!next) next		= cur;
	else write_task_base(*next);
	next->quantum_rem	= next->quantum_val;
	if(next.thread_ptr)	next.activate();
	return next;
}
kthread_ptr scheduler::fallthrough_yield()
{
	if(!__total_tasks) return kthread_ptr(nullptr, nullptr);
	kthread_ptr next			= select_next();
	if(next) next->quantum_rem	= next->quantum_val;
	if(next && next.thread_ptr) next.activate();
	return next;
}
void scheduler::remove_worker_task(kthread_ptr const& w)
{
	task_t* cptr			= get_task_base();
	kthread_ptr cur			= kthread_of(cptr);
	if(cptr == w)
	{
		kthread_ptr next	= __instance.fallthrough_yield();
		if(next && next != cptr)
			__instance.__do_task_change(cur, next);
	}
	__instance.unregister_task(w.task_ptr);
	if(__unlikely(!__instance.__total_tasks)) set_gs_base(std::addressof(kproc));
}