#include "elf64_shared.hpp"
#include "stdexcept"
constexpr size_t mw_bits = sizeof(uint64_t) * CHAR_BIT;
constexpr uint64_t shared_magic = 0x420B1A2E17;
const char* empty_name = "";
static const char* find_so_name(addr_t image_start);
addr_t elf64_shared_object::resolve(uint64_t offs) const { return virtual_load_base.plus(offs); }
addr_t elf64_shared_object::resolve(elf64_sym const& sym) const { return virtual_load_base.plus(sym.st_value); }
bool is_valid_handle(elf64_shared_object const& so) { return so.so_handle_magic == shared_magic; }
void elf64_shared_object::xrelease() { if(frame_tag) { for(block_descr& blk : segment_blocks()) { frame_tag->drop_block(blk); } } }
elf64_shared_object::~elf64_shared_object() = default; // TODO: call the destructors for loaded objects if applicable
elf64_shared_object::elf64_shared_object(file_node* n, uframe_tag* frame) :
    elf64_object            ( n ),
    elf64_dynamic_object    ( n ),
    so_handle_magic         { shared_magic },
    soname                  ( find_so_name(img_ptr()) ),
    virtual_load_base       { frame->dynamic_extent },
    frame_tag               { frame },
    ref_count               { 1UL },
    sticky                  { false }
                            {}
elf64_shared_object::elf64_shared_object(elf64_shared_object&& that) : 
    elf64_object            ( std::move(that) ),
    elf64_dynamic_object    ( std::move(that) ),
    so_handle_magic         { shared_magic },
    soname                  ( std::move(that.soname) ),
    virtual_load_base       { that.virtual_load_base },
    frame_tag               { that.frame_tag },
    ref_count               { that.ref_count },
    sticky                  { that.sticky }
                            { that.frame_tag = nullptr; }
static const char* find_so_name(addr_t image_start)
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
            if(!(strtab_off && name_off)) return empty_name;
            else return image_start.plus(strtab_off + name_off);
        }
    }
    return empty_name;
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
    if(ehdr().e_ident[elf_ident_class_idx] != EC_64 ) { panic("32-bit object files are not yet supported"); return false; }
    if(ehdr().e_type != ET_DYN) { panic("object is not a shared library"); return false; }
    if(!ehdr().e_phnum) { panic("no program headers present"); return false; }
    return true;
}
bool elf64_shared_object::load_segments()
{
    bool have_loads = false;
    kmm.enter_frame(frame_tag);
    for(size_t n = 0; n < ehdr().e_phnum; n++)
    {
        elf64_phdr const& h = phdr(n);
        if(!h.p_memsz) continue;
        if(is_load(h))
        {
            addr_t target = virtual_load_base.plus(h.p_vaddr);
            addr_t blk = kmm.allocate_user_block(h.p_memsz, target, h.p_align, is_write(h), is_exec(h));
            if(!blk) { kmm.exit_frame(); throw std::bad_alloc{}; }
            addr_t idmap(kmm.translate_vaddr_in_current_frame(target));
            size_t actual_size = kernel_memory_mgr::page_aligned_region_size(target, h.p_memsz);
            addr_t img_dat = segment_ptr(n);
            array_copy<uint8_t>(idmap, img_dat, h.p_filesz);
            if(h.p_memsz > h.p_filesz) { array_zero<uint8_t>(idmap.plus(h.p_filesz), static_cast<size_t>(h.p_memsz - h.p_filesz)); }
            new (std::addressof(segments[n])) program_segment_descriptor{ idmap, target, static_cast<off_t>(h.p_offset), h.p_memsz, h.p_align, static_cast<elf_segment_prot>(0b100 | (is_write(h) ? 0b010 : 0) | (is_exec(h) ? 0b001 : 0)) };
            frame_tag->usr_blocks.emplace_back(blk, target, actual_size, is_write(h), is_exec(h));
            frame_tag->dynamic_extent = std::max(frame_tag->dynamic_extent, target.plus(actual_size));
            have_loads = true;
        }
    }
    kmm.exit_frame();
    return have_loads;
}
bool elf64_shared_object::post_load_init()
{
    kmm.enter_frame(frame_tag);
    elf64_phdr const& dyn_ph = phdr(dyn_segment_idx);
    kmm.map_to_current_frame(std::forward<std::vector<block_descr>>({ { dyn_entries, addr_t(dyn_ph.p_vaddr), num_dyn_entries * sizeof(elf64_dyn), is_write(dyn_ph), is_exec(dyn_ph) } }));
    apply_relocations();
    addr_t* got = reinterpret_cast<addr_t*>(kmm.translate_vaddr_in_current_frame(global_offset_table()));
    got[1] = global_offset_table();
    // TODO: also put the address of the resolve-symbol function in the GOT
    kmm.exit_frame();
    try
    {
        std::vector<addr_t> fini_reverse_array{};
        if(init_fn) { init_array.push_back(resolve(init_fn)); }
        if(fini_fn) { fini_reverse_array.push_back(resolve(fini_fn)); }
        if(init_array_size && init_array_ptr) 
        {
            addr_t init_ptrs_vaddr = resolve(init_array_ptr);
            kmm.enter_frame(frame_tag);
            uintptr_t* init_ptrs = reinterpret_cast<uintptr_t*>(kmm.translate_vaddr_in_current_frame(init_ptrs_vaddr));
            kmm.exit_frame();
            for(size_t i = 0; i < init_array_size; i++) { init_array.push_back(addr_t(init_ptrs[i])); }
        }
        if(fini_array_size && fini_array_ptr) 
        {
            addr_t fini_ptrs_vaddr = resolve(fini_array_ptr);
            kmm.enter_frame(frame_tag);
            uintptr_t* fini_ptrs = reinterpret_cast<uintptr_t*>(kmm.translate_vaddr_in_current_frame(fini_ptrs_vaddr)); 
            kmm.exit_frame();
            for(size_t i = 0; i < fini_array_size; i++) { fini_reverse_array.push_back(addr_t(fini_ptrs[i])); } 
        }
        if(!fini_reverse_array.empty()) { fini_array.push_back(fini_reverse_array.rend(), fini_reverse_array.rbegin()); }
        init_array.push_back(nullptr);
        fini_array.push_back(nullptr);
        return true;
    }
    catch(std::exception& e) { panic(e.what()); return false; }
}