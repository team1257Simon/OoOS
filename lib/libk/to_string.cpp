#include "string_impl.tcc"
namespace std
{
	string to_string(int value) { return __impl::__ntos_conv<int, char>::__to_string(value); }
	string to_string(long value) { return __impl::__ntos_conv<long, char>::__to_string(value); }
	string to_string(long long value) { return __impl::__ntos_conv<long long, char>::__to_string(value); }
	string to_string(unsigned int value) { return __impl::__ntos_conv<unsigned int, char>::__to_string(value); }
	string to_string(unsigned long value) { return __impl::__ntos_conv<unsigned long, char>::__to_string(value); }
	string to_string(unsigned long long value) { return __impl::__ntos_conv<unsigned long long, char>::__to_string(value); }
	string to_string(void* ptr) { return __impl::__ntos_conv<uintptr_t, char>::__to_hex_string(std::bit_cast<uintptr_t>(ptr)); }
	string to_string(float value) { return __impl::__fptocs_conv(value, 6); }
	string to_string(double value) { return __impl::__fptocs_conv(value, 10); }
	string to_string(long double value) { return __impl::__fptocs_conv(value, 256); }
	string to_string(bool value) { return value ? "true" : "false"; }
	string to_string(int value, ext::hex_t) { return __impl::__ntos_conv<int, char>::__to_hex_string(value); }
	string to_string(long value, ext::hex_t) { return __impl::__ntos_conv<long, char>::__to_hex_string(value); }
	string to_string(long long value, ext::hex_t) { return __impl::__ntos_conv<long long, char>::__to_hex_string(value); }
	string to_string(unsigned int value, ext::hex_t) { return __impl::__ntos_conv<unsigned int, char>::__to_hex_string(value); }
	string to_string(unsigned long value, ext::hex_t) { return __impl::__ntos_conv<unsigned long, char>::__to_hex_string(value); }
	string to_string(unsigned long long value, ext::hex_t) { return __impl::__ntos_conv<unsigned long long, char>::__to_hex_string(value); }
	string to_string(int value, ext::nphex_t) { return __impl::__ntos_conv<int, char>::__to_bare_hex_string(value); }
	string to_string(long value, ext::nphex_t) { return __impl::__ntos_conv<long, char>::__to_bare_hex_string(value); }
	string to_string(long long value, ext::nphex_t) { return __impl::__ntos_conv<long long, char>::__to_bare_hex_string(value); }
	string to_string(unsigned int value, ext::nphex_t) { return __impl::__ntos_conv<unsigned int, char>::__to_bare_hex_string(value); }
	string to_string(unsigned long value, ext::nphex_t) { return __impl::__ntos_conv<unsigned long, char>::__to_bare_hex_string(value); }
	string to_string(unsigned long long value, ext::nphex_t) { return __impl::__ntos_conv<unsigned long long, char>::__to_bare_hex_string(value); }
	namespace ext
	{
		std::string fcvt(float f, int ndigits) { return std::__impl::__fptocs_conv(f, ndigits); }
		std::string fcvtd(double d, int ndigits) { return std::__impl::__fptocs_conv(d, ndigits); }
		std::string fcvtl(long double ld, int ndigits) { return std::__impl::__fptocs_conv(ld, ndigits); }
		std::string fcvth(float f, int ndigits) { return std::__impl::__fptohs_conv(f, ndigits); }
		std::string fcvthd(double d, int ndigits) { return std::__impl::__fptohs_conv(d, ndigits); }
		std::string fcvthl(long double ld, int ndigits) { return std::__impl::__fptohs_conv(ld, ndigits); }
		std::string to_upper(std::string const& str) { std::string result(str.size(), str.get_allocator()); for(size_t i = 0; i < str.size(); i++) { result.append(__impl::__toupper(str[i])); } return result; }
		std::string to_lower(std::string const& str) { std::string result(str.size(), str.get_allocator()); for(size_t i = 0; i < str.size(); i++) { result.append(__impl::__tolower(str[i])); } return result; }
	}
}
extern "C"
{
	size_t __strcat_basic(char* buffer, size_t max, const char* str1, const char* str2)
	{
		std::string cat(str1);
		cat	+= str2;
		std::strncpy(buffer, cat.c_str(), max);
		return std::min(cat.size(), max);
	}
	size_t __tricat(char* buffer, size_t max, const char* str1, const char* str2, const char* str3)
	{
		std::string cat(str1);
		cat	+= str2;
		cat	+= str3;
		std::strncpy(buffer, cat.c_str(), max);
		return std::min(cat.size(), max);
	}
	int __ltoi_alloc(char** out, long n)
	{
		std::string s		= std::to_string(n);
		char* result		= static_cast<char*>(malloc(s.size() + 1Z));
		if(__unlikely(!out || !result)) return -1;
		*out				= result;
		std::strncpy(result, s.c_str(), s.size());
		result[s.size()]	= 0;
		return static_cast<int>(s.size());
	}
	int __cfcvt(char* buffer, size_t len, float f)
	{
		std::string s		= std::ext::fcvt(f, len - 2); // one for the decimal point or exponent "e", one for the sign
		std::strncpy(buffer, s.c_str(), s.size());
		return static_cast<int>(s.size());
	}
	int __cdcvt(char* buffer, size_t len, double d)
	{
		std::string s		= std::ext::fcvtd(d, len - 2);
		std::strncpy(buffer, s.c_str(), s.size());
		return static_cast<int>(s.size());
	}
	int __cldcvt(char* buffer, size_t len, long double ld)
	{
		std::string s		= std::ext::fcvtl(ld, len - 2);
		std::strncpy(buffer, s.c_str(), s.size());
		return static_cast<int>(s.size());
	}
}