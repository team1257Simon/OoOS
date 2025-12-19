#include <util/user_callback.hpp>
#include <ext/type_erasure.hpp>
namespace ooos
{
	void reset_callback(void* cb)
	{
		user_callback_base* callback	= std::ext::reflective_cast<user_callback_base>(cb);
		if(callback) callback->reset();
		else throw std::bad_cast();
	}
	void user_callback_base::trigger(callback_arg arg)
	{
		bool is_retrigger				= data.cb_thread;
		thread_t* cur					= execution_ctx->current_thread_ptr();
		if(!cur) throw std::out_of_range("[EXEC/CB] virtual address fault");
		update_thread_state(*cur, execution_ctx->task_struct);
		thread_t* created;
		if(is_retrigger) created		= data.cb_thread.thread_ptr;
		else
		{
			created						= execution_ctx->thread_init(*cur, false, data.stack_target_size, data.start_detached);
			created->saved_regs.rsp		-= static_cast<ptrdiff_t>(stack_offs);
			stack_real					= execution_ctx->get_frame().translate(created->saved_regs.rsp);
			if(!stack_real) throw std::out_of_range("[EXEC/THREAD] virtual address fault");
			created->saved_regs.rip		= data.entry_point;
			created->ctl_info.reset_cb	= reset_callback;
			created->ctl_info.reset_arg	= this;
			data.initial_fpstate		= created->fxsv;
			data.initial_regstate		= created->saved_regs;
			data.initial_ctlstate		= created->ctl_info;
		}
		data.arg				= arg;
		insert_arg(*created);
		created->saved_regs.rsp	-= sizeof(register_t);
		stack_real				-= sizeof(register_t);
		stack_real.assign(data.landing);
		execution_ctx->thread_ptr_by_id.insert(std::make_pair(created->ctl_info.thread_id, created));
		kthread_ptr result(std::addressof(execution_ctx->task_struct), created);
		sch.register_task(result);
		created->ctl_info.state	= thread_state::RUNNING;
	}
	void ooos::user_callback_base::reset()
	{
		thread_t* thread	= data.cb_thread.thread_ptr;
		if(thread)
		{
			thread->saved_regs					= data.initial_regstate;
			thread->fxsv						= data.initial_fpstate;
			thread->ctl_info					= data.initial_ctlstate;
		}
	}
}