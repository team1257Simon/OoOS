#include "elf64_exec.hpp"
#include "frame_manager.hpp"
#include "stdexcept"
constexpr static bool is_write(elf64_phdr const& seg) { return seg.p_flags & phdr_flag_write; }
constexpr static bool is_exec(elf64_phdr const& seg) { return seg.p_flags & phdr_flag_execute; }
constexpr static bool is_load(elf64_phdr const& seg) { return seg.p_type == PT_LOAD; }
elf64_executable::elf64_executable(file_inode* n, size_t stack_sz, size_t tls_sz) noexcept : elf64_object{ n }, __tgt_stack_size{ stack_sz }, __tgt_tls_size{ tls_sz } {}
bool elf64_executable::xvalidate()
{
    if(ehdr_ptr()->e_machine != EM_AMD64 || ehdr_ptr()->e_ident[elf_ident_encoding_idx] != ED_LSB) { panic("not an object for the correct machine"); return false; }
    if(ehdr_ptr()->e_ident[elf_ident_class_idx] != EC_64 ) { panic("32-bit executables are not yet supported"); return false; }
    if(ehdr_ptr()->e_type != ET_EXEC) { panic("object is not an executable"); return false; }
    if(!ehdr_ptr()->e_phnum) { panic("no program headers present"); return false; }
    return true;
}
bool elf64_executable::xload()
{
    this->__process_entry_ptr = addr_t(this->ehdr().e_entry);
    for(size_t n = 0; n < this->ehdr().e_phnum; n++)
    {
        if(!is_load(phdr(n))) continue;
        if(!this->__process_frame_base || this->__process_frame_base > phdr(n).p_vaddr) this->__process_frame_base = addr_t(phdr(n).p_vaddr);
        if(!this->__process_frame_extent || phdr(n).p_vaddr + phdr(n).p_memsz > this->__process_frame_extent) this->__process_frame_extent = addr_t(phdr(n).p_vaddr + phdr(n).p_memsz);
    }
    this->__process_frame_base = this->__process_frame_base.page_aligned();
    this->__process_frame_extent = this->__process_frame_extent.plus(static_cast<ptrdiff_t>(PAGESIZE)).page_aligned();
    this->__process_stack_base = this->__process_frame_extent;
    this->__process_frame_extent += this->__tgt_stack_size;
    this->__process_tls_base = this->__process_frame_extent;
    this->__process_frame_extent += this->__tgt_tls_size;
    if((this->__process_frame_tag = std::addressof(frame_manager::get().create_frame(this->__process_frame_base, this->__process_frame_extent)))) try
    {
        heap_allocator::get().enter_frame(this->__process_frame_tag);
        for(size_t n = 0; n < this->ehdr().e_phnum; n++)
        {
            if(!is_load(phdr(n)) || !phdr(n).p_memsz) continue;
            addr_t addr{ phdr(n).p_vaddr };
            addr_t blk = heap_allocator::get().allocate_user_block(phdr(n).p_memsz, addr, phdr(n).p_align, is_write(phdr(n)), is_exec(phdr(n)));
            addr_t idmap{ heap_allocator::get().translate_vaddr_in_current_frame(addr) };
            addr_t img_dat = this->segment_ptr(n);    
            if(!blk) { throw std::bad_alloc{}; }
            this->__process_frame_tag->usr_blocks.emplace_back(blk, heap_allocator::page_aligned_region_size(addr, phdr(n).p_memsz));
            arraycopy<uint8_t>(idmap, img_dat, phdr(n).p_filesz);
            if(phdr(n).p_memsz > phdr(n).p_filesz) { size_t diff = static_cast<size_t>(phdr(n).p_memsz - phdr(n).p_filesz); array_zero<uint8_t>(addr_t(heap_allocator::get().translate_vaddr_in_current_frame(addr_t(phdr(n).p_vaddr + phdr(n).p_filesz))), diff); }
        }
        addr_t stkblk = heap_allocator::get().allocate_user_block(this->__tgt_stack_size, this->__process_stack_base, PAGESIZE, true, false);
        addr_t tlsblk = heap_allocator::get().allocate_user_block(this->__tgt_tls_size, this->__process_tls_base, PAGESIZE, true, false);
        if(!stkblk || !tlsblk) { throw std::bad_alloc{}; }
        this->__process_frame_tag->usr_blocks.emplace_back(stkblk, this->__tgt_stack_size);
        this->__process_frame_tag->usr_blocks.emplace_back(tlsblk, this->__tgt_tls_size);
        heap_allocator::get().exit_frame();
        new(&__descr) elf64_program_descriptor 
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
    catch (...) {  frame_manager::get().destroy_frame(*this->__process_frame_tag); this->__process_frame_tag = nullptr; heap_allocator::get().exit_frame(); panic("could not allocate blocks for executable"); }
    else { panic("could not allocate frame"); }
    return false; 
}
elf64_program_descriptor const& elf64_executable::describe() const noexcept { return __descr; }