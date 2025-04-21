#include "elf64_object.hpp"
#include "libk_decls.h"
#include "stdlib.h"
#include "kernel_mm.hpp"
#include "bits/stdexcept.h"
static std::allocator<char> ch_alloc{};
static std::alignas_allocator<char, elf64_ehdr> elf_alloc{};
static std::allocator<program_segment_descriptor> sd_alloc{};
elf64_object::~elf64_object() { if(__image_start) elf_alloc.deallocate(__image_start, __image_size); if(symtab.data) free(symtab.data); if(symstrtab.data) free(symstrtab.data); if(shstrtab.data) free(shstrtab.data); }
void elf64_object::cleanup() { if(__image_start) elf_alloc.deallocate(__image_start, __image_size); __image_start = nullptr; }
void elf64_object::release_segments() { xrelease(); }
void elf64_object::xrelease() { /* stub; some dynamic object types will need to override this to release segments for local SOs */ }
void elf64_object::process_headers() { for(size_t i = 0; i < ehdr().e_phnum; i++) { if(is_load(phdr(i)) && phdr(i).p_memsz) num_seg_descriptors++; } segments = sd_alloc.allocate(num_seg_descriptors); }
bool elf64_object::validate() noexcept { if(__validated) return true; if(!__image_size) return false; if(__builtin_memcmp(ehdr().e_ident, "\177ELF", 4) != 0) { panic("missing identifier; invalid object"); return false; } try { return (__validated = xvalidate()); } catch(std::exception& e) { panic(e.what()); return false; } }
bool elf64_object::load() noexcept { try { if(__loaded) return true; if(!validate()) { panic("invalid executable"); return false; } __loaded = xload(); if(!__loaded) on_load_failed(); return __loaded; } catch(std::exception& e) { panic(e.what()); on_load_failed(); return false; } }
off_t elf64_object::segment_index(size_t offset) const { for(size_t i = 0; i < num_seg_descriptors; i++) { if(static_cast<uintptr_t>(segments[i].obj_offset) <= offset && offset < static_cast<uintptr_t>(segments[i].obj_offset + segments[i].size)) return static_cast<off_t>(i); } return -1L; }
off_t elf64_object::segment_index(elf64_sym const* sym) const { return segment_index(sym->st_value); }
addr_t elf64_object::resolve(uint64_t offs) const { off_t idx = segment_index(offs); if(idx < 0) return nullptr; return to_segment_ptr(offs, segments[idx]); }
addr_t elf64_object::resolve(elf64_sym const& sym) const { return resolve(sym.st_value); }
void elf64_object::on_load_failed() { /* stub; additional cleanup to perform if the object fails to load goes here for inheritors */ }
elf64_object::elf64_object(addr_t start, size_t size):
    __validated         { false },
    __loaded            { false },
    __image_start       { start },
    __image_size        { size },
    num_seg_descriptors { 0UL },
    segments            { nullptr },
    symtab              {},
    symstrtab           {},
    shstrtab            {}
                        {}
