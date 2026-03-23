#include <sched/task_ctx.hpp>
void kthread_ptr::put_thread_base() const noexcept { asm volatile("wrfsbase %0" :: "r"(thread_ptr->self) : "memory"); }
bool kthread_ptr::is_interruptible() const noexcept
{
	if(thread_ptr && thread_ptr->ctl_info.park)
		return thread_ptr->ctl_info.non_timed_park;
	else if(task_ptr->task_ctl.block)
		return task_ptr->task_ctl.can_interrupt;
	return false;
}
bool kthread_ptr::is_blocking() const noexcept
{
	if(thread_ptr)
		return thread_ptr->ctl_info.park;
	else return task_ptr->task_ctl.block;
}
void kthread_ptr::add_wait_ticks(clock_t ticks) const noexcept
{
	if(thread_ptr)
		thread_ptr->ctl_info.wait_time_delta	+= ticks;
	else task_ptr->task_ctl.wait_ticks_delta	+= ticks;
}
void kthread_ptr::wait_tick() const noexcept
{
	if(thread_ptr)
		thread_ptr->ctl_info.wait_time_delta--;
	else task_ptr->task_ctl.wait_ticks_delta--;
}
clock_t kthread_ptr::get_wait_delta() const noexcept
{
	if(thread_ptr)
		return thread_ptr->ctl_info.wait_time_delta;
	else return task_ptr->task_ctl.wait_ticks_delta;
}
void kthread_ptr::activate() const noexcept
{
	if(!task_ptr->task_ctl.vfork_dirty)
	{
		task_ptr->saved_regs	= thread_ptr->saved_regs;
		task_ptr->fxsv			= thread_ptr->fxsv;
		task_ptr->thread_ptr	= thread_ptr->self;
	}
}
void kthread_ptr::set_blocking(bool can_interrupt) const noexcept
{
	if(thread_ptr) {
		thread_ptr->ctl_info.park				= true;
		thread_ptr->ctl_info.non_timed_park		= can_interrupt;
	}
	else {
		task_ptr->task_ctl.block				= true;
		task_ptr->task_ctl.can_interrupt		= can_interrupt;
	}
}
void kthread_ptr::clear_blocking() const noexcept
{
	task_ptr->task_ctl.block					= false;
	task_ptr->task_ctl.wait_ticks_delta			= 0U;
	task_ptr->task_ctl.can_interrupt			= false;
	if(thread_ptr)
	{
		thread_ptr->ctl_info.park				= false;
		thread_ptr->ctl_info.wait_time_delta	= 0UL;
		thread_ptr->ctl_info.non_timed_park		= false;
	}
}
void kthread_ptr::set_wait_delta(clock_t ticks) const noexcept
{
	if(thread_ptr) {
		thread_ptr->ctl_info.wait_time_delta	= ticks;
		thread_ptr->ctl_info.non_timed_park		= false;
	}
	else task_ptr->task_ctl.wait_ticks_delta 	= ticks;
}