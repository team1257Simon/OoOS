#ifndef __LIBK
#define __LIBK
#include "kernel/kernel_defs.h"
#ifdef __cplusplus
#include "concepts"
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
#include "new"
template<typename T> requires(!std::is_trivial_v<T>) [[gnu::always_inline]] constexpr void arrayset(T* dest, T const& value, std::size_t n) { for(std::size_t i = 0; i < n; i++) ::new (dest + i) T { value }; }
template<typename T> requires(std::is_trivial_v<T>) [[gnu::always_inline]] constexpr void arrayset(void* dest, T value, std::size_t n) { asm volatile("rep stos %1, (%0)" : "+D"(static_cast<T*>(dest)) : "a"(value), "c"(n) : "memory"); }
template<typename T> requires(!std::is_trivial_v<T>) [[gnu::always_inline]] constexpr void arraycopy(T* dest, const T* src, std::size_t n) { for(std::size_t i = 0; i < n; i++, src++) ::new (dest + i) T { *src }; }
template<typename T> requires(std::is_trivial_v<T>) [[gnu::always_inline]] constexpr void arraycopy(void* dest, const T* src, std::size_t n)
{
    if constexpr(sizeof(T) == 2) asm volatile("rep movsw" : "+D"(static_cast<T*>(dest)) : "S"(src), "c"(n): "memory"); 
    else if constexpr(sizeof(T) == 4) asm volatile("rep movsl" : "+D"(static_cast<T*>(dest)) : "S"(src), "c"(n): "memory");
    else if constexpr(sizeof(T) % 8 == 0) asm volatile("rep movsq" : "+D"(static_cast<T*>(dest)) : "S"(src), "c"(n): "memory"); 
    else asm volatile("rep movsb" : "+D"(static_cast<T*>(dest)) : "S"(src), "c"(n): "memory");
}
constexpr inline size_t GIGABYTE = 0x40000000;
#endif
#endif