elf64_object::elf64_object(file_node* n) : elf64_object(elf_alloc.allocate(n->size()), n->size())
{
    if(!n->read(__image_start, __image_size))
    {
        panic("elf object file read failed");
        elf_alloc.deallocate(__image_start, __image_size);
        __image_size = 0;
        __image_start = nullptr;
    }
}
bool elf64_object::load_syms()
{
    elf64_shdr const& shstrtab_shdr = shdr(ehdr().e_shstrndx);
    shstrtab.total_size = shstrtab_shdr.sh_size;
    shstrtab.data = aligned_malloc(shstrtab_shdr.sh_size, shstrtab_shdr.sh_addralign);
    array_copy<char>(shstrtab.data, img_ptr(shstrtab_shdr.sh_offset), shstrtab_shdr.sh_size);
    for(size_t i = 0; i < ehdr().e_shnum; i++)
    {
        elf64_shdr const& h = shdr(i);
        if(h.sh_type == ST_DYNSYM || h.sh_type == ST_SYMTAB)
        {
            elf64_shdr const& strtab_shdr = shdr(h.sh_link);
            symstrtab.total_size = strtab_shdr.sh_size;
            symtab.total_size = h.sh_size;
            symtab.entry_size = h.sh_entsize;
            symstrtab.data = aligned_malloc(strtab_shdr.sh_size, strtab_shdr.sh_addralign);
            symtab.data = aligned_malloc(h.sh_size, h.sh_addralign);
            array_copy<char>(symstrtab.data, img_ptr(strtab_shdr.sh_offset), strtab_shdr.sh_size);
            array_copy<char>(symtab.data, img_ptr(h.sh_offset), h.sh_size);
            return true;
        }
    }
    return false;
}
bool elf64_object::xload()
{
    bool success = true;
    process_headers();
    if(!load_syms()) klog("W: no symbol tables present in object");
    if(!load_segments()) { success = false; }
    cleanup();
    return success;
}
std::vector<block_descr> elf64_object::segment_blocks() const
{
    std::vector<block_descr> result(num_seg_descriptors);
    for(size_t i = 0; i < num_seg_descriptors; i++) 
    { 
        if(segments[i].size)
        {
            size_t align = segments[i].seg_align;
            elf_segment_prot pr = segments[i].perms;
            result.emplace_back(segments[i].absolute_addr.trunc(align), segments[i].virtual_addr.trunc(align), segments[i].size, segments[i].seg_align, is_write(pr), is_exec(pr));
        }
    }
    return result;
}
// Copy and move constructors are nontrivial. Executables and the like delete the copy constructor and can inherit the move constructor (dynamic objects will have to extend the nontrivial constructors)
elf64_object::elf64_object(elf64_object const& that) :
    __validated         { that.__validated },
    __loaded            { that.__loaded },
    __image_start       { that.__image_start }, 
    __image_size        { that.__image_size },
    num_seg_descriptors { that.num_seg_descriptors }, 
    segments            { sd_alloc.allocate(num_seg_descriptors) },
    symtab              { that.symtab.total_size, that.symtab.entry_size, ch_alloc.allocate(that.symtab.total_size) },
    symstrtab           { that.symstrtab.total_size, ch_alloc.allocate(that.symstrtab.total_size) },
    shstrtab            { that.symstrtab.total_size, ch_alloc.allocate(that.shstrtab.total_size) }
{
    if(__loaded)
    {
        array_copy<char>(symtab.data, that.symtab.data, that.symtab.total_size);
        array_copy<char>(symstrtab.data, that.symstrtab.data, that.symstrtab.total_size);
        array_copy<char>(shstrtab.data, that.shstrtab.data, that.shstrtab.total_size);
        array_copy<program_segment_descriptor>(segments, that.segments, that.num_seg_descriptors); // based generic memcpy routine
    }
}
elf64_object::elf64_object(elf64_object&& that) :
    __validated         { that.__validated },
    __loaded            { that.__loaded },
    __image_start       { that.__image_start }, 
    __image_size        { that.__image_size }, 
    num_seg_descriptors { that.num_seg_descriptors },
    segments            { that.segments },
    symtab              { std::move(that.symtab) },
    symstrtab           { std::move(that.symstrtab) },
    shstrtab            { std::move(that.shstrtab) }
{
    that.__validated = false;
    that.__loaded = false;
    that.__image_size = 0;
    that.__image_start = nullptr;
    that.segments = nullptr;
    that.num_seg_descriptors = 0;
}
void elf64_object::on_copy(uframe_tag* new_frame)
{
    if(!new_frame) { throw std::invalid_argument{ "frame tag must not be null" }; }
    set_frame(new_frame);
    for(size_t i = 0; i < num_seg_descriptors; i++)
    {
        if(!segments[i].absolute_addr || !segments[i].size) continue;
        addr_t addr = segments[i].virtual_addr;
        frame_enter();
        segments[i].absolute_addr = addr_t(kmm.frame_translate(addr));
        kmm.exit_frame();
    }
}