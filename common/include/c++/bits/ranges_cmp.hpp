#ifndef __RANGES_COMPARE
#define __RANGES_COMPARE
#include "bits/move.h"
namespace std
{
    struct __is_transparent; // not defined
    struct identity { template<typename T>[[nodiscard]] constexpr T&& operator()(T&& __t) const noexcept { return std::forward<T>(__t); } using is_transparent = __is_transparent; };
    namespace ranges
    {
        namespace __detail { template<typename T, typename U> concept __less_builtin_ptr_cmp = requires (T&& __t, U&& __u) { { __t < __u } -> same_as<bool>; } && convertible_to<T, const volatile void*> && convertible_to<U, const volatile void*> && (! requires(T&& __t, U&& __u) { operator<(std::forward<T>(__t), std::forward<U>(__u)); } && ! requires(T&& __t, U&& __u) { std::forward<T>(__t).operator<(std::forward<U>(__u)); }); }
        struct equal_to
        {
            template<typename T, typename U> 
            requires equality_comparable_with<T, U>
            constexpr bool operator()(T&& __t, U&& __u) const noexcept(noexcept(std::declval<T>() == std::declval<U>())) { return std::forward<T>(__t) == std::forward<U>(__u); }
            using is_transparent = __is_transparent;
        };
        struct not_equal_to
        {
            template<typename T, typename U> 
            requires equality_comparable_with<T, U>
            constexpr bool operator()(T&& __t, U&& __u) const noexcept(noexcept(std::declval<U>() == std::declval<T>())) { return !equal_to{}(std::forward<T>(__t), std::forward<U>(__u)); }
            using is_transparent = __is_transparent;
        };
        struct less
        {
            template<typename T, typename U>
            requires totally_ordered_with<T, U>
            constexpr bool operator()(T&& __t, U&& __u) const noexcept(noexcept(std::declval<T>() < std::declval<U>()))
            {
                if constexpr (__detail::__less_builtin_ptr_cmp<T, U>)
                {
                    auto __x = reinterpret_cast<__UINTPTR_TYPE__>(static_cast<const volatile void*>(std::forward<T>(__t)));
                    auto __y = reinterpret_cast<__UINTPTR_TYPE__>(static_cast<const volatile void*>(std::forward<U>(__u)));
                    return __x < __y;
                }
                else return std::forward<T>(__t) < std::forward<U>(__u);
            }
            using is_transparent = __is_transparent;
        };
        struct greater
        {
            template<typename T, typename U>
            requires totally_ordered_with<T, U>
            constexpr bool operator()(T&& __t, U&& __u) const noexcept(noexcept(std::declval<U>() < std::declval<T>())) { return less{}(std::forward<U>(__u), std::forward<T>(__t)); }
            using is_transparent = __is_transparent;
        };
        struct greater_equal
        {
            template<typename T, typename U>
            requires totally_ordered_with<T, U>
            constexpr bool operator()(T&& __t, U&& __u) const noexcept(noexcept(std::declval<T>() < std::declval<U>())) { return !less{}(std::forward<T>(__t), std::forward<U>(__u)); }
            using is_transparent = __is_transparent;
        };
        struct less_equal
        {
            template<typename T, typename U>
            requires totally_ordered_with<T, U>
            constexpr bool operator()(T&& __t, U&& __u) const noexcept(noexcept(std::declval<U>() < std::declval<T>())) { return !less{}(std::forward<U>(__u), std::forward<T>(__t)); }
            using is_transparent = __is_transparent;
        };
    }
}
#endif