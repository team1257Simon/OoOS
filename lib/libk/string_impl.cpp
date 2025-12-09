/**
 * Most C++ standard library implementations use C standard library functions for their string implementations.
 * Because I like to watch the world burn, I have done the opposite and implemented C standard library string code in terms of my own C++ standard library string implementations.
 * C standard library functions are trivial in terms of the template functions in basic_string.hpp, so we can put them here for compatibility.
 */
#include "string_impl.tcc"
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
	const char* __assert_fail_text(const char* text, const char* fname, const char* filename, int line)
	{
		static std::string estr;
		estr				= std::move(std::string("Assertion failed in function: ", 31UL));
		estr.append(fname ? fname : "");
		estr.append(", file ");
		estr.append(filename ? filename : "");
		std::string linestr	= std::to_string(line);
		estr.append(", line ");
		estr.append(linestr);
		estr.append(": ");
		if(text) estr.append(text);
		return estr.c_str();
	}
	int isalnum(int c)														{ return std::__impl::__isalnum(static_cast<char>(c)) ? 1 : 0; }
	int isalpha(int c)														{ return std::__impl::__isalpha(static_cast<char>(c)) ? 1 : 0; }
	int iscntrl(int c)														{ return std::__impl::__iscntrl(static_cast<char>(c)) ? 1 : 0; }
	int isdigit(int c)														{ return std::__impl::__isdigit(static_cast<char>(c)) ? 1 : 0; }
	int isgraph(int c)														{ return std::__impl::__isgraph(static_cast<char>(c)) ? 1 : 0; }
	int islower(int c)														{ return std::__impl::__islower(static_cast<char>(c)) ? 1 : 0; }
	int isprint(int c)														{ return std::__impl::__isprint(static_cast<char>(c)) ? 1 : 0; }
	int ispunct(int c)														{ return std::__impl::__ispunct(static_cast<char>(c)) ? 1 : 0; }
	int isspace(int c)														{ return std::__impl::__isspace(static_cast<char>(c)) ? 1 : 0; }
	int isupper(int c)														{ return std::__impl::__isupper(static_cast<char>(c)) ? 1 : 0; }
	int isxdigit(int c)														{ return std::__impl::__isxdigit(static_cast<char>(c)) ? 1 : 0; }
	int tolower(int c)														{ return static_cast<int>(std::__impl::__tolower(static_cast<char>(c))); }
	int toupper(int c)														{ return static_cast<int>(std::__impl::__toupper(static_cast<char>(c))); }
	int atoi(const char* str)												{ char* tmp = nullptr; return std::__impl::ston<int>(str, tmp); }
	double atof(const char* str)											{ char* tmp = nullptr; return strtod(str, std::addressof(tmp)); }
	long atol(const char* str)												{ char* tmp = nullptr; return std::__impl::ston<long>(str, tmp); }
	long strtol(const char* str, char** endptr, int base)					{ return std::__impl::ston<long>(str, *endptr, base); }
	long long strtoll(const char* str, char** endptr, int base)				{ return std::__impl::ston<long long>(str, *endptr, base); }
	unsigned long strtoul(const char* str, char** endptr, int base)			{ return std::__impl::ston<unsigned long>(str, *endptr, base); }
	unsigned long long strtoull(const char* str, char** endptr, int base)	{ return std::__impl::ston<unsigned long long>(str, *endptr, base); }
}