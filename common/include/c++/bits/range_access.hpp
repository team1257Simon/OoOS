#ifndef __RANGE_ACCESS
#define __RANGE_ACCESS
#include <initializer_list>
#include <type_traits>
#include <bits/stl_iterator.hpp>
namespace std
{
	namespace ranges
	{
		template<typename CT> constexpr inline auto begin(CT& __cont) -> decltype(__cont.begin()) { return __cont.begin(); }
		template<typename CT> constexpr inline auto begin(const CT& __cont) -> decltype(__cont.begin()) { return __cont.begin(); }
		template<typename CT> constexpr inline auto end(CT& __cont) -> decltype(__cont.end()) { return __cont.end(); }
		template<typename CT> constexpr inline auto end(const CT& __cont) -> decltype(__cont.end()) { return __cont.end(); }
		template<typename T, size_t N> constexpr inline T* begin(T (&__arr)[N]) noexcept { return __arr; }
		template<typename T, size_t N> constexpr inline T* end(T (&__arr)[N]) noexcept { return __arr + N; }
		template<typename T> class valarray;
		template<typename T> T* begin(valarray<T>&) noexcept;
		template<typename T> const T* begin(const valarray<T>&) noexcept;
		template<typename T> T* end(valarray<T>&) noexcept;
		template<typename T> const T* end(const valarray<T>&) noexcept;
		template<typename CT> constexpr inline auto cbegin(const CT& __cont) noexcept(noexcept(begin(__cont))) -> decltype(begin(__cont)) { return begin(__cont); }
		template<typename CT> constexpr inline auto cend(const CT& __cont) noexcept(noexcept(end(__cont))) -> decltype(end(__cont)) { return end(__cont); }
		template<typename CT> constexpr inline auto rbegin(CT& __cont) -> decltype(__cont.rbegin()) { return __cont.rbegin(); }
		template<typename CT> constexpr inline auto rbegin(const CT& __cont) -> decltype(__cont.rbegin()) { return __cont.rbegin(); }
		template<typename CT> constexpr inline auto rend(CT& __cont) -> decltype(__cont.rend()) { return __cont.rend(); }
		template<typename CT> constexpr inline auto rend(const CT& __cont) -> decltype(__cont.rend()) { return __cont.rend(); }
		template<typename T, size_t N> constexpr inline reverse_iterator<T*> rbegin(T (&__arr)[N]) noexcept { return reverse_iterator<T*>(__arr + N); }
		template<typename T, size_t N> constexpr inline reverse_iterator<T*> rend(T (&__arr)[N]) noexcept { return reverse_iterator<T*>(__arr); }
		template<typename T> constexpr inline reverse_iterator<const T*> rbegin(initializer_list<T> __il) noexcept { return reverse_iterator<const T*>(__il.end()); }
		template<typename T> constexpr inline reverse_iterator<const T*> rend(initializer_list<T> __il) noexcept { return reverse_iterator<const T*>(__il.begin()); }
		template<typename CT> constexpr inline auto crbegin(const CT& __cont) -> decltype(rbegin(__cont)) { return rbegin(__cont); }
		template<typename CT> constexpr inline auto crend(const CT& __cont) -> decltype(rend(__cont)) { return rend(__cont); }
		template<typename CT> constexpr auto size(const CT& __cont) noexcept(noexcept(__cont.size())) -> decltype(__cont.size()) { return __cont.size(); }
		template<typename T, size_t N> constexpr size_t size(const T (&)[N]) noexcept { return N; }
		template<typename CT> [[nodiscard]] constexpr auto empty(const CT& __cont) noexcept(noexcept(__cont.empty())) -> decltype(__cont.empty()) { return __cont.empty(); }
		template<typename T, size_t N> [[nodiscard]] constexpr bool empty(const T (&)[N]) noexcept { return false; }
		template<typename T> [[nodiscard]] constexpr bool empty(initializer_list<T> __il) noexcept { return __il.size() == 0;}
		template<typename CT> constexpr auto data(CT& __cont) noexcept(noexcept(__cont.data())) -> decltype(__cont.data()) { return __cont.data(); }
		template<typename CT> constexpr auto data(const CT& __cont) noexcept(noexcept(__cont.data())) -> decltype(__cont.data()) { return __cont.data(); }
		template<typename T, size_t N> constexpr T* data(T (&__array)[N]) noexcept { return __array; }
		template<typename T> constexpr const T* data(initializer_list<T> __il) noexcept { return __il.begin(); }
		template<typename CT> constexpr auto ssize(const CT& __cont)  noexcept(noexcept(__cont.size())) -> common_type_t<ptrdiff_t, make_signed_t<decltype(__cont.size())>> { using type = make_signed_t<decltype(__cont.size())>; return static_cast<common_type_t<ptrdiff_t, type>>(__cont.size()); }
		template<typename T, ptrdiff_t N> constexpr ptrdiff_t ssize(const T (&)[N]) noexcept { return N; }
		template<typename> constexpr inline bool disable_sized_range	= false;
		template<typename> constexpr inline bool enable_borrowed_range	= false;
		namespace __detail { template<typename T> concept __maybe_borrowed_range	= ranges::enable_borrowed_range<remove_cvref_t<T>> || is_lvalue_reference_v<T>; }
		template<typename T>
		concept range	= requires(T& t)
		{
			ranges::begin(t);
			ranges::end(t);
		};
		template<typename T> concept borrowed_range			= range<T> && __detail::__maybe_borrowed_range<T>;
		template<range RT> using iterator_t					= std::__detail::__range_iter_t<RT>;
		template<range RT> using sentinel_t					= decltype(ranges::end(declval<RT&>()));
		template<range RT> using range_difference_t			= iter_difference_t<iterator_t<RT>>;
		template<range RT> using range_value_t				= iter_value_t<iterator_t<RT>>;
		template<range RT> using range_reference_t			= iter_reference_t<iterator_t<RT>>;
		template<range RT> using range_rvalue_reference_t	= iter_rvalue_reference_t<iterator_t<RT>>;
		template<typename T>
		concept sized_range	= requires(T& t)
		{
			ranges::begin(t);
			ranges::end(t);
			ranges::size(t);
		};
		template<sized_range RT> using range_size_t			= decltype(ranges::size(declval<RT&>()));
		struct view_base {};
		template<typename DT> requires is_class_v<DT> && same_as<DT, remove_cv_t<DT>> class view_interface;
		namespace __detail
		{
			template<typename T, typename U> requires(!same_as<T, view_interface<U>>) void __is_derived_from_view_interface_fn(const T&, const view_interface<U>&); // not defined
			template<typename T> concept __is_derived_from_view_interface = requires (T t) { __is_derived_from_view_interface_fn(t, t); };
		}
		template<typename T> inline constexpr bool enable_view	= derived_from<T, view_base> || __detail::__is_derived_from_view_interface<T>;
		template<typename T> concept view						= range<T> && movable<T> && enable_view<T>;
		template<typename RT, typename T> concept output_range	= range<RT> && output_iterator<iterator_t<RT>, T>;
		template<typename T> concept input_range				= range<T> && input_iterator<iterator_t<T>>;
		template<typename T> concept forward_range				= input_range<T> && forward_iterator<iterator_t<T>>;
		template<typename T> concept bidirectional_range		= forward_range<T> && bidirectional_iterator<iterator_t<T>>;
		template<typename T> concept random_access_range		= bidirectional_range<T> && random_access_iterator<iterator_t<T>>;
		template<typename T> concept contiguous_range			= random_access_range<T> && contiguous_iterator<iterator_t<T>> && requires(T& t) { { ranges::data(t) } -> same_as<add_pointer_t<range_reference_t<T>>>; };
		template<typename T> concept common_range				= range<T> && same_as<iterator_t<T>, sentinel_t<T>>;
	}
}
#endif