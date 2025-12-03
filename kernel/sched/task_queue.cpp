#include <sched/scheduler.hpp>
void task_pl_queue::on_skipped() noexcept { if(!empty() && !at_end()) { next()->task_ctl.skips++; } }
bool task_pl_queue::skip_flag() noexcept { return !empty() && !at_end() && next()->task_ctl.skips > __skips_threshold; }
bool task_pl_queue::transfer_next(task_ptr_queue_base& to_whom) { return !empty() && !at_end() && transfer(to_whom) != 0; }
task_pl_queue& prio_level_task_queues::operator[](priority_val pv) noexcept { return __base::operator[](__idx_by_prio(pv)); }
task_pl_queue const& prio_level_task_queues::operator[](priority_val pv) const noexcept { return __base::operator[](__idx_by_prio(pv)); }
clock_t task_wait_queue::next_remaining_wait_ticks() const noexcept
{
	if(!empty() && !at_end())
		return next().get_wait_delta();
	return 0U;
}
clock_t task_wait_queue::cumulative_remaining_ticks() const noexcept
{
	clock_t result		= 0UL;
	for(const_iterator i	= current(); i != end(); i++) result += i->get_wait_delta();
	return result;
}
bool task_wait_queue::interrupt_wait(const_iterator where)
{
	if(__unlikely(!(where < end() && where->is_interruptible()))) return false;
	if(const_iterator subs		= where + 1; subs < end()) subs->add_wait_ticks(where->get_wait_delta());
	where->clear_blocking();
	return erase(where) != 0;
}
void task_wait_queue::tick_wait() noexcept
{
	if(empty() || at_end())
		return;
	next().wait_tick();
}