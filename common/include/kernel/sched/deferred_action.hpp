#ifndef __DEFERRED_ACTION
#define __DEFERRED_ACTION
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
				template<typename GT> constexpr static void construct(__extended_storage& dest, GT&& gt) noexcept(noexcept(FT(std::declval<GT>()))) { new(dest.get()) FT(std::forward<GT>(gt)); }
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
			constexpr __deferred_action_base(time_t delay) noexcept : __manager(), __invoker(), __functor(), __delay_ticks(std::max(delay, 1UL)) {}
		};
	}
	class deferred_action : public __internal::__deferred_action_base
	{
		using __base = __internal::__deferred_action_base;
		template<__internal::__extended_runnable FT> constexpr static bool __nt_construct() noexcept { return noexcept(__base::manager<FT>::construct(std::declval<__internal::__extended_storage&>(), std::declval<FT>())); }
	public:
		/**
		 * Returns true if the function has already executed, i.e. the delay timer is already zero.
		 * Because the decrement occurs immediately before the functor is invoked, the timer will not be zero.
		 * If a zero is passed to the constructor, the timer will be initialized to 1, which effectively creates a functor with a minimal delay.
		 * This is not recommended for precise timings; use the HPET for that instead.
		 */
		constexpr bool is_done() const noexcept { return !__delay_ticks; }
		/**
		 * Essentially the same as std::function<void()>::operator bool(); returns true if the functor is valid and false otherwise.
		 * Note that, unlike std::function, this class will not throw an exception if the functor is invalid when a call to operator() is made.
		 * The lack of an assign operator means the functor will be valid unless the initial constructor was passed a null function pointer or it is moved away.
		 */
		constexpr operator bool() const noexcept { return !__base::__empty(); }
		constexpr deferred_action() noexcept = default;
		constexpr ~deferred_action() = default;
		/**
		 * Unlike std::function, the polymorphic wrapper is more limited: it is fixed-size and always stored in the space allocated to this object.
		 * The limit is enough to store a function pointer and up to 32 bytes of bound arguments, or a pointer to a member function with 24 bytes of bound arguments.
		 * A lambda can store up to 40 bytes of captures. Any other callable object must likewise be 40 bytes or smaller.
		 * The delay is measured in scheduler ticks, which have a length determined by the processor's speed.
		 * As a result one should compute the value using the compute_ticks method on the action queue or the scheduler's ms_to_ticks method.
		 */
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
		/**
		 * If the delay timer is already zero, this function does nothing.
		 * Otherwise, it decrements the delay timer.
		 * After the decrement, if the timer has reached zero, the functor is invoked if it is valid.
		 */
		constexpr void operator()()
		{
			if(__unlikely(!__delay_ticks)) return;
			__delay_ticks--;
			if(__unlikely(__empty())) return;
			if(!__delay_ticks) (*__invoker)(__functor);
		}
	};
	struct deferred_action_queue : public circular_queue<deferred_action>
	{
		__float128 ticks_per_ms;
		deferred_action_queue() noexcept;
		~deferred_action_queue() noexcept;
		void tick();
		time_t compute_ticks(time_t millis) const noexcept;
		template<__internal::__extended_runnable FT> constexpr void add(time_t delay_ms, FT&& f) { this->emplace(compute_ticks(delay_ms), std::forward<FT>(f)); }
	};
}
#endif