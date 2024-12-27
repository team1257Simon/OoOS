#ifndef __LIBK
#define __LIBK
#include "kernel/kernel_defs.h"
#ifdef __cplusplus
#include "concepts"
#include "new"
extern "C"
{
#else
#define noexcept
#define constexpr
#endif
typedef struct spinlock_t { volatile bool : 8; } __pack *mutex_t;
typedef const spinlock_t* cmutex_t;
bool acquire(mutex_t);
void release(mutex_t);
bool testlock(cmutex_t);
void __sysinternal_tlb_flush();
void __sysinternal_set_cr3(void*);
paging_table __sysinternal_get_cr3() noexcept;
vaddr_t sys_mmap(vaddr_t start, uintptr_t phys, size_t pages);
uintptr_t sys_unmap(vaddr_t start, size_t pages);
#ifdef __cplusplus
}
#define DEFAULT_COPY_MOVE(T) constexpr T(T const&) = default; constexpr T(T&&) = default
#define DEFAULT_ASSIGN(T) constexpr T& operator=(T const&) = default; constexpr T& operator=(T&&) = default
#define DEFAULT_BOTH(T) DEFAULT_COPY_MOVE(T); DEFAULT_ASSIGN(T)
#define DEFAULT_PLUS_DTOR(T) DEFAULT_BOTH(T); constexpr ~T() = default
#define DEFAULT_PLUS_CTOR(T) DEFAULT_BOTH(T); constexpr T() = default
#define DEFAULT_ALL(T) DEFAULT_PLUS_CTOR(T); constexpr ~T() = default
template<size_t S> struct granular_num { using type = byte; };
template<> struct granular_num<2> { using type = word; };
template<> struct granular_num<4> { using type = dword; };
template<> struct granular_num<8> { using type = qword; };
template<typename T> constexpr inline typename granular_num<sizeof(T)>::type __zero_v() { return 0; }
template<typename T> constexpr inline size_t __n_zeros(size_t n) { return n * sizeof(T) / sizeof(granular_num<sizeof(T)>::type); }
template<typename T> concept trivial_copy = std::__is_nonvolatile_trivially_copyable_v<T>;
template<typename T> concept nontrivial_copy = !std::__is_nonvolatile_trivially_copyable_v<T>;
template<typename T> requires (nontrivial_copy<T> || std::larger<T, uint64_t>)[[gnu::always_inline]] constexpr void arrayset(T* dest, T const& value, std::size_t n) { for(std::size_t i = 0; i < n; i++, dest++) *dest = value; }
template<trivial_copy T> requires std::not_larger<T, uint64_t> [[gnu::always_inline]] constexpr void arrayset(void* dest, T value, std::size_t n) 
{ 
    if constexpr(sizeof(T) == 2) asm volatile("rep stosw" : "+D"(dest) : "a"(value), "c"(n) : "memory");
    else if constexpr(sizeof(T) == 4) asm volatile("rep stosl": "+D"(dest) : "a"(value), "c"(n) : "memory");
    else if constexpr(sizeof(T) % 8 == 0) asm volatile("rep stosq": "+D"(dest) : "a"(value), "c"(n * sizeof(T) / 8) : "memory");
    else asm volatile("rep stosb" : "+D"(dest) : "a"(value), "c"(n * sizeof(T)) : "memory");
}
template<nontrivial_copy T> [[gnu::always_inline]] constexpr void arraycopy(T* dest, const T* src, std::size_t n) { for(std::size_t i = 0; i < n; i++, ++dest, ++src) *dest = *src; }
template<trivial_copy T> [[gnu::always_inline]] constexpr void arraycopy(void* dest, const T* src, std::size_t n)
{
    if constexpr(sizeof(T) == 2) asm volatile("rep movsw" : "+D"(static_cast<T*>(dest)) : "S"(src), "c"(n): "memory"); 
    else if constexpr(sizeof(T) == 4) asm volatile("rep movsl" : "+D"(static_cast<T*>(dest)) : "S"(src), "c"(n): "memory");
    else if constexpr(sizeof(T) % 8 == 0) asm volatile("rep movsq" : "+D"(static_cast<T*>(dest)) : "S"(src), "c"(n * sizeof(T) / 8) : "memory"); 
    else asm volatile("rep movsb" : "+D"(static_cast<T*>(dest)) : "S"(src), "c"(n * sizeof(T)): "memory");
}
template<typename T> [[gnu::always_inline]] constexpr void arraymove(T* dest, T* src, std::size_t n) { for(size_t i = 0; i < n; ++i, (void)++src) dest[i] = *src; }
constexpr inline size_t GIGABYTE = 0x40000000;
#endif
uintptr_t translate_vaddr(vaddr_t addr);
#endif