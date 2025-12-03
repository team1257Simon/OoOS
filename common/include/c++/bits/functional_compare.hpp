#ifndef __FN_COMPARE
#define __FN_COMPARE
#include <bits/move.h>
namespace std
{
	template<typename T = void> struct less { constexpr bool operator()(T const& __this, T const& __that) const{ return __this < __that; } };
	template<typename T = void> struct greater { constexpr bool operator()(T const& __this, T const& __that) const{ return __this > __that; } };
	template<typename T = void> struct less_equal { constexpr bool operator()(T const& __this, T const& __that) const{ return __this <= __that; } };
	template<typename T = void> struct greater_equal { constexpr bool operator()(T const& __this, T const& __that) const{ return __this >= __that; } };
	template<typename T = void> struct equal_to { constexpr bool operator()(T const& __this, T const& __that) const{ return __this == __that; } };
	template<typename T = void> struct not_equal_to { constexpr bool operator()(T const& __this, T const& __that) const{ return __this != __that; } };
	template<> struct less<void> { template<typename T, typename U> constexpr auto operator()(T&& __this, U&& __that) const -> decltype(forward<T>(__this) < forward<U>(__that)) { return forward<T>(__this) < forward<U>(__that); } };
	template<> struct greater<void> { template<typename T, typename U> constexpr auto operator()(T&& __this, U&& __that) const -> decltype(forward<T>(__this) > forward<U>(__that)) { return forward<T>(__this) > forward<U>(__that); } };
	template<> struct less_equal<void> { template<typename T, typename U> constexpr auto operator()(T&& __this, U&& __that) const -> decltype(forward<T>(__this) <= forward<U>(__that)) { return forward<T>(__this) <= forward<U>(__that); } };
	template<> struct greater_equal<void> { template<typename T, typename U> constexpr auto operator()(T&& __this, U&& __that) const -> decltype(forward<T>(__this) >= forward<U>(__that)) { return forward<T>(__this) >= forward<U>(__that); } };
	template<> struct equal_to<void> { template<typename T, typename U> constexpr auto operator()(T&& __this, U&& __that) const -> decltype(forward<T>(__this) == forward<U>(__that)) { return forward<T>(__this) == forward<U>(__that); } };
	template<> struct not_equal_to<void> { template<typename T, typename U> constexpr auto operator()(T&& __this, U&& __that) const -> decltype(forward<T>(__this) != forward<U>(__that)) { return forward<T>(__this) != forward<U>(__that); } };
}
#endif