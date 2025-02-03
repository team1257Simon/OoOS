#include "elf64_exec.hpp"
#include "frame_manager.hpp"
#include "stdexcept"
constexpr static bool validate_elf(elf64_ehdr const& elf) { return !__builtin_memcmp(elf.e_ident, ELFMAG, SELFMAG) && elf.e_ident[EI_CLASS] == ELFCLASS64 && elf.e_ident[EI_DATA] == ELFDATA2LSB && elf.e_type == ET_EXEC && elf.e_machine == EM_MACH && elf.e_phnum > 0; }
constexpr static bool is_write(elf64_phdr const& seg) { return seg.p_flags & 0x02; }
constexpr static bool is_exec(elf64_phdr const& seg) { return seg.p_flags & 0x04; }
constexpr static bool is_load(elf64_phdr const& seg) { return seg.p_type == PT_LOAD; }
bool elf64_executable::validate() noexcept
{
    if(this->__validated) return true;
    if(!validate_elf(this->__get_ehdr())) { direct_writeln("invalid header"); return false; }
    this->__process_entry_ptr = vaddr_t{ this->__get_ehdr().e_entry };
    elf64_phdr* h = this->__image_start + ptrdiff_t(this->__get_ehdr().e_phoff);
    for(size_t n = 0; n < this->__get_ehdr().e_phnum; n++, h = (vaddr_t{ h } + ptrdiff_t(this->__get_ehdr().e_phentsize)))
    {
        if(!is_load(*h)) continue;
        if(!this->__process_frame_base || this->__process_frame_base > h->p_vaddr) this->__process_frame_base = vaddr_t{ h->p_vaddr };
        if(!this->__process_frame_extent || h->p_vaddr + h->p_memsz > this->__process_frame_extent) this->__process_frame_extent = vaddr_t{ h->p_vaddr + h->p_memsz };
    }
    this->__process_frame_base %= PAGESIZE;
    this->__process_stack_base = this->__process_frame_extent;
    this->__process_frame_extent += this->__tgt_stack_size;
    this->__process_tls_base = this->__process_frame_extent;
    this->__process_frame_extent += this->__tgt_tls_size;
    return (this->__validated = true);
}
void elf64_executable::xload()
{
    if(!this->validate()) throw std::runtime_error{ "invalid executable" };
    if((this->__process_frame_tag = std::addressof(frame_manager::get().create_frame(this->__process_frame_base, this->__process_frame_extent))))
    {
        elf64_phdr* h = this->__image_start + ptrdiff_t(this->__get_ehdr().e_phoff);
        for(size_t n = 0; n < this->__get_ehdr().e_phnum; n++, h = (vaddr_t{ h } + ptrdiff_t(this->__get_ehdr().e_phentsize)))
        {
            if(!is_load(*h) || !h->p_memsz) continue;
            vaddr_t blk = heap_allocator::get().allocate_user_block(h->p_memsz, vaddr_t{ h->p_vaddr }, is_write(*h), is_exec(*h));
            if(!blk) { frame_manager::get().destroy_frame(*this->__process_frame_tag); this->__process_frame_tag = nullptr; throw std::bad_alloc{}; }
            this->__process_frame_tag->usr_blocks.emplace_back(blk, h->p_memsz);
            vaddr_t idmap = heap_allocator::get().identity_map_to_kernel(blk, h->p_memsz);
            vaddr_t img_dat = this->__image_start + ptrdiff_t(h->p_offset);
            __builtin_memcpy(idmap, img_dat, h->p_filesz);
            if(h->p_memsz > h->p_filesz) { size_t diff = static_cast<size_t>(h->p_memsz - h->p_filesz); arrayset(img_dat + ptrdiff_t(h->p_filesz), uint8_t(0), diff); }
        }
    }
    else throw std::bad_alloc{};
}
bool elf64_executable::load() noexcept
{
    if(__is_loaded()) return true;
    try { this->xload(); return true; }
    catch(std::bad_alloc&) { panic("executable could not be loaded due to lack of memory"); }
    catch(std::exception& e) { panic(e.what()); }
    return false;
}