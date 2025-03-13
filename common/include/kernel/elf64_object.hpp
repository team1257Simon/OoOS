#ifndef __ELF64_OBJ
#define __ELF64_OBJ
#include "kernel/kernel_defs.h"
#include "kernel/elf64.h"
#include "kernel/fs/fs.hpp"
constexpr bool is_write(elf64_phdr const& seg) { return seg.p_flags & phdr_flag_write; }
constexpr bool is_exec(elf64_phdr const& seg) { return seg.p_flags & phdr_flag_execute; }
constexpr bool is_load(elf64_phdr const& seg) { return seg.p_type == PT_LOAD; }
constexpr bool is_dynamic(elf64_phdr const& seg) { return seg.p_type == PT_DYNAMIC; }
class elf64_object
{
    bool __validated{ false };
    bool __loaded{ false };
    addr_t __image_start;
    size_t __image_size;
protected:    
    constexpr elf64_ehdr const* ehdr_ptr() const noexcept { return __image_start; }
    constexpr elf64_ehdr const& ehdr() const noexcept { return *ehdr_ptr(); }
    constexpr elf64_phdr const* phdr_ptr(size_t n) const noexcept { return __image_start.plus(ehdr().e_phoff + n * ehdr().e_phentsize); }
    constexpr elf64_phdr const& phdr(size_t n) const noexcept { return *phdr_ptr(n); }
    constexpr elf64_shdr const* shdr_ptr(size_t n) const noexcept { return __image_start.plus(ehdr().e_shoff + n * ehdr().e_shentsize); }
    constexpr elf64_shdr const& shdr(size_t n) const noexcept { return *shdr_ptr(n); }
    constexpr addr_t img_ptr(size_t offs = 0UL) const noexcept { return __image_start.plus(offs); }
    constexpr addr_t segment_ptr(size_t n) const noexcept { return __image_start.plus(phdr(n).p_offset); }
    constexpr addr_t section_ptr(size_t n) const noexcept { return __image_start.plus(shdr(n).sh_offset); }
    void cleanup();
    virtual bool xload() = 0;
    virtual bool xvalidate() = 0;
public:
    elf64_object(file_node* n);
    virtual ~elf64_object();
    bool validate() noexcept;
    bool load() noexcept;
    elf64_object(elf64_object const&) = delete;
    elf64_object& operator=(elf64_object const&) = delete; 
};
#endif