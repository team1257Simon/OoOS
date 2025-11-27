#include <sched/thread.hpp>
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
void kthread_ptr::add_wait_ticks(uint32_t ticks) const noexcept
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
unsigned int kthread_ptr::get_wait_delta() const noexcept
{
	if(thread_ptr)
		return static_cast<unsigned int>(thread_ptr->ctl_info.wait_time_delta);
	else return task_ptr->task_ctl.wait_ticks_delta;
}
void kthread_ptr::activate() const noexcept
{
	if(thread_ptr != task_ptr->thread_ptr && task_ptr->frame_ptr.deref<uint64_t>() == uframe_magic)
	{
		uframe_tag& frame		= task_ptr->frame_ptr.deref<uframe_tag>();
		thread_t* current		= frame.translate(task_ptr->thread_ptr);
		thread_t* next			= frame.translate(thread_ptr);
		if(current && next)
		{
			ooos::update_thread_state(*current, *task_ptr);
			task_ptr->saved_regs	= next->saved_regs;
			task_ptr->fxsv			= next->fxsv;
			task_ptr->thread_ptr	= next->self;
		}
		else klog("[EXEC/THREAD] W: virtual address fault; no thread change occurred");
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
void kthread_ptr::set_wait_delta(uint32_t ticks) const noexcept
{
	if(thread_ptr) {
		thread_ptr->ctl_info.wait_time_delta	= ticks;
		thread_ptr->ctl_info.non_timed_park		= false;
	}
	else task_ptr->task_ctl.wait_ticks_delta 	= ticks;
}