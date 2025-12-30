#include <sched/worker.hpp>
#include <sched/scheduler.hpp>
extern "C"
{
	register_t worker_add(ooos::worker* w) { return scheduler::add_worker_task(kthread_ptr(std::addressof(w->task_struct), w->task_struct.thread_ptr)), 0Z; }
	register_t worker_entry(ooos::worker* w) { return w ? w->fn() : -1Z; }
	[[noreturn]] void worker_finish(ooos::worker* w, register_t code)
	{
		scheduler::remove_worker_task(kthread_ptr(std::addressof(w->task_struct), w->task_struct.thread_ptr));
		worker_return(w->caller_info, code ? code : 1);
		__builtin_unreachable();
	}
}
namespace ooos
{
	void worker::init_state() noexcept
	{
		fx_save(std::addressof(task_struct));
		array_zero(task_struct.fxsv.xmm, sizeof(fx_state::xmm) / sizeof(int128_t));
		array_zero(task_struct.fxsv.stmm, sizeof(fx_state::stmm) / sizeof(long double));
		array_zero(reinterpret_cast<void**>(worker_sig_info.signal_handlers), num_signals);
		task_struct.saved_regs.rsp	-= 8Z;
		task_struct.saved_regs.rsp.assign(addr_t(worker_exit));
	}
	void worker::reset() noexcept
	{
		task_struct.saved_regs	= regstate_t
		{
			.rdi		{ reinterpret_cast<register_t>(this) },
			.rbp		{ stack_base.plus(stack_size) },
			.rsp		{ stack_base.plus(stack_size) },
			.rip		{ addr_t(worker_entry) },
			.cr3		{ get_kernel_cr3() },
			.rflags		{ ini_flags },
			.ds			{ kproc.saved_regs.ds },
			.ss			{ kproc.saved_regs.ss },
			.cs			{ kproc.saved_regs.cs },
		};
		task_struct.task_ctl	= tctl_t
		{
			{
				.block			{ false },
				.can_interrupt	{ false },
				.should_notify	{ false },
				.killed			{ false },
				.prio_base		{ priority_val::PVSYS }
			},
			{
				.parent_pid		{ 0 },
				.task_pid		{ get_id() },
				.task_uid		{ 0U },
				.task_gid		{ 0U },
				.signal_info	{ std::addressof(worker_sig_info) },
			}
		};
		array_zero(stack_base.as<char>(), stack_size);
		array_zero(caller_info, 1UZ);
		init_state();
	}
}