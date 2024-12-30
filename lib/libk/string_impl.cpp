#include "kernel/libk_decls.h"
#include "string"
#include "limits"
// C standard library functions are trivial in terms of the template functions in basic_string.hpp, so we can put them here for compatibility
extern "C"
{
    char* strstr(const char *hs, const char *ne) { return const_cast<char*>(std::__impl::__find_impl(hs, ne)); }
    char* strnchr(const char* str, size_t n, char what) { return std::find<char>(str, n, what); }
    char* strchr(const char* str, char what) { return std::find<char>(str, std::strlen(str), what); }
    int strncmp(const char* lhs, const char* rhs, size_t n) { return std::strncmp<char>(lhs, rhs, n); }
    int strcmp(const char* lhs, const char* rhs) { return std::strcmp<char>(lhs, rhs); }
    size_t strlen(const char* str) { return std::strlen<char>(str); }
    size_t strnlen(const char* str, size_t n) { return std::strnlen<char>(str, n); }
    char* strcpy(char* dest, const char* src) { return std::strcpy<char>(dest, src); }
    char* strncpy(char* dest, const char* src, size_t n) { return std::strncpy<char>(dest, src, n); }
    char* stpcpy(char* dest, const char* src) { return std::stpcpy<char>(dest, src); }
    char* stpncpy(char* dest, const char* src, size_t n) { return std::stpncpy<char>(dest, src, n); }
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
    char* __assert_fail_text(const char* text, const char* fname, const char* filename, int line)
    {
        static char* __errstr;
        if(__errstr) 
        {
            delete[] __errstr;
            __errstr = NULL;
        }
        std::string estr{"Assertion failed in function "};
        estr.append(fname ? fname : "");
        estr.append(", file ");
        estr.append(filename ? filename : "");
        std::string linestr = std::to_string(line);
        estr.append(", line ");
        estr.append(linestr);
        estr.append(": ");
        if(text) estr.append(text);
        __errstr = new char[estr.size() + 1];
        std::strncpy(__errstr, estr.c_str(), estr.size());
        __errstr[estr.size()] = '\0';
        return __errstr;
    }
}
namespace std
{
    namespace __impl
    {
        template<std::char_type CT> struct __ch_digits { constexpr static CT minus = 0; constexpr static const CT digits[] = {}; constexpr static const CT hexpref[] = {};};
        template<> struct __ch_digits<char> { constexpr static char minus = '-'; constexpr static const char digits[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' }; constexpr static const char hexpref[] = "x0"; };
        template<> struct __ch_digits<wchar_t> { constexpr static wchar_t minus = L'-';  constexpr static const wchar_t digits[] = { L'0', L'1', L'2', L'3', L'4', L'5', L'6', L'7', L'8', L'9', L'A', L'B', L'C', L'D', L'E', L'F' }; constexpr static const wchar_t hexpref[] = L"x0"; };
        template<> struct __ch_digits<char8_t> { constexpr static char8_t minus = u8'-'; constexpr static const char8_t digits[] = { u8'0', u8'1', u8'2', u8'3', u8'4', u8'5', u8'6', u8'7', u8'8', u8'9', u8'A', u8'B', u8'C', u8'D', u8'E', u8'F' }; constexpr static const char8_t hexpref[] = u8"x0"; };
        template<> struct __ch_digits<char16_t> { constexpr static char16_t minus = u'-'; constexpr static const char16_t digits[] = { u'0', u'1', u'2', u'3', u'4', u'5', u'6', u'7', u'8', u'9', u'A', u'B', u'C', u'D', u'E', u'F' }; constexpr static const char16_t hexpref[] = u"x0"; };
        template<> struct __ch_digits<char32_t>  { constexpr static char32_t minus = U'-';  constexpr static const char32_t digits[] = { U'0', U'1', U'2', U'3', U'4', U'5', U'6', U'7', U'8', U'9', U'A', U'B', U'C', U'D', U'E', U'F' }; constexpr static const char32_t hexpref[] = U"x0"; };
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
            using __digi_type = __ch_digits<CT>;
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
                if(!i) return basic_string{__digi_type::digits[0]};
                std::basic_string<CT> str{};
                str.reserve(__max_dec);
                IT j;
                if constexpr(std::is_signed_v<IT>) j = (i < 0) ? -i : i;
                else j = i;
                for(size_t n = 0; n < __max_dec && __get_pow10(n) <= j; n++) str.append(__get_dec_digit(j, n));
                if constexpr(std::is_signed_v<IT>) if(i < 0) str.append(__digi_type::minus);
                return std::basic_string<CT>{ str.rend(), str.rbegin() };
            }
            constexpr static std::basic_string<CT> __to_hex_string(IT i)
            {
                if(!i) return basic_string{__digi_type::digits[0]};
                std::basic_string<CT> hstr{};
                hstr.reserve(__max_hex);
                IT j;
                if constexpr(std::is_signed_v<IT>) j = (i < 0) ? -i : i;
                else j = i;
                for(size_t n = 0; n < __max_hex && __get_pow16(n) <= j; n++) hstr.append(__get_hex_digit(j, n));
                hstr.append(__digi_type::hexpref);
                if constexpr(std::is_signed_v<IT>) if(i < 0) hstr.append(__digi_type::minus);
                return std::basic_string<CT>{ hstr.rend(), hstr.rbegin() };
            }
        };
        template<typename T, typename CT> std::basic_string<CT> __to_string(T);
        template<std::integral IT, std::char_type CT> [[gnu::always_inline]] constexpr std::basic_string<CT> __to_string(IT it) { return __ntos_conv<IT, CT>::__to_string(it); }
        template<std::char_type CT> [[gnu::always_inline]] constexpr std::basic_string<CT> __to_string(void* ptr) { return __ntos_conv<uintptr_t, CT>::__to_hex_string(std::bit_cast<uintptr_t>(ptr)); }
    }
    string to_string(int value) { return __impl::__to_string<int, char>(value); }
    string to_string(long value) { return __impl::__to_string<long, char>(value); }
    string to_string(long long value) { return __impl::__to_string<long long, char>(value); }
    string to_string(unsigned int value) { return __impl::__to_string<unsigned int, char>(value); }
    string to_string(unsigned long value) { return __impl::__to_string<unsigned long, char>(value); }
    string to_string(unsigned long long value) { return __impl::__to_string<unsigned long long, char>(value); }
    string to_string(void* ptr) { return __impl::__to_string<char>(ptr); }
    wstring to_wstring(int value) { return __impl::__to_string<int, wchar_t>(value); }
	wstring to_wstring(long value) { return __impl::__to_string<long, wchar_t>(value); }
	wstring to_wstring(long long value) { return __impl::__to_string<long long, wchar_t>(value); }
	wstring to_wstring(unsigned int value) { return __impl::__to_string<unsigned int, wchar_t>(value); }
	wstring to_wstring(unsigned long value) { return __impl::__to_string<unsigned long, wchar_t>(value); }
	wstring to_wstring(unsigned long long value) { return __impl::__to_string<unsigned long long, wchar_t>(value); }
	wstring to_wstring(void* ptr) { return __impl::__to_string<wchar_t>(ptr); }
    u8string to_u8string(int value) { return __impl::__to_string<int, char8_t>(value); }
	u8string to_u8string(long value) { return __impl::__to_string<long, char8_t>(value); }
	u8string to_u8string(long long value) { return __impl::__to_string<long long, char8_t>(value); }
	u8string to_u8string(unsigned int value) { return __impl::__to_string<unsigned int, char8_t>(value); }
	u8string to_u8string(unsigned long value) { return __impl::__to_string<unsigned long, char8_t>(value); }
	u8string to_u8string(unsigned long long value) { return __impl::__to_string<unsigned long long, char8_t>(value); }
	u8string to_u8string(void* ptr) { return __impl::__to_string<char8_t>(ptr); }
    u16string to_u16string(int value) { return __impl::__to_string<int, char16_t>(value); }
	u16string to_u16string(long value) { return __impl::__to_string<long, char16_t>(value); }
	u16string to_u16string(long long value) { return __impl::__to_string<long long, char16_t>(value); }
	u16string to_u16string(unsigned int value) { return __impl::__to_string<unsigned int, char16_t>(value); }
	u16string to_u16string(unsigned long value) { return __impl::__to_string<unsigned long, char16_t>(value); }
	u16string to_u16string(unsigned long long value) { return __impl::__to_string<unsigned long long, char16_t>(value); }
	u16string to_u16string(void* ptr) { return __impl::__to_string<char16_t>(ptr); }
    u32string to_u32string(int value) { return __impl::__to_string<int, char32_t>(value); }
	u32string to_u32string(long value) { return __impl::__to_string<long, char32_t>(value); }
	u32string to_u32string(long long value) { return __impl::__to_string<long long, char32_t>(value); }
	u32string to_u32string(unsigned int value) { return __impl::__to_string<unsigned int, char32_t>(value); }
	u32string to_u32string(unsigned long value) { return __impl::__to_string<unsigned long, char32_t>(value); }
	u32string to_u32string(unsigned long long value) { return __impl::__to_string<unsigned long long, char32_t>(value); }
	u32string to_u32string(void* ptr) { return __impl::__to_string<char32_t>(ptr); }
}