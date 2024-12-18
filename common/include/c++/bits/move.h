#ifndef __MOVE_H
#define __MOVE_H
#include "concepts"
namespace std
{
    namespace __detail
    {
        template<typename TT, std::bit_castable<TT> FT>
        using __nothrow_bit_castable = typename __and_<std::is_nothrow_copy_assignable<FT>, std::is_nothrow_copy_assignable<TT>>::type;
    }
    template<typename T> [[nodiscard]] constexpr T&& forward(typename remove_reference<T>::type& _t) { return static_cast<T&&>(_t); }
    template<typename T> [[nodiscard]] constexpr T&& forward(typename remove_reference<T>::type&& _t) { return static_cast<T&&>(_t); }
    template<typename T> constexpr typename std::remove_reference<T>::type&& move(T&& __t) noexcept { return static_cast<std::remove_reference<T>::type&&>(__t); }
    template<typename T> constexpr T* addressof(T& arg) noexcept { return __builtin_addressof(arg); }
    template<class T> const T* addressof( const T&& ) = delete;
    template<std::swappable T> constexpr inline void swap(T& a, T&b) noexcept(__and_<is_nothrow_move_assignable<T>, is_nothrow_constructible<T>>)
    {
        T __tmp = move(a);
        a = move(b);
        b = move(__tmp);
    }
    template<typename TT, std::bit_castable<TT> FT>
    constexpr TT bit_cast(FT __ft) noexcept(__detail::__nothrow_bit_castable<TT, FT>::value){ return __builtin_bit_cast(TT, __ft); }
}

#endif