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
constexpr void lock(mutex_t m) { while(acquire(m)) { pause(); } }
constexpr bool test_lock(cmutex_t m) { bool b; __atomic_load(m, std::addressof(b), __ATOMIC_SEQ_CST); return b; }
__isrcall void panic(const char* msg) noexcept;
__isrcall void klog(const char* msg) noexcept;
void __register_frame(void*);
extern char __ehframe;
qword get_flags();
uintptr_t translate_vaddr(addr_t addr);
addr_t translate_user_pointer(addr_t ptr);
uint64_t sys_time(uint64_t* tm_target);
paging_table get_kernel_cr3();
uint32_t crc32c_x86_3way(uint32_t, const uint8_t*, size_t);
uint16_t crc16_calc(const void* data, size_t len, uint16_t seed = uint16_t(0));
#ifdef __cplusplus
}
void kfx_save();
void kfx_load();
template<typename T> constexpr void set_fs_base(T* value) { asm volatile("wrfsbase %0" :: "r"(value) : "memory"); }
template<typename T> constexpr void set_gs_base(T* value) { asm volatile("wrgsbase %0" :: "r"(value) : "memory"); }
template<typename T> constexpr T* get_fs_base() { T* result; asm volatile("rdfsbase %0" : "=r"(result) :: "memory"); return result; }
template<typename T> constexpr T* get_gs_base() { T* result; asm volatile("rdgsbase %0" : "=r"(result) :: "memory"); return result; }
template<typename T> inline uint32_t crc32c(T const& t) { return crc32c_x86_3way(~0U, reinterpret_cast<uint8_t const*>(&t), sizeof(T)); }
template<typename T> inline uint32_t crc32c(uint32_t start, T const& t) { return crc32c_x86_3way(start, reinterpret_cast<uint8_t const*>(&t), sizeof(T)); }
inline uint32_t crc32c(uint32_t start, const char* c, size_t l) { return crc32c_x86_3way(start, reinterpret_cast<uint8_t const*>(c), l); }
constexpr uint16_t unix_year_base = 1970U;

