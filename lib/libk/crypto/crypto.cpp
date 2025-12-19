#include <ow-crypt.h>
#include <libk_decls.h>
#include <entropy_source.hpp>
std::string create_hash_setting_string(size_t n)
{
	// The null-terminator in the input buffer is not needed because of the input size argument to crypt_gensalt.
	// While that function takes an input of type const char*, it is more accurately const void* treated as const int8_t* and not a C-string.
	// With the default size of 16 bytes, ignoring the null-terminator allows us to avoid having to allocate heap space for the entropy buffer.
	// Notably, std::string(size_t s, char c) will allocate s + 1 as its size, but will use a local buffer if s < 16.
	std::string buf_str(n * sizeof(uint64_t) - 1Z, '\0');
	// __fill_entropy invokes REX.W RDSEED n times and stores the results in the input buffer.
	// While this would be undefined behavior in normal C++, because this is a "homemade" std::string implementation, it isn't.
	// The high-order 8 bits of the last value will overwrite the null terminator in buf_str, which is acceptable here as mentioned above.
	__fill_entropy(buf_str.data(), n);
	return std::forward<std::string>(crypt_gensalt("$2a$", 0, buf_str.c_str(), n * sizeof(uint64_t)));
}
std::string create_crypto_string(std::string const& key, std::string const& setting)
{
	char* result_str	= crypt(key.c_str(), setting.c_str());
	if(std::strlen(result_str) > setting.size()) return std::string(result_str + setting.size(), result_str + std::strlen(result_str));
	else return result_str;
}