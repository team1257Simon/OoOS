#include "ow-crypt.h"
#include "kernel/libk_decls.h"
#include "kernel/entropy_source.hpp"
constexpr static std::allocator<char> ch_alloc{};
std::string create_hash_setting_string(size_t n)
{
	char* buffer = ch_alloc.allocate(n * sizeof(uint64_t));
	__fill_entropy(buffer, n);
	char* result = crypt_gensalt("$2a$", 0, buffer, n * sizeof(uint64_t));
	ch_alloc.deallocate(buffer, n * sizeof(uint64_t));
	return result;
}
std::string create_crypto_string(std::string const& key, std::string const& setting)
{
	char* result_str = crypt(key.c_str(), setting.c_str());
	if(std::strlen(result_str) > setting.size()) return std::string(result_str + setting.size(), result_str + std::strlen(result_str));
	else return result_str;
}