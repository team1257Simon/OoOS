#include "elf64_dynamic_exec.hpp"
#include "stdlib.h"
std::allocator<const char*> strptr_alloc{};
elf64_dynamic_executable::~elf64_dynamic_executable() { if(program_descriptor.ld_path) strptr_alloc.deallocate(program_descriptor.ld_path, program_descriptor.ld_path_count); }
addr_t elf64_dynamic_executable::segment_vaddr(size_t n) const { return virtual_load_base.plus(phdr(n).p_vaddr); }
addr_t elf64_dynamic_executable::resolve(uintptr_t offs) const { return virtual_load_base ? virtual_load_base.plus(offs) : addr_t(offs); }
addr_t elf64_dynamic_executable::resolve(elf64_sym const& sym) const { return virtual_load_base ? virtual_load_base.plus(sym.st_value) : addr_t(sym.st_value); }
bool elf64_dynamic_executable::xload() { return elf64_dynamic_object::xload(); }
bool elf64_dynamic_executable::load_syms() { return elf64_dynamic_object::load_syms(); }
elf64_dynamic_executable::elf64_dynamic_executable(addr_t start, size_t size, size_t stack_sz, size_t tls_sz, uintptr_t base_offset) :
    elf64_object            ( start, size ),
    elf64_executable        ( start, size, stack_size, tls_size ),
    elf64_dynamic_object    ( start, size ),
    virtual_load_base       ( base_offset ),
    preinit_array           {},
    preinit_array_ptr       { 0UL },
    preinit_array_size      { 0UL }
                            {}
elf64_dynamic_executable::elf64_dynamic_executable(file_node* n, size_t stack_sz, size_t tls_sz, uintptr_t base_offset) :
    elf64_object            ( n ),
    elf64_executable        ( n, stack_sz, tls_sz ),
    elf64_dynamic_object    ( n ),
    virtual_load_base       ( base_offset ),
    preinit_array           {},
    preinit_array_ptr       { 0UL },
    preinit_array_size      { 0UL }
                            {}
elf64_dynamic_executable::elf64_dynamic_executable(elf64_dynamic_executable const& that) :
    elf64_object            ( that ),
    elf64_executable        ( that ),
    elf64_dynamic_object    ( that ),
    virtual_load_base       { that.virtual_load_base },
    preinit_array           { that.preinit_array },
    preinit_array_ptr       { that.preinit_array_ptr },
    preinit_array_size      { that.preinit_array_size }
                            {}
void elf64_dynamic_executable::process_headers()
{
    elf64_executable::process_headers();
    if(virtual_load_base)
    {
        off_t diff = static_cast<off_t>(virtual_load_base.full);
        frame_base += diff;
        frame_extent += diff;
        stack_base += diff;
        tls_base += diff;
        entry += diff;
    }
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
    else if(dyn_entries[i].d_tag == DT_PREINIT_ARRAYSZ) preinit_array_size = (dyn_entries[i].d_val / sizeof(addr_t));
    else elf64_dynamic_object::process_dyn_entry(i);
}
bool elf64_dynamic_executable::load_preinit()
{
    if(preinit_array_ptr && preinit_array_size)
    {
        addr_t preinit_ptrs_vaddr = resolve(preinit_array_ptr);
        frame_enter();
        uintptr_t* preinit_ptrs = reinterpret_cast<uintptr_t*>(kmm.frame_translate(preinit_ptrs_vaddr));
        kmm.exit_frame();
        if(!preinit_ptrs) { panic("pre-initialization array pointer is non-null but is invalid"); return false; }
        for(size_t i = 0; i < preinit_array_size; i++) { preinit_array.push_back(addr_t(preinit_ptrs[i])); }
    }
    return true;
}