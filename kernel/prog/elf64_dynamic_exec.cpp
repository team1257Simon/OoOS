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
bool elf64_dynamic_executable::post_load_init()
{
    kmm.enter_frame(frame_tag);
    apply_relocations();
    if(got_vaddr)
    {
        addr_t* got = reinterpret_cast<addr_t*>(kmm.translate_vaddr_in_current_frame(global_offset_table()));
        got[1] = global_offset_table();
        // TODO: also put the address of the resolve-symbol function in the GOT
    }
    kmm.exit_frame();
    try
    {
        std::vector<addr_t> fini_reverse_array{};
        if(preinit_array_ptr && preinit_array_ptr)
        {
            addr_t preinit_ptrs_vaddr = resolve(preinit_array_ptr);
            kmm.enter_frame(frame_tag);
            uintptr_t* preinit_ptrs = reinterpret_cast<uintptr_t*>(kmm.translate_vaddr_in_current_frame(preinit_ptrs_vaddr));
            kmm.exit_frame();
            for(size_t i = 0; i < preinit_array_size; i++) { preinit_array.push_back(addr_t(preinit_ptrs[i])); }
        }
        if(init_fn) { init_array.push_back(resolve(init_fn)); }
        if(fini_fn) { fini_reverse_array.push_back(resolve(fini_fn)); }
        if(init_array_size && init_array_ptr) 
        {
            addr_t init_ptrs_vaddr = resolve(init_array_ptr);
            kmm.enter_frame(frame_tag);
            uintptr_t* init_ptrs = reinterpret_cast<uintptr_t*>(kmm.translate_vaddr_in_current_frame(init_ptrs_vaddr)); 
            kmm.exit_frame();
            for(size_t i = 0; i < init_array_size; i++) { init_array.push_back(addr_t(init_ptrs[i])); }
        }
        if(fini_array_size && fini_array_ptr)
        {
            addr_t fini_ptrs_vaddr = resolve(fini_array_ptr);
            kmm.enter_frame(frame_tag);
            uintptr_t* fini_ptrs = reinterpret_cast<uintptr_t*>(kmm.translate_vaddr_in_current_frame(fini_ptrs_vaddr)); 
            kmm.exit_frame();
            for(size_t i = 0; i < fini_array_size; i++) { fini_reverse_array.push_back(addr_t(fini_ptrs[i])); } 
        }
        if(!fini_reverse_array.empty()) { fini_array.push_back(fini_reverse_array.rend(), fini_reverse_array.rbegin()); }
        init_array.push_back(nullptr);
        fini_array.push_back(nullptr);
        preinit_array.push_back(nullptr);
        return true;
    }
    catch(std::exception& e) { panic(e.what()); return false; }
}