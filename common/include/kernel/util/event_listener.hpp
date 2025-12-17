#ifndef __EVENT_CB
#define __EVENT_CB
#include <kernel_api.hpp>
namespace ooos
{
	namespace __internal
	{
		template<typename AT, __explicitly_convertible<AT> BT, __callable<AT> FT>
		struct __conversion_bind {
			FT f;
			constexpr void operator()(BT bt) const noexcept(std::is_nothrow_invocable_v<FT, AT>) { f(static_cast<AT>(bt)); }
		};
		template<typename ET>
		struct __listener_base
		{
			template<typename T> constexpr static bool not_empty(T t) { return isr_actor_base::not_empty(t); }
			template<__callable<ET> FT>
			struct __manager
			{
				constexpr static size_t __max_size  	= sizeof(local_functor_types);
            	constexpr static size_t __max_align 	= alignof(local_functor_types);
				using __is_locally_storable 			= std::bool_constant<std::is_trivially_copyable_v<FT> && (sizeof(FT) <= __max_size && alignof(FT) <= __max_align && __max_align % alignof(FT) == 0)>;
            	constexpr static bool __is_local_store 	= __is_locally_storable::value;
				template<typename GT> constexpr static void __create(functor_store& dst, GT&& func, std::false_type) { dst.set_ptr(new FT(std::forward<GT>(func))); }
				template<typename GT> constexpr static void __create(functor_store& dst, GT&& func, std::true_type) { new(dst.access()) FT(std::forward<GT>(func)); }
				template<typename GT> constexpr static void __create(functor_store& dst, GT&& func) { __create(dst, std::forward<GT>(func), __is_locally_storable()); }
				constexpr static void __destroy(functor_store& tgt, std::false_type) { if(FT* fn = tgt.template cast<FT*>()) delete fn; }
				constexpr static void __destroy(functor_store& tgt, std::true_type) { tgt.template cast<FT>().~FT(); }
				constexpr static void __destroy(functor_store& tgt) { __destroy(tgt, __is_locally_storable()); }
				constexpr static FT* __get_ptr(functor_store const& f) { if constexpr(__is_local_store) return std::addressof(const_cast<FT&>(f.template cast<FT>())); else return f.template cast<FT*>(); }
				static void __invoke(functor_store const& f, ET&& e) { __invoke_v(std::forward<FT>(*__get_ptr(f)), std::forward<ET>(e)); }
				static void __action(functor_store& dst, functor_store const& src, mgr_op op)
				{
					switch(op)
					{
					case get_pointer:
						dst.set_ptr(__get_ptr(src));
						break;
					case clone:
					case move:
						__create(dst, *__get_ptr(src));
						break;
					case destroy:
						__destroy(dst);
						break;
					case get_type_info:
						dst.set_ptr(std::addressof(typeid(FT)));
						break;
					}
				}
			};
			using __manager_type 	= void(*)(functor_store&, functor_store const&, mgr_op);
			using __invoke_type		= void(*)(functor_store const&, ET&&);
			functor_store __my_listener;
			__manager_type __my_manager;
			__invoke_type __my_invoke;
			constexpr bool __empty() const noexcept { return __my_manager && __my_invoke; }
			constexpr ~__listener_base() { if(__my_manager) (*__my_manager)(__my_listener, __my_listener, destroy); }
		};
	};
	template<typename ET> struct event_listener;
	namespace __internal { template<typename ET, typename FT> concept __wrappable_cb = __callable<FT, ET> && !std::is_same_v<FT, event_listener<ET>>; }
	template<typename ET>
	struct event_listener : private __internal::__listener_base<ET>
	{
		constexpr operator bool() const noexcept { return !this->__empty(); }
		constexpr event_listener() noexcept = default;
		constexpr ~event_listener() noexcept = default;
		template<__internal::__callable<ET> FT>
		constexpr event_listener(FT&& f) : __internal::__listener_base<ET>()
		{
			typedef typename __internal::__listener_base<ET>::__manager<std::decay_t<FT>> __mgr;
			if(__internal::__listener_base<ET>::not_empty(f))
			{
				__mgr::__create(this->__my_listener, std::forward<FT>(f));
				this->__my_manager	= std::addressof(__mgr::__action);
				this->__my_invoke	= std::addressof(__mgr::__invoke);
			}
		}
		template<typename DT, __internal::__callable<DT> FT> requires(__internal::__explicitly_convertible<ET, DT>)
		constexpr event_listener(parameter_type_t<DT>, FT&& f) : __internal::__listener_base<ET>()
		{
			typedef __internal::__conversion_bind<ET, DT, std::decay_t<FT>> __cvt;
			typedef typename __internal::__listener_base<ET>::__manager<std::decay_t<__cvt>> __mgr;
			if(__internal::__listener_base<ET>::not_empty(f))
			{
				__cvt c(std::forward<FT>(f));
				__mgr::__create(this->__my_listener, std::forward<__cvt>(c));
				this->__my_manager	= std::addressof(__mgr::__action);
				this->__my_invoke	= std::addressof(__mgr::__invoke);
			}
		}
		constexpr event_listener(event_listener const& that) : __internal::__listener_base<ET>()
		{
			if(static_cast<bool>(that))
			{
				that.__my_manager(this->__my_listener, that.__my_listener, clone);
				this->__my_manager 	= that.__my_manager;
				this->__my_invoke	= that.__my_invoke;
			}
		}
		constexpr event_listener(event_listener&& that) : __internal::__listener_base<ET>()
		{
			if(static_cast<bool>(that))
			{
				this->__my_listener = that.__my_listener;
				this->__my_manager 	= that.__my_manager;
				this->__my_invoke	= that.__my_invoke;
				that.__my_manager	= nullptr;
				that.__my_invoke	= nullptr;
			}
		}
		constexpr void swap(event_listener& that)
		{
			using __internal::__swap;
			__swap(this->__my_listener, that.__my_listener);
			__swap(this->__my_manager, that.__my_manager);
			__swap(this->__my_invoke, that.__my_invoke);
		}
		constexpr event_listener& operator=(event_listener const& that) { event_listener(that).swap(*this); return *this; }
		constexpr event_listener& operator=(event_listener&& that) { event_listener(std::move(that)).swap(*this); return *this; }
		constexpr void operator()(ET e) const { if(this->__my_invoke) (*this->__my_invoke)(this->__my_listener, std::forward<ET>(e)); }
		constexpr std::type_info const& target_type() const noexcept { if(this->__my_manager) { functor_store tmp; (*this->__my_manager)(tmp, this->__my_listener, get_type_info); if(std::type_info const* result = tmp.cast<std::type_info const*>()) return *result; } return typeid(nullptr); }
	};
}
#endif