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
void sbcopy(void* restrict dest, const void* restrict src, unsigned long n);
void swcopy(void* restrict dest, const void* restrict src, unsigned long n);
void slcopy(void* restrict dest, const void* restrict src, unsigned long n);
void sqcopy(void* restrict dest, const void* restrict src, unsigned long n);
void abset(void* buf, uint8_t value, unsigned long n);
void awset(void* buf, uint16_t value, unsigned long n);
void alset(void* buf, uint32_t value, unsigned long n);
void aqset(void* buf, uint64_t value, unsigned long n);
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
template<typename T> concept QWordGranular = std::is_trivial_v<T> && (sizeof(T) % 8 == 0 || alignof(T) % 8 == 0);
template<typename T> concept DWordGranular = std::is_trivial_v<T> && (sizeof(T) == 4 || (alignof(T) == 4 && sizeof(T) % 4 == 0 && sizeof(T) % 8 != 0));
template<typename T> concept WordGranular = std::is_trivial_v<T> && (sizeof(T) == 2 || (alignof(T) == 2 && sizeof(T) % 2 == 0 && sizeof(T) % 4 != 0));
template<typename T> concept ByteGranular = std::is_trivial_v<T> && NotVoidPointer<T*> && !QWordGranular<T> && !DWordGranular<T> && !WordGranular<T>;
template<QWordGranular T> constexpr void arraycopy(T* dest, const T* src, std::size_t n) { sqcopy(dest, src, n * (sizeof(T) / 8)); }
template<DWordGranular T> constexpr void arraycopy(T* dest, const T* src, std::size_t n) { slcopy(dest, src, n * (sizeof(T) / 4)); }
template<WordGranular T> constexpr void arraycopy(T* dest, const T* src, std::size_t n) { swcopy(dest, src, n * (sizeof(T) / 2)); }
template<ByteGranular T> constexpr void arraycopy(T* dest, const T* src, std::size_t n) { sbcopy(dest, src, n * sizeof(T)); }
template<QWordGranular T> constexpr void arrayset(T* dest, uint64_t value, std::size_t n) { aqset(dest, value, n * (sizeof(T) / 8)); }
template<DWordGranular T> constexpr void arrayset(T* dest, uint32_t value, std::size_t n) { alset(dest, value, n * (sizeof(T) / 4)); }
template<WordGranular T> constexpr void arrayset(T* dest, uint16_t value, std::size_t n) { awset(dest, value, n * (sizeof(T) / 2)); }
template<ByteGranular T> constexpr void arrayset(T* dest, uint8_t value, std::size_t n) { abset(dest, value, n * sizeof(T)); }
template<typename T> requires(std::is_trivial_v<T>) [[gnu::always_inline]] constexpr void arrayset(void* dest, T value, std::size_t n) { for(std::size_t i = 0; i < n; i++) ::new (dest + i) T { value }; }
template<typename T> [[gnu::always_inline]] constexpr void arrayset(T* dest, T const& value, std::size_t n) { for(std::size_t i = 0; i < n; i++) ::new (dest + i) T { value }; }
constexpr inline size_t GIGABYTE = 0x40000000;
#endif
#endif