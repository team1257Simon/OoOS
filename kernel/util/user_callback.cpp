#include <util/user_callback.hpp>
namespace ooos
{
	void ooos::user_callback_base::trigger(callback_arg arg)
	{
		thread_t* cur			= execution_ctx->current_thread_ptr();
		if(!cur) throw std::out_of_range("[EXEC/CB] virtual address fault");
		update_thread_state(*cur, execution_ctx->task_struct);
		thread_t* created		= execution_ctx->thread_init(*cur, false, data.stack_target_size, data.start_detached);
		data.arg				= arg;
		created->saved_regs.rip	= data.entry_point;
		created->saved_regs.rsp	-= static_cast<ptrdiff_t>(stack_offs);
		stack_real				= execution_ctx->get_frame().translate(created->saved_regs.rsp);
		if(!stack_real) throw std::out_of_range("[EXEC/THREAD] virtual address fault");
		vinit(*created);
		created->saved_regs.rsp	-= sizeof(register_t);
		stack_real				-= sizeof(register_t);
		stack_real.assign(data.landing);
		execution_ctx->thread_ptr_by_id.insert(std::make_pair(created->ctl_info.thread_id, created));
		kthread_ptr result(std::addressof(execution_ctx->task_struct), created);
		sch.register_task(result);
		created->ctl_info.state	= thread_state::RUNNING;
	}
}