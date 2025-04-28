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
    namespace __impl
    {
        template<trivial_copy T> bool __equal_triv(T const* first1, T const* last1, T const* first2) noexcept { if (size_t const len = last1 - first1) return !__builtin_memcmp(first1, first2, len); return true; }
        template<typename T> bool __equal_nontriv(T const* first1, T const* last1, T const* first2) noexcept { for(T const* cur = first1; cur != last1; ++cur, ++first2) { if(!(*first2 == *cur)) return false; } return true; }
        template<trivial_copy T> int __lex_compare_triv(T const* first1, T const* last1, T const* first2, T const* last2) noexcept { size_t n1 = last1 - first1; size_t n2 = last2 - first2; int result = __builtin_memcmp(first1, first2, n1 < n2 ? n1 : n2);  if(result == 0 && n1 != n2) return n1 < n2 ? -1 : 1; else return result; }
        template<typename T> int __lex_compare_nontriv(T const* first1, T const* last1, T const* first2, T const* last2) noexcept { for(; first1 != last1; first1++, first2++) { if(first2 == last2 || *first1 > *first2) return 1; if(*first1 < *first2) return -1; } return first2 == last2 ? 0 : -1; }
    }
    template<std::forward_iterator FI1, std::forward_iterator FI2> constexpr inline FI2 swap_ranges(FI1 first1, FI1 last1, FI2 first2) { for(; first1 != last1; ++first1, (void)++first2) swap(*first1, *first2); return first2; }
    template<__detail::__lt_comparable T> constexpr inline T const& min(T const& a, T const& b) { if(a < b) return a; return b; }
    template<__detail::__lt_comparable T> constexpr inline T const& max(T const& a, T const& b) { if(a < b) return b; return a; }
    template<typename T, __detail::__comparator<T> C> constexpr inline T const& min(T const& a, T const& b, C c) { if(c(a, b)) return a; return b; } 
    template<typename T, __detail::__comparator<T> C> constexpr inline T const& max(T const& a, T const& b, C c) { if(c(a, b)) return b; return a; }
    template<typename T, std::matching_forward_iterator<T> IIT> constexpr void fill(IIT first, IIT last, T const& value) { for(IIT cur = first; cur != last; ++cur) *cur = value; }
    template<typename T> constexpr void fill(T* first, T* last, T const& value) { if constexpr(trivial_copy<T>) array_fill<T>(first, value, distance(first, last)); else if constexpr(copy_constructible<T>) array_fill<T>(first, distance(first, last), value); }
    template<typename T> constexpr void fill_n(T* first, size_t n, T const& value) { if constexpr(trivial_copy<T>) array_fill<T>(first, value, n); else if constexpr(copy_constructible<T>) array_fill<T>(first, n, value); }
    template<typename T, std::matching_input_iterator<T> IIT, std::output_iterator<T> OIT> constexpr OIT copy(IIT first, IIT last, OIT dfirst) { for(IIT cur = first; cur != last; ++cur, ++dfirst) *dfirst = *cur; return dfirst; }
    template<typename T> constexpr T* copy(const T* first, const T* last, T* dfirst) { size_t n = distance(first, last); array_copy<T>(dfirst, first, n); return dfirst + n; }
    template<typename T> constexpr bool equal(T const* first1, T const* last1, T const* first2) noexcept { if constexpr(std::__is_nonvolatile_trivially_copyable_v<T>) return __impl::__equal_triv(first1, last1, first2); else return __impl::__equal_nontriv(first1, last1, first2); }
    template<typename T> constexpr int __lex_compare(T const* first1, T const* first2, T const* last1, T const* last2) noexcept { if constexpr(std::__is_nonvolatile_trivially_copyable_v<T>) return __impl::__lex_compare_triv(first1, last1, first2, last2);  else return __impl::__lex_compare_nontriv(first1, last1, first2, last2); }
}
#endif