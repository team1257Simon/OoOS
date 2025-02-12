#ifndef __ELF64_OBJ
#define __ELF64_OBJ
#include "kernel/kernel_defs.h"
#include "kernel/elf64.h"
class elf64_object
{
    bool __validated{ false };
    bool __loaded{ false };
    vaddr_t __image_start;  
protected:    
    constexpr elf64_ehdr* ehdr_ptr() noexcept { return __image_start; }
    constexpr elf64_ehdr const* ehdr_ptr() const noexcept { return __image_start; }
    constexpr elf64_ehdr& ehdr() noexcept { return *ehdr_ptr(); }
    constexpr elf64_ehdr const& ehdr() const noexcept { return *ehdr_ptr(); }
    constexpr elf64_phdr* phdr_ptr(size_t n) noexcept { return this->__image_start.plus(this->ehdr().e_phoff + n * this->ehdr().e_phentsize); }
    constexpr elf64_phdr const* phdr_ptr(size_t n) const noexcept { return this->__image_start.plus(this->ehdr().e_phoff + n * this->ehdr().e_phentsize); }
    constexpr elf64_phdr& phdr(size_t n) noexcept { return *phdr_ptr(n); }
    constexpr elf64_phdr const& phdr(size_t n) const noexcept { return *phdr_ptr(n); }
    constexpr elf64_shdr* shdr_ptr(size_t n) noexcept { return this->__image_start.plus(this->ehdr().e_shoff + n * this->ehdr().e_shentsize); }
    constexpr elf64_shdr const* shdr_ptr(size_t n) const noexcept { return this->__image_start.plus(this->ehdr().e_shoff + n * this->ehdr().e_shentsize); }
    constexpr elf64_shdr& shdr(size_t n) noexcept { return *shdr_ptr(n); }
    constexpr elf64_shdr const& shdr(size_t n) const noexcept { return *shdr_ptr(n); }
    constexpr vaddr_t segment_ptr(size_t n) const noexcept { return this->__image_start.plus(this->phdr(n).p_offset); }
    virtual bool xload() = 0;
    virtual bool xvalidate() = 0;
public:
    elf64_object(vaddr_t image) noexcept;
    bool validate() noexcept;
    bool load() noexcept;
};
#endif