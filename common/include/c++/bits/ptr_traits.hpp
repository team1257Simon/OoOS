#ifndef __PTR_TRAITS
#define __PTR_TRAITS
#include "bits/move.h"
namespace __debug { struct __safe_iterator_base; }
namespace std
{
	class __undefined;
	template<typename T, typename U> struct __replace_first_arg {};
	template<template<typename, typename...> class TCT, typename U, typename T, typename... Ts> struct __replace_first_arg<TCT<T, Ts...>, U> { using type = TCT<U, Ts...>; };
	template<typename T, typename U> using __replace_first_arg_t = typename __replace_first_arg<T, U>::type;
	template<typename T> using __make_not_void = typename conditional<is_void<T>::value, __undefined, T>::type;
	template<typename PT> struct __ptr_traits_elem_1 {};
	template<template<typename, typename...> class PT, typename T, typename... Args>
	struct __ptr_traits_elem_1<PT<T, Args...>>
	{
		using element_type = T;
		using pointer = PT<T, Args...>;
		static pointer pointer_to(__make_not_void<element_type>& __e) { return pointer::pointer_to(__e); }
	};
	template<typename PT, typename = void> struct __ptr_traits_elem : __ptr_traits_elem_1<PT> {};
	template<typename PT>
	struct __ptr_traits_elem<PT, __void_t<typename PT::element_type>>
	{
		using element_type = typename PT::element_type;
		static PT pointer_to(__make_not_void<element_type>& __e) { return PT::pointer_to(__e); }
	};
	template<typename PT>
	struct pointer_traits : __ptr_traits_elem<PT>
	{
	private:
		template<typename T> using __difference_type = typename T::difference_type;
		template<typename T, typename U, typename = void> struct __rebind : __replace_first_arg<T, U> {};
		template<typename T, typename U> struct __rebind<T, U, __void_t<typename T::template rebind<U>>> { using type = typename T::template rebind<U>; };
	public:
		using pointer = PT;
		using difference_type = __detected_or_t<ptrdiff_t, __difference_type, PT>;
		template<typename U> using rebind = typename __rebind<PT, U>::type;
	};
	template<typename T>
	struct pointer_traits<T*>
	{
		typedef T* pointer;
		typedef T  element_type;
		typedef ptrdiff_t difference_type;
		template<typename U> using rebind = U*;
		static constexpr pointer pointer_to(__make_not_void<element_type>& __r) noexcept { return std::addressof(__r); }
	};
	template<typename PT, typename T> using __ptr_rebind = typename pointer_traits<PT>::template rebind<T>;
	template<typename T> constexpr T* __to_address(T* __ptr) noexcept { static_assert(!std::is_function<T>::value, "not a function pointer"); return __ptr; }
	template<typename PT> constexpr auto __to_address(const PT& __ptr) noexcept -> decltype(std::pointer_traits<PT>::to_address(__ptr)) { return std::pointer_traits<PT>::to_address(__ptr); }
	template<typename PT, typename... None> constexpr auto __to_address(const PT& __ptr, None...) noexcept { if constexpr (is_base_of_v<__debug::__safe_iterator_base, PT>) return std::__to_address(__ptr.base().operator->()); else return std::__to_address(__ptr.operator->()); }
	template<typename T>  constexpr T* to_address(T* __ptr) noexcept { return std::__to_address(__ptr); }
	template<typename PT> constexpr auto to_address(const PT& __ptr) noexcept { return std::__to_address(__ptr); }
}
#endif