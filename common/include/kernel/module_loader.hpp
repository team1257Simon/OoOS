#ifndef __MOD_LOADER
#define __MOD_LOADER
#include "elf64_kernel_object.hpp"
#include "unordered_map"
class module_loader : std::unordered_map<std::string, elf64_kernel_object>
{
	typedef std::unordered_map<std::string, elf64_kernel_object> __base;
	static module_loader __instance;
	using typename __base::__node_type;
	module_loader();
public:
	using typename __base::iterator;
	using typename __base::const_iterator;
	using __base::begin;
	using __base::end;
	using __base::cbegin;
	using __base::cend;
	static module_loader& get_instance();
	std::pair<iterator, bool> add(file_node* file);
	std::pair<iterator, bool> add(addr_t start, size_t size);
	bool remove(std::string const& name);
	bool remove(ooos::abstract_module_base* mod);
};
#endif