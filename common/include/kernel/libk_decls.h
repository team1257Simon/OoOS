#ifndef __LIBK
#define __LIBK
#include "kernel/kernel_defs.h"
#ifdef __cplusplus
#include "concepts"
extern "C"
{
#endif
void sbcopy(void* restrict dest, const void* restrict src, size_t n);
void swcopy(void* restrict dest, const void* restrict src, size_t n);
void slcopy(void* restrict dest, const void* restrict src, size_t n);
void sqcopy(void* restrict dest, const void* restrict src, size_t n);
void abset(void* buf, uint8_t value, size_t n);
void awset(void* buf, uint16_t value, size_t n);
void alset(void* buf, uint32_t value, size_t n);
void aqset(void* buf, uint64_t value, size_t n);
inline void cli() { asm volatile("cli" ::: "memory"); }
inline void sti() { asm volatile("sti" ::: "memory"); }
#ifdef __cplusplus
}
template<typename T> concept QWordGranular = (sizeof(T) % 8 == 0 || alignof(T) % 8 == 0);
template<typename T> concept DWordGranular = (sizeof(T) == 4 || (alignof(T) == 4 && sizeof(T) % 4 == 0));
template<typename T> concept WordGranular = (sizeof(T) == 2 || (alignof(T) == 2 && sizeof(T) % 2 == 0));
template<typename T> concept ByteGranular = NotVoidPointer<T> && !QWordGranular<T> && !DWordGranular<T> && !WordGranular<T>;
template<QWordGranular T> void arraycopy(T* dest, const T* src, size_t n) { sqcopy(dest, src, n * (sizeof(T) / 8)); }
template<DWordGranular T> void arraycopy(T* dest, const T* src, size_t n) { slcopy(dest, src, n * (sizeof(T) / 4)); }
template<WordGranular T> void arraycopy(T* dest, const T* src, size_t n) { swcopy(dest, src, n * (sizeof(T) / 2)); }
template<ByteGranular T> void arraycopy(T* dest, const T* src, size_t n) { sbcopy(dest, src, n * sizeof(T)); }
template<QWordGranular T> void arrayset(T* dest, uint64_t value, size_t n) { aqset(dest, value, n * (sizeof(T) / 8)); }
template<DWordGranular T> void arrayset(T* dest, uint32_t value, size_t n) { alset(dest, value, n * (sizeof(T) / 4)); }
template<WordGranular T> void arrayset(T* dest, uint16_t value, size_t n) { awset(dest, value, n * (sizeof(T) / 2)); }
template<ByteGranular T> void arrayset(T* dest, uint8_t value, size_t n) { abset(dest, value, n * sizeof(T)); }
[[nodiscard]] constexpr void* operator new(size_t, void* ptr) noexcept { return ptr; }
[[nodiscard]] constexpr void* operator new[](size_t, void* ptr) noexcept { return ptr; }
template<std::integral I> constexpr inline I in(uint16_t from) { I result; asm volatile(" in %1, %0 " : "=a"(result) : "Nd"(from) : "memory"); return result; }
template<std::integral I> constexpr inline void out(uint16_t to, I value) { asm volatile(" out %0, %1 " :: "a"(value), "Nd"(to) : "memory"); }
#endif
#endif