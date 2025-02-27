#ifndef __LIBK
#define __LIBK
#include "kernel/kernel_defs.h"
#ifdef __cplusplus
#include "concepts"
#include "new"
#include "memory"
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
__isrcall void klog(const char* msg) noexcept;
void __register_frame(void*);
extern char __ehframe;
qword get_flags();
uintptr_t translate_vaddr(addr_t addr);
addr_t translate_user_pointer(addr_t ptr);
uint64_t syscall_time(uint64_t* tm_target);
paging_table kernel_cr3();
uint32_t crc32_calc(const void* data, size_t len, uint32_t seed = 0U);
uint16_t crc16_calc(const void* data, size_t len, uint16_t seed = uint16_t(0));
#define dhang() direct_write(__builtin_FUNCTION()); while(1);
#ifdef __cplusplus
}
template<typename T> constexpr void set_fs_base(T* value) { asm volatile("wrfsbase %0" :: "r"(value) : "memory"); }
template<typename T> constexpr void set_gs_base(T* value) { asm volatile("wrgsbase %0" :: "r"(value) : "memory"); }
template<typename T> constexpr T* get_fs_base() { T* result; asm volatile("rdfsbase %0" : "=r"(result) :: "memory"); return result; }
template<typename T> constexpr T* get_gs_base() { T* result; asm volatile("rdgsbase %0" : "=r"(result) :: "memory"); return result; }
template<typename ... Ts> uint32_t crc32c(Ts const*...);
template<> inline uint32_t crc32c() { return 0U; }
template<typename T, typename ... Us> inline uint32_t crc32c(T const* t, Us const* ... rem) { return crc32_calc(t, sizeof(T), crc32c(rem...)); }
template<typename ... Ts> uint16_t crc16(Ts const*...);
template<> inline uint16_t crc16() { return uint16_t(0); }
template<typename T, typename ... Us> inline uint16_t crc16(T const* t, Us const* ... rem) { return crc16_calc(t, sizeof(T), crc16(rem...)); }
constexpr uint16_t unix_year_base = 1970u;
constexpr uint8_t days_in_month(uint8_t month, bool leap) { if(month == 2U) return leap ? 29U : 28U; if(month == 1U || month == 3U || month == 5U || month == 7U || month == 10U || month == 12U) return 31U; return 30U; }
constexpr uint32_t years_to_days(uint16_t yr, uint16_t from){ return ((yr - from) * 365U + (yr - from) / 4U + (((yr % 4U == 0U) || (from % 4U == 0U)) ? 1U : 0U)) - 1U; }
constexpr uint16_t day_of_year(uint8_t month, uint16_t day, bool leap) { uint16_t result = day - 1U; for(uint8_t i = 1U; i < month; i++) result += days_in_month(i, leap); return result; }
inline void set_cr3(void* val) noexcept { asm volatile("movq %0, %%cr3" :: "a"(val) : "memory"); }
inline paging_table get_cr3() noexcept { paging_table result; asm volatile("movq %%cr3, %0" : "=a"(result) :: "memory"); return result; }
inline void tlb_flush() noexcept { set_cr3(get_cr3()); }
constexpr inline size_t GIGABYTE = 0x40000000;
template<typename T> concept trivial_copy = std::__is_nonvolatile_trivially_copyable_v<T>;
template<typename T> concept nontrivial_copy = !std::__is_nonvolatile_trivially_copyable_v<T>;
template<typename T> constexpr void init_if_consteval(T* array, std::size_t n) { if constexpr(std::is_default_constructible_v<T>) { if(__builtin_is_constant_evaluated()) { for(size_t i = 0; i < n; i++) { std::construct_at(std::addressof(array[i])); } } } }
template<typename T> constexpr void arrayset(T* dest, T const& value, std::size_t n) { init_if_consteval(dest, n); for(std::size_t i = 0; i < n; i++, dest++) *dest = value; }
template<nontrivial_copy T> constexpr void arraycopy(T* dest, const T* src, std::size_t n) { init_if_consteval(dest, n); for(std::size_t i = 0; i < n; i++, ++dest, ++src) *dest = *src; }
template<typename T> constexpr void arraymove(T* dest, T* src, std::size_t n) { init_if_consteval(dest, n); for(size_t i = 0; i < n; ++i, (void)++src) dest[i] = *src; }
template<trivial_copy T> requires std::not_larger<T, uint64_t> constexpr void arrayset(void* dest, T value, std::size_t n) { init_if_consteval(dest, n); for(size_t i = 0; i < n; i++) { *std::bit_cast<T*>(std::bit_cast<uintptr_t>(dest) + (i * sizeof(T))) = value; } }
template<trivial_copy T> constexpr void arraycopy(void* dest, const T* src, std::size_t n) { init_if_consteval(dest, n); if(__builtin_is_constant_evaluated()) for(size_t i = 0; i < n; i++) { *std::bit_cast<T*>(std::bit_cast<uintptr_t>(dest) + (i * sizeof(T))) = src[i]; } else __builtin_memcpy(dest, src, static_cast<size_t>(n * sizeof(T))); }
template<trivial_copy T> requires std::not_larger<T, uint64_t> constexpr void array_zero(T* dest, std::size_t n)
{
    if constexpr(std::is_default_constructible_v<T>) for(std::size_t i = 0; i < n; i++, dest++) { new (__builtin_addressof(dest[i])) T(); }
    else if constexpr(sizeof(T) == 8) arrayset<uint64_t>(dest, 0UL, n);
    else if constexpr(sizeof(T) == 4) arrayset<uint32_t>(dest, 0U, n);
    else if constexpr(sizeof(T) == 2) arrayset<uint16_t>(dest, 0U, n);
    else arrayset<uint8_t>(dest, 0U, n * sizeof(T));
}
template<trivial_copy T> requires std::larger<T, uint64_t> constexpr void array_zero(T* dest, std::size_t n)
{
    if constexpr(std::is_default_constructible_v<T>) for(std::size_t i = 0; i < n; i++, dest++) { new (__builtin_addressof(dest[i])) T(); }
    else if constexpr(sizeof(T) % 8 == 0) arrayset<uint64_t>(dest, 0UL, n * sizeof(T) / 8);
    else if constexpr(sizeof(T) % 4 == 0) arrayset<uint32_t>(dest, 0U, n * sizeof(T) / 4);
    else if constexpr(sizeof(T) % 2 == 0) arrayset<uint16_t>(dest, 0U, n * sizeof(T) / 2);
    else arrayset<uint8_t>(dest, 0U, n * sizeof(T));
}
// If a nontrivial type is default-constructible, "zeroing" an array really means resetting it to the default value for that type. Otherwise we leave it alone.
template<nontrivial_copy T> constexpr void array_zero(T* dest, std::size_t n) { if constexpr(std::is_default_constructible_v<T>) { for(std::size_t i = 0; i < n; i++, dest++) { new (__builtin_addressof(dest[i])) T(); } } }
constexpr uint64_t div_roundup(size_t num, size_t denom) { return (num % denom == 0) ? (num / denom) : (1 + (num / denom)); }
constexpr uint64_t truncate(uint64_t n, uint64_t unit) { return (n % unit == 0) ? n : n - (n % unit); }
constexpr uint64_t up_to_nearest(uint64_t n, uint64_t unit) { return (n % unit == 0) ? n : truncate(n + unit, unit); }
#endif
#endif