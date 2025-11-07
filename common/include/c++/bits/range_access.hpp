#ifndef __RANGE_ACCESS
#define __RANGE_ACCESS
#include "initializer_list"
#include "type_traits"
#include "bits/stl_iterator.hpp"
namespace std 
{
	template<typename CT> constexpr inline auto begin(CT& __cont) -> decltype(__cont.begin()) { return __cont.begin(); }
	template<typename CT> constexpr inline auto begin(const CT& __cont) -> decltype(__cont.begin()) { return __cont.begin(); }
	template<typename CT> constexpr inline auto end(CT& __cont) -> decltype(__cont.end()) { return __cont.end(); }
	template<typename CT> constexpr inline auto end(const CT& __cont) -> decltype(__cont.end()) { return __cont.end(); }
	template<typename T, size_t N> constexpr inline T* begin(T (&__arr)[N]) noexcept { return __arr; }
	template<typename T, size_t N> constexpr inline T* end(T (&__arr)[N]) noexcept  { return __arr + N; }
	template<typename T> class valarray;
	template<typename T> T* begin(valarray<T>&) noexcept;
	template<typename T> const T* begin(const valarray<T>&) noexcept;
	template<typename T> T* end(valarray<T>&) noexcept;
	template<typename T> const T* end(const valarray<T>&) noexcept;
	template<typename CT> constexpr inline auto cbegin(const CT& __cont) noexcept(noexcept(std::begin(__cont))) -> decltype(std::begin(__cont)) { return std::begin(__cont); }
	template<typename CT> constexpr inline auto cend(const CT& __cont) noexcept(noexcept(std::end(__cont))) -> decltype(std::end(__cont)) { return std::end(__cont); }
	template<typename CT> constexpr inline auto rbegin(CT& __cont) -> decltype(__cont.rbegin()) { return __cont.rbegin(); }
	template<typename CT> constexpr inline auto rbegin(const CT& __cont) -> decltype(__cont.rbegin()) { return __cont.rbegin(); }
	template<typename CT> constexpr inline auto rend(CT& __cont) -> decltype(__cont.rend()) { return __cont.rend(); }
	template<typename CT> constexpr inline auto rend(const CT& __cont) -> decltype(__cont.rend()) { return __cont.rend(); }
	template<typename T, size_t N> constexpr inline reverse_iterator<T*> rbegin(T (&__arr)[N]) noexcept { return reverse_iterator<T*>(__arr + N); }
	template<typename T, size_t N> constexpr inline reverse_iterator<T*> rend(T (&__arr)[N]) noexcept { return reverse_iterator<T*>(__arr); }
	template<typename T> constexpr inline reverse_iterator<const T*> rbegin(initializer_list<T> __il) noexcept { return reverse_iterator<const T*>(__il.end()); }
	template<typename T> constexpr inline reverse_iterator<const T*> rend(initializer_list<T> __il) noexcept { return reverse_iterator<const T*>(__il.begin()); }
	template<typename CT> constexpr inline auto crbegin(const CT& __cont) -> decltype(std::rbegin(__cont)) { return std::rbegin(__cont); }
	template<typename CT> constexpr inline auto crend(const CT& __cont) -> decltype(std::rend(__cont)) { return std::rend(__cont); }
	template<typename CT> constexpr auto size(const CT& __cont) noexcept(noexcept(__cont.size())) -> decltype(__cont.size()) { return __cont.size(); }
	template<typename T, size_t N> constexpr size_t size(const T (&)[N]) noexcept { return N; }
	template<typename CT> [[nodiscard]] constexpr auto empty(const CT& __cont) noexcept(noexcept(__cont.empty())) -> decltype(__cont.empty()) { return __cont.empty(); }
	template<typename T, size_t N> [[nodiscard]] constexpr bool empty(const T (&)[N]) noexcept { return false; }
	template<typename T> [[nodiscard]] constexpr bool empty(initializer_list<T> __il) noexcept { return __il.size() == 0;}
	template<typename CT> constexpr auto data(CT& __cont) noexcept(noexcept(__cont.data())) -> decltype(__cont.data()) { return __cont.data(); }
	template<typename CT> constexpr auto data(const CT& __cont) noexcept(noexcept(__cont.data())) -> decltype(__cont.data()) { return __cont.data(); }
	template<typename T, size_t N> constexpr T* data(T (&__array)[N]) noexcept { return __array; }
	template<typename T> constexpr const T* data(initializer_list<T> __il) noexcept { return __il.begin(); }
	template<typename CT> constexpr auto  ssize(const CT& __cont)  noexcept(noexcept(__cont.size())) -> common_type_t<ptrdiff_t, make_signed_t<decltype(__cont.size())>> { using type = make_signed_t<decltype(__cont.size())>; return static_cast<common_type_t<ptrdiff_t, type>>(__cont.size()); }
	template<typename T, ptrdiff_t N> constexpr ptrdiff_t ssize(const T (&)[N]) noexcept { return N; }
}
#endif