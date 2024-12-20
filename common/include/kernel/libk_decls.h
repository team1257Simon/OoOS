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
void sbcopy(void* restrict dest, const void* restrict src, size_t n);
void swcopy(void* restrict dest, const void* restrict src, size_t n);
void slcopy(void* restrict dest, const void* restrict src, size_t n);
void sqcopy(void* restrict dest, const void* restrict src, size_t n);
void abset(void* buf, uint8_t value, size_t n);
void awset(void* buf, uint16_t value, size_t n);
void alset(void* buf, uint32_t value, size_t n);
void aqset(void* buf, uint64_t value, size_t n);
bool acquire(mutex_t);
void release(mutex_t);
bool testlock(cmutex_t);
void __sysinternal_tlb_flush();
void __sysinternal_set_cr3(void*);
paging_table __sysinternal_get_cr3() noexcept;
inline void cli() noexcept { asm volatile("cli" ::: "memory"); }
inline void sti() noexcept { asm volatile("sti" ::: "memory"); }
vaddr_t sys_mmap(vaddr_t start, uintptr_t phys, size_t pages);
uintptr_t sys_unmap(vaddr_t start, size_t pages);
#ifdef __cplusplus
}
// We need the "in<size>" and "out<size>" code to inline to avoid assembler errors, and the placement-new operators need to be inline to avoid linker errors
#include "new"
template<typename T> concept QWordGranular = (sizeof(T) % 8 == 0 || alignof(T) % 8 == 0);
template<typename T> concept DWordGranular = (sizeof(T) == 4 || (alignof(T) == 4 && sizeof(T) % 4 == 0));
template<typename T> concept WordGranular = (sizeof(T) == 2 || (alignof(T) == 2 && sizeof(T) % 2 == 0));
template<typename T> concept ByteGranular = NotVoidPointer<T> && !QWordGranular<T> && !DWordGranular<T> && !WordGranular<T>;
template<QWordGranular T> constexpr void arraycopy(T* dest, const T* src, size_t n) { sqcopy(dest, src, n * (sizeof(T) / 8)); }
template<DWordGranular T> constexpr void arraycopy(T* dest, const T* src, size_t n) { slcopy(dest, src, n * (sizeof(T) / 4)); }
template<WordGranular T> constexpr void arraycopy(T* dest, const T* src, size_t n) { swcopy(dest, src, n * (sizeof(T) / 2)); }
template<ByteGranular T> constexpr void arraycopy(T* dest, const T* src, size_t n) { sbcopy(dest, src, n * sizeof(T)); }
template<QWordGranular T> constexpr void arrayset(T* dest, uint64_t value, size_t n) { aqset(dest, value, n * (sizeof(T) / 8)); }
template<DWordGranular T> constexpr void arrayset(T* dest, uint32_t value, size_t n) { alset(dest, value, n * (sizeof(T) / 4)); }
template<WordGranular T> constexpr void arrayset(T* dest, uint16_t value, size_t n) { awset(dest, value, n * (sizeof(T) / 2)); }
template<ByteGranular T> constexpr void arrayset(T* dest, uint8_t value, size_t n) { abset(dest, value, n * sizeof(T)); }
template<std::integral I> constexpr INLINE I in(uint16_t from) { I result; asm volatile(" in %1, %0 " : "=a"(result) : "Nd"(from) : "memory"); return result; }
template<std::integral I> constexpr INLINE void out(uint16_t to, I value) { asm volatile(" out %0, %1 " :: "a"(value), "Nd"(to) : "memory"); }
constexpr inline size_t GIGABYTE = 0x40000000;
#endif
#endif