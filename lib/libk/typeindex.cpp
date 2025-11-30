#include "typeindex"
namespace std
{
	static elf64_gnu_hash __ti_hash{};
	type_index::type_index(type_info const& i) : info(addressof(i)) {}
	const char* type_index::name() const { return info->name(); }
	size_t type_index::hash_code() const { return __ti_hash(info->name()); }
}