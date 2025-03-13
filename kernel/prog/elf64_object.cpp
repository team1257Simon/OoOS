#include "elf64_object.hpp"
#include "libk_decls.h"
static std::alignas_allocator<char, elf64_ehdr> elf_alloc{};
elf64_object::~elf64_object() { if(__image_start) elf_alloc.deallocate(__image_start, __image_size); }
void elf64_object::cleanup() { if(__image_start) elf_alloc.deallocate(__image_start, __image_size); __image_start = nullptr; }
elf64_object::elf64_object(file_node *n) : __image_start{ elf_alloc.allocate(n->size()) }, __image_size{ n->size() } { n->read(__image_start, __image_size); }
bool elf64_object::validate() noexcept { if(__validated) return true; if(__builtin_memcmp(ehdr_ptr()->e_ident, "\177ELF", 4) != 0) { panic("missing identifier; invalid object"); return false; } try { return (__validated = xvalidate()); } catch(std::exception& e) { panic(e.what()); return false; } }
bool elf64_object::load() noexcept { try { if(__loaded) return true; if(!validate()) { panic("invalid executable"); return false; } return (__loaded = xload()); } catch(std::exception& e) { panic(e.what()); return false; } }