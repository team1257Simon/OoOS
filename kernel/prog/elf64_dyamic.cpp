#include "elf64_dynamic.hpp"
#include "kernel_mm.hpp"
#include "stdlib.h"
static std::allocator<program_segment_descriptor> sd_alloc{};
static std::allocator<uint32_t> w_alloc{};
static std::allocator<uint64_t> q_alloc{};
static std::allocator<addr_t> p_alloc{};
static std::alignval_allocator<elf64_dyn, std::align_val_t(PAGESIZE)> dynseg_alloc;
addr_t elf64_dynamic_object::resolve_rela_target(elf64_rela const& r) const { return resolve(r.r_offset); }
elf64_dynamic_object::elf64_dynamic_object(file_node *n) : elf64_object{ n }, symbol_index{ symstrtab, symtab } {}
elf64_dynamic_object::~elf64_dynamic_object()
{
    symbol_index.destroy_if_present();
    if(segments && num_seg_descriptors) { for(size_t i = 0; i < num_seg_descriptors; i++) { kernel_memory_mgr::get().deallocate_block(segments[i].absolute_addr, segments[i].size); } sd_alloc.deallocate(segments, num_seg_descriptors); } 
    if(dyn_entries) dynseg_alloc.deallocate(dyn_entries, num_dyn_entries);
    if(got_entry_ptrs) free(got_entry_ptrs);
}
bool elf64_dynamic_object::recognize_rela_type(elf64_rela const& r)
{
    elf_rel_type t = r.r_info.type;
    if(t == R_X86_64_64 || t == R_X86_64_RELATIVE || t == R_X86_64_DTPOFF64 || t == R_X86_64_JUMP_SLOT || t == R_X86_64_GLOB_DAT) return true;
    else return false;
}
void elf64_dynamic_object::apply_relocations()
{
    // Assuming everything works as planned, the relocation value will be calculated here for each relocation.
    for(elf64_relocation const& r : relocations)
    {
        reloc_result result = r();
        if(result.target && result.value) result.target.ref<uint64_t>() = result.value;
        else klog("W: invalid relocation");
    }
}
uint64_t elf64_dynamic_object::resolve_rela_sym(elf64_sym const& s, elf64_rela const& r) const
{
    switch(r.r_info.type)
    {
    case R_X86_64_64:
        return resolve(s.st_value + r.r_addend);
    case R_X86_64_RELATIVE:
        return resolve(r.r_addend);
    case R_X86_64_DTPOFF64:
        return r.r_addend + s.st_value;
    case R_X86_64_JUMP_SLOT:
    case R_X86_64_GLOB_DAT:
        return resolve(s.st_value);
    default:
        return 0UL;
    }
}
bool elf64_dynamic_object::xload()
{
        // allocate the array to have enough space for all indices, but the non-load segments will be zeroed
        bool success = true;
        process_headers();
        if(!load_syms()) { panic("no symbol table links present"); success = false; goto end; }
        if(!load_segments()) { panic("object contains no loadable segments"); success = false; goto end; }
        if(!load_dynamic_syms()) { panic("failed to load dynamic symbol index"); success = false; goto end; }
        // other segments and sections, if/when needed, can be handled here; free the rest up
    end:
        cleanup();
        return success;
}
void elf64_dynamic_object::process_dynamic_relas()
{
    size_t rela_offs = 0, rela_sz = 0;
    for(size_t i = 0; i < num_dyn_entries; i++)
    {
        if(dyn_entries[i].d_tag == DT_RELA) rela_offs = dyn_entries[i].d_ptr;
        else if(dyn_entries[i].d_tag == DT_RELASZ) rela_sz = dyn_entries[i].d_val;
        if(rela_sz && rela_offs) break;
    }
    if(rela_sz && rela_offs)
    {
        elf64_rela* rela = img_ptr(rela_offs);
        size_t n = rela_sz / sizeof(elf64_rela);
        reloc_tar_resolve tres = std::bind(&elf64_dynamic_object::resolve_rela_target, this, std::placeholders::_1);
        reloc_sym_resolve sres = std::bind(&elf64_dynamic_object::resolve_rela_sym, this, std::placeholders::_1, std::placeholders::_2);
        for(size_t i = 0; i < n; i++)
        {
            if(!recognize_rela_type(rela[i])) { klog("W: unrecognized rela type"); continue; }
            if(rela[i].r_info.sym_index) { relocations.emplace_back(symtab[rela[i].r_info.sym_index], rela[i], sres, tres); }
            else relocations.emplace_back(rela[i], sres, tres);
        }
    }
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
    process_dynamic_relas();
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
    size_t got_offs = 0, rela_offs = 0, rela_sz = 0;
    for(size_t i = 0; i < num_dyn_entries; i++)
    {
        if(dyn_entries[i].d_tag == DT_JMPREL) rela_offs = dyn_entries[i].d_ptr;
        else if(dyn_entries[i].d_tag == DT_PLTGOT) got_offs = dyn_entries[i].d_ptr;
        else if(dyn_entries[i].d_tag == DT_PLTRELSZ) rela_sz = dyn_entries[i].d_val;
        if(got_offs && rela_offs && rela_sz) break;
    }
    if(!(got_offs && rela_offs && rela_sz)) { panic("missing got info"); return false; }
    got_entry_ptrs = p_alloc.allocate(symtab.total_size / symtab.entry_size);
    program_segment_descriptor const* got_seg = segment_of(got_offs);
    got_seg_index = static_cast<size_t>(got_seg - segments);
    if(!got_seg) { panic("got is in an invalid segment"); return false; }
    size_t n_entries = rela_sz / sizeof(elf64_rela);
    elf64_rela* rela_entries = img_ptr(rela_offs);
    reloc_tar_resolve tres = std::bind(&elf64_dynamic_object::resolve_rela_target, this, std::placeholders::_1);
    reloc_sym_resolve sres = std::bind(&elf64_dynamic_object::resolve_rela_sym, this, std::placeholders::_1, std::placeholders::_2);
    for(size_t i = 0; i < n_entries; i++)
    { 
        if(rela_entries[i].r_info.type == R_X86_64_GLOB_DAT || rela_entries[i].r_info.type == R_X86_64_JUMP_SLOT)
        { 
            got_entry_ptrs[rela_entries[i].r_info.sym_index] = to_segment_ptr(rela_entries[i].r_offset, *got_seg);
            elf64_sym const* s = symtab + rela_entries[i].r_info.sym_index;
            if(s) relocations.emplace_back(*s, rela_entries[i], sres, tres);
        }
    }
    return true;
}
elf64_dynamic_object::elf64_dynamic_object(elf64_dynamic_object const& that) : 
    elf64_object    { that },
    num_dyn_entries { that.num_dyn_entries },
    dyn_entries     { dynseg_alloc.allocate(num_dyn_entries) },
    got_entry_ptrs  { that.got_entry_ptrs ? p_alloc.allocate(that.symtab.total_size / that.symtab.entry_size) : nullptr },
    got_seg_index   { that.got_seg_index },
    relocations     { that.relocations.size() },
    symbol_index    { symstrtab, symtab, elf64_gnu_htbl{ .header{ that.symbol_index.htbl.header } } }
{
    symbol_index.htbl.bloom_filter_words = q_alloc.allocate(symbol_index.htbl.header.maskwords);
    symbol_index.htbl.buckets = w_alloc.allocate(symbol_index.htbl.header.nbucket);
    size_t nhash = static_cast<size_t>((symtab.total_size / symtab.entry_size) - symbol_index.htbl.header.symndx);
    symbol_index.htbl.hash_value_array =  w_alloc.allocate(nhash);
    array_copy(symbol_index.htbl.bloom_filter_words, that.symbol_index.htbl.bloom_filter_words, symbol_index.htbl.header.maskwords);
    array_copy(symbol_index.htbl.buckets, that.symbol_index.htbl.buckets, symbol_index.htbl.header.nbucket);
    array_copy(symbol_index.htbl.hash_value_array, that.symbol_index.htbl.hash_value_array, nhash);
    if(got_entry_ptrs) { for(size_t i = 0; i < (that.symtab.total_size / that.symtab.entry_size); i++) { if(that.got_entry_ptrs[i]) { this->got_entry_ptrs[i] = resegment_ptr(that.got_entry_ptrs[i], that.segments[that.got_seg_index], this->segments[this->got_seg_index]); } } }
    reloc_tar_resolve tres = std::bind(&elf64_dynamic_object::resolve_rela_target, this, std::placeholders::_1);
    reloc_sym_resolve sres = std::bind(&elf64_dynamic_object::resolve_rela_sym, this, std::placeholders::_1, std::placeholders::_2);
    for(elf64_relocation const& r : that.relocations) { this->relocations.emplace_back(r.symbol, r.rela_entry, sres, tres); }
}
elf64_dynamic_object::elf64_dynamic_object(elf64_dynamic_object && that) : 
    elf64_object    { std::move(that) },
    num_dyn_entries { that.num_dyn_entries },
    dyn_entries     { that.dyn_entries },
    got_entry_ptrs  { that.got_entry_ptrs },
    got_seg_index   { that.got_seg_index },
    relocations     { that.relocations.size() },
    symbol_index    { symstrtab, symtab, elf64_gnu_htbl{ std::move(that.symbol_index.htbl) } }
{
    that.symbol_index.htbl.bloom_filter_words = nullptr;
    that.symbol_index.htbl.buckets = nullptr;
    that.symbol_index.htbl.hash_value_array = nullptr;
    reloc_tar_resolve tres = std::bind(&elf64_dynamic_object::resolve_rela_target, this, std::placeholders::_1);
    reloc_sym_resolve sres = std::bind(&elf64_dynamic_object::resolve_rela_sym, this, std::placeholders::_1, std::placeholders::_2);
    for(elf64_relocation const& r : that.relocations) { this->relocations.emplace_back(r.symbol, r.rela_entry, sres, tres); }
}