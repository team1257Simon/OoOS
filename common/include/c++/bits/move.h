#ifndef __MOVE_H
#define __MOVE_H
#include "concepts"
namespace std
{
	template<typename T> concept __can_swap = __and_<__not_<__is_tuple_like<T>>, is_move_constructible<T>, is_move_assignable<T>>::value;
	template<typename TT, bit_castable<TT> FT> using __nothrow_bit_castable = typename __and_<is_nothrow_copy_assignable<FT>, is_nothrow_copy_assignable<TT>>::type;
	template<typename T> [[nodiscard]] constexpr T&& forward(typename remove_reference<T>::type& t) { return static_cast<T&&>(t); }
	template<typename T> [[nodiscard]] constexpr T&& forward(typename remove_reference<T>::type&& t) { return static_cast<T&&>(t); }
	template<typename T> constexpr typename remove_reference<T>::type&& move(T&& __t) noexcept { return static_cast<remove_reference<T>::type&&>(__t); }
	template<typename T> constexpr T* addressof(T& arg) noexcept { return __builtin_addressof(arg); }
	template<typename T> const T* addressof(const T&&) = delete;
	template<__can_swap T> constexpr inline void swap(T& a, T& b) noexcept(__and_v<is_nothrow_move_assignable<T>, is_nothrow_constructible<T>>) { T __tmp = move(a); a = move(b); b = move(__tmp); }
	template<typename TT, bit_castable<TT> FT> constexpr TT bit_cast(FT __ft) noexcept(__nothrow_bit_castable<TT, FT>::value) { return __builtin_bit_cast(TT, __ft); }
}
#endif