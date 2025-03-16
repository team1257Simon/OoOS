#include "elf64_dynamic.hpp"
#include "kernel_mm.hpp"
#include "stdlib.h"
static std::allocator<program_segment_descriptor> sd_alloc{};
static std::allocator<uint32_t> w_alloc{};
static std::allocator<uint64_t> q_alloc{};
static std::alignval_allocator<elf64_dyn, std::align_val_t(PAGESIZE)> dynseg_alloc;
elf64_dynamic_object::elf64_dynamic_object(file_node *n) : elf64_object{ n }, symbol_index{ symstrtab, symtab } {}
elf64_dynamic_object::~elf64_dynamic_object()
{
    symbol_index.destroy_if_present();
    if(segments && num_seg_descriptors) { for(size_t i = 0; i < num_seg_descriptors; i++) { kernel_memory_mgr::get().deallocate_block(segments[i].absolute_addr, segments[i].size); } sd_alloc.deallocate(segments, num_seg_descriptors); } 
    if(dyn_entries) dynseg_alloc.deallocate(dyn_entries, num_dyn_entries);
    if(got_entry_ptrs) free(got_entry_ptrs);
}
bool elf64_dynamic_object::xload()
{
        // allocate the array to have enough space for all indices, but the non-load segments will be zeroed
        num_seg_descriptors = ehdr().e_phnum;
        segments = sd_alloc.allocate(num_seg_descriptors);
        bool success = true;
        if(!load_syms()) { panic("no symbol table links present"); success = false; goto end; }
        if(!load_segments()) { panic("object contains no loadable segments"); success = false; goto end; }
        if(!load_dynamic_syms()) { panic("failed to load dynamic symbol index"); success = false; goto end; }
        // other segments and sections, if/when needed, can be handled here; free the rest up
    end:
        cleanup();
        return success;
}
bool elf64_dynamic_object::load_dynamic_syms()
{
    bool have_dyn = false;
    for(size_t n = 0; n < ehdr().e_phnum && !have_dyn; n++)
    {
        if(is_dynamic(phdr(n)))
        {
            this->num_dyn_entries = phdr(n).p_filesz / sizeof(elf64_dyn);
            this->dyn_entries = dynseg_alloc.allocate(num_dyn_entries);
            array_copy<elf64_dyn>(dyn_entries, segment_ptr(n), num_dyn_entries);
            have_dyn = true;
        }
    }
    if(!have_dyn) { panic("no dynamic segment present"); return false; }
    for(size_t i = 0; i < num_dyn_entries; i++)
    {
        if(dyn_entries[i].d_tag == DT_GNU_HASH)
        {
            addr_t ht_addr = img_ptr(dyn_entries[i].d_ptr);
            elf64_gnu_htbl::hdr* h = ht_addr;
            uint64_t* bloom_filter = q_alloc.allocate(h->maskwords);
            uint64_t* og_filter = ht_addr.plus(sizeof(elf64_gnu_htbl::hdr));
            uint32_t* buckets = w_alloc.allocate(h->nbucket);
            uint32_t* og_buckets = addr_t(og_filter).plus(h->maskwords * sizeof(uint64_t));
            size_t n_hvals = static_cast<size_t>((symtab.total_size / symtab.entry_size) - h->symndx);
            uint32_t* hval_array = w_alloc.allocate(n_hvals);
            uint32_t* og_hvals = addr_t(og_buckets).plus(h->nbucket * sizeof(uint32_t));
            array_copy(bloom_filter, og_filter, h->maskwords);
            array_copy(buckets, og_buckets, h->nbucket);
            array_copy(hval_array, og_hvals, n_hvals);
            new (std::addressof(symbol_index.htbl)) elf64_gnu_htbl
            { 
                    .header
                    { 
                        .nbucket    { h->nbucket }, 
                        .symndx     { h->symndx }, 
                        .maskwords  { h->maskwords }, 
                        .shift2     { h->shift2 } 
                    }, 
                    .bloom_filter_words { bloom_filter }, 
                    .buckets            { buckets }, 
                    .hash_value_array   { hval_array } 
            };
            return process_got();
        }
    }
    panic("required data missing"); 
    return false; 
}
bool elf64_dynamic_object::process_got()
{
    got_entry_ptrs = std::allocator<addr_t>().allocate(symtab.total_size / symtab.entry_size);
    size_t got_offs = 0, rela_offs = 0, rela_sz = 0;
    for(size_t i = 0; i < num_dyn_entries; i++)
    {
        if(dyn_entries[i].d_tag == DT_JMPREL) rela_offs = dyn_entries[i].d_ptr;
        else if(dyn_entries[i].d_tag == DT_PLTGOT) got_offs = dyn_entries[i].d_ptr;
        else if(dyn_entries[i].d_tag == DT_PLTRELSZ) rela_sz = dyn_entries[i].d_val;
        if(got_offs && rela_offs && rela_sz) break;
    }
    if(!got_offs) { for(size_t i = 0; i < ehdr().e_shnum; i++) { if(!std::strncmp(".got", shstrtab[shdr(i).sh_name], 4)) { got_offs = shdr(i).sh_addr; break; } } }
    if(!rela_offs || !rela_sz) { for(size_t i = 0; i < ehdr().e_shnum; i++) { if(shdr(i).sh_type == ST_RELA) { if(!std::strncmp(".rela.plt", shstrtab[shdr(i).sh_name], 9)) { rela_offs = shdr(i).sh_offset; rela_sz = shdr(i).sh_size; break; } } } }
    if(!(got_offs && rela_offs && rela_sz)) { panic("missing got info"); return false; }
    program_segment_descriptor const* got_seg = segment_of(got_offs);
    if(!got_seg) { panic("got is in an invalid segment"); return false; }
    uint64_t got_v_base = got_seg->obj_offset;
    size_t n_entries = rela_sz / sizeof(elf64_rela);
    elf64_rela* rela_entries = img_ptr(rela_offs);
    for(size_t i = 0; i < n_entries; i++) if(rela_entries[i].r_info.type == R_X86_64_GLOB_DAT || rela_entries[i].r_info.type == R_X86_64_JUMP_SLOT) got_entry_ptrs[rela_entries[i].r_info.sym_index] = got_seg->absolute_addr.plus(rela_entries[i].r_offset - got_v_base);
    return true;
}