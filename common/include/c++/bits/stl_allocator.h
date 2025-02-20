#ifndef __ALLOCATOR_BASE
#define __ALLOCATOR_BASE
#ifndef _GCC_STDINT_H
#include "stdint.h"
#endif
#include "stddef.h"
#include "bits/ptr_traits.hpp"
#include "type_traits"
#include "new"
namespace std
{
    using size_t = decltype(sizeof(int));
    enum class align_val_t : decltype(alignof(int)) {};
    struct __allocator_traits_base
    {
        template<typename T, typename U, typename = void> struct __rebind : __replace_first_arg<T, U> {};
        template<typename T, typename U> struct __rebind<T, U, __void_t<typename T::template rebind<U>::other>> { using type = typename T::template rebind<U>::other; };
    protected:
        template<typename T> using __pointer = typename T::pointer;
        template<typename T> using __c_pointer = typename T::const_pointer;
        template<typename T> using __v_pointer = typename T::void_pointer;
        template<typename T> using __cv_pointer = typename T::const_void_pointer;
        template<typename T> using __pocca = typename T::propagate_on_container_copy_assignment;
        template<typename T> using __pocma = typename T::propagate_on_container_move_assignment;
        template<typename T> using __pocs = typename T::propagate_on_container_swap;
        template<typename T> using __equal = typename T::is_always_equal;
    };
    template<typename AT, typename U> using __alloc_rebind = typename __allocator_traits_base::template __rebind<AT, U>::type;
    template<typename T, std::size_t A = alignof(T)>
    struct __base_allocator
    {
        constexpr static std::size_t __align_val { A };
        constexpr static std::size_t __size_val { sizeof(T) };
        constexpr __base_allocator() noexcept = default;
        constexpr __base_allocator(__base_allocator const&) noexcept = default;
        constexpr ~__base_allocator() = default;
        template<class U> constexpr __base_allocator(__base_allocator<U, alignof(U)> const&) noexcept {}
        [[nodiscard]] [[gnu::always_inline]] constexpr T* __allocate(std::size_t n) const { if(!n) return nullptr; return static_cast<T*>(::operator new(n * __size_val, static_cast<std::align_val_t>(__align_val))); }
        [[gnu::always_inline]] constexpr void __deallocate(T* ptr, std::size_t n) const { ::operator delete(ptr, n * __size_val, static_cast<std::align_val_t>(__align_val)); }
    };
    namespace __detail
    {
        [[nodiscard]] [[gnu::externally_visible]] void* __aligned_reallocate(void* ptr, size_t n, size_t align);
        template<typename T> concept __non_array = !std::is_array_v<T>;
        template<typename ... Args> concept __zero_size = sizeof...(Args) == 0;
        template<typename T, typename ... Args> concept __dynamic_constructible = std::constructible_from<T, Args...> && (__non_array<T> || __zero_size<Args...>);
    }
#pragma region non-standard memory functions
    extension template<typename T> [[nodiscard]] [[gnu::always_inline]] constexpr T* resize(T* array, size_t ocount, size_t ncount) 
    {
        if(__builtin_is_constant_evaluated())
        {
            __base_allocator<T> alloc{};
            T* result = alloc.__allocate(ncount);
            for(size_t i = 0; i < ncount; i++) { new (__builtin_addressof(result[i])) T(array[i]); }
            alloc.__deallocate(array, ocount);
            return result;
        }
        return reinterpret_cast<T*>(__detail::__aligned_reallocate(array, ncount * sizeof(T), alignof(T)));
    }
#pragma endregion
    template<typename T>
    struct allocator : __base_allocator<T>
    {
        typedef T value_type;
        typedef T* pointer;
        typedef typename std::pointer_traits<pointer>::rebind<const value_type> const_pointer;
        typedef typename std::pointer_traits<pointer>::rebind<void> void_pointer;
        typedef typename std::pointer_traits<pointer>::rebind<const void> const_void_pointer;
        template<typename U> using rebind = std::allocator<U>;
        typedef true_type propagate_on_container_move_assignment;
        typedef decltype(sizeof(T)) size_type;
        typedef decltype(declval<pointer>() - declval<pointer>()) difference_type;
        constexpr allocator() noexcept = default;
        constexpr allocator(allocator const&) noexcept = default;
        template<typename U> constexpr allocator(allocator<U> const&) noexcept {};
        constexpr ~allocator() noexcept = default;
        [[nodiscard]] constexpr pointer allocate(size_type count) const { return this->__allocate(count); }
        constexpr void deallocate(pointer ptr, size_type count) const { this->__deallocate(ptr, count); }
    };
    template<typename T, typename U> constexpr bool operator==(allocator<T> const&, allocator<U> const&) noexcept { return true; }
    extension template<typename T, typename U> 
    struct aligned_allocator : __base_allocator<T, alignof(U)> 
    {
        typedef T value_type;
        typedef T* pointer;
        typedef typename std::pointer_traits<pointer>::rebind<const value_type> const_pointer;
        typedef typename std::pointer_traits<pointer>::rebind<void> void_pointer;
        typedef typename std::pointer_traits<pointer>::rebind<const void> const_void_pointer;
        template<typename V, typename W = U> using rebind = std::aligned_allocator<V, W>;
        typedef true_type propagate_on_container_move_assignment;
        typedef decltype(sizeof(T)) size_type;
        typedef decltype(declval<pointer>() - declval<pointer>()) difference_type;
        constexpr aligned_allocator() noexcept = default;
        constexpr aligned_allocator(aligned_allocator const&) noexcept = default;
        template<typename V, typename W = U> constexpr aligned_allocator(aligned_allocator<V, W> const&) noexcept {};
        constexpr ~aligned_allocator() noexcept = default;
        [[nodiscard]] constexpr pointer allocate(size_type count) const { return this->__allocate(count); }
        constexpr void deallocate(pointer ptr, size_type count) const { this->__deallocate(ptr, count); }
    };
}
#endif