#ifndef __ALGO_BASE
#define __ALGO_BASE
#include "bits/stl_iterator.hpp"
#include "kernel/libk_decls.h"
namespace std
{
    namespace __detail
    {
        template<typename T> concept __lt_comparable = requires(T a, T b) { { a < b } -> std::__detail::__boolean_testable; };
        template<typename T, typename C> concept __comparator = requires(T a, T b, C c) { { c(a, b) } -> std::__detail::__boolean_testable; };
    }
    template<std::forward_iterator FI1, std::forward_iterator FI2> constexpr inline void swap_ranges(FI1 first1, FI1 last1, FI2 first2) { for(; first1 != last1; ++first1, (void)++first2) swap(*first1, *first2); return first2; }
    template<__detail::__lt_comparable T> constexpr inline T const& min(T const& a, T const& b) { if(a < b) return a; return b; }
    template<__detail::__lt_comparable T> constexpr inline T const& max(T const& a, T const& b) { if(a < b) return b; return a; }
    template<typename T, __detail::__comparator<T> C> constexpr inline T const& min(T const& a, T const& b, C c) { if(c(a, b)) return a; return b; } 
    template<typename T, __detail::__comparator<T> C> constexpr inline T const& max(T const& a, T const& b, C c) { if(c(a, b)) return b; return a; }
    template<typename T, std::matching_forward_iterator<T> IIT> constexpr void fill(IIT first, IIT last, T const& value) { for(IIT cur = first; cur != last; ++cur) *cur = value; }
    template<trivial_copy T> constexpr void fill(T* first, T* last, T const& value) { arrayset<T>(first, value, distance(first, last)); }
    template<typename T, std::matching_input_iterator<T> IIT, std::output_iterator<T> OIT> constexpr OIT copy(IIT first, IIT last, OIT dfirst) { for(IIT cur = first; cur != last; ++cur, ++dfirst) *dfirst = *cur; return dfirst; }
    template<trivial_copy T> constexpr T* copy(const T* first, const T* last, T* dfirst) { size_t n = distance(first, last); arraycopy<T>(dfirst, first, n); return dfirst + n; }
}
#endif