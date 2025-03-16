#include "elf64_object.hpp"
#include "libk_decls.h"
#include "stdlib.h"
#include "kernel_mm.hpp"
static std::alignas_allocator<char, elf64_ehdr> elf_alloc{};
elf64_object::~elf64_object() { if(__image_start) elf_alloc.deallocate(__image_start, __image_size); if(symtab.data) free(symtab.data); if(symstrtab.data) free(symstrtab.data); if(shstrtab.data) free(shstrtab.data); }
void elf64_object::cleanup() { if(__image_start) elf_alloc.deallocate(__image_start, __image_size); __image_start = nullptr; }
elf64_object::elf64_object(file_node* n) : __image_start{ elf_alloc.allocate(n->size()) }, __image_size{ n->size() } { n->read(__image_start, __image_size); }
bool elf64_object::validate() noexcept { if(__validated) return true; if(__builtin_memcmp(ehdr_ptr()->e_ident, "\177ELF", 4) != 0) { panic("missing identifier; invalid object"); return false; } try { return (__validated = xvalidate()); } catch(std::exception& e) { panic(e.what()); return false; } }
bool elf64_object::load() noexcept { try { if(__loaded) return true; if(!validate()) { panic("invalid executable"); return false; } return (__loaded = xload()); } catch(std::exception& e) { panic(e.what()); return false; } }
program_segment_descriptor const *elf64_object::segment_of(size_t offset) const { for(size_t i = 0; i < num_seg_descriptors; i++) { if(static_cast<uintptr_t>(segments[i].obj_offset) <= offset && offset < static_cast<uintptr_t>(segments[i].obj_offset + segments[i].size)) return segments + i; } return nullptr; }
program_segment_descriptor const *elf64_object::segment_of(elf64_sym const *sym) const { return segment_of(sym->st_value); }
bool elf64_object::load_syms()
{
    elf64_shdr const* shstrtab_shdr = shdr_ptr(ehdr().e_shstrndx);
    shstrtab.total_size = shstrtab_shdr->sh_size;
    shstrtab.data = aligned_malloc(shstrtab_shdr->sh_size, shstrtab_shdr->sh_addralign);
    array_copy<char>(shstrtab.data, img_ptr(shstrtab_shdr->sh_offset), shstrtab_shdr->sh_size);
    for(size_t i = 0; i < ehdr().e_shnum; i++)
    {
        if(shdr(i).sh_type == ST_DYNSYM || shdr(i).sh_type == ST_SYMTAB)
        {
            elf64_shdr const* symtab_shdr = shdr_ptr(i);
            elf64_shdr const* strtab_shdr  = shdr_ptr(symtab_shdr->sh_link);
            symstrtab.total_size = strtab_shdr->sh_size;
            symtab.total_size = symtab_shdr->sh_size;
            symtab.entry_size = symtab_shdr->sh_entsize;
            symstrtab.data = aligned_malloc(strtab_shdr->sh_size, strtab_shdr->sh_addralign);
            symtab.data = aligned_malloc(symtab_shdr->sh_size, symtab_shdr->sh_addralign);
            array_copy<char>(symstrtab.data, img_ptr(strtab_shdr->sh_offset), strtab_shdr->sh_size);
            array_copy<char>(symtab.data, img_ptr(symtab_shdr->sh_offset), symtab_shdr->sh_size);
            return true;
        }
    }
    return false;
}