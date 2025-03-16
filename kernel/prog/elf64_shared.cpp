#include "elf64_shared.hpp"
#include "stdexcept"
#include "kernel_mm.hpp"
constexpr size_t mw_bits = sizeof(uint64_t) * CHAR_BIT;
elf64_shared_object::elf64_shared_object(file_node *n) : elf64_dynamic_object{ n } {}
elf64_shared_object::~elf64_shared_object() = default;
bool elf64_shared_object::xvalidate()
{
    if(ehdr_ptr()->e_machine != EM_AMD64 || ehdr_ptr()->e_ident[elf_ident_encoding_idx] != ED_LSB) { panic("not an object for the correct machine"); return false; }
    if(ehdr_ptr()->e_ident[elf_ident_class_idx] != EC_64 ) { panic("32-bit object files are not yet supported"); return false; }
    if(ehdr_ptr()->e_type != ET_DYN) { panic("object is not a shared library"); return false; }
    if(!ehdr_ptr()->e_phnum) { panic("no program headers present"); return false; }
    return true;
}
bool elf64_shared_object::load_segments()
{
    bool have_loads = false;
    for(size_t n = 0; n < ehdr().e_phnum; n++)
    {
        if(!phdr(n).p_memsz) continue;
        if(is_load(phdr(n)))
        {
            have_loads = true;
            addr_t blk = kernel_memory_mgr::get().allocate_user_block(phdr(n).p_memsz, nullptr, phdr(n).p_align, is_write(phdr(n)), is_exec(phdr(n)));
            addr_t img_dat = segment_ptr(n); 
            if(!blk) { throw std::bad_alloc{}; }
            array_copy<uint8_t>(blk, img_dat, phdr(n).p_filesz);
            elf_segment_prot perms = static_cast<elf_segment_prot>(0b100 | (is_write(phdr(n)) ? 0b010 : 0) | (is_exec(phdr(n)) ? 0b001 : 0));
            if(phdr(n).p_memsz > phdr(n).p_filesz) { size_t diff = static_cast<size_t>(phdr(n).p_memsz - phdr(n).p_filesz); array_zero<uint8_t>(blk.plus(phdr(n).p_filesz), diff); }
            new (std::addressof(segments[n])) program_segment_descriptor{ blk, static_cast<off_t>(phdr(n).p_vaddr), phdr(n).p_memsz, phdr(n).p_align, perms };
        }
    }
    return have_loads;
}
addr_t elf64_shared_object::resolve(std::string const& symbol) const
{
    if(!segments || !num_seg_descriptors || !symbol_index) { panic("cannot load symbols from an uninitialized object"); return nullptr; }
    elf64_sym const* sym = symbol_index[symbol];
    return sym ? elf64_object::resolve(*sym) : nullptr;
}
