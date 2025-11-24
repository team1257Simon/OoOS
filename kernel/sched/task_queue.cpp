#include "sched/task_queue.hpp"
#include "arch/arch_amd64.h"
#include "sched/scheduler.hpp"
extern "C" tss system_tss;
constexpr static unsigned int get_ticks_for(task_wait_queue::const_iterator where) { return where->thread_ptr ? static_cast<unsigned int>(where->thread_ptr->ctl_info.wait_time_delta) : where->task_ptr->task_ctl.wait_ticks_delta; }
void task_pl_queue::on_skipped() noexcept { if(!empty() && !at_end()) { next()->task_ctl.skips++; } }
bool task_pl_queue::skip_flag() noexcept { return !empty() && !at_end() && next()->task_ctl.skips > __skips_threshold; }
bool task_pl_queue::transfer_next(task_ptr_queue_base& to_whom) { return !empty() && !at_end() && transfer(to_whom) != 0; }
task_pl_queue& prio_level_task_queues::operator[](priority_val pv) noexcept { return __base::operator[](__idx_by_prio(pv)); }
task_pl_queue const& prio_level_task_queues::operator[](priority_val pv) const noexcept { return __base::operator[](__idx_by_prio(pv)); }
constexpr static void add_ticks_for(task_wait_queue::const_iterator where, uint32_t how_many)
{
	if(where->thread_ptr)
		where->thread_ptr->ctl_info.wait_time_delta	+= how_many;
	else where->task_ptr->task_ctl.wait_ticks_delta	+= how_many;
}
extern "C" void init_tss(addr_t k_rsp)
{
	system_tss.rsp[0] 		= k_rsp;
	system_tss.rsp[1] 		= k_rsp;
	system_tss.rsp[2] 		= k_rsp;
	for(int i = 0; i < 7; i++)
		system_tss.ist[i] 	= k_rsp;
}
unsigned int task_wait_queue::next_remaining_wait_ticks() const noexcept
{
	if(!empty() && !at_end())
	{
		kthread_ptr const& thr	= next();
		if(thr.thread_ptr) return thr.thread_ptr->ctl_info.wait_time_delta;
		else return thr->task_ctl.wait_ticks_delta;
	}
	return 0U;
}
unsigned int task_wait_queue::cumulative_remaining_ticks() const noexcept
{
	unsigned int result		= 0;
	for(const_iterator i	= current(); i != end(); i++) result += get_ticks_for(i);
	return result;
}
bool task_wait_queue::interrupt_wait(const_iterator where)
{
	if(__unlikely(!(where < end() && where->task_ptr->task_ctl.can_interrupt))) return false;
	if(const_iterator subs		= where + 1; subs < end()) add_ticks_for(subs, get_ticks_for(where));
	clear_blocking(*where);
	return erase(where) != 0;
}
void task_wait_queue::tick_wait() noexcept
{
	if(!empty() && !at_end())
	{
		kthread_ptr& thr	= next();
		if(thr.thread_ptr) thr.thread_ptr->ctl_info.wait_time_delta--;
		else thr->task_ctl.wait_ticks_delta--;
	}
}