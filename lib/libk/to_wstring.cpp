#include "string_impl.tcc"
namespace std
{
	wstring to_wstring(int value) { return __impl::__ntos_conv<int, wchar_t>::__to_string(value); }
	wstring to_wstring(long value) { return __impl::__ntos_conv<long, wchar_t>::__to_string(value); }
	wstring to_wstring(long long value) { return __impl::__ntos_conv<long long, wchar_t>::__to_string(value); }
	wstring to_wstring(unsigned int value) { return __impl::__ntos_conv<unsigned int, wchar_t>::__to_string(value); }
	wstring to_wstring(unsigned long value) { return __impl::__ntos_conv<unsigned long, wchar_t>::__to_string(value); }
	wstring to_wstring(unsigned long long value) { return __impl::__ntos_conv<unsigned long long, wchar_t>::__to_string(value); }
	wstring to_wstring(void* ptr) { return __impl::__ntos_conv<uintptr_t, wchar_t>::__to_hex_string(std::bit_cast<uintptr_t>(ptr)); }
	wstring to_wstring(float value) { return __impl::__cvt_digits<wchar_t>(to_string(value)); }
	wstring to_wstring(double value) { return __impl::__cvt_digits<wchar_t>(to_string(value)); }
	wstring to_wstring(long double value) { return __impl::__cvt_digits<wchar_t>(to_string(value)); }
	wstring to_wstring(bool value) { return value ? L"true" : L"false"; }
	wstring to_wstring(int value, ext::hex_t) { return __impl::__ntos_conv<int, wchar_t>::__to_hex_string(value); }
	wstring to_wstring(long value, ext::hex_t) { return __impl::__ntos_conv<long, wchar_t>::__to_hex_string(value); }
	wstring to_wstring(long long value, ext::hex_t) { return __impl::__ntos_conv<long long, wchar_t>::__to_hex_string(value); }
	wstring to_wstring(unsigned int value, ext::hex_t) { return __impl::__ntos_conv<unsigned int, wchar_t>::__to_hex_string(value); }
	wstring to_wstring(unsigned long value, ext::hex_t) { return __impl::__ntos_conv<unsigned long, wchar_t>::__to_hex_string(value); }
	wstring to_wstring(unsigned long long value, ext::hex_t) { return __impl::__ntos_conv<unsigned long long, wchar_t>::__to_hex_string(value); }
	wstring to_wstring(int value, ext::nphex_t) { return __impl::__ntos_conv<int, wchar_t>::__to_bare_hex_string(value); }
	wstring to_wstring(long value, ext::nphex_t) { return __impl::__ntos_conv<long, wchar_t>::__to_bare_hex_string(value); }
	wstring to_wstring(long long value, ext::nphex_t) { return __impl::__ntos_conv<long long, wchar_t>::__to_bare_hex_string(value); }
	wstring to_wstring(unsigned int value, ext::nphex_t) { return __impl::__ntos_conv<unsigned int, wchar_t>::__to_bare_hex_string(value); }
	wstring to_wstring(unsigned long value, ext::nphex_t) { return __impl::__ntos_conv<unsigned long, wchar_t>::__to_bare_hex_string(value); }
	wstring to_wstring(unsigned long long value, ext::nphex_t) { return __impl::__ntos_conv<unsigned long long, wchar_t>::__to_bare_hex_string(value); }
}