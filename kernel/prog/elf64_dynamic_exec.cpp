#include "elf64_dynamic_exec.hpp"
#include "stdlib.h"
constexpr std::allocator<const char*> strptr_alloc{};
elf64_dynamic_executable::~elf64_dynamic_executable() { if(program_descriptor.ld_path) strptr_alloc.deallocate(program_descriptor.ld_path, program_descriptor.ld_path_count); }
addr_t elf64_dynamic_executable::segment_vaddr(size_t n) const { return virtual_load_base.plus(phdr(n).p_vaddr); }
addr_t elf64_dynamic_executable::resolve(uintptr_t offs) const { return virtual_load_base ? virtual_load_base.plus(offs) : addr_t(offs); }
addr_t elf64_dynamic_executable::resolve(elf64_sym const& sym) const { return virtual_load_base ? virtual_load_base.plus(sym.st_value) : addr_t(sym.st_value); }
bool elf64_dynamic_executable::xload() { return elf64_dynamic_object::xload(); }
bool elf64_dynamic_executable::load_syms() { return elf64_dynamic_object::load_syms(); }
elf64_dynamic_executable::elf64_dynamic_executable(addr_t start, size_t size, size_t stack_sz, uintptr_t base_offset) :
	elf64_object(start, size),
	elf64_executable(start, size, stack_size),
	elf64_dynamic_object(start, size),
	virtual_load_base(base_offset)
{}
elf64_dynamic_executable::elf64_dynamic_executable(file_vnode* n, size_t stack_sz, uintptr_t base_offset) :
	elf64_object(n),
	elf64_executable(n, stack_sz),
	elf64_dynamic_object(n),
	virtual_load_base(base_offset)
{}
elf64_dynamic_executable::elf64_dynamic_executable(elf64_dynamic_executable const& that) :
	elf64_object(that),
	elf64_executable(that),
	elf64_dynamic_object(that),
	virtual_load_base(that.virtual_load_base),
	preinit_array(that.preinit_array),
	preinit_array_ptr(that.preinit_array_ptr),
	preinit_array_size(that.preinit_array_size)
{}
void elf64_dynamic_executable::process_headers()
{
	elf64_executable::process_headers();
	if(virtual_load_base)
	{
		off_t diff = static_cast<off_t>(virtual_load_base.full);
		if(diff > 0Z)
		{
			frame_base			+= diff;
			frame_extent		+= diff;
			stack_base			+= diff;
			entry				+= diff;
			if(tls_base)
				tls_base		+= diff;
		}
	}
}
bool elf64_dynamic_executable::load_segments()
{
	if(elf64_executable::load_segments())
	{
		program_descriptor.ld_path_count	= ld_paths.size();
		program_descriptor.ld_path			= strptr_alloc.allocate(program_descriptor.ld_path_count);
		size_t i							= 0;
		for(std::string const& str : ld_paths) { program_descriptor.ld_path[i++] = str.c_str(); }
		return true;
	}
	else return false;
}
void elf64_dynamic_executable::process_dyn_entry(size_t i)
{
	if(dyn_entries[i].d_tag == DT_PREINIT_ARRAY)		preinit_array_ptr	= dyn_entries[i].d_ptr;
	else if(dyn_entries[i].d_tag == DT_PREINIT_ARRAYSZ) preinit_array_size	= (dyn_entries[i].d_val / sizeof(addr_t));
	else elf64_dynamic_object::process_dyn_entry(i);
}
bool elf64_dynamic_executable::load_preinit()
{
	if(preinit_array_ptr && preinit_array_size)
	{
		addr_t preinit_ptrs_vaddr	= resolve(preinit_array_ptr);
		uintptr_t* preinit_ptrs		= translate_in_frame(preinit_ptrs_vaddr);
		if(__unlikely(!preinit_ptrs)) {
			panic("[PRG/DYN-EXEC] pre-initialization array pointer is non-null but is invalid");
			return false;
		}
		for(size_t i = 0; i < preinit_array_size; i++) preinit_array.push_back(addr_t(preinit_ptrs[i]));
	}
	return true;
}
bool elf64_dynamic_executable::process_got()
{
	if(__unlikely(!elf64_dynamic_object::process_got())) return false;
	else if(!virtual_load_base) return true;
	for(size_t i = 0UZ; i < num_plt_relas; i++)
	{
		elf64_rela const& r	= plt_relas[i];
		addr_t target		= frame_tag->translate(virtual_load_base.plus(r.r_offset));
		if(__unlikely(!target)) { panic("[PRG/DYNEXEC] fault in PLT rela offset"); return false; }
		addr_t adjusted		= virtual_load_base.plus(target.deref<uintptr_t>());
		target.assign(adjusted);
	}
	return true;
}