inline void set_cr3(void* val) noexcept { asm volatile("movq %0, %%cr3" :: "a"(val) : "memory"); }
inline paging_table get_cr3() noexcept { paging_table result; asm volatile("movq %%cr3, %0" : "=a"(result) :: "memory"); return result; }
inline void tlb_flush() noexcept { set_cr3(get_cr3()); }
constexpr inline size_t gigabyte = 0x40000000;
template<typename T> concept trivial_copy = std::__is_nonvolatile_trivially_copyable_v<T>;
template<typename T> concept nontrivial_copy = !std::__is_nonvolatile_trivially_copyable_v<T>;
template<typename T> constexpr void init_if_consteval(T* array, std::size_t n) { if constexpr(std::is_default_constructible_v<T>) { if(std::is_constant_evaluated()) { for(size_t i = 0; i < n; i++) { std::construct_at(std::addressof(array[i])); } } } }
template<trivial_copy T> requires std::larger<T, uint64_t> constexpr void array_fill(T* dest, T const& value, std::size_t n) { init_if_consteval(dest, n); if(std::is_constant_evaluated()) { for(std::size_t i = 0; i < n; i++, dest++) *dest = value; } else { __builtin_memset(dest, value, n); } }
template<nontrivial_copy T> constexpr void array_copy(T* dest, const T* src, std::size_t n) { for(std::size_t i = 0; i < n; i++, ++dest, ++src) std::construct_at(dest, *src); }
template<trivial_copy T> constexpr void array_copy(void* dest, const T* src, std::size_t n) { if(std::is_constant_evaluated()) for(size_t i = 0; i < n; i++) { std::construct_at(std::bit_cast<T*>(std::bit_cast<uintptr_t>(dest) + (i * sizeof(T))), src[i]); } else __builtin_memcpy(dest, src, static_cast<size_t>(n * sizeof(T))); }
template<trivial_copy T> requires std::not_larger<T, uint64_t> constexpr void array_fill(void* dest, T value, std::size_t n)  { if constexpr(std::is_copy_constructible_v<T> && !std::integral<T>) { for(size_t i = 0; i < n; i++) { std::construct_at(std::addressof(static_cast<T*>(dest)[i]), value); } } else { init_if_consteval(dest, n); if(std::is_constant_evaluated()) { for(size_t i = 0; i < n; i++) { *std::bit_cast<T*>(std::bit_cast<uintptr_t>(dest) + (i * sizeof(T))) = value; } } else { __builtin_memset(dest, value, n); } } }
// Constructs n elements at the destination location using the constructor arguments. If any of the arguments are move-assigned away from their original location by the constructor, the behavior is undefined.
template<nontrivial_copy T, typename ... Args> requires std::constructible_from<T, Args...> constexpr void array_fill(T* dest, std::size_t n, Args&& ... args) { for(std::size_t i = 0; i < n; i++) { std::construct_at(std::addressof(dest[i]), std::forward<Args>(args)...); } }
// If T is default-constructible, this default-initializes n elements at the destination location. Otherwise, this does nothing (use array_fill instead and provide constructor arguments)
template<trivial_copy T> requires std::not_larger<T, uint64_t> constexpr void array_zero(T* dest, std::size_t n);
template<trivial_copy T> requires std::larger<T, uint64_t> constexpr void array_zero(T* dest, std::size_t n);
template<nontrivial_copy T> constexpr void array_zero(T* dest, std::size_t n) { if constexpr(std::is_default_constructible_v<T>) { for(std::size_t i = 0; i < n; i++) { std::construct_at(std::addressof(dest[i])); } } }
constexpr uint64_t div_round_up(size_t num, size_t denom) { return (num % denom == 0) ? (num / denom) : (1 + (num / denom)); }
constexpr uint64_t truncate(uint64_t n, uint64_t unit) { return (n % unit == 0) ? n : n - (n % unit); }
constexpr uint64_t up_to_nearest(uint64_t n, uint64_t unit) { return (n % unit == 0) ? n : (unit * div_round_up(n, unit)); }
template<typename T> constexpr void array_move(T* dest, T* src, std::size_t n) { array_copy(dest, src, n); array_zero(src, n); }
constexpr uint8_t days_in_month(uint8_t month, bool leap) { if(month == 2U) return leap ? 29U : 28U; if(month == 1U || month == 3U || month == 5U || month == 7U || month == 10U || month == 12U) return 31U; return 30U; }
constexpr uint32_t years_to_days(uint16_t yr, uint16_t from) { return ((yr - from) * 365U + (yr - up_to_nearest(from, 4)) / 4U + 1U); }
constexpr uint16_t day_of_year(uint8_t month, uint16_t day, bool leap) { uint16_t result = day - 1U; for(uint8_t i = 1U; i < month; i++) result += days_in_month(i, leap); return result; }
template<trivial_copy T> requires std::not_larger<T, uint64_t> constexpr void array_zero(T* dest, std::size_t n)
{
    if constexpr(std::is_default_constructible_v<T> && !std::integral<T>) for(std::size_t i = 0; i < n; i++) { std::construct_at(std::addressof(dest[i])); }
    else if constexpr(sizeof(T) == 8) array_fill<uint64_t>(dest, 0UL, n);
    else if constexpr(sizeof(T) == 4) array_fill<uint32_t>(dest, 0U, n);
    else if constexpr(sizeof(T) == 2) array_fill<uint16_t>(dest, 0U, n);
    else array_fill<uint8_t>(dest, 0U, n * sizeof(T));
}
template<trivial_copy T> requires std::larger<T, uint64_t> constexpr void array_zero(T* dest, std::size_t n)
{
    if constexpr(std::is_default_constructible_v<T>) for(std::size_t i = 0; i < n; i++) { std::construct_at(std::addressof(dest[i])); }
    else if constexpr(sizeof(T) % 8 == 0) array_fill<uint64_t>(dest, 0UL, n * sizeof(T) / 8);
    else if constexpr(sizeof(T) % 4 == 0) array_fill<uint32_t>(dest, 0U, n * sizeof(T) / 4);
    else if constexpr(sizeof(T) % 2 == 0) array_fill<uint16_t>(dest, 0U, n * sizeof(T) / 2);
    else array_fill<uint8_t>(dest, 0U, n * sizeof(T));
}
template<std::integral I, template<I> class S> using __integer_constant_helper = std::bool_constant<std::is_same_v<I, typename S<I(0)>::value_type>>;
template<std::integral I, template<I> class S> constexpr bool is_integer_constant = __integer_constant_helper<I, S>::value;
template<std::integral I, I V> using bit_shift = std::integral_constant<I, I(I(1) << V)>;
template<std::integral I, template<I> class S> requires is_integer_constant<I, S> struct bit_or
{
    constexpr static I value() { return I(); }
    template<I I1> constexpr static I value(S<I1>) { return I1; }
    template<I I1, I ... Js> constexpr static I value(S<I1>, S<Js>...) { return value(S<I1>()) | value(S<Js>()...); }
};
template<uint64_t V> using u64_shift = bit_shift<uint64_t, V>;
template<uint64_t V> using c_u64 = std::integral_constant<uint64_t, V>;
typedef bit_or<uint64_t, c_u64> u64_or;
template<uint64_t ... Is> using bit_mask = c_u64<u64_or::template value(u64_shift<Is>()...)>;
#endif
#endif