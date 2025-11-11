#include "prog_manager.hpp"
#include "stdlib.h" // rand(), srand()
static std::alignas_allocator<char, elf64_ehdr> elf_alloc{};
prog_manager prog_manager::__instance{};
prog_manager::prog_manager() : __static_base(), __dynamic_base() {}
prog_manager& prog_manager::get_instance() { return __instance; }
constexpr static elf64_phdr const& phdr(elf64_ehdr const& ehdr, size_t i) {
	size_t ph_off = ehdr.e_phoff + i * ehdr.e_phentsize;
	return addr_t(std::addressof(ehdr)).plus(ph_off).deref<elf64_phdr>();
}
static size_t find_dyn(elf64_ehdr const& ehdr)
{ 
	for(size_t i = 0; i < ehdr.e_phnum; i++) 
		if(phdr(ehdr, i).p_type == PT_DYNAMIC) 
			return i;
	return 0UZ;
}
static bool is_pic_exec(elf64_dyn* dyn_entries, size_t n)
{
	for(size_t i = 0; i < n; i++)
		if(dyn_entries[i].d_tag == DT_FLAGS_1 && (dyn_entries[i].d_val & DF_1_PIE))
			return true;
	return false;
}
static uintptr_t get_load_base(addr_t img_start, size_t dyn_idx)
{
	elf64_ehdr const& hdr		= img_start.deref<elf64_ehdr const>();
	elf64_phdr const& dyn_phdr	= phdr(hdr, dyn_idx);
	if(is_pic_exec(img_start.plus(dyn_phdr.p_offset), dyn_phdr.p_filesz / sizeof(elf64_dyn)))
	{
		srand(static_cast<unsigned int>(sys_time(nullptr)));
		uintptr_t generated = rand() & 0x000FFFFF;
		return generated << 12;
	}
	return 0UL;
}
elf64_executable* prog_manager::__add(addr_t img_start, size_t img_size, size_t stack_sz, size_t tls_sz)
{
	if(__builtin_memcmp(img_start, "\177ELF", 4) != 0) { panic("[PRG] missing identifier; invalid object"); return nullptr; }
	if(size_t dyn = find_dyn(img_start.deref<elf64_ehdr>()))
	{
		__dynamic_base::iterator result	= __dynamic_base::emplace_back(img_start, img_size, stack_sz, tls_sz, get_load_base(img_start, dyn));
		if(__unlikely(!result->load())) { __dynamic_base::erase(result); return nullptr; }
		return result.base();
	}
	__static_base::iterator result = __static_base::emplace_back(img_start, img_size, stack_sz, tls_sz);
	if(__unlikely(!result->load())) { __static_base::erase(result); return nullptr; }
	return result.base();
}
elf64_executable* prog_manager::add(file_vnode* exec_file, size_t stack_sz, size_t tls_sz)
{
	size_t size		= static_cast<size_t>(exec_file->size() - exec_file->tell());
	addr_t start	= elf_alloc.allocate(size);
	if(__unlikely(!exec_file->read(start, size)))
	{ 
		elf_alloc.deallocate(start, size); 
		panic("[PRG] read failed"); 
		return nullptr;
	}
	elf64_executable* result = __add(start, size, stack_sz, tls_sz);
	if(__unlikely(!result))
	{
		elf_alloc.deallocate(start, size);
		panic("[PRG] load failed");
		return nullptr;
	}
	return result;
}
void prog_manager::remove(elf64_executable* e)
{
	if(elf64_dynamic_executable* dyn = dynamic_cast<elf64_dynamic_executable*>(e))
		__dynamic_base::erase(__dynamic_base::iterator(addr_t(dyn).minus(__dynamic_node_offset)));
	else
		__static_base::erase(__static_base::iterator(addr_t(e).minus(__static_node_offset)));
}
elf64_executable* prog_manager::clone(elf64_executable const* exec)
{
	try
	{
		if(elf64_dynamic_executable const* dyn = dynamic_cast<elf64_dynamic_executable const*>(exec))
			return __dynamic_base::emplace_back(*dyn).base();
		return __static_base::emplace_back(*exec).base();
	}
	catch(std::exception& e) { panic(e.what()); }
	return nullptr;
}