#include <libk_decls.h>
#include <string>
#include <string.h>
#include <limits>
#include <memory>
#include <gdtoa.h>
extern "C"
{
	extern char* __dtoa(double d0, int mode, int ndigits, int* decpt, int* sign, char** rve);
	extern char* __ldtoa(long double* ld, int mode, int ndigits, int* decpt, int* sign, char** rve);
}
namespace std
{
	namespace __impl
	{
		template<std::integral IT> struct __pow_10;
		template<> struct __pow_10<uint8_t>									{ constexpr static uint8_t	values[] = { 1U, uint8_t(1E1), uint8_t(1E2) }; };
		template<> struct __pow_10<uint16_t>								{ constexpr static uint16_t values[] = { 1U, uint16_t(1E1), uint16_t(1E2), uint16_t(1E3), uint16_t(1E4) }; };
		template<> struct __pow_10<uint32_t>								{ constexpr static uint32_t values[] = { 1U, uint32_t(1E1), uint32_t(1E2), uint32_t(1E3), uint32_t(1E4), uint32_t(1E5), uint32_t(1E6), uint32_t(1E7), uint32_t(1E8), uint32_t(1E9) }; };
		template<> struct __pow_10<uint64_t>								{ constexpr static uint64_t values[] = { 1, uint64_t(1E1), uint64_t(1E2), uint64_t(1E3), uint64_t(1E4), uint64_t(1E5), uint64_t(1E6), uint64_t(1E7), uint64_t(1E8), uint64_t(1E9), uint64_t(1E10), uint64_t(1E11), uint64_t(1E12), uint64_t(1E13), uint64_t(1E14), uint64_t(1E15), uint64_t(1E16), uint64_t(1E17), uint64_t(1E18), uint64_t(1E19) }; };
		template<> struct __pow_10<int8_t>									{ constexpr static int8_t	values[] = { 1, int8_t(1E1), int8_t(1E2) }; };
		template<> struct __pow_10<int16_t>									{ constexpr static int16_t	values[] = { 1, int16_t(1E1), int16_t(1E2), int16_t(1E3), int16_t(1E4) }; };
		template<> struct __pow_10<int32_t>									{ constexpr static int32_t	values[] = { 1, int32_t(1E1), int32_t(1E2), int32_t(1E3), int32_t(1E4), int32_t(1E5), int32_t(1E6),	int32_t(1E7), int32_t(1E8),	int32_t(1E9) }; };
		template<> struct __pow_10<int64_t>									{ constexpr static int64_t	values[] = { 1, int64_t(1E1), int64_t(1E2), int64_t(1E3), int64_t(1E4), int64_t(1E5), int64_t(1E6), int64_t(1E7), int64_t(1E8), int64_t(1E9), int64_t(1E10), int64_t(1E11), int64_t(1E12), int64_t(1E13), int64_t(1E14), int64_t(1E15), int64_t(1E16), int64_t(1E17), int64_t(1E18) }; };
		template<> struct __pow_10<long long> : __pow_10<int64_t>			{};
		template<> struct __pow_10<unsigned long long> : __pow_10<uint64_t>	{};
		template<std::integral IT> constexpr static size_t __max_dec_digits() noexcept { return sizeof(__pow_10<IT>::values) / sizeof(IT); }
		inline bool __isalnum(char c)	{ return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
		inline bool __isalpha(char c)	{ return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
		inline bool __iscntrl(char c)	{ return (c > '\0' && c < ' ') || (c == '\x7F'); }
		inline bool __isdigit(char c)	{ return c <= '9' && c >= '0'; }
		inline bool __isgraph(char c)	{ return c > '\x1F' && c < '\x7F'; }
		inline bool __islower(char c)	{ return c <= 'z' && c >= 'a'; }
		inline bool __isprint(char c)	{ return __isgraph(c); }
		inline bool __isspace(char c)	{ return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v'; }
		inline bool __ispunct(char c)	{ return !__isalnum(c)&& !__isspace(c) && __isprint(c); }
		inline bool __isupper(char c)	{ return c >= 'A' && c <= 'Z'; }
		inline bool __isxdigit(char c) { return __isdigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'); }
		inline char __tolower(char c)	{ return __isupper(c) ? (c + ('a' - 'A')) : c; }
		inline char __toupper(char c)	{ return __islower(c) ? (c - ('a' - 'A')) : c; }
		template<std::char_type CT> struct __char_encode;
		template<>
		struct __char_encode<char>
		{
			constexpr static char minus				= '-';
			constexpr static const char digits[]	= { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
			constexpr static const char hexpref[]	= "x0";
			constexpr static int casediff			= 'a' - 'A';
			constexpr static char dot				= '.';
			constexpr static char __to_other(char c) { return c; }
			constexpr static bool __is_upper(char c) { return c >= 'A' && c <= 'Z'; }
			constexpr static bool __is_lower(char c) { return c >= 'a' && c <= 'z';	}
			constexpr static char __to_upper(char c) { return __is_lower(c) ? c - casediff : c; }
			constexpr static char __to_lower(char c) { return __is_upper(c) ? c + casediff : c; }
			template <std::char_type DT> requires (!is_same_v<DT, char>)
			constexpr static DT __to_other(char c)
			{
				if(c <= '9' && c >= '0') return __char_encode<DT>::digits[c - '0'];
				else if(c >= 'A' && c <= 'F') return __char_encode<DT>::digits[10 + c - 'A'];
				else if(c >= 'a' && c <= 'f') return __char_encode<DT>::digits[10 + c - 'a'];
				else return __char_encode<DT>::dot;
			}
		};
		template<>
		struct __char_encode<wchar_t>
		{
			constexpr static wchar_t minus				= L'-';
			constexpr static const wchar_t digits[]		= { L'0', L'1', L'2', L'3', L'4', L'5', L'6', L'7', L'8', L'9', L'A', L'B', L'C', L'D', L'E', L'F' };
			constexpr static const wchar_t hexpref[]	= L"x0";
			constexpr static int casediff				= L'a' - L'A';
			constexpr static wchar_t dot				= L'.';
			constexpr static wchar_t __to_other(wchar_t c) { return c; }
			constexpr static bool __is_upper(wchar_t c) { return c >= L'A' && c <= L'Z'; }
			constexpr static bool __is_lower(wchar_t c) { return c >= L'a' && c <= L'z';	}
			constexpr static wchar_t __to_upper(wchar_t c) { return __is_lower(c) ? c - casediff : c; }
			constexpr static wchar_t __to_lower(wchar_t c) { return __is_upper(c) ? c + casediff : c; }
			template <std::char_type DT> requires(!is_same_v<DT, wchar_t>)
			constexpr static DT __to_other(wchar_t c)
			{
				if(c <= L'9' && c >= L'0') return __char_encode<DT>::digits[c - L'0'];
				else if(c >= L'A' && c <= L'F') return __char_encode<DT>::digits[10 + c - L'A'];
				else if(c >= L'a' && c <= L'f') return __char_encode<DT>::digits[10 + c - L'a'];
				else return __char_encode<DT>::dot;
			}
		};
		template<>
		struct	__char_encode<char8_t>
		{
			constexpr static char8_t minus				= u8'-';
			constexpr static const char8_t digits[]		= { u8'0', u8'1', u8'2', u8'3', u8'4', u8'5', u8'6', u8'7', u8'8', u8'9', u8'A', u8'B', u8'C', u8'D', u8'E', u8'F' };
			constexpr static const char8_t hexpref[]	= u8"x0";
			constexpr static int casediff				= u8'a' - u8'A';
			constexpr static char8_t dot				= u8'.';
			constexpr static char8_t __to_other(char8_t c) { return c; }
			constexpr static bool __is_upper(char8_t c) { return c >= u8'A' && c <= u8'Z'; }
			constexpr static bool __is_lower(char8_t c) { return c >= u8'a' && c <= u8'z';	}
			constexpr static char8_t __to_upper(char8_t c) { return __is_lower(c) ? c - casediff : c; }
			constexpr static char8_t __to_lower(char8_t c) { return __is_upper(c) ? c + casediff : c; }
			template <std::char_type DT> requires(!is_same_v<DT, char8_t>)
			constexpr static DT __to_other(char8_t c)
			{
				if(c <= u8'9' && c >= u8'0') return __char_encode<DT>::digits[c - u8'0'];
				else if(c >= u8'A' && c <= u8'F') return __char_encode<DT>::digits[10 + c - u8'A'];
				else if(c >= u8'a' && c <= u8'f') return __char_encode<DT>::digits[10 + c - u8'a'];
				else return __char_encode<DT>::dot;
			}
		};
		template<>
		struct __char_encode<char16_t>
		{
			constexpr static char16_t minus				= u'-';
			constexpr static const char16_t digits[]	= { u'0', u'1', u'2', u'3', u'4', u'5', u'6', u'7', u'8', u'9', u'A', u'B', u'C', u'D', u'E', u'F' };
			constexpr static const char16_t hexpref[]	= u"x0";
			constexpr static int casediff				= u'a' - u'A';
			constexpr static char16_t dot				= u'.';
			constexpr static char16_t __to_other(char16_t c) { return c; }
			constexpr static bool __is_upper(char16_t c) { return c >= u'A' && c <= u'Z'; }
			constexpr static bool __is_lower(char16_t c) { return c >= u'a' && c <= u'z';	}
			constexpr static char16_t __to_upper(char16_t c) { return __is_lower(c) ? c - casediff : c; }
			constexpr static char16_t __to_lower(char16_t c) { return __is_upper(c) ? c + casediff : c; }
			template <std::char_type DT> requires(!is_same_v<DT, char16_t>)
			constexpr static DT __to_other(char16_t c)
			{
				if(c <= u'9' && c >= u'0') return __char_encode<DT>::digits[c - u'0'];
				else if(c >= u'A' && c <= u'F') return __char_encode<DT>::digits[10 + c - u'A'];
				else if(c >= u'a' && c <= u'f') return __char_encode<DT>::digits[10 + c - u'a'];
				else return __char_encode<DT>::dot;
			}
		};
		template <>
		struct __char_encode<char32_t>
		{
			constexpr static char32_t minus				= U'-';
			constexpr static const char32_t digits[]	= { U'0', U'1', U'2', U'3', U'4', U'5', U'6', U'7', U'8', U'9', U'A', U'B', U'C', U'D', U'E', U'F' };
			constexpr static const char32_t hexpref[]	= U"x0";
			constexpr static int casediff				= U'a' - U'A';
			constexpr static char32_t dot				= U'.';
			constexpr static char32_t __to_other(char32_t c) { return c; }
			constexpr static bool __is_upper(char32_t c) { return c >= U'A' && c <= U'Z'; }
			constexpr static bool __is_lower(char32_t c) { return c >= U'a' && c <= U'z';	}
			constexpr static char32_t __to_upper(char32_t c) { return __is_lower(c) ? c - casediff : c; }
			constexpr static char32_t __to_lower(char32_t c) { return __is_upper(c) ? c + casediff : c; }
			template <std::char_type DT> requires(!is_same_v<DT, char32_t>)
			constexpr static DT __to_other(char32_t c)
			{
				if(c <= U'9' && c >= U'0') return __char_encode<DT>::digits[c - U'0'];
				else if(c >= U'A' && c <= U'F') return __char_encode<DT>::digits[10 + c - U'A'];
				else if(c >= U'a' && c <= U'f') return __char_encode<DT>::digits[10 + c - U'a'];
				else return __char_encode<DT>::dot;
			}
		};
		template<std::unsigned_integral UIT> size_t __clzg(UIT i, size_t x = 1UL)
		{
			if(!i) return x;
			if constexpr(sizeof(UIT) > sizeof(uint32_t)) return __builtin_clzll(static_cast<uint64_t>(i));
			else return __builtin_clz(i);
		}
		template<std::integral IT, std::char_type CT>
		struct __ntos_conv
		{
			using __digi_type					= __char_encode<CT>;
			using __trait_type					= std::char_traits<CT>;
			constexpr static size_t __max_dec	= __max_dec_digits<IT>();
			constexpr static size_t __max_hex	= 2UZ * sizeof(IT);
			constexpr static size_t __xdigits(IT i) noexcept { using UIT = typename std::make_unsigned<IT>::type; return div_round_up(sizeof(IT) * CHAR_BIT - __clzg(static_cast<UIT>(i)), 4); }
			constexpr static IT __get_pow10(size_t idx) noexcept { return __pow_10<IT>::values[idx]; }
			constexpr static IT __get_pow16(size_t idx) noexcept { return IT(1) << (idx * 4); }
			constexpr static IT __get_dec_digit_v(IT num, size_t idx) noexcept { if(idx < __max_dec - 1) return (num % __get_pow10(idx + 1)) / __get_pow10(idx); else return num / __get_pow10(idx); }
			constexpr static IT __get_hex_digit_v(IT num, size_t idx) noexcept { return (num & (static_cast<IT>(0xF) << (idx * 4))) >> (idx * 4); }
			constexpr static CT __get_dec_digit(IT num, size_t idx) noexcept { return __digi_type::digits[__get_dec_digit_v(num, idx)]; }
			constexpr static CT __get_hex_digit(IT num, size_t idx) noexcept { return __digi_type::digits[__get_hex_digit_v(num, idx)]; }
			constexpr static IT __abs(IT i) noexcept { if constexpr(std::is_signed_v<IT>) return (i < 0) ? -i : i; else return i; }
			constexpr static std::basic_string<CT> __to_string(IT i)
			{
				if(!i) return basic_string<CT>(1UZ, __digi_type::digits[0]);
				std::basic_string<CT> str(static_cast<size_t>(__max_dec + 1));
				IT j			= __abs(i);
				for(size_t n	= 0UZ; n < __max_dec && __get_pow10(n) <= j; n++) str.append(__get_dec_digit(j, n));
				if constexpr(std::is_signed_v<IT>) { if(i < 0) str.append(__digi_type::minus); }
				return std::basic_string<CT>(str.rend(), str.rbegin());
			}
			constexpr static std::basic_string<CT> __to_hex_string(IT i)
			{
				if(!i) return basic_string<CT>(1UZ, __digi_type::digits[0]);
				size_t nd		= __xdigits(i);
				std::basic_string<CT> hstr(static_cast<size_t>(nd + 3));
				IT j			= __abs(i);
				for(size_t n	= 0UZ; n < nd; n++) hstr.append(__get_hex_digit(j, n));
				hstr.append(__digi_type::hexpref);
				if constexpr(std::is_signed_v<IT>) { if(i < 0) hstr.append(__digi_type::minus); }
				return basic_string<CT>(hstr.rend(), hstr.rbegin());
			}
			constexpr static std::basic_string<CT> __to_bare_hex_string(IT i)
			{
				if(!i) return basic_string<CT>(1UZ, __digi_type::digits[0]);
				size_t nd		= __xdigits(i);
				std::basic_string<CT> hstr(static_cast<size_t>(nd + 1));
				IT j			= __abs(i);
				for(size_t n	= 0UZ; n < nd; n++) hstr.append(__get_hex_digit(j, n));
				if constexpr(std::is_signed_v<IT>) { if(i < 0) hstr.append(__digi_type::minus); }
				return basic_string<CT>(hstr.rend(), hstr.rbegin());
			}
		};
		inline std::string __fptocs_conv(float f, int digits)
		{
			int dp			= 0, sign	= 0;
			char* rve		= nullptr;
			char* result	= __dtoa(double(f), 2, digits, &dp, &sign, &rve);
			if(__unlikely(!result)) { return "ERROR"; }
			std::string str(result, rve);
			if(sign) str.insert(str.cbegin(), '-');
			if(dp != 9999) str.insert(str.cbegin() + dp, '.');
			return str;
		}
		inline std::string __fptocs_conv(double d, int digits)
		{
			int dp			= 0, sign	= 0;
			char* rve		= nullptr;
			char* result	= __dtoa(d, 0, digits, &dp, &sign, &rve);
			if(__unlikely(!result)) { return "ERROR"; }
			std::string str(result, rve);
			if(sign) str.insert(str.cbegin(), '-');
			if(dp != 9999) str.insert(str.cbegin() + dp, '.');
			return str;
		}
		inline std::string __fptocs_conv(long double ld, int digits)
		{
			int dp			= 0, sign	= 0;
			char* rve		= nullptr;
			char* result	= __ldtoa(&ld, 1, digits, &dp, &sign, &rve);
			if(__unlikely(!result)) { return "ERROR"; }
			std::string str(result, rve);
			if(sign) str.insert(str.cbegin(), '-');
			if(dp != INT_MAX) str.insert(str.cbegin() + dp, '.');
			return str;
		}
		inline std::string __fptohs_conv(float f, int digits)
		{
			int dp			= 0, sign = 0;
			char* rve		= nullptr;
			char* result	= __hdtoa(double(f), __char_encode<char>::digits, digits, &dp, &sign, &rve);
			if(__unlikely(!result)) { return "ERROR"; }
			std::string str(result, rve);
			if(sign) str.insert(str.cbegin(), '-');
			if(dp != 9999) str.insert(str.cbegin() + dp, '.');
			return str;
		}
		inline std::string __fptohs_conv(double d, int digits)
		{
			int dp			= 0, sign	= 0;
			char* rve		= nullptr;
			char* result	= __hdtoa(d, __char_encode<char>::digits, digits, &dp, &sign, &rve);
			if(__unlikely(!result)) { return "ERROR"; }
			std::string str(result, rve);
			if(sign) str.insert(str.cbegin(), '-');
			if(dp != 9999) str.insert(str.cbegin() + dp, '.');
			return str;
		}
		inline std::string __fptohs_conv(long double ld, int digits)
		{
			int dp			= 0, sign	= 0;
			char* rve		= nullptr;
			char* result	= __hldtoa(ld, __char_encode<char>::digits, digits, &dp, &sign, &rve);
			if(__unlikely(!result)) { return "ERROR"; }
			std::string str(result, rve);
			if(sign) str.insert(str.cbegin(), '-');
			if(dp != INT_MAX) str.insert(str.cbegin() + dp, '.');
			return str;
		}
		template<std::char_type DT> requires(!is_same_v<DT, char>)
		constexpr std::basic_string<DT> __cvt_digits(std::basic_string<char> const& in_str)
		{
			size_t n	= in_str.size();
			std::basic_string<DT> str{};
			str.reserve(n + 1UZ);
			for(size_t i = 0UZ; i < n; i++) { str.append(__char_encode<char>::template __to_other<DT>(in_str[i])); }
			return str;
		}
		template<std::integral IT>
		IT ston(const char* str, char*& eptr, int base = 10)
		{
			if(base > 32) return 0;
			bool neg	= false;
			if(str[0] == '+') str++;
			if(str[0] == '-') { neg = true; str++; }
			if((base == 0 || base == 16) && (str[0] == '0' && __tolower(str[1]) == 'x')) { base = 16; str += 2; }
			else if(base == 0) base = (str[0] == '0') ? 8 : 10;
			for(eptr	= const_cast<char*>(str); *eptr; eptr++)
			{
				char c	= __tolower(*eptr);
				if(__isspace(c)) continue;
				if(!__isalnum(c) || (__isalpha(c) && (base < 11 || (base - 10) <= (c - 'a'))) || (__isdigit(c) && (c - '0') >= base)) break;
			}
			IT result	= 0;
			for(const char* cc = str; cc < eptr; cc++)
			{
				result	*= base;
				char cdc = __tolower(*cc);
				if(__isspace(cdc)) continue;
				if(__isalpha(cdc)) result += ((cdc - 'a') + 10);
				else result	+= (cdc - '0');
			}
			if(neg) result	= -result;
			return result;
		}
	}
}