#include "elf64_shared.hpp"
#include "stdexcept"
constexpr size_t mw_bits = sizeof(uint64_t) * CHAR_BIT;
const char* empty_name = "";
static const char* find_so_name(addr_t, file_node*);
addr_t elf64_shared_object::resolve(uint64_t offs) const { return virtual_load_base.plus(offs); }
addr_t elf64_shared_object::resolve(elf64_sym const& sym) const { return virtual_load_base.plus(sym.st_value); }
void elf64_shared_object::frame_enter() { kmm.enter_frame(frame_tag); }
void elf64_shared_object::xrelease() { if(frame_tag) { for(block_descr& blk : segment_blocks()) { frame_tag->drop_block(blk); } } }
void elf64_shared_object::process_dyn_entry(size_t i) { if(dyn_entries[i].d_tag == DT_SYMBOLIC || (dyn_entries[i].d_tag == DT_FLAGS && dyn_entries[i].d_val & 0x02)) { symbolic = true; } elf64_dynamic_object::process_dyn_entry(i); }
elf64_shared_object::~elf64_shared_object() = default;
elf64_shared_object::elf64_shared_object(file_node* n, uframe_tag* frame) :
    elf64_object            ( n ),
    elf64_dynamic_object    ( n ),
    soname                  ( find_so_name(img_ptr(), n) ),
    virtual_load_base       { frame->dynamic_extent },
    total_segment_size      { 0UL },
    frame_tag               { frame },
    ref_count               { 1UL },
    sticky                  { false },
    symbolic                { false },
    entry                   { nullptr }
                            {}
elf64_shared_object::elf64_shared_object(elf64_shared_object&& that) : 
    elf64_object            ( std::move(that) ),
    elf64_dynamic_object    ( std::move(that) ),
    soname                  ( std::move(that.soname) ),
    virtual_load_base       { that.virtual_load_base },
    total_segment_size      { that.total_segment_size },
    frame_tag               { that.frame_tag },
    ref_count               { that.ref_count },
    sticky                  { that.sticky },
    symbolic                { that.symbolic },
    entry                   { that.entry }
                            { that.frame_tag = nullptr; }
static const char* find_so_name(addr_t image_start, file_node* so_file)
{
    elf64_ehdr const& eh = image_start.ref<elf64_ehdr>();
    for(size_t i = 0; i < eh.e_phnum; i++)
    {
        elf64_phdr const* phptr = image_start.plus(eh.e_phoff + i * eh.e_phentsize);
        elf64_phdr const& ph = *phptr;
        if(is_dynamic(ph))
        {
            elf64_dyn* dyn_ent = image_start.plus(ph.p_offset);
            size_t n = ph.p_filesz / sizeof(elf64_dyn);
            size_t strtab_off = 0UL, name_off = 0UL;
            for(size_t j = 0; j < n; j++)
            {
                if(dyn_ent[j].d_tag == DT_STRTAB) strtab_off = dyn_ent[j].d_ptr;
                else if(dyn_ent[j].d_tag == DT_SONAME) name_off = dyn_ent[j].d_val;
                if(strtab_off && name_off) break;
            }
            if(!(strtab_off && name_off)) return so_file->name();
            else return image_start.plus(strtab_off + name_off);
        }
    }
    return empty_name;
}
const char* elf64_shared_object::sym_lookup(addr_t addr) const
{
    if(!could_contain(addr)) return nullptr;
    size_t nsym = symtab.entries();
    for(size_t i = 0; i < nsym; i++) 
    {
        elf64_sym const& sym = symtab[i];
        addr_t sym_base = resolve(sym);
        if(addr >= sym_base && sym_base.plus(sym.st_size) > addr) return symstrtab[sym.st_name];
    }
    return nullptr;
}
program_segment_descriptor const* elf64_shared_object::segment_of(addr_t symbol_vaddr) const
{
    off_t seg_idx = segment_index(symbol_vaddr - virtual_load_base);
    if(seg_idx < 0) return nullptr;
    return segments + seg_idx;
}
bool elf64_shared_object::xvalidate()
{
    if(ehdr().e_machine != EM_AMD64 || ehdr().e_ident[elf_ident_encoding_idx] != ED_LSB) { panic("not an object for the correct machine"); return false; }
    if(ehdr().e_ident[elf_ident_class_idx] != EC_64 ) { panic("32-bit object files are not supported"); return false; }
    if(ehdr().e_type != ET_DYN) { panic("not a shared object"); return false; }
    if(!ehdr().e_phnum) { panic("no program headers present"); return false; }
    return true;
}
bool elf64_shared_object::load_segments()
{
    bool have_loads = false;
    if(ehdr().e_entry) { entry = virtual_load_base.plus(ehdr().e_entry); }
    frame_enter();
    for(size_t n = 0; n < ehdr().e_phnum; n++)
    {
        elf64_phdr const& h = phdr(n);
        if(!h.p_memsz) continue;
        if(is_load(h))
        {
            addr_t addr = virtual_load_base.plus(h.p_vaddr);
            addr_t target = addr.trunc(h.p_align);
            size_t full_size = h.p_memsz + (addr - target);
            addr_t blk = kmm.allocate_user_block(full_size, target, h.p_align, is_write(h), is_exec(h));
            if(!blk) { kmm.exit_frame(); throw std::bad_alloc{}; }
            addr_t idmap(kmm.frame_translate(addr));
            size_t actual_size = kernel_memory_mgr::aligned_size(target, full_size);
            addr_t img_dat = segment_ptr(n);
            array_copy<uint8_t>(idmap, img_dat, h.p_filesz);
            if(h.p_memsz > h.p_filesz) { array_zero<uint8_t>(idmap.plus(h.p_filesz), static_cast<size_t>(h.p_memsz - h.p_filesz)); }
            new (std::addressof(segments[n])) program_segment_descriptor{ idmap, addr, static_cast<off_t>(h.p_offset), h.p_memsz, h.p_align, static_cast<elf_segment_prot>(0b100 | (is_write(h) ? 0b010 : 0) | (is_exec(h) ? 0b001 : 0)) };
            kmm.exit_frame();
            frame_tag->usr_blocks.emplace_back(blk, target, actual_size, is_write(h), is_exec(h));
            frame_enter();
            frame_tag->dynamic_extent = std::max(frame_tag->dynamic_extent, target.plus(actual_size).next_page_aligned());
            total_segment_size += actual_size;
            have_loads = true;
        }
        else new (std::addressof(segments[n])) program_segment_descriptor();
    }
    kmm.exit_frame();
    return have_loads;
}