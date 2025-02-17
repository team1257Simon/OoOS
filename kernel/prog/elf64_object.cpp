#include "elf64_object.hpp"
#include "libk_decls.h"
elf64_object::~elf64_object() { std::aligned_allocator<char, elf64_ehdr>().deallocate(__image_start, __image_size); }
elf64_object::elf64_object(file_node *n) noexcept : __image_start{ std::aligned_allocator<char, elf64_ehdr>().allocate(n->size()) }, __image_size{ n->size() } { n->read(__image_start, __image_size); }
bool elf64_object::validate() noexcept { if(this->__validated) return true; if(__builtin_memcmp(ehdr_ptr()->e_ident, "\177ELF", 4) != 0) { panic("missing identifier; invalid object"); return false; } return (this->__validated = xvalidate()); }
bool elf64_object::load() noexcept { if(this->__loaded) return true; if(!this->validate()) { panic("invalid executable"); return false; } return this->__loaded = xload(); }