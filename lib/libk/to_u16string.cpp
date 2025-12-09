#include "string_impl.tcc"
namespace std
{
	u16string to_u16string(int value) { return __impl::__ntos_conv<int, char16_t>::__to_string(value); }
	u16string to_u16string(long value) { return __impl::__ntos_conv<long, char16_t>::__to_string(value); }
	u16string to_u16string(long long value) { return __impl::__ntos_conv<long long, char16_t>::__to_string(value); }
	u16string to_u16string(unsigned int value) { return __impl::__ntos_conv<unsigned int, char16_t>::__to_string(value); }
	u16string to_u16string(unsigned long value) { return __impl::__ntos_conv<unsigned long, char16_t>::__to_string(value); }
	u16string to_u16string(unsigned long long value) { return __impl::__ntos_conv<unsigned long long, char16_t>::__to_string(value); }
	u16string to_u16string(void* ptr) { return __impl::__ntos_conv<uintptr_t, char16_t>::__to_hex_string(std::bit_cast<uintptr_t>(ptr)); }
	u16string to_u16string(float value) { return __impl::__cvt_digits<char16_t>(to_string(value)); }
	u16string to_u16string(double value) { return __impl::__cvt_digits<char16_t>(to_string(value)); }
	u16string to_u16string(long double value) { return __impl::__cvt_digits<char16_t>(to_string(value)); }
	u16string to_u16string(bool value) { return value ? u"true" : u"false"; }
	u16string to_u16string(int value, ext::hex_t) { return __impl::__ntos_conv<int, char16_t>::__to_hex_string(value); }
	u16string to_u16string(long value, ext::hex_t) { return __impl::__ntos_conv<long, char16_t>::__to_hex_string(value); }
	u16string to_u16string(long long value, ext::hex_t) { return __impl::__ntos_conv<long long, char16_t>::__to_hex_string(value); }
	u16string to_u16string(unsigned int value, ext::hex_t) { return __impl::__ntos_conv<unsigned int, char16_t>::__to_hex_string(value); }
	u16string to_u16string(unsigned long value, ext::hex_t) { return __impl::__ntos_conv<unsigned long, char16_t>::__to_hex_string(value); }
	u16string to_u16string(unsigned long long value, ext::hex_t) { return __impl::__ntos_conv<unsigned long long, char16_t>::__to_hex_string(value); }
	u16string to_u16string(int value, ext::nphex_t) { return __impl::__ntos_conv<int, char16_t>::__to_bare_hex_string(value); }
	u16string to_u16string(long value, ext::nphex_t) { return __impl::__ntos_conv<long, char16_t>::__to_bare_hex_string(value); }
	u16string to_u16string(long long value, ext::nphex_t) { return __impl::__ntos_conv<long long, char16_t>::__to_bare_hex_string(value); }
	u16string to_u16string(unsigned int value, ext::nphex_t) { return __impl::__ntos_conv<unsigned int, char16_t>::__to_bare_hex_string(value); }
	u16string to_u16string(unsigned long value, ext::nphex_t) { return __impl::__ntos_conv<unsigned long, char16_t>::__to_bare_hex_string(value); }
	u16string to_u16string(unsigned long long value, ext::nphex_t) { return __impl::__ntos_conv<unsigned long long, char16_t>::__to_bare_hex_string(value); }
}