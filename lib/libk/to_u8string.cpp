#include "string_impl.tcc"
namespace std
{
	u8string to_u8string(int value) { return __impl::__ntos_conv<int, char8_t>::__to_string(value); }
	u8string to_u8string(long value) { return __impl::__ntos_conv<long, char8_t>::__to_string(value); }
	u8string to_u8string(long long value) { return __impl::__ntos_conv<long long, char8_t>::__to_string(value); }
	u8string to_u8string(unsigned int value) { return __impl::__ntos_conv<unsigned int, char8_t>::__to_string(value); }
	u8string to_u8string(unsigned long value) { return __impl::__ntos_conv<unsigned long, char8_t>::__to_string(value); }
	u8string to_u8string(unsigned long long value) { return __impl::__ntos_conv<unsigned long long, char8_t>::__to_string(value); }
	u8string to_u8string(void* ptr) { return __impl::__ntos_conv<uintptr_t, char8_t>::__to_hex_string(std::bit_cast<uintptr_t>(ptr)); }
	u8string to_u8string(float value) { return __impl::__cvt_digits<char8_t>(to_string(value)); }
	u8string to_u8string(double value) { return __impl::__cvt_digits<char8_t>(to_string(value)); }
	u8string to_u8string(long double value) { return __impl::__cvt_digits<char8_t>(to_string(value)); }
	u8string to_u8string(bool value) { return value ? u8"true" : u8"false"; }
	u8string to_u8string(int value, ext::hex_t) { return __impl::__ntos_conv<int, char8_t>::__to_hex_string(value); }
	u8string to_u8string(long value, ext::hex_t) { return __impl::__ntos_conv<long, char8_t>::__to_hex_string(value); }
	u8string to_u8string(long long value, ext::hex_t) { return __impl::__ntos_conv<long long, char8_t>::__to_hex_string(value); }
	u8string to_u8string(unsigned int value, ext::hex_t) { return __impl::__ntos_conv<unsigned int, char8_t>::__to_hex_string(value); }
	u8string to_u8string(unsigned long value, ext::hex_t) { return __impl::__ntos_conv<unsigned long, char8_t>::__to_hex_string(value); }
	u8string to_u8string(unsigned long long value, ext::hex_t) { return __impl::__ntos_conv<unsigned long long, char8_t>::__to_hex_string(value); }
	u8string to_u8string(int value, ext::nphex_t) { return __impl::__ntos_conv<int, char8_t>::__to_bare_hex_string(value); }
	u8string to_u8string(long value, ext::nphex_t) { return __impl::__ntos_conv<long, char8_t>::__to_bare_hex_string(value); }
	u8string to_u8string(long long value, ext::nphex_t) { return __impl::__ntos_conv<long long, char8_t>::__to_bare_hex_string(value); }
	u8string to_u8string(unsigned int value, ext::nphex_t) { return __impl::__ntos_conv<unsigned int, char8_t>::__to_bare_hex_string(value); }
	u8string to_u8string(unsigned long value, ext::nphex_t) { return __impl::__ntos_conv<unsigned long, char8_t>::__to_bare_hex_string(value); }
	u8string to_u8string(unsigned long long value, ext::nphex_t) { return __impl::__ntos_conv<unsigned long long, char8_t>::__to_bare_hex_string(value); }
}