#ifndef __LIBK_DECL
#define __LIBK_DECL
#include <kernel_defs.h>
#include <new>
#include <memory>
#include <tuple>
#ifdef __cplusplus
namespace std
{
	attribute(__always_inline__, __visibility__("default")) inline void __libk_assert_fail(){}
	#define __libk_assert(cond)					\
	do {										\
		if(is_constant_evaluated() && !(cond))	\
		__libk_assert_fail();					\
	} while(false)
	template<bool ... Bs, size_t ... Is>
	consteval size_t __first_false_helper(tuple<bool_constant<Bs>...>, index_sequence<Is...>)
	{
		template for(constexpr size_t i : { Is ... })
			if constexpr(!(Bs...[i]))
				return i;
		return sizeof...(Bs);
	}
	template<bool ... Bs>
	consteval size_t first_false_in() { return __first_false_helper(tuple<bool_constant<Bs>...>(), make_index_sequence<sizeof...(Bs)>()); }
}
extern "C"
{
#else
#define noexcept
// constexpr implies inline in c++, and inline functions exist in c
#define constexpr inline
#endif
typedef struct spinlock_t { volatile bool : 8; } *mutex_t;
typedef struct __jmp_buf
{
    uint64_t rbx;
    uint64_t rbp;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    uint64_t rsp;
    uint64_t rip;
} jmp_buf[1];
typedef const spinlock_t* cmutex_t;
constexpr bool acquire(mutex_t m) { return __atomic_test_and_set(m, __ATOMIC_SEQ_CST); }
constexpr void release(mutex_t m) { __atomic_clear(m, __ATOMIC_SEQ_CST); }
constexpr void lock(mutex_t m) { while(acquire(m)) { pause(); } }
constexpr bool test_lock(cmutex_t m) { bool b; __atomic_load(m, std::addressof(b), __ATOMIC_SEQ_CST); return b; }
qword get_flags();
extern int setjmp(jmp_buf jb) attribute(returns_twice);
extern void longjmp(jmp_buf jb, int status) attribute(noreturn);
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
constexpr uint16_t unix_year_base				= 1970U;
constexpr inline size_t gigabyte				= 0x40000000;
template<typename T> concept trivial_copy		= std::is_trivially_copyable_v<T>;
template<typename T> concept nontrivial_copy	= !trivial_copy<T> && std::is_copy_constructible_v<T>;
template<typename T> concept trivial_move		= std::is_trivially_move_constructible_v<T>;
template<typename T> concept nontrivial_move	= !trivial_move<T> && std::is_move_constructible_v<T>;
template<typename T> concept standard_layout	= std::is_standard_layout_v<T>;
template<typename T> concept qword_size			= (sizeof(T) == sizeof(uint64_t));
template<typename T> concept dword_size			= (sizeof(T) == sizeof(uint32_t));
template<typename T> concept word_size			= (sizeof(T) == sizeof(uint16_t));
template<typename T> concept byte_size			= (sizeof(T) == sizeof(uint8_t));
template<typename T, typename U>
concept equality_comparable_to = requires
{
	{ (std::declval<T>() == std::declval<U>()) ? 1 : 0 } -> std::convertible_to<int>;
	{ (std::declval<U>() == std::declval<T>()) ? 1 : 0 } -> std::convertible_to<int>;
	{ (std::declval<U>() == std::declval<U>()) ? 1 : 0 } -> std::convertible_to<int>;
};
template<integral_structure I, integral_structure J>
struct arithmetic_result
{
    typedef decltype(std::declval<I>() + std::declval<J>()) sum_type;
    typedef decltype(std::declval<I>() - std::declval<J>()) difference_type;
    typedef decltype(std::declval<I>() * std::declval<J>()) product_type;
    typedef decltype(std::declval<I>() / std::declval<J>()) quotient_type;
    typedef decltype(std::declval<I>() % std::declval<J>()) modulus_type;
};
template<integral_structure I, integral_structure J> using alignup_type				= typename arithmetic_result<I, typename arithmetic_result<I, J>::quotient_type>::product_type;
template<std::integral I, template<I> class S> using __integer_constant_helper		= std::bool_constant<std::is_same_v<I, typename S<I(0)>::value_type>>;
template<std::integral I, I V> using bit_shift										= std::integral_constant<I, I(I(1) << V)>;
template<uint8_t V> using c_u8														= std::integral_constant<uint8_t, V>;
template<uint16_t V> using c_u16													= std::integral_constant<uint16_t, V>;
template<uint32_t V> using c_u32													= std::integral_constant<uint32_t, V>;
template<uint64_t V> using c_u64													= std::integral_constant<uint64_t, V>;
template<std::integral I, template<I> class S> constexpr bool is_integer_constant	= __integer_constant_helper<I, S>::value;
template<typename T> constexpr T& nonnull_or_else(T* __this, T& __that) noexcept { return __this ? *__this : __that; }
template<typename T> constexpr void set_fs_base(T* value) { asm volatile("wrfsbase %0" :: "r"(value) : "memory"); }
template<typename T> constexpr void set_gs_base(T* value) { asm volatile("wrgsbase %0" :: "r"(value) : "memory"); }
template<typename T> constexpr T* get_fs_base() { T* result; asm volatile("rdfsbase %0" : "=r"(result) :: "memory"); return result; }
template<typename T> constexpr T* get_gs_base() { T* result; asm volatile("rdgsbase %0" : "=r"(result) :: "memory"); return result; }
inline void set_cr3(void* val) noexcept { asm volatile("movq %0, %%cr3" :: "a"(val) : "memory"); }
inline paging_table get_cr3() noexcept { paging_table result; asm volatile("movq %%cr3, %0" : "=a"(result) :: "memory"); return result; }
inline void tlb_flush() noexcept { set_cr3(get_cr3()); }
// If T is default-constructible, this default-initializes n elements at the destination location. Otherwise, this does nothing (use array_fill instead and provide constructor arguments)
template<trivial_copy T> requires(std::not_larger<T, uint64_t>) constexpr T* array_zero(T* dest, std::size_t n) noexcept;
template<trivial_copy T> requires(std::larger<T, uint64_t>) constexpr T* array_zero(T* dest, std::size_t n) noexcept;
template<trivial_copy T> constexpr T* array_copy(void* dest, T const* src, std::size_t n) noexcept;
template<nontrivial_copy T> constexpr T* array_copy(T* dest, T const* src, std::size_t n) noexcept(std::is_nothrow_copy_constructible_v<T>);
template<integral_structure I, integral_structure J> constexpr typename arithmetic_result<I, J>::quotient_type div_round_up(I num, J denom) { return (num % denom == 0) ? (num / denom) : (1 + (num / denom)); }
template<integral_structure I, integral_structure J> constexpr typename arithmetic_result<I, J>::modulus_type truncate(I n, J unit) { return (n % unit == 0) ? n : n - (n % unit); }
template<integral_structure I, integral_structure J> constexpr alignup_type<I, J> up_to_nearest(I n, J unit) { return (n % unit == 0) ? static_cast<alignup_type<I, J>>(n) : (unit * div_round_up(n, unit)); }
template<integral_structure I, integral_structure J> constexpr typename arithmetic_result<I, J>::quotient_type div_to_nearest(I num, J denom) { return (num % denom >= div_round_up(denom, static_cast<J>(2))) ? div_round_up(num, denom) : num / denom; }
// Returns the power of 10 with the same number of digits as the input in standard (i.e. not scientific) base-10 notation.
template<integral_structure I> constexpr I magnitude(I num) { I i; for(i = I(1); num >= I(10); i *= I(10), num /= I(10)); return i; }
constexpr uint32_t years_to_days(uint16_t yr, uint16_t from) { return ((yr - from) * 365U + (yr - up_to_nearest(from, 4US)) / 4U + 1U); }
template<trivial_move T> constexpr T* array_move(T* dest, T* src, std::size_t n) noexcept { return array_copy(dest, src, n); }
template<trivial_copy T> constexpr T* array_copy(void* dest, T const* src, std::size_t n) noexcept { return static_cast<T*>(__builtin_memcpy(dest, src, static_cast<size_t>(n * sizeof(T)))); }
template<nontrivial_copy T>
constexpr T* array_copy(T* dest, T const* src, std::size_t n)
noexcept(std::is_nothrow_copy_constructible_v<T>)
{
	T* p			= dest;
	for(size_t i	= 0; i < n; i++, p++) std::construct_at(p, src[i]);
	return dest;
}
template<trivial_copy T> requires(std::larger<T, uint64_t>)
constexpr T* array_fill(T* dest, T const& value, std::size_t n) noexcept
{
	T* ptr			= dest;
	for(size_t i	= 0; i < n; i++, ptr++)	new(ptr) T(value);
	return dest;
}
template<trivial_copy T> requires(std::not_larger<T, uint64_t>)
constexpr T* array_fill(void* dest, T value, std::size_t n) noexcept
{
	if constexpr(!std::integral<T>)
	{
		for(size_t i = 0; i < n; i++)
			std::construct_at(std::addressof(static_cast<T*>(dest)[i]), value);
		return static_cast<T*>(dest);
	}
	else return static_cast<T*>(__builtin_memset(dest, value, n));

}
template<trivial_copy T> constexpr T* array_init(T* dest, T const* src, size_t n)
{
	if consteval
	{
		for(size_t i 	= 0UZ; i < n; i++)
			dest[i]		= src[i];
		return dest;
	}
	return array_copy(dest, src, n);
}
template<typename T, typename IT>
requires(std::is_assignable_v<T&, decltype(*std::declval<IT>())>)
constexpr T* array_init(T* dest, IT src_start, IT src_end)
{
	T* p	= dest;
	for(IT i = src_start; i != src_end; i++, p++) *p = *i;
	return dest;
}
// Constructs n elements at the destination location using the constructor arguments. If any of the arguments are move-assigned away from their original location by the constructor with n > 1, the behavior is undefined.
template<typename T, typename ... Args> requires(std::constructible_from<T, Args...>)
constexpr T* array_fill(T* dest, std::tuple<Args...>&& arg_tuple, size_t n)
noexcept(std::is_nothrow_constructible_v<T, Args...>)
{
	T* p = dest;
	for(size_t i = 0; i < n; i++, p++) std::ext::tuple_construct(p, std::forward<std::tuple<Args...>>(arg_tuple));
	return dest;
}
template<nontrivial_copy T>
constexpr T* array_zero(T* dest, std::size_t n)
noexcept(std::is_nothrow_default_constructible_v<T> || !std::is_default_constructible_v<T>)
{
	if constexpr(std::is_default_constructible_v<T>)
		for(std::size_t i = 0; i < n; i++)
			std::construct_at(std::addressof(dest[i]));
	return dest;
}
template<integral_structure I, integral_structure J>
constexpr typename arithmetic_result<I, I>::product_type raise_power(I base, J power)
{
	if(power < static_cast<J>(2)) return power ? base : static_cast<I>(1);
	I srt	= raise_power(base, static_cast<J>(power >> 1));
	return static_cast<I>(srt * srt * (power % 2 ? base : 1));
}
template<nontrivial_move T>
constexpr T* array_move(T* dest, T* src, std::size_t n)
noexcept(std::is_nothrow_move_constructible_v<T>)
{
	T* p			= dest;
	for(size_t i	= 0; i < n; i++, p++) new(p) T(std::move(src[i]));
	return dest;
}
template<typename T>
constexpr T* copy_or_move(T* dest, T* src, size_t n)
noexcept(trivial_copy<T> || noexcept(array_move(dest, src, n)))
{
    if constexpr(!trivial_copy<T>)
        return array_move(dest, src, n);
    else return array_copy(dest, src, n);
}
constexpr uint8_t days_in_month(uint8_t month, bool leap)
{
	if(month == 9UC || month == 4UC || month == 6UC || month == 11UC) return 30UC;	// Thirty days hath September, April, June, and November...
	else if(month == 2UC) return leap ? 29UC : 28UC;
	return 31UC;
}
constexpr uint16_t day_of_year(uint8_t month, uint16_t day, bool leap)
{
	uint16_t result = day - 1US;
	for(uint8_t i = 1UC; i < month; i++) result += days_in_month(i, leap);
	return result;
}
template<trivial_copy T> requires(std::not_larger<T, uint64_t>)
constexpr T* array_zero(T* dest, std::size_t n) noexcept
{
	if consteval { for(std::size_t i = 0; i < n; i++) { dest[i]	= T(); } return dest; }
    if constexpr(std::is_default_constructible_v<T> && !std::integral<T>) for(std::size_t i = 0; i < n; i++) { std::construct_at(std::addressof(dest[i])); }
    else if constexpr(sizeof(T) == 8) array_fill(dest, 0UL, n);
    else if constexpr(sizeof(T) == 4) array_fill(dest, 0U, n);
    else if constexpr(sizeof(T) == 2) array_fill(dest, 0US, n);
    else array_fill(dest, 0UC, n * sizeof(T));
	return dest;
}
template<trivial_copy T> requires(std::larger<T, uint64_t>)
constexpr T* array_zero(T* dest, std::size_t n) noexcept
{
	if consteval { for(std::size_t i = 0; i < n; i++) { dest[i]	= T(); } return dest; }
    if constexpr(std::is_default_constructible_v<T>) for(std::size_t i = 0; i < n; i++) { std::construct_at(std::addressof(dest[i])); }
    else if constexpr(sizeof(T) % 8 == 0) array_fill(dest, 0UL, n * sizeof(T) / 8);
    else if constexpr(sizeof(T) % 4 == 0) array_fill(dest, 0U, n * sizeof(T) / 4);
    else if constexpr(sizeof(T) % 2 == 0) array_fill(dest, 0US, n * sizeof(T) / 2);
    else array_fill(dest, 0UC, n * sizeof(T));
	return dest;
}
template<typename T>
constexpr T* atomic_copy(T* dest, T const* src, size_t n) noexcept(noexcept(array_copy(dest, src, n)))
{
    push_cli();
    array_copy(dest, src, n);
    pop_flags();
	return dest;
}
template<std::integral I, template<I> class S>
requires(is_integer_constant<I, S>)
struct bit_or
{
    constexpr static I value() { return I(); }
    template<I I1> constexpr static I value(S<I1>) { return I1; }
    template<I I1, I ... Js> constexpr static I value(S<I1>, S<Js>...) { return value(S<I1>()) | value(S<Js>()...); }
};
template<uint64_t V> using u64_shift	= bit_shift<uint64_t, V>;
typedef bit_or<uint64_t, c_u64> u64_or;
template<uint64_t ... Is> using bit_mask = c_u64<u64_or::template value(u64_shift<Is>()...)>;
template<uint16_t PV, uint16_t BV = 0US, size_t NB = 8UZ>
constexpr uint16_t crc16_table_val(uint16_t i)
{
	uint16_t res	= BV;
	uint16_t c		= i;
	for(size_t j	= 0UZ; j < NB; j++)
	{
		bool b		= ((res ^ c) % 2) != 0US;
		c			>>= 1;
		res			>>= 1;
		if(b) res ^= PV;
	}
	return res ^ BV;
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