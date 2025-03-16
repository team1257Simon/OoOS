#include "elf64_object.hpp"
#include "libk_decls.h"
#include "stdlib.h"
#include "kernel_mm.hpp"
static std::allocator<char> ch_alloc{};
static std::alignas_allocator<char, elf64_ehdr> elf_alloc{};
static std::allocator<program_segment_descriptor> sd_alloc{};
elf64_object::~elf64_object() { if(__image_start) elf_alloc.deallocate(__image_start, __image_size); if(symtab.data) free(symtab.data); if(symstrtab.data) free(symstrtab.data); if(shstrtab.data) free(shstrtab.data); }
void elf64_object::cleanup() { if(__image_start) elf_alloc.deallocate(__image_start, __image_size); __image_start = nullptr; }
void elf64_object::process_headers() { num_seg_descriptors = ehdr().e_phnum; segments = sd_alloc.allocate(num_seg_descriptors); }
elf64_object::elf64_object(file_node* n) : __image_start{ elf_alloc.allocate(n->size()) }, __image_size{ n->size() } { n->read(__image_start, __image_size); }
bool elf64_object::validate() noexcept { if(__validated) return true; if(__builtin_memcmp(ehdr_ptr()->e_ident, "\177ELF", 4) != 0) { panic("missing identifier; invalid object"); return false; } try { return (__validated = xvalidate()); } catch(std::exception& e) { panic(e.what()); return false; } }
bool elf64_object::load() noexcept { try { if(__loaded) return true; if(!validate()) { panic("invalid executable"); return false; } return (__loaded = xload()); } catch(std::exception& e) { panic(e.what()); return false; } }
program_segment_descriptor const *elf64_object::segment_of(size_t offset) const { for(size_t i = 0; i < num_seg_descriptors; i++) { if(static_cast<uintptr_t>(segments[i].obj_offset) <= offset && offset < static_cast<uintptr_t>(segments[i].obj_offset + segments[i].size)) return segments + i; } return nullptr; }
program_segment_descriptor const *elf64_object::segment_of(elf64_sym const *sym) const { return segment_of(sym->st_value); }
addr_t elf64_object::resolve(uint64_t offs) const { program_segment_descriptor const* sd = segment_of(offs); return sd ? to_segment_ptr(offs, *sd) : nullptr; }
addr_t elf64_object::resolve(elf64_sym const& sym) const { return resolve(sym.st_value); }
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
// Copy and move constructors are nontrivial. Executables and the like delete the copy constructor and can inherit the move constructor (dynamic objects will have to extend the nontrivial constructors)
elf64_object::elf64_object(elf64_object const& that) : 
    __image_start{ nullptr }, 
    __image_size{ that.__image_size }, 
    num_seg_descriptors{ that.num_seg_descriptors }, 
    segments{ sd_alloc.allocate(num_seg_descriptors) },
    symtab{ .total_size = that.symtab.total_size, .entry_size = that.symtab.entry_size, .data = ch_alloc.allocate(that.symtab.total_size) },
    symstrtab{ .total_size = that.symstrtab.total_size, .data = ch_alloc.allocate(that.symstrtab.total_size) },
    shstrtab{ .total_size = that.symstrtab.total_size, .data = ch_alloc.allocate(that.shstrtab.total_size) }
{
    array_copy<char>(symtab.data, that.symtab.data, that.symtab.total_size);
    array_copy<char>(symstrtab.data, that.symstrtab.data, that.symstrtab.total_size);
    array_copy<char>(shstrtab.data, that.shstrtab.data, that.shstrtab.total_size);
    for(size_t i = 0; i < num_seg_descriptors; i++)
    {
        new (std::addressof(segments[i])) program_segment_descriptor(kernel_memory_mgr::get().allocate_user_block(that.segments[i].size, nullptr, that.segments[i].seg_align, that.segments[i].perms & PROT_WRITE, that.segments[i].perms & PROT_EXEC), that.segments[i].obj_offset, that.segments[i].size, that.segments[i].seg_align, that.segments[i].perms);
        if(segments[i].absolute_addr) array_copy<char>(segments[i].absolute_addr, that.segments[i].absolute_addr, that.segments[i].size);
        else { panic("failed to allocate copy segment"); break; }
    }
}
elf64_object::elf64_object(elf64_object&& that) : 
    __image_start{ that.__image_start }, 
    __image_size{ that.__image_size }, 
    num_seg_descriptors{ that.num_seg_descriptors }, 
    segments{ that.segments },
    symtab{ std::move(that.symtab) },
    symstrtab{ std::move(that.symstrtab) },
    shstrtab{ std::move(that.shstrtab) }
{
    that.__image_start = nullptr;
    that.segments = nullptr;
    that.symtab.data = nullptr;
    that.symstrtab.data = nullptr;
    that.shstrtab.data = nullptr;
}