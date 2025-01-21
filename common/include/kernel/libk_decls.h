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
// constexpr implies inline in c++, and inline functions exist in c
#define constexpr inline
#endif
typedef struct spinlock_t { volatile bool : 8; } __pack *mutex_t;
typedef const spinlock_t* cmutex_t;
constexpr bool acquire(mutex_t m) { return __atomic_test_and_set(m, __ATOMIC_SEQ_CST); }
constexpr void release(mutex_t m) { __atomic_clear(m, __ATOMIC_SEQ_CST); }
constexpr void lock(mutex_t m) { while(acquire(m)) PAUSE; }
constexpr bool test_lock(cmutex_t m) { bool b; __atomic_load(m, &b, __ATOMIC_SEQ_CST); return b; }
void direct_write(const char* str);
void debug_print_num(uintptr_t num, int lenmax = 16);
void direct_writeln(const char* str);
__isrcall void panic(const char* msg) noexcept;
void __register_frame(void*);
extern char __ehframe;
void tlb_flush();
void set_cr3(void*);
paging_table get_cr3() noexcept;
void set_fs_base(void*);
void set_gs_base(void*);
void* get_fs_base();
void* get_gs_base();
qword get_flags();
uintptr_t translate_vaddr(vaddr_t addr);
#ifdef __cplusplus
}
constexpr inline size_t GIGABYTE = 0x40000000;
template<typename T> concept trivial_copy = std::__is_nonvolatile_trivially_copyable_v<T>;
template<typename T> concept nontrivial_copy = !std::__is_nonvolatile_trivially_copyable_v<T>;
template<typename T> requires (nontrivial_copy<T> || std::larger<T, uint64_t>)[[gnu::always_inline]] constexpr void arrayset(T* dest, T const& value, std::size_t n) { for(std::size_t i = 0; i < n; i++, dest++) *dest = value; }
template<nontrivial_copy T> [[gnu::always_inline]] constexpr void arraycopy(T* dest, const T* src, std::size_t n) { for(std::size_t i = 0; i < n; i++, ++dest, ++src) *dest = *src; }
template<typename T> [[gnu::always_inline]] constexpr void arraymove(T* dest, T* src, std::size_t n) { for(size_t i = 0; i < n; ++i, (void)++src) dest[i] = *src; }
#if defined(__x86_64__) || defined(_M_X64)
template<trivial_copy T> requires std::not_larger<T, uint64_t> [[gnu::always_inline]] constexpr void arrayset(void* dest, T value, std::size_t n) 
{ 
    if constexpr(sizeof(T) == 2) asm volatile("rep stosw" : "+D"(dest) : "a"(value), "c"(n) : "memory");
    else if constexpr(sizeof(T) == 4) asm volatile("rep stosl": "+D"(dest) : "a"(value), "c"(n) : "memory");
    else if constexpr(sizeof(T) == 8) asm volatile("rep stosq": "+D"(dest) : "a"(value), "c"(n * sizeof(T) / 8) : "memory");
    else asm volatile("rep stosb" : "+D"(dest) : "a"(value), "c"(n * sizeof(T)) : "memory");
}
template<typename T> concept qword_copy = trivial_copy<T> && (sizeof(T) == 8 || (std::is_integral_v<T> && sizeof(T) % 8 == 0 && alignof(T) % 8 == 0)); 
template<trivial_copy T> [[gnu::always_inline]] constexpr void arraycopy(void* dest, const T* src, std::size_t n)
{
    if constexpr(sizeof(T) == 2) asm volatile("rep movsw" : "+D"(static_cast<T*>(dest)) : "S"(src), "c"(n): "memory"); 
    else if constexpr(sizeof(T) == 4) asm volatile("rep movsl" : "+D"(static_cast<T*>(dest)) : "S"(src), "c"(n): "memory");
    else if constexpr(qword_copy<T>) asm volatile("rep movsq" : "+D"(static_cast<T*>(dest)) : "S"(src), "c"(n * sizeof(T) / 8) : "memory"); 
    else asm volatile("rep movsb" : "+D"(static_cast<T*>(dest)) : "S"(src), "c"(n * sizeof(T)): "memory");
}
#else
template<trivial_copy T> requires std::not_larger<T, uint64_t> [[gnu::always_inline]] constexpr void arrayset(void* dest, T value, std::size_t n) { __builtin_memset(dest, value, n * sizeof(T)); }
template<trivial_copy T> [[gnu::always_inline]] constexpr void arraycopy(void* dest, const T* src, std::size_t n) { __builtn_memcpy(dest, src, n * sizeof(T)); }
#endif
template<trivial_copy T> requires std::not_larger<T, uint64_t> [[gnu::always_inline]] constexpr void array_zero(T* dest, std::size_t n)
{
    if constexpr(std::is_default_constructible_v<T>) arrayset(dest, T{}, n);
    if constexpr(sizeof(T) == 8) arrayset<uint64_t>(dest, 0ul, n);
    else if constexpr(sizeof(T) == 4) arrayset<uint32_t>(dest, 0u, n);
    else if constexpr(sizeof(T) == 2) arrayset<uint16_t>(dest, 0u, n);
    else arrayset<uint8_t>(dest, 0u, n * sizeof(T));
}
template<trivial_copy T> requires std::larger<T, uint64_t> [[gnu::always_inline]] constexpr void array_zero(T* dest, std::size_t n)
{
    if constexpr(sizeof(T) % 8 == 0) arrayset<uint64_t>(dest, 0ul, n * sizeof(T) / 8);
    else if constexpr(sizeof(T) % 4 == 0) arrayset<uint32_t>(dest, 0u, n * sizeof(T) / 4);
    else if constexpr(sizeof(T) % 2 == 0) arrayset<uint16_t>(dest, 0u, n * sizeof(T) / 2);
    else arrayset<uint8_t>(dest, 0u, n * sizeof(T));
}
template<nontrivial_copy T> [[gnu::always_inline]] constexpr void array_zero(T* dest, std::size_t n) { /* don't actually touch nontrivial arrays */ }
#endif
#endif