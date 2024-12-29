#ifndef __STL_PAIR
#define __STL_PAIR
#include "bits/move.h"
#include "concepts"
#include "compare"
namespace std
{
    template <bool, typename T1, typename T2>
    struct __pair_constraints
    {
        template <typename U1, typename U2> constexpr static bool __constructible_pair() { return __and_<is_constructible<T1, const U1&>, is_constructible<T2, const U2&>>::value; }
        template <typename U1, typename U2> constexpr static bool __implicitly_convertible_pair() { return __and_<is_convertible<const U1&, T1>, is_convertible<const U2&, T2>>::value; }
        template <typename U1, typename U2> constexpr static bool __move_constructible_pair() { return __and_<is_constructible<T1, U1&&>, is_constructible<T2, U2&&>>::value; }
        template <typename U1, typename U2> constexpr static bool __implicitly_move_convertible_pair() { return __and_<is_convertible<U1&&, T1>, is_convertible<U2&&, T2>>::value; }
    };
    template <typename T1, typename T2> 
    struct __pair_constraints<false, T1, T2> 
    { 
        template <typename U1, typename U2> constexpr static bool __constructible_pair() { return false; }
        template <typename U1, typename U2> constexpr static bool __implicitly_convertible_pair() { return false; }
        template <typename U1, typename U2> constexpr static bool __move_constructible_pair() { return false; }
        template <typename U1, typename U2> constexpr static bool __implicitly_move_convertible_pair() { return false; }
    };
    template<typename U1, typename U2> class __pair_base
    {
        template<typename T1, typename T2> friend struct pair;
        __pair_base() = default;
        ~__pair_base() = default;
        __pair_base(const __pair_base&) = default;
        __pair_base& operator=(const __pair_base&) = delete;
    };
    template<typename T1, typename T2>
    struct pair : private __pair_base<T1, T2>
    {
        typedef T1 first_type;
        typedef T2 second_type;
        T1 first;
        T2 second;
        constexpr pair() requires (__is_implicitly_default_constructible<T1>::value && __is_implicitly_default_constructible<T2>::value) : first{}, second{} {}
        constexpr explicit pair() requires (is_default_constructible_v<T1> && is_default_constructible_v<T2> && !(__is_implicitly_default_constructible<T1>::value && __is_implicitly_default_constructible<T2>::value)) : first(), second() {}
    private:
        using __constraint_type = __pair_constraints<true, T1, T2>;
        template<typename U1, typename U2> using __others_constraint_type = __pair_constraints<!is_same_v<T1, U2> || !is_same_v<T2, U2>, T1, T2>;
        constexpr static bool __copy_assignable() { return __and_<is_copy_assignable<T1>, is_copy_assignable<T2>>::value; }
        constexpr static bool __move_assignable() { return __and_<is_move_assignable<T1>, is_move_assignable<T2>>::value; }
        constexpr static bool __swappable() { return __and_<is_swappable<T1>, is_swappable<T2>>::value; }
        constexpr static bool __nothrow_copy_assignable() { return __and_<is_nothrow_copy_assignable<T1>, is_nothrow_copy_assignable<T2>>::value; }
        constexpr static bool __nothrow_move_assignable() { return __and_<is_nothrow_move_assignable<T1>, is_nothrow_move_assignable<T2>>::value; }
        constexpr static bool __nothrow_swappable() { return __and_<is_nothrow_swappable<T1>, is_nothrow_swappable<T2>>::value; }
        template<typename U1, typename U2> constexpr bool __other_copy_assignable() { return __and_<is_assignable<T1&, U1 const&>, is_assignable<T2&, U2 const&>>::value; }
        template<typename U1, typename U2> constexpr bool __other_move_assignable() { return __and_<is_assignable<T1&, U1&&>, is_assignable<T2&, U2&&>>::value; }
    public:
        template<typename U1 = T1, typename U2 = T2> constexpr pair(T1 const& __first, T2 const& __second) requires (__constraint_type::template __constructible_pair<U1, U2>() && __constraint_type::template __implicitly_convertible_pair<U1, U2>()) : first{__first}, second{__second} {}
        template<typename U1 = T1, typename U2 = T2> constexpr explicit pair(T1 const& __first, T2 const& __second) requires (__constraint_type::template __constructible_pair<U1, U2>() && !__constraint_type::template __implicitly_convertible_pair<U1, U2>()) : first(__first), second(__second) {}
        template<typename U1, typename U2> constexpr pair(U1&& __first, U2&& __second) requires (__constraint_type::template __move_constructible_pair<U1, U2>() && __constraint_type::template __implicitly_move_convertible_pair<U1, U2>()) : first{ forward<U1>(__first) }, second{ forward<U2>(__second) } {}
        template<typename U1, typename U2> constexpr explicit pair(U1&& __first, U2&& __second) requires (__constraint_type::template __move_constructible_pair<U1, U2>() && !__constraint_type::template __implicitly_move_convertible_pair<U1, U2>()) : first(forward<U1>(__first)), second(forward<U2>(__second)) {}
        template<typename U1, typename U2> requires(__others_constraint_type<U1, U2>::template __constructible_pair<U1, U2>() && __others_constraint_type<U1, U2>::template __implicitly_convertible_pair<U1, U2>()) constexpr pair(pair<U1, U2> const& that) : first{ that.first }, second{ that.second } {}
        template<typename U1, typename U2> requires(__others_constraint_type<U1, U2>::template __constructible_pair<U1, U2>() && !__others_constraint_type<U1, U2>::template __implicitly_convertible_pair<U1, U2>()) constexpr explicit pair(pair<U1, U2> const& that) : first(that.first), second(that.second) {}
        template<typename U1, typename U2> requires(__others_constraint_type<U1, U2>::template __move_constructible_pair<U1, U2>() && __others_constraint_type<U1, U2>::template __implicitly_move_convertible_pair<U1, U2>()) constexpr pair(pair<U1, U2>&& that) : first{ forward<U1>(that.first) }, second{ forward<U2>(that.second) } {}
        template<typename U1, typename U2> requires(__others_constraint_type<U1, U2>::template __move_constructible_pair<U1, U2>() && !__others_constraint_type<U1, U2>::template __implicitly_move_convertible_pair<U1, U2>()) constexpr explicit pair(pair<U1, U2>&& that) : first(forward<U1>(that.first)), second(forward<U2>(that.second)) {}
        constexpr pair(pair const&) = default;
        constexpr pair(pair&&) = default;
        constexpr pair& operator=(pair const& that) noexcept(__nothrow_copy_assignable()) requires (__copy_assignable()) { this->first = that.first; this->second = that.second; return *this; }
        constexpr pair& operator=(pair&& that) noexcept(__nothrow_move_assignable())  requires (__move_assignable()) { this->first = forward<T1>(that.first); this->second = forward<T2>(that.second); return *this; }
        template<typename U1, typename U2> constexpr pair& operator=(pair<U1, U2> const& that) requires (__other_copy_assignable<U1, U2>()) { this->first = that.first; second = this->that.second; return *this; }
        template<typename U1, typename U2> constexpr pair& operator=(pair<U1, U2>&& that) requires (__other_move_assignable<U1, U2>()) { this->first = forward<U1>(that.first); this->second = forward<U2>(that.second); return *this; }
        constexpr void swap(pair& that) noexcept(__nothrow_swappable()) requires (__swappable()) { using std::swap; swap(this->first, that.first); swap(this->second, that.second); }
    };
    template<typename T1, typename T2> pair(T1, T2) -> pair<T1, T2>;
    template<typename T1, typename T2> constexpr inline bool operator==(pair<T1, T2> const& __this, pair<T1, T2> const& __that) { return __this.first == __that.first && __this.second == __that.second; }
    template<typename T1, typename T2> constexpr common_comparison_category_t<__detail::__synth3way_t<T1>, __detail::__synth3way_t<T2>> operator<=>(pair<T1, T2> const& __this, pair<T1, T2> const& __that) { auto __c = __detail::__synth3way(__this.first, __that.first); if(__c != 0) return __c; return __detail::__synth3way(__this.second, __that.second); }
    template<typename T1, typename T2> constexpr pair<typename __decay_and_strip<T1>::type, typename __decay_and_strip<T2>::type> make_pair(T1 && t1, T2 && t2) { typedef typename __decay_and_strip<T1>::type d1; typedef typename __decay_and_strip<T2>::type d2; typedef pair<d1, d2> p; return p { forward<T1>(t1), forward<T2>(t2) }; }
}
#endif