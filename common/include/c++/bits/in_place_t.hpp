#ifndef __STL_IN_PLACE
#define __STL_IN_PLACE
#include "concepts"
namespace std
{
    constexpr inline struct in_place_t { explicit in_place_t() = default; } in_place;
    template<typename T> struct in_place_type_t { explicit in_place_type_t() = default; };
    template<typename T> constexpr inline in_place_type_t<T> in_place_type{};
    template<size_t I> struct in_place_index_t { explicit in_place_index_t() = default; };
    template<size_t I> constexpr inline in_place_index_t<I> in_place_index{};
}
#endif