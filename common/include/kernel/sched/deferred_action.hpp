#ifndef __DEFERRED_ACTION
#define __DEFERRED_ACTION
#include <kernel_api.hpp>
#include <util/circular_queue.hpp>
namespace ooos
{
	namespace __internal
	{
		union __extended_runnables
		{
			struct __optr {
				void* ptr;
				register_t bound_args[4];
			} object;
			struct __coptr {
				const void* ptr;
				register_t bound_args[4];
			} const_object;
			struct __fptr {
				void (*fnptr)(...);
				register_t bound_args[4];
			} functor;
			struct __mfptr {
				void (__anything::*memfn)(...);
				register_t bound_args[3];
			} member_functor;
		};
		union [[gnu::may_alias]] __extended_storage
		{
		private:
			__extended_runnables __ignore;
			char data[sizeof(__extended_runnables)];
			constexpr void* __ptr() noexcept { return std::addressof(data[0]); }
			constexpr const void* __ptr() const noexcept { return std::addressof(data[0]); }
		public:
			constexpr void* get() noexcept { return __ptr(); }
			constexpr const void* get() const noexcept { return __ptr(); }
			template<typename T> constexpr T* get_as() noexcept { return static_cast<T*>(__ptr()); }
			template<typename T> constexpr T const* get_as() const noexcept { return static_cast<T const*>(__ptr()); }
			constexpr void write_ptr(const void* ptr) noexcept { *get_as<const void*>() = ptr; }
		};
		template<typename T>
		concept __extended_runnable	=	no_args_invoke<T>
									&&	sizeof(T) <= sizeof(__extended_runnables)
									&&	alignof(T) <= alignof(__extended_runnables)
									&&	!(alignof(__extended_runnables) % alignof(T));
		struct __deferred_action_base
		{
			template<typename T> constexpr static bool not_empty(T t) { return isr_actor_base::not_empty(t); }
			template<__extended_runnable FT>
			struct manager
			{
				template<typename GT> constexpr static void construct(__extended_storage& dest, GT&& gt) noexcept(std::is_nothrow_constructible_v<FT, GT>) { new(dest.get()) FT(std::forward<GT>(gt)); }
				constexpr static void destruct(__extended_storage& tgt) noexcept(std::is_nothrow_destructible_v<FT>) { tgt.template get_as<FT>()->~FT(); }
				constexpr static FT* get_ptr(__extended_storage& fn) noexcept { return fn.template get_as<FT>(); }
				constexpr static FT const* get_ptr(__extended_storage const& fn) { return fn.template get_as<FT>(); }
				static void invoke(__extended_storage& fn) noexcept(std::is_nothrow_invocable_v<FT>) { __invoke_f(std::forward<FT>(*get_ptr(fn))); }
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
			typedef void (*invoker_type)(__extended_storage&);
			manager_type __manager;
			invoker_type __invoker;
			__extended_storage __functor;
			time_t __delay_ticks;
			constexpr bool __empty() const noexcept { return !__manager || !__invoker; }
			constexpr ~__deferred_action_base() { if(__manager) (*__manager)(__functor, __functor, destroy); }
			constexpr __deferred_action_base() noexcept = default;
			constexpr __deferred_action_base(time_t delay) noexcept : __manager(), __invoker(), __functor(), __delay_ticks(delay) {}
		};
	}
	class deferred_action;
	class deferred_action : public __internal::__deferred_action_base
	{
		using __base = __internal::__deferred_action_base;
		template<__internal::__extended_runnable FT> constexpr static bool __nt_construct() noexcept { return noexcept(__base::manager<FT>::construct(std::declval<__internal::__extended_storage&>(), std::declval<FT>())); }
	public:
		constexpr bool is_done() const noexcept { return !__delay_ticks; }
		constexpr operator bool() const noexcept { return !__base::__empty(); }
		constexpr deferred_action() noexcept = default;
		constexpr ~deferred_action() = default;
		template<__internal::__extended_runnable FT>
		constexpr deferred_action(time_t delay, FT&& ft) noexcept(__nt_construct<FT>()) : __base(delay)
		{
			typedef __base::manager<std::decay_t<FT>> __mgr;
			if(__base::not_empty(ft))
			{
				__mgr::construct(__functor, std::forward<FT>(ft));
				__manager	= std::addressof(__mgr::action);
				__invoker	= std::addressof(__mgr::invoke);
			}
		}
		constexpr deferred_action(deferred_action const& that) : __base(that.__delay_ticks)
		{
			if(that)
			{
				that.__manager(this->__functor, that.__functor, clone);
				this->__manager	= that.__manager;
				this->__invoker	= that.__invoker;
			}
		}
		constexpr deferred_action(deferred_action&& that) : __base(that.__delay_ticks)
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
		constexpr void operator()() 
		{
			
			if(__unlikely(__empty())) return;
			else if(__delay_ticks) __delay_ticks--;
			if(!__delay_ticks) (*__invoker)(__functor);
		}
	};
	struct deferred_action_queue : public circular_queue<deferred_action>
	{
		long double ticks_per_ms;
		deferred_action_queue() noexcept;
		~deferred_action_queue() noexcept;
		void tick();
		time_t compute_ticks(time_t millis);
		template<__internal::__extended_runnable FT> constexpr void add(time_t delay_ms, FT&& f) { this->emplace(compute_ticks(delay_ms), std::forward<FT>(f)); }
	};
}
#endif