#include "elf64_exec.hpp"
#include "frame_manager.hpp"
#include "stdexcept"
static std::allocator<program_segment_descriptor> sd_alloc{};
elf64_program_descriptor const& elf64_executable::describe() const noexcept { return __descr; }
elf64_executable::elf64_executable(file_node* n, size_t stack_sz, size_t tls_sz) : elf64_object{ n }, __tgt_stack_size{ stack_sz }, __tgt_tls_size{ tls_sz } {}
elf64_executable::elf64_executable(elf64_executable&&) = default; // we can inherit the nontrivial move constructor from the parent as the remainder is trivial (same reason as destructor)
elf64_executable::~elf64_executable() = default; // the resources allocated for the executable's segments are freed and returned to the kernel when the frame is destroyed
bool elf64_executable::xvalidate()
{
    if(ehdr().e_machine != EM_AMD64 || ehdr().e_ident[elf_ident_encoding_idx] != ED_LSB) { panic("not an object for the correct machine"); return false; }
    if(ehdr().e_ident[elf_ident_class_idx] != EC_64 ) { panic("32-bit executables are not yet supported"); return false; }
    if(ehdr().e_type != ET_EXEC) { panic("object is not an executable"); return false; }
    if(!ehdr().e_phnum) { panic("no program headers present"); return false; }
    return true;
}
void elf64_executable::process_headers()
{
    elf64_object::process_headers();
    __process_entry_ptr = addr_t(ehdr().e_entry);
    for(size_t n = 0; n < ehdr().e_phnum; n++)
    {
        elf64_phdr const& h = phdr(n);
        if(!is_load(h)) continue;
        if(!__process_frame_base || __process_frame_base > h.p_vaddr) __process_frame_base = addr_t(h.p_vaddr);
        if(!__process_frame_extent || h.p_vaddr + h.p_memsz > __process_frame_extent) __process_frame_extent = addr_t(h.p_vaddr + h.p_memsz);
    }
    __process_frame_base = __process_frame_base.page_aligned();
    __process_frame_extent = __process_frame_extent.next_page_aligned();
    __process_stack_base = __process_frame_extent;
    __process_frame_extent += __tgt_stack_size;
    __process_tls_base = __process_frame_extent;
    __process_frame_extent += __tgt_tls_size;
}
bool elf64_executable::load_segments()
{
    if((__process_frame_tag = std::addressof(frame_manager::get().create_frame(__process_frame_base, __process_frame_extent))))
    {
        kernel_memory_mgr::get().enter_frame(__process_frame_tag);
        for(size_t n = 0; n < ehdr().e_phnum; n++)
        {
            elf64_phdr const& h = phdr(n);
            if(!is_load(h) || !h.p_memsz) continue;
            addr_t addr{ h.p_vaddr };
            addr_t blk = kernel_memory_mgr::get().allocate_user_block(h.p_memsz, addr, h.p_align, is_write(h), is_exec(h));
            addr_t idmap{ kernel_memory_mgr::get().translate_vaddr_in_current_frame(addr) };
            addr_t img_dat = segment_ptr(n);
            if(!blk) { panic("could not allocate blocks for executable"); return false; }
            __process_frame_tag->usr_blocks.emplace_back(blk, addr, kernel_memory_mgr::page_aligned_region_size(addr, h.p_memsz));
            array_copy<uint8_t>(idmap, img_dat, h.p_filesz);
            if(h.p_memsz > h.p_filesz) { array_zero<uint8_t>(idmap.plus(h.p_filesz), static_cast<size_t>(h.p_memsz - h.p_filesz)); }
            new (std::addressof(segments[n])) program_segment_descriptor{ idmap, addr, static_cast<off_t>(h.p_offset), h.p_memsz, h.p_align, static_cast<elf_segment_prot>(0b100 | (is_write(h) ? 0b010 : 0) | (is_exec(h) ? 0b001 : 0)) };
        }
        addr_t stkblk = kernel_memory_mgr::get().allocate_user_block(__tgt_stack_size, __process_stack_base, PAGESIZE, true, false);
        addr_t tlsblk = kernel_memory_mgr::get().allocate_user_block(__tgt_tls_size, __process_tls_base, PAGESIZE, true, false);
        if(!stkblk || !tlsblk) { panic("could not allocate blocks for stack");  return false; }
        __process_frame_tag->usr_blocks.emplace_back(stkblk, __process_stack_base, __tgt_stack_size);
        __process_frame_tag->usr_blocks.emplace_back(tlsblk, __process_tls_base, __tgt_tls_size);
        kernel_memory_mgr::get().exit_frame();
        new (std::addressof(__descr)) elf64_program_descriptor 
        { 
            .frame_ptr = __process_frame_tag, 
            .prg_stack = __process_stack_base, 
            .stack_size = __tgt_stack_size,
            .prg_tls = __process_tls_base, 
            .tls_size = __tgt_tls_size, 
            .entry = __process_entry_ptr 
        };
        return true;
    }
    else { panic("could not allocate frame"); return false; }
}
bool elf64_executable::xload()
{
    bool success = true;
    process_headers();
    if(!load_syms()) klog("W: no symbol tables present in executable");
    if(!load_segments()) { frame_manager::get().destroy_frame(*__process_frame_tag); __process_frame_tag = nullptr; kernel_memory_mgr::get().exit_frame(); success = false; }
    cleanup();
    return success;
}