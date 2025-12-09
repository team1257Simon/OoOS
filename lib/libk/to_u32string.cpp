#include "string_impl.tcc"
namespace std
{
	u32string to_u32string(int value) { return __impl::__ntos_conv<int, char32_t>::__to_string(value); }
	u32string to_u32string(long value) { return __impl::__ntos_conv<long, char32_t>::__to_string(value); }
	u32string to_u32string(long long value) { return __impl::__ntos_conv<long long, char32_t>::__to_string(value); }
	u32string to_u32string(unsigned int value) { return __impl::__ntos_conv<unsigned int, char32_t>::__to_string(value); }
	u32string to_u32string(unsigned long value) { return __impl::__ntos_conv<unsigned long, char32_t>::__to_string(value); }
	u32string to_u32string(unsigned long long value) { return __impl::__ntos_conv<unsigned long long, char32_t>::__to_string(value); }
	u32string to_u32string(void* ptr) { return __impl::__ntos_conv<uintptr_t, char32_t>::__to_hex_string(std::bit_cast<uintptr_t>(ptr)); }
	u32string to_u32string(float value) { return __impl::__cvt_digits<char32_t>(to_string(value)); }
	u32string to_u32string(double value) { return __impl::__cvt_digits<char32_t>(to_string(value)); }
	u32string to_u32string(long double value) { return __impl::__cvt_digits<char32_t>(to_string(value)); }
	u32string to_u32string(bool value) { return value ? U"true" : U"false"; }
	u32string to_u32string(int value, ext::hex_t) { return __impl::__ntos_conv<int, char32_t>::__to_hex_string(value); }
	u32string to_u32string(long value, ext::hex_t) { return __impl::__ntos_conv<long, char32_t>::__to_hex_string(value); }
	u32string to_u32string(long long value, ext::hex_t) { return __impl::__ntos_conv<long long, char32_t>::__to_hex_string(value); }
	u32string to_u32string(unsigned int value, ext::hex_t) { return __impl::__ntos_conv<unsigned int, char32_t>::__to_hex_string(value); }
	u32string to_u32string(unsigned long value, ext::hex_t) { return __impl::__ntos_conv<unsigned long, char32_t>::__to_hex_string(value); }
	u32string to_u32string(unsigned long long value, ext::hex_t) { return __impl::__ntos_conv<unsigned long long, char32_t>::__to_hex_string(value); }
	u32string to_u32string(int value, ext::nphex_t) { return __impl::__ntos_conv<int, char32_t>::__to_bare_hex_string(value); }
	u32string to_u32string(long value, ext::nphex_t) { return __impl::__ntos_conv<long, char32_t>::__to_bare_hex_string(value); }
	u32string to_u32string(long long value, ext::nphex_t) { return __impl::__ntos_conv<long long, char32_t>::__to_bare_hex_string(value); }
	u32string to_u32string(unsigned int value, ext::nphex_t) { return __impl::__ntos_conv<unsigned int, char32_t>::__to_bare_hex_string(value); }
	u32string to_u32string(unsigned long value, ext::nphex_t) { return __impl::__ntos_conv<unsigned long, char32_t>::__to_bare_hex_string(value); }
	u32string to_u32string(unsigned long long value, ext::nphex_t) { return __impl::__ntos_conv<unsigned long long, char32_t>::__to_bare_hex_string(value); }
}