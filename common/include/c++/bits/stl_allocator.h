#ifndef __ALLOCATOR_BASE
#define __ALLOCATOR_BASE
#ifndef _GCC_STDINT_H
#include "stdint.h"
#endif
#include "stddef.h"
#include "type_traits"
namespace std
{
    using size_t = decltype(sizeof(int));
    enum class align_val_t : decltype(alignof(int)) {};
    template<typename T>
    struct __base_allocator
    {
        constexpr static std::size_t __align_val { alignof(T) };
        constexpr static std::size_t __size_val { sizeof(T) };
        constexpr __base_allocator() noexcept = default;
        constexpr __base_allocator(__base_allocator<T> const&) noexcept = default;
        constexpr ~__base_allocator() = default;
        template<class U> constexpr __base_allocator(__base_allocator<U> const&) noexcept {}
        [[nodiscard]][[gnu::always_inline]] constexpr T* __allocate(std::size_t n) { return static_cast<T*>(::operator new(n * __size_val, static_cast<std::align_val_t>(__align_val))); }
        [[gnu::always_inline]] constexpr void __deallocate(T* ptr, std::size_t n) { ::operator delete(ptr, n * __size_val, static_cast<std::align_val_t>(__align_val)); }
    };
    namespace __detail
    {
        void* __aligned_reallocate(void* ptr, size_t n, size_t align);
    }
    template<typename T> [[nodiscard]][[gnu::always_inline]] T* resize(T* array, size_t ncount) { return reinterpret_cast<T*>(__detail::__aligned_reallocate(array, ncount * sizeof(T), alignof(T))); }
}
#endif