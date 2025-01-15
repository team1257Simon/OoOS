#include "kernel/libk_decls.h"
#include "string"
#include "string.h"
#include "limits"
#include "memory"
#include "gdtoa.h"
// C standard library functions are trivial in terms of the template functions in basic_string.hpp, so we can put them here for compatibility
extern "C"
{
    char* strstr(const char* str, const char* what) { return const_cast<char*>(std::__impl::__find_impl(str, what)); }
    char* strnchr(const char* str, size_t n, int what) { return std::find<char>(str, n, char(what)); }
    char* strchr(const char* str, int what) { return std::find<char>(str, std::strlen(str), char(what)); }
    int strncmp(const char* restrict lhs, const char* restrict rhs, size_t n) { return std::strncmp<char>(lhs, rhs, n); }
    int strcmp(const char* restrict lhs, const char* restrict rhs) { return std::strcmp<char>(lhs, rhs); }
    size_t strlen(const char* str) { return std::strlen<char>(str); }
    size_t strnlen(const char* str, size_t n) { return std::strnlen<char>(str, n); }
    char* strcpy(char* restrict dest, const char* restrict src) { return std::strcpy<char>(dest, src); }
    char* strncpy(char* restrict dest, const char* restrict src, size_t n) { return std::strncpy<char>(dest, src, n); }
    char* stpcpy(char* restrict dest, const char* restrict src) { return std::stpcpy<char>(dest, src); }
    char* stpncpy(char* restrict dest, const char* restrict src, size_t n) { return std::stpncpy<char>(dest, src, n); }
    wchar_t* wstrnchr(const wchar_t* str, size_t n, wchar_t what) { return std::find<wchar_t>(str, n, what); }
    wchar_t* wstrchr(const wchar_t* str, wchar_t what) { return std::find<wchar_t>(str, std::strlen(str), what); }
    int wstrncmp(const wchar_t* lhs, const wchar_t* rhs, size_t n) { return std::strncmp<wchar_t>(lhs, rhs, n); }
    int wstrcmp(const wchar_t* lhs, const wchar_t* rhs) { return std::strcmp<wchar_t>(lhs, rhs); }
    size_t wstrlen(const wchar_t* str) { return std::strlen<wchar_t>(str); }
    size_t wstrnlen(const wchar_t* str, size_t n) { return std::strnlen<wchar_t>(str, n); }
    wchar_t* wstrcpy(wchar_t* dest, const wchar_t* src) { return std::strcpy<wchar_t>(dest, src); }
    wchar_t* wstrncpy(wchar_t* dest, const wchar_t* src, size_t n) { return std::strncpy<wchar_t>(dest, src, n); }
    wchar_t* wstpcpy(wchar_t* dest, const wchar_t* src) { return std::stpcpy<wchar_t>(dest, src); }
    wchar_t* wstpncpy(wchar_t* dest, const wchar_t* src, size_t n) { return std::stpncpy<wchar_t>(dest, src, n); }
    void* memset(void* buffer, int value, size_t n) { return std::memset<int>(buffer, value, n); }
    char* strdup(const char* str) { size_t n = std::strlen(str) + 1; if(!n) return nullptr; char* result = std::allocator<char>{}.allocate(n); arraycopy<char>(result, str, n); return result; }
    char* strndup(const char* str, size_t max) { size_t n = std::strnlen(str, max); if(!n) return nullptr; char* result = std::allocator<char>{}.allocate(n); arraycopy<char>(result, str, n); return result; }
    const char* __assert_fail_text(const char* text, const char* fname, const char* filename, int line)
    {
        static std::string estr;
        estr = {"Assertion failed in function "};
        estr.append(fname ? fname : "");
        estr.append(", file ");
        estr.append(filename ? filename : "");
        std::string linestr = std::to_string(line);
        estr.append(", line ");
        estr.append(linestr);
        estr.append(": ");
        if(text) estr.append(text);
        return estr.c_str();
    }
    extern char* __dtoa(double d0, int mode, int ndigits, int* decpt, int* sign, char** rve);
    extern char* __ldtoa(long double* ld, int mode, int ndigits, int* decpt, int* sign, char** rve);
}
namespace std
{
    namespace __impl
    {
        template <std::char_type CT> struct __char_encode
        {
            constexpr static CT minus = CT{ 0 };
            constexpr static const CT digits[] = {};
            constexpr static const CT hexpref[] = {};
            constexpr static int casediff = 32;
            constexpr static CT dot = CT { 0 };
            template <std::char_type DT> constexpr static DT __to_other(CT ct) { return DT{0}; }
            constexpr static bool __is_upper(CT c) { return false; }
            constexpr static bool __is_lower(CT c) { return false; }
            constexpr static CT __to_upper(CT c) { return c - casediff; }
            constexpr static CT __to_lower(CT c) { return c + casediff; }
        };
        template<> 
        struct __char_encode<char>
        {
            constexpr static char minus = '-';
            constexpr static const char digits[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
            constexpr static const char hexpref[] = "x0";
            constexpr static int casediff = 'a' - 'A';
            constexpr static char dot = '.';
            template <std::char_type DT> requires (!is_same_v<DT, char>) constexpr static DT __to_other(char c) 
            { 
                if(c <= '9' && c >= '0') return __char_encode<DT>::digits[c - '0']; 
                else if(c >= 'A' && c <= 'F') return __char_encode<DT>::digits[10 + c - 'A'];
                else if(c >= 'a' && c <= 'f') return __char_encode<DT>::digits[10 + c - 'a'];
                else return __char_encode<DT>::dot;
            }
            constexpr static char __to_other(char c) { return c; }
            constexpr static bool __is_upper(char c) { return c >= 'A' && c <= 'Z'; }
            constexpr static bool __is_lower(char c) { return c >= 'a' && c <= 'z';  }
            constexpr static char __to_upper(char c) { return __is_lower(c) ? c - casediff : c; }
            constexpr static char __to_lower(char c) { return __is_upper(c) ? c + casediff : c; }
        };
        template<> 
        struct __char_encode<wchar_t> 
        {
            constexpr static wchar_t minus = L'-';
            constexpr static const wchar_t digits[] = { L'0', L'1', L'2', L'3', L'4', L'5', L'6', L'7', L'8', L'9', L'A', L'B', L'C', L'D', L'E', L'F' };
            constexpr static const wchar_t hexpref[] = L"x0";
            constexpr static int casediff = L'a' - L'A';
            constexpr static wchar_t dot = L'.';
            template <std::char_type DT> requires(!is_same_v<DT, wchar_t>) constexpr static DT __to_other(wchar_t c) 
            { 
                if(c <= L'9' && c >= L'0') return __char_encode<DT>::digits[c - L'0']; 
                else if(c >= L'A' && c <= L'F') return __char_encode<DT>::digits[10 + c - L'A'];
                else if(c >= L'a' && c <= L'f') return __char_encode<DT>::digits[10 + c - L'a'];
                else return __char_encode<DT>::dot;
            }
            constexpr static wchar_t __to_other(wchar_t c) { return c; }
            constexpr static bool __is_upper(wchar_t c) { return c >= L'A' && c <= L'Z'; }
            constexpr static bool __is_lower(wchar_t c) { return c >= L'a' && c <= L'z';  }
            constexpr static wchar_t __to_upper(wchar_t c) { return __is_lower(c) ? c - casediff : c; }
            constexpr static wchar_t __to_lower(wchar_t c) { return __is_upper(c) ? c + casediff : c; }
        };
        template<> 
        struct  __char_encode<char8_t> 
        {
            constexpr static char8_t minus = u8'-';
            constexpr static const char8_t digits[] = { u8'0', u8'1', u8'2', u8'3', u8'4', u8'5', u8'6', u8'7', u8'8', u8'9', u8'A', u8'B', u8'C', u8'D', u8'E', u8'F' };
            constexpr static const char8_t hexpref[] = u8"x0";
            constexpr static int casediff = u8'a' - u8'A';
            constexpr static char8_t dot = u8'.';
            template <std::char_type DT> requires(!is_same_v<DT, char8_t>) constexpr static DT __to_other(char8_t c) 
            { 
                if(c <= u8'9' && c >= u8'0') return __char_encode<DT>::digits[c - u8'0']; 
                else if(c >= u8'A' && c <= u8'F') return __char_encode<DT>::digits[10 + c - u8'A'];
                else if(c >= u8'a' && c <= u8'f') return __char_encode<DT>::digits[10 + c - u8'a'];
                else return __char_encode<DT>::dot;
            }
            constexpr static char8_t __to_other(char8_t c) { return c; }
            constexpr static bool __is_upper(char8_t c) { return c >= u8'A' && c <= u8'Z'; }
            constexpr static bool __is_lower(char8_t c) { return c >= u8'a' && c <= u8'z';  }
            constexpr static char8_t __to_upper(char8_t c) { return __is_lower(c) ? c - casediff : c; }
            constexpr static char8_t __to_lower(char8_t c) { return __is_upper(c) ? c + casediff : c; }
        };
        template<>
        struct __char_encode<char16_t> 
        {
            constexpr static char16_t minus = u'-';
            constexpr static const char16_t digits[] = { u'0', u'1', u'2', u'3', u'4', u'5', u'6', u'7', u'8', u'9', u'A', u'B', u'C', u'D', u'E', u'F' };
            constexpr static const char16_t hexpref[] = u"x0";
            constexpr static int casediff = u'a' - u'A';
            constexpr static char16_t dot = u'.';
            template <std::char_type DT> requires(!is_same_v<DT, char16_t>) constexpr static DT __to_other(char16_t c) 
            { 
                if(c <= u'9' && c >= u'0') return __char_encode<DT>::digits[c - u'0']; 
                else if(c >= u'A' && c <= u'F') return __char_encode<DT>::digits[10 + c - u'A'];
                else if(c >= u'a' && c <= u'f') return __char_encode<DT>::digits[10 + c - u'a'];
                else return __char_encode<DT>::dot;
            }
            constexpr static char16_t __to_other(char16_t c) { return c; }
            constexpr static bool __is_upper(char16_t c) { return c >= u'A' && c <= u'Z'; }
            constexpr static bool __is_lower(char16_t c) { return c >= u'a' && c <= u'z';  }
            constexpr static char16_t __to_upper(char16_t c) { return __is_lower(c) ? c - casediff : c; }
            constexpr static char16_t __to_lower(char16_t c) { return __is_upper(c) ? c + casediff : c; }
        };
        template <> 
        struct __char_encode<char32_t> 
        {
            constexpr static char32_t minus = U'-';
            constexpr static const char32_t digits[] = { U'0', U'1', U'2', U'3', U'4', U'5', U'6', U'7', U'8', U'9', U'A', U'B', U'C', U'D', U'E', U'F' };
            constexpr static const char32_t hexpref[] = U"x0";
            constexpr static int casediff = U'a' - U'A';
            constexpr static char32_t dot = U'.';
            template <std::char_type DT> requires(!is_same_v<DT, char32_t>) constexpr static DT __to_other(char32_t c) 
            { 
                if(c <= U'9' && c >= U'0') return __char_encode<DT>::digits[c - U'0']; 
                else if(c >= U'A' && c <= U'F') return __char_encode<DT>::digits[10 + c - U'A'];
                else if(c >= U'a' && c <= U'f') return __char_encode<DT>::digits[10 + c - U'a'];
                else return __char_encode<DT>::dot;
            }
            constexpr static char32_t __to_other(char32_t c) { return c; }
            constexpr static bool __is_upper(char32_t c) { return c >= U'A' && c <= U'Z'; }
            constexpr static bool __is_lower(char32_t c) { return c >= U'a' && c <= U'z';  }
            constexpr static char32_t __to_upper(char32_t c) { return __is_lower(c) ? c - casediff : c; }
            constexpr static char32_t __to_lower(char32_t c) { return __is_upper(c) ? c + casediff : c; }
        };
        template<std::integral IT> struct __pow_10 { constexpr static IT values[] = {}; };
        template<> struct __pow_10<uint8_t> { constexpr static uint8_t values[] = {1u, uint8_t(1E1), uint8_t(1E2)}; };
        template<> struct __pow_10<uint16_t> { constexpr static uint16_t values[] = {1u, uint16_t(1E1), uint16_t(1E2), uint16_t(1E3), uint16_t(1E4)}; };
        template<> struct __pow_10<uint32_t> { constexpr static uint32_t values[] = {1u, uint32_t(1E1), uint32_t(1E2), uint32_t(1E3), uint32_t(1E4),  uint32_t(1E5),  uint32_t(1E6),  uint32_t(1E7),  uint32_t(1E8),  uint32_t(1E9)}; };
        template<> struct __pow_10<uint64_t> { constexpr static uint64_t values[] = {1, uint64_t(1E1), uint64_t(1E2), uint64_t(1E3), uint64_t(1E4), uint64_t(1E5), uint64_t(1E6), uint64_t(1E7), uint64_t(1E8), uint64_t(1E9), uint64_t(1E10), uint64_t(1E11), uint64_t(1E12), uint64_t(1E13), uint64_t(1E14), uint64_t(1E15), uint64_t(1E16), uint64_t(1E17), uint64_t(1E18), uint64_t(1E19) }; };
        template<> struct __pow_10<int8_t> { constexpr static int8_t values[] = {1u, int8_t(1E1), int8_t(1E2)}; };
        template<> struct __pow_10<int16_t> { constexpr static int16_t values[] = {1u, int16_t(1E1), int16_t(1E2), int16_t(1E3), int16_t(1E4)}; };
        template<> struct __pow_10<int32_t> { constexpr static int32_t values[] = {1u, int32_t(1E1), int32_t(1E2), int32_t(1E3), int32_t(1E4),  int32_t(1E5),  int32_t(1E6),  int32_t(1E7),  int32_t(1E8),  int32_t(1E9)}; };
        template<> struct __pow_10<int64_t> { constexpr static int64_t values[] = {1, int64_t(1E1), int64_t(1E2), int64_t(1E3), int64_t(1E4), int64_t(1E5), int64_t(1E6), int64_t(1E7), int64_t(1E8), int64_t(1E9), int64_t(1E10), int64_t(1E11), int64_t(1E12), int64_t(1E13), int64_t(1E14), int64_t(1E15), int64_t(1E16), int64_t(1E17), int64_t(1E18) }; };
        template<std::integral IT> constexpr static size_t __max_dec_digits() noexcept { return sizeof(__pow_10<IT>::values) / sizeof(IT); }
        template<std::integral IT, std::char_type CT> struct __ntos_conv
        {
            using __digi_type = __char_encode<CT>;
            using __trait_type = std::char_traits<CT>;
            constexpr static size_t __max_dec = __max_dec_digits<IT>();
            constexpr static size_t __max_hex = 2 * sizeof(IT);
            constexpr static IT __get_pow10(size_t idx) noexcept { return __pow_10<IT>::values[idx]; }
            constexpr static IT __get_pow16(size_t idx) noexcept { return IT(1) << (idx * 4); }
            constexpr static IT __get_dec_digit_v(IT num, size_t idx) noexcept { return (num % __get_pow10(idx + 1)) / __get_pow10(idx); }
            constexpr static IT __get_hex_digit_v(IT num, size_t idx) noexcept { return (num & (IT(0xF) << (idx * 4))) >> (idx * 4); }
            constexpr static CT __get_dec_digit(IT num, size_t idx) noexcept { return __digi_type::digits[__get_dec_digit_v(num, idx)]; }
            constexpr static CT __get_hex_digit(IT num, size_t idx) noexcept { return __digi_type::digits[__get_hex_digit_v(num, idx)]; }
            constexpr static std::basic_string<CT> __to_string(IT i)
            {
                if(!i) return basic_string{ __digi_type::digits[0], CT(0) };
                std::basic_string<CT> str{};
                str.reserve(__max_dec);
                IT j;
                if constexpr(std::is_signed_v<IT>) { j = (i < 0) ? -i : i; }
                else j = i;
                for(size_t n = 0; n < __max_dec && __get_pow10(n) <= j; n++) str.append(__get_dec_digit(j, n));
                if constexpr(std::is_signed_v<IT>) { if(i < 0) str.append(__digi_type::minus); }
                return std::basic_string<CT>{ str.rend(), str.rbegin() };
            }
            constexpr static std::basic_string<CT> __to_hex_string(IT i)
            {
                if(!i) return basic_string{ __digi_type::digits[0], CT(0) };
                std::basic_string<CT> hstr{};
                hstr.reserve(__max_hex);
                IT j;
                if constexpr(std::is_signed_v<IT>) { j = (i < 0) ? -i : i; }
                else j = i;
                for(size_t n = 0; n < __max_hex && __get_pow16(n) <= j; n++) hstr.append(__get_hex_digit(j, n));
                hstr.append(__digi_type::hexpref);
                if constexpr(std::is_signed_v<IT>) { if(i < 0) hstr.append(__digi_type::minus); }
                return std::basic_string<CT>{ hstr.rend(), hstr.rbegin() };
            }
        };
        inline std::string __fptocs_conv(float f, int digits)
        {
            int dp = 0, sign = 0;
            char *rve = NULL, *result = __dtoa(double(f), 2, digits, &dp, &sign, &rve);
            if(!result) { return {"E"}; }
            std::string str { result, rve };
            if(sign) str.insert(str.cbegin(), '-');
            if(dp != 9999) str.insert(str.cbegin() + dp, '.');
            return str;
        }
        inline std::string __fptocs_conv(double d, int digits)
        {
            int dp = 0, sign = 0;
            char *rve = NULL, *result = __dtoa(d, 0, digits, &dp, &sign, &rve);
            if(!result) { return {"E"}; }
            std::string str { result, rve };
            if(sign) str.insert(str.cbegin(), '-');
            if(dp != 9999) str.insert(str.cbegin() + dp, '.');
            return str;
        }
        inline std::string __fptocs_conv(long double ld, int digits)
        {
            int dp = 0, sign = 0;
            char *rve = NULL, *result = __ldtoa(&ld, 1, digits, &dp, &sign, &rve);
            if(!result) { return {"E"}; }
            std::string str { result, rve };
            if(sign) str.insert(str.cbegin(), '-');
            if(dp != INT_MAX) str.insert(str.cbegin() + dp, '.');
            return str;
        }
        template<std::char_type DT> requires(!is_same_v<DT, char>) constexpr std::basic_string<DT> __cvt_digits(std::basic_string<char> const& in_str) { size_t n = in_str.size(); std::basic_string<DT> str{}; str.reserve(n + 1); for(size_t i = 0; i < n; i++) { str.append(__char_encode<char>::template __to_other<DT>(in_str[i])); } return str; }
        template<std::integral IT, std::char_type CT> [[gnu::always_inline]] constexpr std::basic_string<CT> __to_string(IT it) { return __ntos_conv<IT, CT>::__to_string(it); }
        template<std::char_type CT> [[gnu::always_inline]] constexpr std::basic_string<CT> __ptr_to_string(void* ptr) { return __ntos_conv<uintptr_t, CT>::__to_hex_string(std::bit_cast<uintptr_t>(ptr)); }
        bool __isalnum(char c)  { return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
        bool __isalpha(char c)  { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
        bool __iscntrl(char c)  { return (c > '\0' && c < ' ') || (c == '\x7F'); }
        bool __isdigit(char c)  { return c <= '9' && c >= '0'; }
        bool __isgraph(char c)  { return c > '\x1F' && c < '\x7F'; }
        bool __islower(char c)  { return c <= 'z' && c >= 'a'; }
        bool __isprint(char c)  { return __isgraph(c); }
        bool __isspace(char c)  { return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v'; }
        bool __ispunct(char c)  { return !__isalnum(c)&& !__isspace(c) && __isprint(c); }
        bool __isupper(char c)  { return c >= 'A' && c <= 'Z'; }
        bool __isxdigit(char c) { return __isdigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'); }
        char __tolower(char c) { return __char_encode<char>::__to_lower(c); }
        char __toupper(char c) { return __char_encode<char>::__to_upper(c); }
        template<std::integral IT>
        IT ston(const char* str, char* &eptr, int base = 10)
        {
            if(base > 32) return 0;
            bool neg = false;
            if(str[0] == '+') str++;
            if(str[0] == '-') { neg = true; str++; }
            if((base == 0 || base == 16) && (str[0] == '0' && __tolower(str[1]) == 'x')) { base = 16; str += 2; }
            else if(base == 0) base = (str[0] == '0') ? 8 : 10;
            for(eptr = const_cast<char*>(str); *eptr; eptr++)
            {
                char c = __tolower(*eptr);
                if(__isspace(c)) continue;
                if(!__isalnum(c) || (__isalpha(c) && (base < 11 || (base - 10) <= (c - 'a'))) || (__isdigit(c) && (c - '0') >= base)) break;
            }
            IT result = 0;
            for(const char* cc = str; cc < eptr; cc++)
            {
                result *= base;
                char cdc = __tolower(*cc);
                if(__isspace(cdc)) continue;
                if(__isalpha(cdc)) result += ((cdc - 'a') + 10);
                else result += (cdc - '0');
            }
            if(neg) result = -result;
            return result;
        }
    }
    string to_string(int value) { return __impl::__to_string<int, char>(value); }
    string to_string(long value) { return __impl::__to_string<long, char>(value); }
    string to_string(long long value) { return __impl::__to_string<long long, char>(value); }
    string to_string(unsigned int value) { return __impl::__to_string<unsigned int, char>(value); }
    string to_string(unsigned long value) { return __impl::__to_string<unsigned long, char>(value); }
    string to_string(unsigned long long value) { return __impl::__to_string<unsigned long long, char>(value); }
    string to_string(void* ptr) { return __impl::__ptr_to_string<char>(ptr); }
    string to_string(float value) { return __impl::__fptocs_conv(value, 6); }
    string to_string(double value) { return __impl::__fptocs_conv(value, 10); }
    string to_string(long double value) { return __impl::__fptocs_conv(value, 256); }
    string to_string(bool value) { return value ? "true" : "false"; }
    wstring to_wstring(int value) { return __impl::__to_string<int, wchar_t>(value); }
	wstring to_wstring(long value) { return __impl::__to_string<long, wchar_t>(value); }
	wstring to_wstring(long long value) { return __impl::__to_string<long long, wchar_t>(value); }
	wstring to_wstring(unsigned int value) { return __impl::__to_string<unsigned int, wchar_t>(value); }
	wstring to_wstring(unsigned long value) { return __impl::__to_string<unsigned long, wchar_t>(value); }
	wstring to_wstring(unsigned long long value) { return __impl::__to_string<unsigned long long, wchar_t>(value); }
	wstring to_wstring(void* ptr) { return __impl::__ptr_to_string<wchar_t>(ptr); }
    wstring to_wstring(float value) { return __impl::__cvt_digits<wchar_t>(to_string(value)); }
    wstring to_wstring(double value) { return __impl::__cvt_digits<wchar_t>(to_string(value)); }
    wstring to_wstring(long double value) { return __impl::__cvt_digits<wchar_t>(to_string(value)); }
    wstring to_wstring(bool value) { return value ? L"true" : L"false"; }
    u8string to_u8string(int value) { return __impl::__to_string<int, char8_t>(value); }
	u8string to_u8string(long value) { return __impl::__to_string<long, char8_t>(value); }
	u8string to_u8string(long long value) { return __impl::__to_string<long long, char8_t>(value); }
	u8string to_u8string(unsigned int value) { return __impl::__to_string<unsigned int, char8_t>(value); }
	u8string to_u8string(unsigned long value) { return __impl::__to_string<unsigned long, char8_t>(value); }
	u8string to_u8string(unsigned long long value) { return __impl::__to_string<unsigned long long, char8_t>(value); }
	u8string to_u8string(void* ptr) { return __impl::__ptr_to_string<char8_t>(ptr); }
    u8string to_u8string(float value) { return __impl::__cvt_digits<char8_t>(to_string(value)); }
    u8string to_u8string(double value) { return __impl::__cvt_digits<char8_t>(to_string(value)); }
    u8string to_u8string(long double value) { return __impl::__cvt_digits<char8_t>(to_string(value));; }
    u8string to_u8string(bool value) { return value ? u8"true" : u8"false"; }
    u16string to_u16string(int value) { return __impl::__to_string<int, char16_t>(value); }
	u16string to_u16string(long value) { return __impl::__to_string<long, char16_t>(value); }
	u16string to_u16string(long long value) { return __impl::__to_string<long long, char16_t>(value); }
	u16string to_u16string(unsigned int value) { return __impl::__to_string<unsigned int, char16_t>(value); }
	u16string to_u16string(unsigned long value) { return __impl::__to_string<unsigned long, char16_t>(value); }
	u16string to_u16string(unsigned long long value) { return __impl::__to_string<unsigned long long, char16_t>(value); }
	u16string to_u16string(void* ptr) { return __impl::__ptr_to_string<char16_t>(ptr); }
    u16string to_u16string(float value) { return __impl::__cvt_digits<char16_t>(to_string(value)); }
    u16string to_u16string(double value) { return __impl::__cvt_digits<char16_t>(to_string(value)); }
    u16string to_u16string(long double value) { return __impl::__cvt_digits<char16_t>(to_string(value)); }
    u16string to_u16string(bool value) { return value ? u"true" : u"false"; }
    u32string to_u32string(int value) { return __impl::__to_string<int, char32_t>(value); }
	u32string to_u32string(long value) { return __impl::__to_string<long, char32_t>(value); }
	u32string to_u32string(long long value) { return __impl::__to_string<long long, char32_t>(value); }
	u32string to_u32string(unsigned int value) { return __impl::__to_string<unsigned int, char32_t>(value); }
	u32string to_u32string(unsigned long value) { return __impl::__to_string<unsigned long, char32_t>(value); }
	u32string to_u32string(unsigned long long value) { return __impl::__to_string<unsigned long long, char32_t>(value); }
	u32string to_u32string(void* ptr) { return __impl::__ptr_to_string<char32_t>(ptr); }
    u32string to_u32string(float value) { return __impl::__cvt_digits<char32_t>(to_string(value)); }
    u32string to_u32string(double value) { return __impl::__cvt_digits<char32_t>(to_string(value)); }
    u32string to_u32string(long double value) { return __impl::__cvt_digits<char32_t>(to_string(value)); }
    u32string to_u32string(bool value) { return value ? U"true" : U"false"; }
    namespace ext
    {
        std::string fcvt(float f, int ndigits) { return std::__impl::__fptocs_conv(f, ndigits); }
        std::string fcvtd(double d, int ndigits) { return std::__impl::__fptocs_conv(d, ndigits); }
        std::string fcvtl(long double ld, int ndigits) { return std::__impl::__fptocs_conv(ld, ndigits); }
    }
}
using namespace std::__impl;
// A few things I wrote to avoid having to implement printf just for a few basic string concatenations in libsupcxx, and a few other basic 
extern "C"
{
    size_t __strcat_basic(char* buffer, size_t max, const char* str1, const char* str2)
    {
        std::string cat{str1};
        cat += str2;
        strncpy(buffer, cat.c_str(), max);
        return std::min(cat.size(), max);
    }
    size_t __tricat(char* buffer, size_t max, const char* str1, const char* str2, const char* str3)
    {
        std::string cat{ str1 };
        cat += str2;
        cat += str3;
        strncpy(buffer, cat.c_str(), max);
        return std::min(cat.size(), max);
    }
    int __ltoi_alloc(char** out, long n)
    {
        std::string s = std::to_string(n);
        *out = new char[s.size() + 1];
        if(!out) return -1;
        std::strncpy(*out, s.c_str(), s.size());
        (*out)[s.size()] = 0;
        return int(s.size());
    }
    int __cfcvt(char* buffer, size_t len, float f)
    {
        std::string s = std::ext::fcvt(f, len - 2); // one for the decimal point or exponent "e", one for the sign
        std::strncpy(buffer, s.c_str(), s.size());
        return int(s.size());
    }
    int __cdcvt(char* buffer, size_t len, double d)
    {
        std::string s = std::ext::fcvtd(d, len - 2);
        std::strncpy(buffer, s.c_str(), s.size());
        return int(s.size());
    }
    int __cldcvt(char* buffer, size_t len, long double ld)
    {
        std::string s = std::ext::fcvtl(ld, len - 2);
        std::strncpy(buffer, s.c_str(), s.size());
        return int(s.size());
    }
    int isalnum(int c)	   { return __isalnum(static_cast<char>(c)) ? 1 : 0; }
    int isalpha(int c)     { return __isalpha(static_cast<char>(c)) ? 1 : 0; }
    int iscntrl(int c)     { return __iscntrl(static_cast<char>(c)) ? 1 : 0; }
    int isdigit(int c)     { return __isdigit(static_cast<char>(c)) ? 1 : 0; }
    int isgraph(int c)     { return __isgraph(static_cast<char>(c)) ? 1 : 0; }
    int islower(int c)     { return __islower(static_cast<char>(c)) ? 1 : 0; }
    int isprint(int c)     { return __isprint(static_cast<char>(c)) ? 1 : 0; }
    int ispunct(int c)     { return __ispunct(static_cast<char>(c)) ? 1 : 0; }
    int isspace(int c)     { return __isspace(static_cast<char>(c)) ? 1 : 0; }
    int isupper(int c)     { return __isupper(static_cast<char>(c)) ? 1 : 0; }
    int isxdigit(int c)    { return __isxdigit(static_cast<char>(c)) ? 1 : 0; }
    int tolower(int c)     { return static_cast<int>(__tolower(static_cast<char>(c))); }
    int toupper(int c)     { return static_cast<int>(__toupper(static_cast<char>(c))); }
    int atoi(const char *str) { char* tmp = NULL; return ston<int>(str, tmp); }
    double atof(const char *str) { char* tmp = NULL; return strtod(str, &tmp); }
    long atol(const char *str) { char* tmp = NULL; return ston<long>(str, tmp); }
    long strtol(const char *str, char **endptr, int base) { return ston<long>(str, *endptr, base); }
    long long strtoll(const char *str, char **endptr, int base) { return ston<long long>(str, *endptr, base); }
    unsigned long strtoul(const char *str, char **endptr, int base) { return ston<unsigned long>(str, *endptr, base); }
    unsigned long long strtoull(const char *str, char **endptr, int base) { return ston<unsigned long long>(str, *endptr, base); } 
}