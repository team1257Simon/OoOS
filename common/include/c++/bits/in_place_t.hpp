#ifndef __STL_IN_PLACE
#define __STL_IN_PLACE
#include <concepts>
namespace std
{
	namespace __detail { template<typename T, T V> struct __nontype_impl { constexpr explicit __nontype_impl() = default; }; }
	constexpr inline struct in_place_t { constexpr explicit in_place_t() = default; } in_place;
	template<typename T> struct in_place_type_t { constexpr explicit in_place_type_t() = default; };
	template<typename T> constexpr inline in_place_type_t<T> in_place_type{};
	template<size_t I> struct in_place_index_t { constexpr explicit in_place_index_t() = default; };
	template<size_t I> constexpr inline in_place_index_t<I> in_place_index{};
	struct sorted_unique_t { constexpr explicit sorted_unique_t() = default; };
	inline constexpr sorted_unique_t sorted_unique{};
	struct sorted_equivalent_t { constexpr explicit sorted_equivalent_t() = default; };
	inline constexpr sorted_equivalent_t sorted_equivalent{};
	template<auto V> using nontype_t = __detail::__nontype_impl<decltype(V), V>;
	template<auto V> constexpr nontype_t<V> nontype{};
	template<typename> inline constexpr bool __is_nontype_v = false;
	template<auto V> inline constexpr bool __is_nontype_v<nontype_t<V>> = true;
	struct from_range_t { constexpr explicit from_range_t() = default; };
	inline constexpr from_range_t from_range{};
}
#endif