#ifndef __ELF64_OBJ
#define __ELF64_OBJ
#include "kernel/kernel_defs.h"
#include "kernel/elf64.h"
#include "kernel/fs/fs.hpp"
class elf64_object
{
    bool __validated{ false };
    bool __loaded{ false };
    addr_t __image_start;
    size_t __image_size;
protected:
    size_t num_seg_descriptors{ 0UL };
    program_segment_descriptor* segments{ nullptr };
    elf64_sym_table symtab{};
    elf64_string_table symstrtab{};
    elf64_string_table shstrtab{};
    constexpr elf64_ehdr const& ehdr() const noexcept { return __image_start.ref<elf64_ehdr>(); }
    constexpr elf64_phdr const& phdr(size_t n) const noexcept { return __image_start.plus(ehdr().e_phoff + n * ehdr().e_phentsize).ref<elf64_phdr>(); }
    constexpr elf64_shdr const& shdr(size_t n) const noexcept { return __image_start.plus(ehdr().e_shoff + n * ehdr().e_shentsize).ref<elf64_shdr>(); }
    constexpr addr_t img_ptr(size_t offs = 0UL) const noexcept { return __image_start.plus(offs); }
    constexpr addr_t segment_ptr(size_t n) const noexcept { return __image_start.plus(phdr(n).p_offset); }
    constexpr addr_t section_ptr(size_t n) const noexcept { return __image_start.plus(shdr(n).sh_offset); }
    virtual void process_headers();
    virtual bool load_segments() = 0;
    virtual bool xload() = 0;
    virtual bool xvalidate() = 0;
    bool load_syms();
    off_t segment_index(size_t offset) const;
    off_t segment_index(elf64_sym const* sym) const;
    void cleanup();
public:
    elf64_object(file_node* n);
    elf64_object(elf64_object const&);
    elf64_object(elf64_object&&);
    virtual addr_t resolve(uint64_t offs) const;    
    virtual addr_t resolve(elf64_sym const& sym) const;
    virtual ~elf64_object();
    bool validate() noexcept;
    bool load() noexcept;
};
#endif