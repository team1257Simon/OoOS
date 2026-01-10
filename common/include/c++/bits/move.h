#ifndef __MOVE_H
#define __MOVE_H
#include <concepts>
namespace std
{
	template<typename T> concept __defined_swap = requires(T& t1, T& t2) { t1.swap(t2); };
	template<typename T> using __has_defined_swap = bool_constant<__defined_swap<T>>;
	template<typename T> concept __can_swap = __and_<__not_<__is_tuple_like<T>>, __not_<__has_defined_swap<T>>, is_move_constructible<T>, is_move_assignable<T>>::value;
	template<__defined_swap T> using __nothrow_defined_swap = std::bool_constant<noexcept(std::declval<T&>().swap(std::declval<T&>()))>;
	template<__defined_swap T> constexpr static bool __nothrow_defined_swap_v = __nothrow_defined_swap<T>::value;
	template<typename TT, bit_castable<TT> FT> using __nothrow_bit_castable = typename __and_<is_nothrow_copy_assignable<FT>, is_nothrow_copy_assignable<TT>>::type;
	template<typename T> [[nodiscard]] constexpr T&& forward(typename remove_reference<T>::type& t) { return static_cast<T&&>(t); }
	template<typename T> [[nodiscard]] constexpr T&& forward(typename remove_reference<T>::type&& t) { return static_cast<T&&>(t); }
	template<typename T> constexpr typename remove_reference<T>::type&& move(T&& __t) noexcept { return static_cast<remove_reference<T>::type&&>(__t); }
	template<typename T> constexpr T* addressof(T& arg) noexcept { return __builtin_addressof(arg); }
	template<typename T> const T* addressof(const T&&) = delete;
	template<__can_swap T> constexpr inline void swap(T& a, T& b) noexcept(__and_v<is_nothrow_move_assignable<T>, is_nothrow_constructible<T>>) { T __tmp = move(a); a = move(b); b = move(__tmp); }
	template<__defined_swap T> constexpr inline void swap(T& a, T& b) noexcept(__nothrow_defined_swap<T>::value) { a.swap(b); }
	template<typename TT, bit_castable<TT> FT> constexpr TT bit_cast(FT __ft) noexcept(__nothrow_bit_castable<TT, FT>::value) { return __builtin_bit_cast(TT, __ft); }
	template<typename T, typename U> struct __like_impl; // T must be a reference and U an lvalue reference
	template<typename T, typename U> struct __like_impl<T&, U&> { using type = U&; };
	template<typename T, typename U> struct __like_impl<const T&, U&> { using type = U const&; };
	template<typename T, typename U> struct __like_impl<T&&, U&> { using type = U&&; };
	template<typename T, typename U> struct __like_impl<const T&&, U&> { using type = const U&&; };
	template<typename T, typename U> using __like_t = typename __like_impl<T&&, U&>::type;
	template<typename T, typename U> [[nodiscard, gnu::always_inline]] constexpr __like_t<T, U> forward_like(U&& u) noexcept { return static_cast<__like_t<T, U>>(u); }
	template<typename T> constexpr typename remove_reference<T>::type& unmove(T&& t) { return static_cast<typename remove_reference<T>::type&>(t); } // Opposite of std::move :)
	template<typename T> constexpr T& backward(typename remove_reference<T>::type& t) { return static_cast<T&>(t); }	// opposite of std::forward :)
	template<typename T> constexpr T& backward(typename remove_reference<T>::type&& t) { return static_cast<T&>(t); }	// opposite of std::forward :)
}
#endif