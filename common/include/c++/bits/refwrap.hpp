#ifndef _REF_WRAPPERS
#define _REF_WRAPPERS
#include <bits/move.h>
#include <bits/invoke.hpp>
namespace std
{
	template<typename AT, typename RT> struct unary_function { typedef AT argument_type; typedef RT result_type; };
	template<typename AT, typename BT, typename RT> struct binary_function { typedef AT first_argument_type; typedef BT second_argument_type; typedef RT result_type; };
	template<typename RT, typename ... Args> struct __maybe_unary_or_binary_function {};
	template<typename RT, typename T1> struct __maybe_unary_or_binary_function<RT, T1> : std::unary_function<T1, RT> {};
	template<typename RT, typename T1, typename T2> struct __maybe_unary_or_binary_function<RT, T1, T2> : std::binary_function<T1, T2, RT> {};
	template<typename ST> struct __mem_fn_traits;
	template<typename RT, typename CT, typename ... Args> struct __mem_fn_traits_base { using __result_type = RT; using __maybe_type = __maybe_unary_or_binary_function<RT, CT*, Args...>; using __arity = integral_constant<size_t, sizeof...(Args)>; };
#define __MEM_FN_TRAITS2(CV, REF, LVAL, RVAL) template<typename RT, typename CT, typename ... Args> struct __mem_fn_traits<RT (CT::*)(Args...) CV REF> : __mem_fn_traits_base<RT, CV CT, Args...>	 { using __vararg = false_type; }; template<typename RT, typename CT, typename ... Args> struct __mem_fn_traits<RT(CT::*)(Args...,...) CV REF>  : __mem_fn_traits_base<RT, CV CT, Args...>	 { using __vararg = true_type; };
#define __MEM_FN_TRAITS(REF, LVAL, RVAL)  __MEM_FN_TRAITS2( , REF, LVAL, RVAL) __MEM_FN_TRAITS2(const , REF, LVAL, RVAL) __MEM_FN_TRAITS2(volatile , REF, LVAL, RVAL) __MEM_FN_TRAITS2(const volatile , REF, LVAL, RVAL)
__MEM_FN_TRAITS( , true_type, true_type)
__MEM_FN_TRAITS(&, true_type, false_type)
__MEM_FN_TRAITS(&&, false_type, true_type)
__MEM_FN_TRAITS(noexcept, true_type, true_type)
__MEM_FN_TRAITS(& noexcept, true_type, false_type)
__MEM_FN_TRAITS(&& noexcept, false_type, true_type)
#undef __MEM_FN_TRAITS
#undef __MEM_FN_TRAITS2
	template<typename FT, typename = __void_t<>> struct __maybe_get_result_type {};
	template<typename FT> struct __maybe_get_result_type<FT, __void_t<typename FT::result_type>> { typedef typename FT::result_type result_type; };
	template<typename FT> struct __weak_result_type_impl : __maybe_get_result_type<FT> {};
	template<typename RT, typename ... Args> struct __weak_result_type_impl<RT(Args...)> { typedef RT result_type; };
	template<typename RT, typename ... Args> struct __weak_result_type_impl<RT(Args...,...)> { typedef RT result_type; };
	template<typename RT, typename ... Args> struct __weak_result_type_impl<RT(*)(Args...)> { typedef RT result_type; };
	template<typename RT, typename ... Args> struct __weak_result_type_impl<RT(*)(Args...,...)> { typedef RT result_type; };
	template<typename FT, bool = is_member_function_pointer<FT>::value> struct __weak_result_type_memfun : __weak_result_type_impl<FT> {};
	template<typename MFT> struct __weak_result_type_memfun<MFT, true> { using result_type = typename __mem_fn_traits<MFT>::__result_type; };
	template<typename FT, typename CT> struct __weak_result_type_memfun<FT CT::*, false> {};
	template<typename FT> struct __weak_result_type : __weak_result_type_memfun<typename remove_cv<FT>::type> {};
	template<typename T>
	class reference_wrapper
	{
		T* __my_object;
		constexpr static T* __get_ptr(T& __r) noexcept { return std::addressof(__r); }
		static void __get_ptr(T&&) = delete;
	public:
		typedef T type;
		template<typename U, typename = decltype(reference_wrapper::__get_ptr(std::declval<U>()))> requires (!std::is_same_v<reference_wrapper, typename std::__remove_cv_t<U>::type>)
		constexpr reference_wrapper(U&& __uref) noexcept(noexcept(reference_wrapper::__get_ptr(std::declval<U>()))) : __my_object(reference_wrapper::__get_ptr(std::forward<U>(__uref))){}
		constexpr reference_wrapper(reference_wrapper const&) = default;
		constexpr reference_wrapper& operator=(reference_wrapper const&) = default;
		constexpr operator T&() const noexcept { return this->get(); }
		constexpr T& get() const noexcept { return *__my_object; }
		template<typename ... Args> constexpr typename result_of<T&(Args&&...)>::type operator()(Args&& ... __args) const { if constexpr (is_object_v<type>) static_assert(sizeof(type), "type must be complete"); return std::__invoke(get(), std::forward<Args>(__args)...); }
	};
	template<typename T> reference_wrapper(T&) -> reference_wrapper<T>;
	template<typename T> constexpr inline reference_wrapper<T> ref(T& __t) noexcept { return reference_wrapper<T>(__t); }
	template<typename T> constexpr inline reference_wrapper<const T> cref(const T& __t) noexcept { return reference_wrapper<const T>(__t); }
	template<typename T> void ref(const T&&) = delete; 
	template<typename T> void cref(const T&&) = delete;
	template<typename T> constexpr inline reference_wrapper<T> ref(reference_wrapper<T> __t) noexcept { return __t; }
	template<typename T> constexpr inline reference_wrapper<const T> cref(reference_wrapper<T> __t) noexcept { return { __t.get() }; }
}
#endif