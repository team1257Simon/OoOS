#ifndef __USR_CB
#define __USR_CB
#include <sched/scheduler.hpp>
namespace ooos
{
	union [[gnu::may_alias]] callback_arg
	{
		register_t		integral_argument;
		addr_t			pointer_argument;
		register_data	register_data_argument;
	};
	struct user_callback_data
	{
		addr_t entry_point;
		addr_t landing;
		size_t stack_target_size;
		bool start_detached;
		kthread_ptr cb_thread;
		callback_arg arg;
	};
	template<typename T> concept register_passable	= (sizeof(T) <= sizeof(register_t));
	struct user_callback_base
	{
		task_ctx* execution_ctx;
		user_callback_data data;
		size_t stack_offs;
		addr_t stack_real;
		void trigger(callback_arg arg);
	protected:
		virtual void vinit(thread_t& thread) = 0;
	};
	template<typename AT>
	struct user_callback : public user_callback_base
	{
		constexpr static size_t needed_stack_size	= up_to_nearest(sizeof(AT), alignof(register_t));
		constexpr user_callback(user_callback_data&& data, task_ctx* ctx) noexcept : user_callback_base(ctx, std::move(data), needed_stack_size, nullptr) {}
		void operator()(AT const& arg) { this->trigger(callback_arg{ .pointer_argument = addr_t(std::addressof(arg)) }); }
	protected:
		virtual void vinit(thread_t& thread) override
		{
			AT const* in_arg_value	= data.arg.pointer_argument;
			AT* arg_pos				= stack_real;
			array_copy(arg_pos, in_arg_value, 1UZ);
			thread.saved_regs.rdi	= static_cast<register_t>(thread.saved_regs.rbp.full);
		}
	};
	template<register_passable AT>
	struct user_callback<AT> : public user_callback_base
	{
		constexpr user_callback(user_callback_data&& data, task_ctx* ctx) noexcept : user_callback_base(ctx, std::move(data), 0UZ, nullptr) {}
		void operator()(AT a) { this->trigger(callback_arg{ .register_data_argument	= register_data(a) }); }
	protected:
		virtual void vinit(thread_t& thread) override { thread.saved_regs.rdi		= data.arg.integral_argument; }
	};
}
#endif