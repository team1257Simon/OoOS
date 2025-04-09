#include "elf64_dynamic_exec.hpp"
#include "stdlib.h"
std::allocator<const char*> strptr_alloc{};
elf64_dynamic_executable::~elf64_dynamic_executable() { if(program_descriptor.ld_path) strptr_alloc.deallocate(program_descriptor.ld_path, program_descriptor.ld_path_count); }
addr_t elf64_dynamic_executable::segment_vaddr(size_t n) const { return virtual_load_base.plus(phdr(n).p_vaddr); }
addr_t elf64_dynamic_executable::resolve(uint64_t offs) const { return virtual_load_base.plus(offs); }
addr_t elf64_dynamic_executable::resolve(elf64_sym const& sym) const { return virtual_load_base.plus(sym.st_value); }
elf64_dynamic_executable::elf64_dynamic_executable(file_node* n, uintptr_t base_offset, size_t stack_sz, size_t tls_sz) :
    elf64_object            ( n ),
    elf64_executable        ( n, stack_sz, tls_sz ),
    elf64_dynamic_object    ( n ),
    virtual_load_base       ( base_offset ),
    preinit_array           {},
    preinit_array_ptr       { 0UL },
    preinit_array_size      { 0UL }
                            {}
void elf64_dynamic_executable::process_headers()
{
    elf64_executable::process_headers();
    off_t diff = static_cast<off_t>(virtual_load_base.full);
    frame_base += diff;
    frame_extent += diff;
    stack_base += diff;
    tls_base += diff;
    entry += diff;
}
bool elf64_dynamic_executable::load_segments()
{
    if(elf64_executable::load_segments()) 
    { 
        program_descriptor.ld_path_count = ld_paths.size();
        program_descriptor.ld_path = strptr_alloc.allocate(program_descriptor.ld_path_count);
        size_t i = 0;
        for(std::string const& str : ld_paths) { program_descriptor.ld_path[i++] = str.c_str(); }
        return true;
    }
    else return false;
}
void elf64_dynamic_executable::process_dyn_entry(size_t i)
{
    if(dyn_entries[i].d_tag == DT_PREINIT_ARRAY) preinit_array_ptr = dyn_entries[i].d_ptr;
    else if(dyn_entries[i].d_tag == DT_PREINIT_ARRAYSZ) preinit_array_size = dyn_entries[i].d_val;
    else elf64_dynamic_object::process_dyn_entry(i);
}
bool elf64_dynamic_executable::load_preinit()
{
    if(preinit_array_ptr && preinit_array_ptr)
    {
        addr_t preinit_ptrs_vaddr = resolve(preinit_array_ptr);
        frame_enter();
        uintptr_t* preinit_ptrs = reinterpret_cast<uintptr_t*>(kmm.translate_vaddr_in_current_frame(preinit_ptrs_vaddr));
        kmm.exit_frame();
        if(!preinit_ptrs) { panic("pre-initialization array pointer is non-null but is invalid"); return false; }
        for(size_t i = 0; i < preinit_array_size; i++) { preinit_array.push_back(addr_t(preinit_ptrs[i])); }
    }
    preinit_array.push_back(nullptr);
    return true;
}