#ifndef __LIBK
#define __LIBK
#include "kernel_defs.h"
#ifdef __cplusplus
#if defined(__KERNEL__) || defined(__LIBK__)
#include "new"
#include "memory"
#else
#include <new>
#include <memory>
#include <tuple>
namespace std
{
#pragma region non-standard useful concepts
    extension template<typename LT, typename RT> concept not_self = !is_same_v<__remove_cvref_t<LT>, RT>;
    extension template<typename LT, typename RT> concept same_size = sizeof(LT) == sizeof(RT);
    extension template<typename LT, typename RT> concept smaller = sizeof(LT) < sizeof(RT);
    extension template<typename LT, typename RT> concept larger = sizeof(LT) > sizeof(RT);
    extension template<typename LT, typename RT> concept not_smaller = same_size<LT, RT> || larger<LT, RT>;
    extension template<typename LT, typename RT> concept not_larger = same_size<LT, RT> || smaller<LT, RT>;
    extension template<typename FT, typename RT, typename ST, typename ... Args> concept member_function = requires { { ((declval<ST&&>()).*(declval<FT&&>()))(declval<Args&&>()...) } -> same_as<RT>; };
    extension template<typename FT, typename RT, typename ST, typename ... Args> concept deref_to_member_function = requires { { ((*declval<ST&&>()).*(declval<FT&&>()))(declval<Args&&>()...) } -> same_as<RT>; };
    extension template<typename FT, typename RT, typename ... Args> concept functor = requires { { declval<FT&&>()(declval<Args&&>()...) } -> same_as<RT>; };
    extension template<typename T> concept non_object = !is_object_v<T>;
    extension template<typename T> concept object = is_object_v<T>;
    extension template<class A, typename T> concept __allocator_object = is_default_constructible_v<A> && requires { { declval<A>().allocate(declval<size_t>()) } -> same_as<T*>; declval<A>().deallocate(declval<T*>(), declval<size_t>()); };
    struct __somesuch{};
    extension template<class A, typename T> concept allocator_object = __allocator_object<A, T> && __allocator_object<typename A::template rebind<__somesuch>::other, __somesuch>;
    extension template<template<typename> class PT, typename T> concept recursive_template_derived = derived_from<T, PT<T>>;
    extension namespace ext
    {
        template<typename T, typename ... Args, size_t ... Is> constexpr T* __tuple_construct(T* ptr, tuple<Args...>&& args, index_sequence<Is...>) { return construct_at(ptr, get<Is>(forward<tuple<Args...>>(args))...);  }
        template<typename T, typename ... Args> requires constructible_from<T, Args...> constexpr T* tuple_construct(T* ptr, tuple<Args...>&& args) { return __tuple_construct(ptr, forward<tuple<Args...>>(args), make_index_sequence<sizeof...(Args)>{}); }
    }
    #pragma endregion
}
#endif
extern "C"
{
#else
#define noexcept
// constexpr implies inline in c++, and inline functions exist in c
#define constexpr inline
#endif
typedef struct spinlock_t { volatile bool : 8; } __pack *mutex_t;
typedef struct __jmp_buf
{
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t rbp;
    uint64_t rsp;
    uint64_t rip;
} jmp_buf[1];
typedef const spinlock_t* cmutex_t;
constexpr bool acquire(mutex_t m) { return __atomic_test_and_set(m, __ATOMIC_SEQ_CST); }
constexpr void release(mutex_t m) { __atomic_clear(m, __ATOMIC_SEQ_CST); }
constexpr void lock(mutex_t m) { while(acquire(m)) { pause(); } }
constexpr bool test_lock(cmutex_t m) { bool b; __atomic_load(m, std::addressof(b), __ATOMIC_SEQ_CST); return b; }
qword get_flags();
extern int setjmp(jmp_buf jb);
extern void longjmp(jmp_buf jb, int status);
#if defined(__KERNEL__) || defined(__LIBK__)
void panic(const char* msg) noexcept;
void klog(const char* msg) noexcept;
void __register_frame(void*);
extern char __ehframe;
uintptr_t translate_vaddr(addr_t addr);
addr_t translate_user_pointer(addr_t ptr);
uint64_t sys_time(uint64_t* tm_target);
paging_table get_kernel_cr3();
uint32_t crc32c_x86_3way(uint32_t, const uint8_t*, size_t);
uint16_t crc16_calc(const void* data, size_t len, uint16_t seed = 0);
#endif
#ifdef __cplusplus
}
template<typename T> constexpr void set_fs_base(T* value) { asm volatile("wrfsbase %0" :: "r"(value) : "memory"); }
template<typename T> constexpr void set_gs_base(T* value) { asm volatile("wrgsbase %0" :: "r"(value) : "memory"); }
template<typename T> constexpr T* get_fs_base() { T* result; asm volatile("rdfsbase %0" : "=r"(result) :: "memory"); return result; }
template<typename T> constexpr T* get_gs_base() { T* result; asm volatile("rdgsbase %0" : "=r"(result) :: "memory"); return result; }
constexpr uint16_t unix_year_base = 1970U;
inline void set_cr3(void* val) noexcept { asm volatile("movq %0, %%cr3" :: "a"(val) : "memory"); }
inline paging_table get_cr3() noexcept { paging_table result; asm volatile("movq %%cr3, %0" : "=a"(result) :: "memory"); return result; }
inline void tlb_flush() noexcept { set_cr3(get_cr3()); }
constexpr inline size_t gigabyte = 0x40000000;
template<typename T> concept trivial_copy = std::is_trivially_copyable_v<T>;
template<typename T> concept nontrivial_copy = !std::is_trivially_copyable_v<T>;
template<typename T> concept standard_layout = std::is_standard_layout_v<T>;
template<typename T> constexpr void init_if_consteval(T* array, std::size_t n) { if constexpr(std::is_default_constructible_v<T>) { if consteval { for(size_t i = 0; i < n; i++) { std::construct_at(std::addressof(array[i])); } } } }
template<trivial_copy T> requires std::larger<T, uint64_t> constexpr void array_fill(T* dest, T const& value, std::size_t n) { init_if_consteval(dest, n); for(std::size_t i = 0; i < n; i++) { dest[i] = value; } }
template<nontrivial_copy T> constexpr void array_copy(T* dest, const T* src, std::size_t n) { for(std::size_t i = 0; i < n; i++) std::construct_at(std::addressof(dest[i]), src[i]); }
template<trivial_copy T> constexpr void array_copy(void* dest, const T* src, std::size_t n) { if consteval { for(size_t i = 0; i < n; i++) { new(std::addressof(static_cast<T*>(dest)[i])) T(src[i]); } } else { __builtin_memcpy(dest, src, static_cast<size_t>(n * sizeof(T))); } }
template<trivial_copy T> requires std::not_larger<T, uint64_t> constexpr void array_fill(void* dest, T value, std::size_t n)  { if constexpr(std::is_copy_constructible_v<T> && !std::integral<T>) { for(size_t i = 0; i < n; i++) { std::construct_at(std::addressof(static_cast<T*>(dest)[i]), value); } } else { init_if_consteval(dest, n); if consteval { for(size_t i = 0; i < n; i++) { *std::bit_cast<T*>(std::bit_cast<uintptr_t>(dest) + (i * sizeof(T))) = value; } } else { __builtin_memset(dest, value, n); } } }
// Constructs n elements at the destination location using the constructor arguments. If any of the arguments are move-assigned away from their original location by the constructor with n > 1, the behavior is undefined.
template<nontrivial_copy T, typename ... Args> requires std::constructible_from<T, Args...> constexpr void array_fill(T* dest, std::tuple<Args...>&& arg_tuple, size_t n) { for(size_t i = 0; i < n; i++) std::ext::tuple_construct(std::addressof(dest[i]), std::forward<std::tuple<Args...>>(arg_tuple)); }
// If T is default-constructible, this default-initializes n elements at the destination location. Otherwise, this does nothing (use array_fill instead and provide constructor arguments)
template<trivial_copy T> requires std::not_larger<T, uint64_t> constexpr void array_zero(T* dest, std::size_t n);
template<trivial_copy T> requires std::larger<T, uint64_t> constexpr void array_zero(T* dest, std::size_t n);
template<nontrivial_copy T> constexpr void array_zero(T* dest, std::size_t n) { if constexpr(std::is_default_constructible_v<T>) { for(std::size_t i = 0; i < n; i++) { std::construct_at(std::addressof(dest[i])); } } }
template<integral_structure I, integral_structure J> constexpr I div_round_up(I num, J denom) { return (num % denom == 0) ? (num / denom) : (1 + (num / denom)); }
template<integral_structure I, integral_structure J> constexpr I truncate(I n, J unit) { return (n % unit == 0) ? n : n - (n % unit); }
template<integral_structure I, integral_structure J> constexpr I up_to_nearest(I n, J unit) { return (n % unit == 0) ? n : (unit * div_round_up(n, unit)); }
template<integral_structure I, integral_structure J> constexpr I div_to_nearest(I num, J denom) { return (num % denom >= div_round_up(denom, static_cast<J>(2))) ? div_round_up(num, denom) : num / denom; }
template<integral_structure I, integral_structure J> constexpr I raise_power(I base, J power) { if(power < static_cast<J>(2)) return power ? base : static_cast<I>(1); I srt = raise_power(base, static_cast<J>(power >> 1)); return static_cast<I>(srt * srt * (power % 2 ? base : 1)); }
// Returns the power of 10 with the same number of digits as the input in standard (i.e. not scientific) base-10 notation.
template<integral_structure I> constexpr I magnitude(I num) { I i; for(i = I(1); num > I(10); i *= I(10), num /= I(10)); return i; }
template<trivial_copy T> constexpr void array_move(T* dest, T* src, std::size_t n) { array_copy(dest, src, n); }
template<nontrivial_copy T> constexpr void array_move(T* dest, T* src, std::size_t n) { for(size_t i = 0; i < n; i++) { new(dest + i) T(std::move(src[i])); } }
constexpr uint8_t days_in_month(uint8_t month, bool leap) { if(month == 2UC) return leap ? 29UC : 28UC; if(month == 1UC || month == 3UC || month == 5UC || month == 7UC || month == 10UC || month == 12UC) return 31UC; return 30UC; }
constexpr uint32_t years_to_days(uint16_t yr, uint16_t from) { return ((yr - from) * 365U + (yr - up_to_nearest(from, 4US)) / 4U + 1U); }
constexpr uint16_t day_of_year(uint8_t month, uint16_t day, bool leap) { uint16_t result = day - 1US; for(uint8_t i = 1UC; i < month; i++) result += days_in_month(i, leap); return result; }
template<trivial_copy T> requires std::not_larger<T, uint64_t> constexpr void array_zero(T* dest, std::size_t n)
{
    if constexpr(std::is_default_constructible_v<T> && !std::integral<T>) for(std::size_t i = 0; i < n; i++) { std::construct_at(std::addressof(dest[i])); }
    else if constexpr(sizeof(T) == 8) array_fill(dest, 0UL, n);
    else if constexpr(sizeof(T) == 4) array_fill(dest, 0U, n);
    else if constexpr(sizeof(T) == 2) array_fill(dest, 0US, n);
    else array_fill(dest, 0UC, n * sizeof(T));
}
template<trivial_copy T> requires std::larger<T, uint64_t> constexpr void array_zero(T* dest, std::size_t n)
{
    if constexpr(std::is_default_constructible_v<T>) for(std::size_t i = 0; i < n; i++) { std::construct_at(std::addressof(dest[i])); }
    else if constexpr(sizeof(T) % 8 == 0) array_fill(dest, 0UL, n * sizeof(T) / 8);
    else if constexpr(sizeof(T) % 4 == 0) array_fill(dest, 0U, n * sizeof(T) / 4);
    else if constexpr(sizeof(T) % 2 == 0) array_fill(dest, 0US, n * sizeof(T) / 2);
    else array_fill(dest, 0UC, n * sizeof(T));
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
template<typename T> constexpr T& nonnull_or_else(T* __this, T& __that) noexcept { return __this ? *__this : __that; }
template<typename T> constexpr void atomic_copy(T* dest, T const* src, size_t n) noexcept 
{ 
    push_cli();
    array_copy(dest, src, n);
    pop_flags();
}
#if defined(__KERNEL__) || defined(__LIBK__)
template<typename T> inline uint32_t crc32c(T const& t) { return crc32c_x86_3way(~0U, reinterpret_cast<uint8_t const*>(&t), sizeof(T)); }
template<typename T> inline uint32_t crc32c(uint32_t start, T const& t) { return crc32c_x86_3way(start, reinterpret_cast<uint8_t const*>(&t), sizeof(T)); }
inline uint32_t crc32c(uint32_t start, const char* c, size_t l) { return crc32c_x86_3way(start, reinterpret_cast<uint8_t const*>(c), l); }
void kfx_save();
void kfx_load();
#endif
#endif
#endif