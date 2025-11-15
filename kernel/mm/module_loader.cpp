#include "module_loader.hpp"
using namespace ooos;
typedef std::pair<std::unordered_map<std::string, elf64_kernel_object>::iterator, bool> result_pair;
constexpr static const char* name_for(abstract_module_base* mod) { return typeid(*mod).name(); }
template<typename ... Args> requires(std::constructible_from<elf64_kernel_object, Args...>) static result_pair add_obj(std::unordered_map<std::string, elf64_kernel_object>& map, Args&& ... args);
module_loader module_loader::__instance{};
module_loader::module_loader() : __base(64UZ) {}
module_loader& module_loader::get_instance() { return __instance; }
std::pair<module_loader::iterator, bool> module_loader::add(file_vnode* file) { return add_obj(*this, file); }
std::pair<module_loader::iterator, bool> module_loader::add(addr_t start, size_t size) { return add_obj(*this, start, size); }
bool module_loader::remove(std::string const& name) { return this->erase(name) != 0UZ; }
bool module_loader::remove(abstract_module_base* mod) { return this->erase(name_for(mod)) != 0UZ; }
template<typename ... Args> requires(std::constructible_from<elf64_kernel_object, Args...>)
static result_pair add_obj(std::unordered_map<std::string, elf64_kernel_object>& map, Args&& ... args)
{
	elf64_kernel_object obj(std::forward<Args>(args)...);
	abstract_module_base* mod	= obj.load_module();
	if(mod)
	{
		std::string key(name_for(mod));
		if(__unlikely(map.contains(key))) obj.unload_pre_init();
		else
		{
			result_pair result	= map.emplace(std::move(key), std::move(obj));
			if(__unlikely(!mod->initialize())) map.erase(result.first);
			else return result;
		}
	}
	return std::make_pair(map.end(), false);
}