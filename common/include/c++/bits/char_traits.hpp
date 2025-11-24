#ifndef __CHAR_TRAITS
#define __CHAR_TRAITS
#include "bits/ios_base.hpp"
#include "bits/stl_algobase.hpp"
#include "compare"
namespace std
{
	namespace __detail
	{
		template<typename T> struct __is_totally_ordered : false_type {};
		template<std::totally_ordered T> struct __is_totally_ordered<T> : true_type {};
		template<typename CT> struct __is_char_type : __and_<__or_<is_trivial<CT>, is_standard_layout<CT>>, __not_<is_array<CT>>, __is_totally_ordered<CT>> {};
		template<typename CT, typename VT = remove_cvref_t<CT>> struct __basic_char_type : __or_<is_same<VT, char>, is_same<VT, wchar_t>, is_same<VT, char8_t>, is_same<VT, char16_t>, is_same<VT, char32_t>> {};
	}
	extension template<typename CT> concept char_type = __detail::__is_char_type<CT>::value;
	extension template<typename CT> concept basic_char_type = __detail::__basic_char_type<CT>::value;
	template<std::char_type CT> constexpr CT* find(const CT* ptr, size_t n, CT c) { for(size_t i = 0; i < n && *ptr != c; i++, ++ptr); return const_cast<CT*>(ptr); }
	extension template<std::char_type CT> constexpr size_t strnlen(const CT* str, size_t max) { return (str && *str) ? size_t(std::find(str, max, CT(0)) - str) : 0; }
	extension template<std::char_type CT> constexpr size_t strlen(const CT* str) { return std::strnlen(str, SIZE_MAX / 2); }
	extension template<std::integral  IT> constexpr void* memset(void* ptr, IT val, size_t n) { fill_n<IT>(static_cast<IT*>(ptr), n, val); return ptr; }
	extension template<std::char_type CT> constexpr CT* memset(CT* ptr, CT c, size_t n) { fill_n(ptr, n, c); return ptr; }
	extension template<std::char_type CT> constexpr void assign(CT& c1, CT const& c2) { c1 = c2; }
	extension template<std::char_type CT> constexpr CT* memmove(CT* dest, const CT* src, size_t n) { return static_cast<CT*>(__builtin_memmove(dest, src, n)); }
	extension template<std::char_type CT> constexpr CT to_char_type(int i) { return static_cast<CT>(i); }
	extension template<std::char_type CT> constexpr int to_int_type(CT c) { return static_cast<int>(c); }
	extension template<std::char_type CT> constexpr bool lt(CT a, CT b) { return a < b; }
	extension template<std::char_type CT> constexpr bool eq(CT a, CT b) { return a == b; }
	extension template<std::char_type CT> constexpr CT* strcpy(CT* dest, const CT* src) { return array_copy<CT>(dest, src, std::strlen(src)); }
	extension template<std::char_type CT> constexpr CT* strncpy(CT* dest, const CT* src, size_t n) { return array_copy<CT>(dest, src, std::strnlen(src, n)); }
	extension template<std::char_type CT> constexpr CT* stpcpy(CT* dest, const CT* src) { size_t n = std::strlen(src); return array_copy<CT>(dest, src, n + 1) + n; }
	extension template<std::char_type CT> constexpr CT* stpncpy(CT* dest, const CT* src, size_t max) { size_t n = std::strnlen(src, max); return array_copy<CT>(dest, src, n + 1) + n; }
	extension template<std::char_type CT> constexpr int strncmp(const CT* s1, const CT* s2, size_t n) { for(size_t i = 0; i < n && (*s2 == *s1) && (*s1 != 0) && (*s2 != 0); ++i, ++s1, ++s2); return (*s1 < *s2) ? -1 : (*s1 > *s2 ? 1 : 0); }
	extension template<std::char_type CT> constexpr int strcmp(const CT* s1, const CT* s2) { return std::strncmp(s1, s2, std::strlen(s1)); }
	extension template<std::char_type CT> constexpr const CT* find(const CT* str, const CT* what) noexcept { if (!what[0]) return str; for (size_t i = 1; str[0]; str++) { if (str[0] == what[0]) { for (i = 1; what[i]; i++) { if (str[i] != what[i]) break; } if (!what[i]) return str; } } return nullptr; }
	template<> constexpr bool eq<char>(char a, char b) { return static_cast<unsigned char>(a) == static_cast<unsigned char>(b); }
	template<> constexpr bool lt<char>(char a, char b) { return static_cast<unsigned char>(a) < static_cast<unsigned char>(b); }
	constexpr int memcmp(const void* s1, const void* s2, size_t n) { return __builtin_memcmp(s1, s2, n); }
	typedef int64_t streamoff;
	template<typename ST>
	class fpos
	{
		ST __state{};
		streamoff __offs = 0;
	public:
		constexpr ST state() const { return __state; }
		constexpr void state(ST st) { __state = st; }
		constexpr fpos() = default;
		constexpr fpos(streamoff offs) : __state{}, __offs{ offs }{}
		constexpr fpos(fpos const&) = default;
		constexpr fpos(fpos&&) = default;
		constexpr fpos& operator=(fpos const&) = default;
		constexpr fpos& operator=(fpos&&) = default;
		constexpr operator streamoff() const { return __offs; }
		constexpr bool operator==(fpos const& that){ return this->__state == that.__state && this->__offs == that.__offs; }
		constexpr fpos& operator+=(streamoff off) { __offs += off; return *this; }
		constexpr fpos& operator-=(streamoff off) { __offs -= off; return *this; }
		constexpr fpos operator+(streamoff off) const { fpos<ST> r(*this); r += off; return r; }
		constexpr fpos operator-(streamoff off) const { fpos<ST> r(*this); r -= off; return r; }
		constexpr streamoff operator-(fpos const& that) const { return this->__offs - that.__offs; }
		template<typename PT> friend constexpr PT* operator+(PT* you, fpos<ST> const& me) { return (you + me.__offs); }
	};
	template<typename ST> constexpr fpos<ST> operator+(streamoff off, fpos<ST> const& p) { return p + off; }
	typedef fpos<mbstate_t> fpos_t;
	template<std::char_type CT>
	struct char_traits
	{
		typedef CT char_type;
		typedef int int_type;
		typedef streamoff off_type;
		typedef mbstate_t state_type;
		typedef fpos<mbstate_t> pos_type;
		typedef strong_ordering comparison_category;
		constexpr static char_type* assign(char_type* ptr, char_type c, size_t n) { if(!n) return ptr; return std::memset(ptr, c, n); }
		constexpr static void assign(char_type& c1, char_type const& c2) noexcept { c1 = c2; }
		constexpr static bool eq(char_type const& a, char_type const& b) noexcept { return a == b; }
		constexpr static bool eq_int_type(int_type const& a, int_type const& b) { return a == b;}
		constexpr static bool lt(char_type const& a, char_type const& b) noexcept { return a < b; }
		constexpr static char_type const* find(const char_type* ptr, size_t n, char_type const& c) noexcept { return std::find(ptr, n, c); }
		constexpr static char_type const* find(const char_type* ptr, const char_type* what) noexcept { return std::find(ptr, what); }
		constexpr static int_type to_int_type(char_type c) noexcept { return static_cast<int>(c); }
		constexpr static char_type to_char_type(int_type i) noexcept { return static_cast<char_type>(i);}
		constexpr static char_type* copy(char_type* dest, const char_type* src, size_t n) { return std::strncpy(dest, src, n); }
		constexpr static size_t length(const char_type* str) noexcept { return std::strlen(str); }
		constexpr static int_type compare(const char_type* lhs, const char_type* rhs, size_t n) noexcept { return std::strncmp(lhs, rhs, n); }
		constexpr static int_type eof() noexcept { return -1; }
		constexpr static int_type not_eof(int_type e) noexcept { return e > 0 ? e : (e * -1); }
		constexpr static char_type* move(char_type* dest, char_type const* src, size_t n) { return std::memmove(dest, src, n); }
	};
	typedef char_traits<char>::pos_type streampos;
	extension template<typename TT, typename CT> concept char_traits_type = std::char_type<CT> && requires
	{
		{ typename TT::char_type() } -> std::same_as<CT>;
		{ typename TT::int_type() } -> std::integral;
		{ typename TT::off_type() } -> std::signed_integral;
		{ TT::compare(declval<CT*>(), declval<CT*>(), declval<size_t>()) } -> std::same_as<typename TT::int_type>;
		{ TT::assign(declval<CT*>(), declval<CT>(), declval<size_t>()) } -> std::same_as<CT*>;
		{ TT::move(declval<CT*>(), declval<CT const*>(), declval<size_t>()) } -> std::same_as<CT*>;
		{ TT::copy(declval<CT*>(), declval<CT*>(), declval<size_t>()) } -> std::same_as<CT*>;
		{ TT::find(declval<CT*>(), declval<CT>(), declval<size_t>()) } -> std::same_as<const CT*>;
		{ TT::to_int_type(declval<CT>()) } -> std::same_as<typename TT::int_type>;
		{ TT::to_char_type(declval<typename TT::int_type>()) } -> std::same_as<CT>;
		{ TT::eq_int_type(declval<typename TT::int_type>(), declval<typename TT::int_type>()) } -> std::__detail::__boolean_testable;
		{ TT::find(declval<CT*>(),declval<CT*>()) } -> std::same_as<const CT*>;
		{ TT::length(declval<CT*>()) } -> std::same_as<decltype(sizeof(CT))>;
		{ TT::lt(declval<CT>(), declval<CT>()) } -> std::__detail::__boolean_testable;
		{ TT::eq(declval<CT>(), declval<CT>()) } -> std::__detail::__boolean_testable;
		{ TT::not_eof(declval<typename TT::int_type>()) } -> std::integral;
		{ TT::eof() } -> std::signed_integral;
		TT::assign(declval<CT&>(), declval<CT>());
		typename TT::state_type;
		typename TT::pos_type;
	};
	namespace __detail
	{
		template<typename TT> concept __has_comparison_category = requires { typename TT::comparison_category; };
		template<typename TT> constexpr auto __char_traits_cmp_cat(int __cmp) noexcept
		{ 
			if constexpr(__has_comparison_category<TT>)
			{
				using CT = typename TT::comparison_category;
				static_assert(!is_void_v<common_comparison_category_t<CT>>);
				return static_cast<CT>(__cmp <=> 0);
			}
			else return static_cast<weak_ordering>(__cmp <=> 0);
		}
	}
	extension namespace ext
	{
		template<char_type CT, char_traits_type<CT> TT = char_traits<CT>>
		struct lexcial_equals
		{
			typedef TT traits_type;
			constexpr bool operator()(const CT* const& a, const CT* const& b) const noexcept
			{
				size_t a_len = traits_type::length(a);
				if(traits_type::length(b) != a_len) return false;
				return !traits_type::compare(a, b, a_len);
			}
		};
	}
}
#endif