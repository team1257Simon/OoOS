#ifndef __WORKER
#define __WORKER
#include <sched/task.h>
#include <sched/deferred_action.hpp> // extended runnable, etc
namespace ooos { class worker; }
extern "C"
{
	extern void worker_exit();
	extern task_t kproc;
	int worker_entry(ooos::worker* w);
}
namespace ooos
{
	namespace __internal
	{
		template<typename FT> concept __worker_fn = __extended_runnable<FT> && no_args_supplier<FT, int>;
		struct __worker_func_base
		{
			template<typename T> constexpr static bool not_empty(T t) { return isr_actor_base::not_empty(t); }
			template<__worker_fn FT>
			struct manager
			{
				template<typename GT> constexpr static void construct(__extended_storage& dest, GT&& gt) noexcept(noexcept(FT(std::declval<GT>()))) { new(dest.get()) FT(std::forward<GT>(gt)); }
				constexpr static void destruct(__extended_storage& tgt) noexcept(std::is_nothrow_destructible_v<FT>) { tgt.template get_as<FT>()->~FT(); }
				constexpr static FT* get_ptr(__extended_storage& fn) noexcept { return fn.template get_as<FT>(); }
				constexpr static FT const* get_ptr(__extended_storage const& fn) { return fn.template get_as<FT>(); }
				static int invoke(__extended_storage& fn) noexcept(std::is_nothrow_invocable_v<FT>) { return static_cast<int>(__invoke_f(std::forward<FT>(*get_ptr(fn)))); }
				static void action(__extended_storage& dst, __extended_storage const& src, mgr_op op)
				{
					switch(op)
					{
					case get_pointer:
						dst.write_ptr(get_ptr(src));
						break;
					case get_type_info:
						dst.write_ptr(std::addressof(typeid(FT)));
						break;
					case clone:
						construct(dst, *get_ptr(src));
						break;
					case move:
						construct(dst, std::move(*get_ptr(const_cast<__extended_storage&>(src))));
					case destroy:
						destruct(dst);
						break;
					}
				}
			};
			typedef void (*manager_type)(__extended_storage&, __extended_storage const&, mgr_op);
			typedef int (*invoker_type)(__extended_storage&);
			manager_type __manager;
			invoker_type __invoker;
			__extended_storage __functor;
			constexpr bool __empty() const noexcept { return !__manager || !__invoker; }
			constexpr ~__worker_func_base() { if(__manager) (*__manager)(__functor, __functor, destroy); }
			constexpr __worker_func_base() noexcept = default;
		};
	}
	class worker_function : __internal::__worker_func_base
	{
		using __base = __internal::__worker_func_base;
		template<__internal::__worker_fn FT> constexpr static bool __nt_construct() noexcept { return noexcept(__base::manager<FT>::construct(std::declval<__internal::__extended_storage&>(), std::declval<FT>())); }
	public:
		constexpr operator bool() const noexcept { return !__empty(); }
		constexpr worker_function() noexcept = default;
		constexpr ~worker_function() noexcept = default;
		template<__internal::__worker_fn FT>
		constexpr worker_function(FT&& ft) noexcept(__nt_construct<FT>()) : __base()
		{
			typedef __base::manager<std::decay_t<FT>> __mgr;
			if(__base::not_empty(ft))
			{
				__mgr::construct(__functor, std::forward<FT>(ft));
				__manager	= std::addressof(__mgr::action);
				__invoker	= std::addressof(__mgr::invoke);
			}
		}
		constexpr worker_function(worker_function const& that) : __base()
		{
			if(that)
			{
				that.__manager(this->__functor, that.__functor, clone);
				this->__manager	= that.__manager;
				this->__invoker	= that.__invoker;
			}
		}
		constexpr worker_function(worker_function&& that) : __base()
		{
			if(that)
			{
				that.__manager(this->__functor, that.__functor, move);
				this->__manager	= that.__manager;
				this->__invoker	= that.__invoker;
				that.__manager	= nullptr;
				that.__invoker	= nullptr;
			}
		}
		constexpr int operator()()
		{
			if(__unlikely(__empty()))
				return -1;
			return (*__invoker)(__functor);
		}
	};
	struct worker
	{
		task_t task_struct;
		jmp_buf caller_info;
		worker_function fn;
		addr_t stack_base;
		size_t stack_size;
		task_signal_info_t worker_sig_info;
		template<__internal::__worker_fn FT> inline worker(FT&& wfn, addr_t stack, size_t stacksz, pid_t tid, uint16_t qv);
		constexpr pid_t get_id() const noexcept { return task_struct.task_ctl.task_id; }
		friend constexpr std::strong_ordering operator<=>(worker const& __this, worker const& __that) noexcept { return __this.get_id() <=> __that.get_id(); }
		friend constexpr std::strong_ordering operator<=>(worker const& __this, pid_t const& __that) noexcept { return __this.get_id() <=> __that; }
		friend constexpr std::strong_ordering operator<=>(pid_t const& __this, worker const& __that) noexcept { return __this <=> __that.get_id(); }
		friend constexpr bool operator==(worker const& __this, worker const& __that) noexcept { return __this.task_struct.self == __that.task_struct.self; }
	private:
		void init_state();
		void stack_push(addr_t value);
	} __align(16);
	template<__internal::__worker_fn FT>
	inline worker::worker(FT&& wfn, addr_t stack, size_t stacksz, pid_t tid, uint16_t qv) :
		task_struct
		{
			.self			{ std::addressof(task_struct) },
			.frame_ptr		{ kproc.frame_ptr },
			.saved_regs
			{
				.rdi		{ reinterpret_cast<register_t>(this) },
				.rbp		{ stack.plus(stacksz) },
				.rsp		{ stack.plus(stacksz) },
				.rip		{ addr_t(worker_entry) },
				.cr3		{ get_kernel_cr3() },
				.rflags		{ ini_flags },
				.ds			{ kproc.saved_regs.ds },
				.ss			{ kproc.saved_regs.ss },
				.cs			{ kproc.saved_regs.cs },
			},
			.quantum_val	{ qv },
			.task_ctl
			{
				{
					.block			{ false },
					.can_interrupt	{ false },
					.should_notify	{ false },
					.killed			{ false },
					.prio_base		{ priority_val::PVSYS }
				},
				{
					.signal_info	{ std::addressof(worker_sig_info) },
					.parent_pid		{ 0 },
					.task_id		{ tid }
				}
			},
			.tls_master				{ nullptr },
			.tls_size				{ 0UZ },
			.tls_align				{ 0UZ }
		},
		caller_info			{},
		fn					{ std::forward<FT>(wfn) },
		stack_base			{ stack },
		stack_size			{ stacksz },
		worker_sig_info		{}
							{ init_state(); }
}
extern "C" int start_worker(ooos::worker* w) attribute(returns_twice);
#endif