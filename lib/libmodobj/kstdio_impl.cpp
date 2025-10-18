#pragma GCC visibility push(hidden)
#include "ext/dynamic_streambuf.hpp"
#include "kernel/libk_decls.h"
#include "stdarg.h"
#include "string"
#include "string.h"
#include "limits"
#include "memory"
#include "gdtoa.h"
#pragma gcc visibility pop
// C standard library functions are trivial in terms of the template functions in basic_string.hpp, so we can put them here for compatibility
extern "C"
{
    extern char* __dtoa(double d0, int mode, int ndigits, int* decpt, int* sign, char** rve);
    extern char* __ldtoa(long double* ld, int mode, int ndigits, int* decpt, int* sign, char** rve);
    char* strstr(const char* str, const char* what) { return const_cast<char*>(std::find(str, what)); }
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
    char* strdup(const char* str) { size_t n = std::strlen(str) + 1; if(!n) return nullptr; char* result = std::allocator<char>().allocate(n); array_copy<char>(result, str, n); return result; }
    char* strndup(const char* str, size_t max) { size_t n = std::strnlen(str, max); if(!n) return nullptr; char* result = std::allocator<char>().allocate(n); array_copy<char>(result, str, n); return result; }
    void* memmove(void* dest, const void* src, size_t n)
    {
        char* dend = static_cast<char*>(dest) + n;
        const char* send = static_cast<const char*>(src) + n;
        if((dest < src && dend < src) || (src < dest && send < dest)) { return memcpy(dest, src, n); }
        void* tmp_src = ::operator new(n);
        memcpy(tmp_src, src, n);
        memcpy(dest, tmp_src, n);
        ::operator delete(tmp_src, n);
        return dest;
    }
}
#pragma GCC visibility push(hidden)
namespace std
{
    namespace __impl
    {
        template<std::integral IT> struct __pow_10  { constexpr static IT       values[] = {}; };
        template<> struct __pow_10<uint8_t>         { constexpr static uint8_t  values[] = { 1U, uint8_t(1E1), uint8_t(1E2) }; };
        template<> struct __pow_10<uint16_t>        { constexpr static uint16_t values[] = { 1U, uint16_t(1E1), uint16_t(1E2), uint16_t(1E3), uint16_t(1E4) }; };
        template<> struct __pow_10<uint32_t>        { constexpr static uint32_t values[] = { 1U, uint32_t(1E1), uint32_t(1E2), uint32_t(1E3), uint32_t(1E4), uint32_t(1E5), uint32_t(1E6), uint32_t(1E7), uint32_t(1E8),  uint32_t(1E9) }; };
        template<> struct __pow_10<uint64_t>        { constexpr static uint64_t values[] = { 1, uint64_t(1E1), uint64_t(1E2), uint64_t(1E3), uint64_t(1E4), uint64_t(1E5), uint64_t(1E6), uint64_t(1E7), uint64_t(1E8), uint64_t(1E9), uint64_t(1E10), uint64_t(1E11), uint64_t(1E12), uint64_t(1E13), uint64_t(1E14), uint64_t(1E15), uint64_t(1E16), uint64_t(1E17), uint64_t(1E18), uint64_t(1E19) }; };
        template<> struct __pow_10<int8_t>          { constexpr static int8_t   values[] = { 1, int8_t(1E1), int8_t(1E2) }; };
        template<> struct __pow_10<int16_t>         { constexpr static int16_t  values[] = { 1, int16_t(1E1), int16_t(1E2), int16_t(1E3), int16_t(1E4) }; };
        template<> struct __pow_10<int32_t>         { constexpr static int32_t  values[] = { 1, int32_t(1E1), int32_t(1E2), int32_t(1E3), int32_t(1E4), int32_t(1E5),  int32_t(1E6),  int32_t(1E7),  int32_t(1E8),  int32_t(1E9) }; };
        template<> struct __pow_10<int64_t>         { constexpr static int64_t  values[] = { 1, int64_t(1E1), int64_t(1E2), int64_t(1E3), int64_t(1E4), int64_t(1E5), int64_t(1E6), int64_t(1E7), int64_t(1E8), int64_t(1E9), int64_t(1E10), int64_t(1E11), int64_t(1E12), int64_t(1E13), int64_t(1E14), int64_t(1E15), int64_t(1E16), int64_t(1E17), int64_t(1E18) }; };
        template<std::integral IT> constexpr static size_t __max_dec_digits() noexcept { return sizeof(__pow_10<IT>::values) / sizeof(IT); } 
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
        char __tolower(char c)  { return __isupper(c) ? (c + ('a' - 'A')) : c; }
        char __toupper(char c)  { return __islower(c) ? (c - ('a' - 'A')) : c; }
        template <std::char_type CT> struct __char_encode
        {
            constexpr static CT minus           = static_cast<CT>(0);
            constexpr static const CT digits[]  = {};
            constexpr static const CT hexpref[] = {};
            constexpr static int casediff       = 32;
            constexpr static CT dot             = static_cast<CT>(0);
            template <std::char_type DT> constexpr static DT __to_other(CT ct) { return static_cast<DT>(ct); }
            constexpr static bool __is_upper(CT c) { return false; }
            constexpr static bool __is_lower(CT c) { return false; }
            constexpr static CT __to_upper(CT c) { return c - casediff; }
            constexpr static CT __to_lower(CT c) { return c + casediff; }
        };
        template<> 
        struct __char_encode<char>
        {
            constexpr static char minus             = '-';
            constexpr static const char digits[]    = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
            constexpr static const char hexpref[]   = "x0";
            constexpr static int casediff           = 'a' - 'A';
            constexpr static char dot               = '.';
            constexpr static char __to_other(char c) { return c; }
            constexpr static bool __is_upper(char c) { return c >= 'A' && c <= 'Z'; }
            constexpr static bool __is_lower(char c) { return c >= 'a' && c <= 'z';  }
            constexpr static char __to_upper(char c) { return __is_lower(c) ? c - casediff : c; }
            constexpr static char __to_lower(char c) { return __is_upper(c) ? c + casediff : c; }
            template <std::char_type DT> requires (!is_same_v<DT, char>) constexpr static DT __to_other(char c) 
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
            constexpr static wchar_t minus              = L'-';
            constexpr static const wchar_t digits[]     = { L'0', L'1', L'2', L'3', L'4', L'5', L'6', L'7', L'8', L'9', L'A', L'B', L'C', L'D', L'E', L'F' };
            constexpr static const wchar_t hexpref[]    = L"x0";
            constexpr static int casediff               = L'a' - L'A';
            constexpr static wchar_t dot                = L'.';
            constexpr static wchar_t __to_other(wchar_t c) { return c; }
            constexpr static bool __is_upper(wchar_t c) { return c >= L'A' && c <= L'Z'; }
            constexpr static bool __is_lower(wchar_t c) { return c >= L'a' && c <= L'z';  }
            constexpr static wchar_t __to_upper(wchar_t c) { return __is_lower(c) ? c - casediff : c; }
            constexpr static wchar_t __to_lower(wchar_t c) { return __is_upper(c) ? c + casediff : c; }
            template <std::char_type DT> requires(!is_same_v<DT, wchar_t>) constexpr static DT __to_other(wchar_t c) 
            { 
                if(c <= L'9' && c >= L'0') return __char_encode<DT>::digits[c - L'0']; 
                else if(c >= L'A' && c <= L'F') return __char_encode<DT>::digits[10 + c - L'A'];
                else if(c >= L'a' && c <= L'f') return __char_encode<DT>::digits[10 + c - L'a'];
                else return __char_encode<DT>::dot;
            }
        };
        template<> 
        struct  __char_encode<char8_t> 
        {
            constexpr static char8_t minus              = u8'-';
            constexpr static const char8_t digits[]     = { u8'0', u8'1', u8'2', u8'3', u8'4', u8'5', u8'6', u8'7', u8'8', u8'9', u8'A', u8'B', u8'C', u8'D', u8'E', u8'F' };
            constexpr static const char8_t hexpref[]    = u8"x0";
            constexpr static int casediff               = u8'a' - u8'A';
            constexpr static char8_t dot                = u8'.';
            constexpr static char8_t __to_other(char8_t c) { return c; }
            constexpr static bool __is_upper(char8_t c) { return c >= u8'A' && c <= u8'Z'; }
            constexpr static bool __is_lower(char8_t c) { return c >= u8'a' && c <= u8'z';  }
            constexpr static char8_t __to_upper(char8_t c) { return __is_lower(c) ? c - casediff : c; }
            constexpr static char8_t __to_lower(char8_t c) { return __is_upper(c) ? c + casediff : c; }
            template <std::char_type DT> requires(!is_same_v<DT, char8_t>) constexpr static DT __to_other(char8_t c) 
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
            constexpr static char16_t minus             = u'-';
            constexpr static const char16_t digits[]    = { u'0', u'1', u'2', u'3', u'4', u'5', u'6', u'7', u'8', u'9', u'A', u'B', u'C', u'D', u'E', u'F' };
            constexpr static const char16_t hexpref[]   = u"x0";
            constexpr static int casediff               = u'a' - u'A';
            constexpr static char16_t dot               = u'.';
            constexpr static char16_t __to_other(char16_t c) { return c; }
            constexpr static bool __is_upper(char16_t c) { return c >= u'A' && c <= u'Z'; }
            constexpr static bool __is_lower(char16_t c) { return c >= u'a' && c <= u'z';  }
            constexpr static char16_t __to_upper(char16_t c) { return __is_lower(c) ? c - casediff : c; }
            constexpr static char16_t __to_lower(char16_t c) { return __is_upper(c) ? c + casediff : c; }
            template <std::char_type DT> requires(!is_same_v<DT, char16_t>) constexpr static DT __to_other(char16_t c) 
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
            constexpr static char32_t minus             = U'-';
            constexpr static const char32_t digits[]    = { U'0', U'1', U'2', U'3', U'4', U'5', U'6', U'7', U'8', U'9', U'A', U'B', U'C', U'D', U'E', U'F' };
            constexpr static const char32_t hexpref[]   = U"x0";
            constexpr static int casediff               = U'a' - U'A';
            constexpr static char32_t dot               = U'.';
            constexpr static char32_t __to_other(char32_t c) { return c; }
            constexpr static bool __is_upper(char32_t c) { return c >= U'A' && c <= U'Z'; }
            constexpr static bool __is_lower(char32_t c) { return c >= U'a' && c <= U'z';  }
            constexpr static char32_t __to_upper(char32_t c) { return __is_lower(c) ? c - casediff : c; }
            constexpr static char32_t __to_lower(char32_t c) { return __is_upper(c) ? c + casediff : c; }
            template <std::char_type DT> requires(!is_same_v<DT, char32_t>) constexpr static DT __to_other(char32_t c) 
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
        template<std::integral IT, std::char_type CT> struct __ntos_conv
        {
            using __digi_type                   = __char_encode<CT>;
            using __trait_type                  = std::char_traits<CT>;
            constexpr static size_t __max_dec   = __max_dec_digits<IT>();
            constexpr static size_t __max_hex   = 2 * sizeof(IT);
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
                if(!i) return basic_string(1U, __digi_type::digits[0]);
                std::basic_string<CT> str(static_cast<size_t>(__max_dec + 1));
                IT j = __abs(i);
                for(size_t n = 0; n < __max_dec && __get_pow10(n) <= j; n++) str.append(__get_dec_digit(j, n));
                if constexpr(std::is_signed_v<IT>) { if(i < 0) str.append(__digi_type::minus); }
                str.shrink_to_fit();
                return std::basic_string<CT>(str.rend(), str.rbegin());
            }
            constexpr static std::basic_string<CT> __to_hex_string(IT i)
            {
                if(!i) return basic_string(1U, __digi_type::digits[0]);
                size_t nd   = __xdigits(i);
                std::basic_string<CT> hstr(static_cast<size_t>(nd + 3));
                IT j        = __abs(i);
                for(size_t n = 0; n < nd; n++) hstr.append(__get_hex_digit(j, n));
                hstr.append(__digi_type::hexpref);
                if constexpr(std::is_signed_v<IT>) { if(i < 0) hstr.append(__digi_type::minus); }
                hstr.shrink_to_fit();
                return basic_string<CT>(hstr.rend(), hstr.rbegin());
            }
            constexpr static std::basic_string<CT> __to_bare_hex_string(IT i)
            {
                if(!i) return basic_string(1U, __digi_type::digits[0]);
                size_t nd   = __xdigits(i);
                std::basic_string<CT> hstr(static_cast<size_t>(nd + 1));
                IT j        = __abs(i);
                for(size_t n = 0; n < nd; n++) hstr.append(__get_hex_digit(j, n));
                if constexpr(std::is_signed_v<IT>) { if(i < 0) hstr.append(__digi_type::minus); }
                hstr.shrink_to_fit();
                return basic_string<CT>(hstr.rend(), hstr.rbegin());
            }
        };
        inline std::string __fptocs_conv(float f, int digits)
        {
            int dp          = 0, sign = 0;
            char* rve       = nullptr;
            char* result    = __dtoa(double(f), 2, digits, &dp, &sign, &rve);
            if(__unlikely(!result || !(rve >= result))) { return "ERROR"; }
            std::string str(result, rve);
            if(sign) str.insert(str.cbegin(), '-');
            if(dp != 9999) str.insert(str.cbegin() + dp, '.');
            return str;
        }
        inline std::string __fptocs_conv(double d, int digits)
        {
            int dp          = 0, sign = 0;
            char* rve       = nullptr;
            char* result    = __dtoa(d, 0, digits, &dp, &sign, &rve);
            if(__unlikely(!result || !(rve >= result))) { return "ERROR"; }
            std::string str(result, rve);
            if(sign) str.insert(str.cbegin(), '-');
            if(dp != 9999) str.insert(str.cbegin() + dp, '.');
            return str;
        }
        inline std::string __fptocs_conv(long double ld, int digits)
        {
            int dp          = 0, sign = 0;
            char* rve       = nullptr;
            char* result    = __ldtoa(&ld, 1, digits, &dp, &sign, &rve);
            if(__unlikely(!result || !(rve >= result))) { return "ERROR"; }
            std::string str(result, rve);
            if(sign) str.insert(str.cbegin(), '-');
            if(dp != INT_MAX) str.insert(str.cbegin() + dp, '.');
            return str;
        }
        inline std::string __fptohs_conv(float f, int digits)
        {
            int dp          = 0, sign = 0;
            char* rve       = nullptr;
            char* result    = __hdtoa(double(f), __char_encode<char>::digits, digits, &dp, &sign, &rve);
            if(__unlikely(!result || !(rve >= result))) { return "ERROR"; }
            std::string str(result, rve);
            if(sign) str.insert(str.cbegin(), '-');
            if(dp != 9999) str.insert(str.cbegin() + dp, '.');
            return str;
        }
        inline std::string __fptohs_conv(double d, int digits)
        {
            int dp          = 0, sign = 0;
            char* rve       = nullptr;
            char* result    = __hdtoa(d, __char_encode<char>::digits, digits, &dp, &sign, &rve);
            if(__unlikely(!result || !(rve >= result))) { return "ERROR"; }
            std::string str(result, rve);
            if(sign) str.insert(str.cbegin(), '-');
            if(dp != 9999) str.insert(str.cbegin() + dp, '.');
            return str;
        }
        inline std::string __fptohs_conv(long double ld, int digits)
        {
            int dp          = 0, sign = 0;
            char* rve       = nullptr;
            char* result    = __hldtoa(ld, __char_encode<char>::digits, digits, &dp, &sign, &rve);
            if(__unlikely(!result || !(rve >= result))) { return "ERROR"; }
            std::string str(result, rve);
            if(sign) str.insert(str.cbegin(), '-');
            if(dp != INT_MAX) str.insert(str.cbegin() + dp, '.');
            return str;
        }
        template<std::char_type DT> requires(!is_same_v<DT, char>) constexpr std::basic_string<DT> __cvt_digits(std::basic_string<char> const& in_str) 
        { 
            size_t n = in_str.size(); 
            std::basic_string<DT> str{}; 
            str.reserve(n + 1); 
            for(size_t i = 0; i < n; i++) { str.append(__char_encode<char>::template __to_other<DT>(in_str[i])); } 
            return str; 
        }
        template<std::integral IT>
        IT ston(const char* str, char*& eptr, int base = 10)
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
    inline string to_string(int value) { return __impl::__ntos_conv<int, char>::__to_string(value); }
    inline string to_string(long value) { return __impl::__ntos_conv<long, char>::__to_string(value); }
    inline string to_string(long long value) { return __impl::__ntos_conv<long long, char>::__to_string(value); }
    inline string to_string(unsigned int value) { return __impl::__ntos_conv<unsigned int, char>::__to_string(value); }
    inline string to_string(unsigned long value) { return __impl::__ntos_conv<unsigned long, char>::__to_string(value); }
    inline string to_string(unsigned long long value) { return __impl::__ntos_conv<unsigned long long, char>::__to_string(value); }
    inline string to_string(void* ptr) { return __impl::__ntos_conv<uintptr_t, char>::__to_hex_string(std::bit_cast<uintptr_t>(ptr)); }
    inline string to_string(float value) { return __impl::__fptocs_conv(value, 6); }
    inline string to_string(double value) { return __impl::__fptocs_conv(value, 10); }
    inline string to_string(long double value) { return __impl::__fptocs_conv(value, 256); }
    inline string to_string(bool value) { return value ? "true" : "false"; }
    inline string to_string(int value, ext::hex_t) { return __impl::__ntos_conv<int, char>::__to_hex_string(value); }
    inline string to_string(long value, ext::hex_t) { return __impl::__ntos_conv<long, char>::__to_hex_string(value); }
    inline string to_string(long long value, ext::hex_t) { return __impl::__ntos_conv<long long, char>::__to_hex_string(value); }
    inline string to_string(unsigned int value, ext::hex_t) { return __impl::__ntos_conv<unsigned int, char>::__to_hex_string(value); }
    inline string to_string(unsigned long value, ext::hex_t) { return __impl::__ntos_conv<unsigned long, char>::__to_hex_string(value); }
    inline string to_string(unsigned long long value, ext::hex_t) { return __impl::__ntos_conv<unsigned long long, char>::__to_hex_string(value); }
    namespace ext
    {
        inline std::string fcvt(float f, int ndigits) { return std::__impl::__fptocs_conv(f, ndigits); }
        inline std::string fcvtd(double d, int ndigits) { return std::__impl::__fptocs_conv(d, ndigits); }
        inline std::string fcvtl(long double ld, int ndigits) { return std::__impl::__fptocs_conv(ld, ndigits); }
        inline std::string fcvth(float f, int ndigits) { return std::__impl::__fptohs_conv(f, ndigits); }
        inline std::string fcvthd(double d, int ndigits) { return std::__impl::__fptohs_conv(d, ndigits); }
        inline std::string fcvthl(long double ld, int ndigits) { return std::__impl::__fptohs_conv(ld, ndigits); }
        inline std::string to_upper(std::string const& str) { std::string result(str.size(), str.get_allocator()); for(size_t i = 0; i < str.size(); i++) { result.append(__impl::__toupper(str[i])); } return result; }
        inline std::string to_lower(std::string const& str) { std::string result(str.size(), str.get_allocator()); for(size_t i = 0; i < str.size(); i++) { result.append(__impl::__tolower(str[i])); } return result; }
    }
}
#pragma GCC visibility pop
extern "C"
{
    int isalnum(int c)	   { return std::__impl::__isalnum(static_cast<char>(c)) ? 1 : 0; }
    int isalpha(int c)     { return std::__impl::__isalpha(static_cast<char>(c)) ? 1 : 0; }
    int iscntrl(int c)     { return std::__impl::__iscntrl(static_cast<char>(c)) ? 1 : 0; }
    int isdigit(int c)     { return std::__impl::__isdigit(static_cast<char>(c)) ? 1 : 0; }
    int isgraph(int c)     { return std::__impl::__isgraph(static_cast<char>(c)) ? 1 : 0; }
    int islower(int c)     { return std::__impl::__islower(static_cast<char>(c)) ? 1 : 0; }
    int isprint(int c)     { return std::__impl::__isprint(static_cast<char>(c)) ? 1 : 0; }
    int ispunct(int c)     { return std::__impl::__ispunct(static_cast<char>(c)) ? 1 : 0; }
    int isspace(int c)     { return std::__impl::__isspace(static_cast<char>(c)) ? 1 : 0; }
    int isupper(int c)     { return std::__impl::__isupper(static_cast<char>(c)) ? 1 : 0; }
    int isxdigit(int c)    { return std::__impl::__isxdigit(static_cast<char>(c)) ? 1 : 0; }
    int tolower(int c)     { return static_cast<int>(std::__impl::__tolower(static_cast<char>(c))); }
    int toupper(int c)     { return static_cast<int>(std::__impl::__toupper(static_cast<char>(c))); }
    int atoi(const char* str) { char* tmp = nullptr; return std::__impl::ston<int>(str, tmp); }
    double atof(const char* str) { char* tmp = nullptr; return strtod(str, std::addressof(tmp)); }
    long atol(const char* str) { char* tmp = nullptr; return std::__impl::ston<long>(str, tmp); }
    long strtol(const char* str, char** endptr, int base) { return std::__impl::ston<long>(str, *endptr, base); }
    long long strtoll(const char* str, char** endptr, int base) { return std::__impl::ston<long long>(str, *endptr, base); }
    unsigned long strtoul(const char* str, char** endptr, int base) { return std::__impl::ston<unsigned long>(str, *endptr, base); }
    unsigned long long strtoull(const char* str, char** endptr, int base) { return std::__impl::ston<unsigned long long>(str, *endptr, base); } 
#pragma GCC visibility push(hidden)
    size_t __strcat_basic(char* buffer, size_t max, const char* str1, const char* str2)
    {
        std::string cat{ str1 };
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
        *out = std::allocator<char>().allocate(s.size() + 1);
        if(__unlikely(!out)) return -1;
        std::strncpy(*out, s.c_str(), s.size());
        (*out)[s.size()] = 0;
        return static_cast<int>(s.size());
    }
    int __cfcvt(char* buffer, size_t len, float f)
    {
        std::string s = std::ext::fcvt(f, len - 2); // one for the decimal point or exponent "e", one for the sign
        std::strncpy(buffer, s.c_str(), s.size());
        return static_cast<int>(s.size());
    }
    int __cdcvt(char* buffer, size_t len, double d)
    {
        std::string s = std::ext::fcvtd(d, len - 2);
        std::strncpy(buffer, s.c_str(), s.size());
        return static_cast<int>(s.size());
    }
    int __cldcvt(char* buffer, size_t len, long double ld)
    {
        std::string s = std::ext::fcvtl(ld, len - 2);
        std::strncpy(buffer, s.c_str(), s.size());
        return static_cast<int>(s.size());
    }
}
constexpr const char errstr[] = "[SPECIFIER ERROR]";
static int stderr_fd_placeholder = 1;
static int stdout_stdin_placeholder = 0;
size_t __arg_insert_ptr(void* ptr, std::basic_streambuf<char>* stream) { std::string str = std::to_string(ptr); return stream->sputn(str.c_str(), str.size()); }
template<std::floating_point FT> std::string fcvtg(FT ft, size_t ndigit);
template<std::floating_point FT> std::string fcvtg(FT ft, size_t ndigit, std::ext::hex_t);
template<> inline std::string fcvtg(float f, size_t ndigit) { return std::ext::fcvt(f, ndigit); }
template<> inline std::string fcvtg(double d, size_t ndigit) { return std::ext::fcvtd(d, ndigit); }
template<> inline std::string fcvtg(long double ld, size_t ndigit) { return std::ext::fcvtl(ld, ndigit); }
template<> inline std::string fcvtg(float f, size_t ndigit, std::ext::hex_t) { return std::ext::fcvth(f, ndigit); }
template<> inline std::string fcvtg(double d, size_t ndigit, std::ext::hex_t) { return std::ext::fcvthd(d, ndigit); }
template<> inline std::string fcvtg(long double ld, size_t ndigit, std::ext::hex_t) { return std::ext::fcvthl(ld, ndigit); }
template<std::integral IT>
inline size_t __arg_insert_dec(IT i, std::basic_streambuf<char>* stream, size_t minwid, bool zeropad, bool left, bool sign)
{
    std::string str = std::to_string(i);
    if(i > 0 && sign) str.insert(str.begin(), '+');
    if(str.size() < minwid) 
    { 
        size_t diff = std::min(SIZE_MAX / 4, static_cast<size_t>(minwid - str.size()));
        if(left) str.append(std::string(diff, ' ')); 
        else str.insert(str.begin() + (i < 0 || sign ? 1 : 0), std::string(diff, zeropad ? '0' : ' '));
    }
    return stream->sputn(str.c_str(), str.size());
}
template<std::integral IT>
inline size_t __arg_insert_hex(IT i, std::basic_streambuf<char>* stream, size_t minwid, bool zeropad, bool left, bool caps, bool pref)
{
    std::string str = std::to_string(i, std::ext::hex);
    if(!pref) str.erase(str.begin(), str.begin() + 2);
    if(str.size() < minwid) 
    { 
        size_t diff = std::min(SIZE_MAX / 4, static_cast<size_t>(minwid - str.size()));
        if(left) str.append(std::string(diff, ' ')); 
        else str.insert(str.begin() + (pref && zeropad ? 2 : 0), std::string(diff, zeropad ? '0' : ' '));
    }
    if(caps) return stream->sputn(str.c_str(), str.size());
    std::string lower = std::ext::to_lower(str);
    return stream->sputn(lower.c_str(), lower.size());
}
template<std::floating_point FT>
inline size_t __arg_insert_fp(FT f, std::basic_streambuf<char>* stream, size_t minwid, unsigned int precision, bool zeropad, bool left, bool sign)
{
    std::string str = fcvtg(f, precision);
    if(f > 0 && sign) str.insert(str.begin(), '+');
     if(str.size() < minwid) 
    { 
        size_t diff = std::min(SIZE_MAX / 4, static_cast<size_t>(minwid - str.size()));
        if(left) str.append(std::string(diff, ' ')); 
        else str.insert(str.begin() + (f < 0 || sign ? 1 : 0), std::string(diff, zeropad ? '0' : ' '));
    }
    return stream->sputn(str.c_str(), str.size());
}
template<std::floating_point FT>
inline size_t __arg_insert_fpx(FT f, std::basic_streambuf<char>* stream, size_t minwid, unsigned int precision, bool zeropad, bool left, bool caps, bool pref)
{
    std::string str = fcvtg(f, precision, std::ext::hex);
    if(pref) str.insert(str.begin(), std::move(std::string("0x")));
    if(str.size() < minwid) 
    { 
        size_t diff = std::min(SIZE_MAX / 4, static_cast<size_t>(minwid - str.size()));
        if(left) str.append(std::string(diff, ' ')); 
        else str.insert(str.begin() + (pref && zeropad ? 2 : 0), std::string(diff, zeropad ? '0' : ' '));
    }
    if(caps) return stream->sputn(str.c_str(), str.size());
    std::string lower = std::ext::to_lower(str);
    return stream->sputn(lower.c_str(), lower.size());
}
inline size_t __kvfprintf_impl(std::basic_streambuf<char>* stream, const char* fmt, va_list args)
{
    size_t n = std::strlen(fmt);
    typedef const char* cstr;
    size_t cnt = 0;
    cstr c, d, end;
    for(c = fmt, d = std::find(fmt, n, '%'), end = fmt + n; d && d < end; c = d + 1, d = std::find(c, n, '%'))
    {
        cnt += stream->sputn(c, static_cast<std::streamsize>(d - c));
        char spec               = d[1];
        bool zeropad            = false;
        bool left               = false;
        bool alt                = false;
        bool sign               = false;
        bool caps               = false;
        bool dot                = false;
        size_t minwid           = 0;
        int lenarg              = 0;
        unsigned int precision  = 1;
        bool have_prec          = false;
        int widarg              = 0;
        bool finish             = false;
        char* tmpptr            = nullptr;
        int tmpint              = 0;
        for(cstr e = d + 1; e < end && !finish; e++) // <ObligatoryReference>E</ObligatoryReference>
        {
            switch(spec)
            {
            case '%':
                stream->sputc('%');
                cnt++;
                finish = true;
                break;;
            case '.':
                dot = true;
                break;
            case '+':
                sign = true;
                break;
            case '#':
                alt = true;
                break;
            case '-':
                left = true;
                break;
            case '*':
                widarg = va_arg(args, int);
            case '0':
                if(!dot && !zeropad) { zeropad = true; break; }
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                widarg = spec - '0';
                if(dot)
                {
                    precision   = widarg > 0 ? widarg : precision;
                    have_prec   = true;
                    dot         = false;
                }
                else { if(widarg < 0) { left = true; widarg *= -1; } minwid = widarg; }
                break;
            case 'c':
                stream->sputc(va_arg(args, int));
                cnt++;
                finish = true;
                break;
            case 's':
                tmpptr          = va_arg(args, char*);
                if(!have_prec)
                    precision   = std::strlen(tmpptr);
                cnt             += stream->sputn(tmpptr, precision);
                finish          = true;
                break;
            case 'h':
                lenarg--;
                break;
            case 'L':
                lenarg++;
            case 't':
            case 'z':
            case 'j':
            case 'l':
                lenarg++;
                break;
            case 'i':
            case 'd':
                if(lenarg > 1) cnt          += __arg_insert_dec(va_arg(args, long long), stream, minwid, zeropad, left, sign);
                else if(lenarg == 1) cnt    += __arg_insert_dec(va_arg(args, long), stream, minwid, zeropad, left, sign);
                else if(lenarg == 0) cnt    += __arg_insert_dec(va_arg(args, int), stream, minwid, zeropad, left, sign);
                else
                {
                    tmpint                  = va_arg(args, int);
                    if(lenarg == -1) cnt    += __arg_insert_dec(static_cast<short>(tmpint), stream, minwid, zeropad, left, sign);
                    else cnt                += __arg_insert_dec(static_cast<signed char>(tmpint), stream, minwid, zeropad, left, sign);
                }
                finish = true;
                break;
            case 'u':
                if(lenarg > 1) cnt          += __arg_insert_dec(va_arg(args, unsigned long long), stream, minwid, zeropad, left, sign);
                else if(lenarg == 1) cnt    += __arg_insert_dec(va_arg(args, unsigned long), stream, minwid, zeropad, left, sign);
                else if(lenarg == 0) cnt    += __arg_insert_dec(va_arg(args, unsigned int), stream, minwid, zeropad, left, sign);
                else
                {
                    tmpint                  = va_arg(args, int);
                    if(lenarg == -1) cnt    += __arg_insert_dec(static_cast<unsigned short>(tmpint), stream, minwid, zeropad, left, sign);
                    else cnt                += __arg_insert_dec(static_cast<unsigned char>(tmpint), stream, minwid, zeropad, left, sign);
                }
                finish = true;
                break;
            case 'X': 
                caps = true;
            case 'x':
                if(lenarg > 1) cnt          += __arg_insert_hex(va_arg(args, unsigned long long), stream, minwid, zeropad, left, caps, alt);
                else if(lenarg == 1) cnt    += __arg_insert_hex(va_arg(args, unsigned long), stream, minwid, zeropad, left, caps, alt);
                else if(lenarg == 0) cnt    += __arg_insert_hex(va_arg(args, unsigned int), stream, minwid, zeropad, left, caps, alt);
                else 
                {
                    tmpint                  = va_arg(args, int);
                    if(lenarg == -1) cnt    += __arg_insert_hex(static_cast<unsigned short>(tmpint), stream, minwid, zeropad, left, sign, alt);
                    else cnt                += __arg_insert_hex(static_cast<unsigned char>(tmpint), stream, minwid, zeropad, left, sign, alt);
                }
                finish = true;
                break;
            case 'f':
            case 'F':
                if(lenarg > 1) cnt  += __arg_insert_fp(va_arg(args, long double), stream, minwid, have_prec ? precision : 6, zeropad, left, sign);
                else cnt            += __arg_insert_fp(va_arg(args, double), stream, minwid, have_prec ? precision : 6, zeropad, left, sign);
                finish = true;
                break;
            case 'A':
                caps = true;
            case 'a':
                if(lenarg > 1) cnt  += __arg_insert_fpx(va_arg(args, long double), stream, minwid, have_prec ? precision : 6, zeropad, left, caps, !alt);
                else cnt            += __arg_insert_fpx(va_arg(args, double), stream, minwid, have_prec ? precision : 6, zeropad, left, caps, !alt);
                finish = true;
                break;
            case 'n':
                *(va_arg(args, size_t*)) = cnt;
                finish = true;
                break;
            case 'p':
                cnt     += __arg_insert_ptr(va_arg(args, void*), stream);
                finish  = true;
                break;
            default:
                cnt     += stream->sputn(errstr, sizeof(errstr));
                finish  = true;
                break;
            }
        }
    }
    stream->pubsync();
    return cnt;
}
typedef int FILE;
#pragma GCC visibility pop
extern "C"
{
    FILE* stdin     = std::addressof(stdout_stdin_placeholder);
    FILE* stdout    = std::addressof(stdout_stdin_placeholder);
    FILE* stderr    = std::addressof(stderr_fd_placeholder);
    // The buffers used for the fprintf implementations here currently lead nowhere.
    // TODO: tie in the kernel logging somehow when possible
    size_t kvfprintf(FILE* fd, const char* fmt, va_list args)
    {
        std::ext::dynamic_streambuf<char> db;
        std::basic_streambuf<char>* stream = &db;
        if(*fd) stream->sputn("[!]", 3);
        return __kvfprintf_impl(stream, fmt, args);
    }
    size_t kvsnprintf(char* restrict buffer, size_t n, const char* restrict fmt, va_list args)
    {
        std::ext::dynamic_streambuf<char> db(n);
        size_t result = __kvfprintf_impl(std::addressof(db), fmt, args);
        size_t actual = std::min(n, result);
        array_copy(buffer, db.data(), actual);
        return actual;
    }
    size_t kvsprintf(char* restrict buffer, const char* restrict fmt, va_list args)
    {
        std::ext::dynamic_streambuf<char> db;
        size_t result = __kvfprintf_impl(std::addressof(db), fmt, args);
        array_copy(buffer, db.data(), result);
        return result;
    }
    size_t kvasprintf(char** restrict strp, const char* restrict fmt, va_list args)
    {
        std::ext::dynamic_streambuf<char> db;
        size_t result   = __kvfprintf_impl(std::addressof(db), fmt, args);
        *strp           = std::allocator<char>().allocate(result);
        array_copy(*strp, db.data(), result);
        return result;
    }
    int kfputc(int ch, FILE* fd)
    {
        std::ext::dynamic_streambuf<char> db;
        std::basic_streambuf<char>* stream = &db;
        char actual = static_cast<char>(ch);
        return stream->sputc(actual);
    }
    int kfputs(const char* restrict str, FILE* restrict fd)
    {
        std::ext::dynamic_streambuf<char> db;
        std::basic_streambuf<char>* stream = &db;
        std::streamsize result = stream->sputn(str, std::strlen(str));
        return result > 0 ? 0 : -1;
    }
    size_t kfwrite(const void* restrict buffer, size_t size, size_t count, FILE* restrict fd)
    {
        std::ext::dynamic_streambuf<char> db;
        std::basic_streambuf<char>* stream = &db;
        size_t total = size * count;
        return stream->sputn(static_cast<const char*>(buffer), total);
    }
    int fputc(int ch, FILE*) attribute(alias("kfputc"));
    int fputs(const char* restrict str, FILE* restrict fd) attribute(alias("kfputs"));
    size_t fwrite(const void* restrict buffer, size_t size, size_t count, FILE* restrict fd) attribute(alias("kfwrite"));
}