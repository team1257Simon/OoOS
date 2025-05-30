#include "elf64_dynamic.hpp"
#include "kernel_mm.hpp"
#include "stdlib.h"
#include "algorithm"
#include "kdebug.hpp"
static std::allocator<program_segment_descriptor> sd_alloc{};
static std::allocator<uint32_t> w_alloc{};
static std::allocator<uint64_t> q_alloc{};
static std::allocator<elf64_rela> r_alloc{};
static std::alignval_allocator<elf64_dyn, std::align_val_t(PAGESIZE)> dynseg_alloc;
static bool is_object_rela(elf64_rela const& r) { return r.r_info.type == R_X86_64_GLOB_DAT; }
bool elf64_dynamic_object::load_preinit() { return true; /* stub; only applicable to executables */ }
addr_t elf64_dynamic_object::resolve_rela_target(elf64_rela const& r) const { return resolve(r.r_offset); }
addr_t elf64_dynamic_object::global_offset_table() const { return resolve(got_vaddr); }
addr_t elf64_dynamic_object::dyn_segment_ptr() const { return resolve(phdr(dyn_segment_idx).p_vaddr); }
size_t elf64_dynamic_object::to_image_offset(size_t offs) { for(size_t i = 0; i < ehdr().e_phnum; i++) { if(phdr(i).p_vaddr <= offs && phdr(i).p_vaddr + phdr(i).p_memsz > offs) return offs - (phdr(i).p_vaddr - phdr(i).p_offset); } return offs; }
elf64_dynamic_object::elf64_dynamic_object(addr_t start, size_t size) :
    elf64_object    ( start, size ),
    num_dyn_entries { 0UL },
    dyn_entries     { nullptr },
    num_plt_relas   { 0UL },
    plt_relas       { nullptr },
    got_vaddr       { 0UL },
    dyn_segment_idx { 0UL },
    relocations     {},
    object_relas    {},
    dependencies    {},
    ld_paths        {},
    init_array      {},
    fini_array      {},
    init_fn         { 0UL },
    fini_fn         { 0UL },
    init_array_ptr  { 0UL },
    fini_array_ptr  { 0UL },
    init_array_size { 0UL },
    fini_array_size { 0UL },
    symbol_index    { symstrtab, symtab }
                    {}
elf64_dynamic_object::elf64_dynamic_object(file_node* n) :
    elf64_object    ( n ),
    num_dyn_entries { 0UL },
    dyn_entries     { nullptr },
    num_plt_relas   { 0UL },
    plt_relas       { nullptr },
    got_vaddr       { 0UL },
    dyn_segment_idx { 0UL },
    relocations     {},
    object_relas    {},
    dependencies    {},
    ld_paths        {},
    init_array      {},
    fini_array      {},
    init_fn         { 0UL },
    fini_fn         { 0UL },
    init_array_ptr  { 0UL },
    fini_array_ptr  { 0UL },
    init_array_size { 0UL },
    fini_array_size { 0UL },
    symbol_index    { symstrtab, symtab } 
                    {}
elf64_dynamic_object::~elf64_dynamic_object()
{
    symbol_index.destroy_if_present();
    if(segments && num_seg_descriptors) { release_segments(); sd_alloc.deallocate(segments, num_seg_descriptors); }
    if(dyn_entries) dynseg_alloc.deallocate(dyn_entries, num_dyn_entries);
    if(plt_relas) free(plt_relas);
}
static bool is_dynamic_relocation(elf64_rela const& r)
{
    elf_rel_type t = r.r_info.type;
    if(t == R_X86_64_JUMP_SLOT || t == R_X86_64_GLOB_DAT || t == R_X86_64_TPOFF64 || t == R_X86_64_TPOFF32 || t == R_X86_64_DPTMOD64 || t == R_X86_64_DTPOFF64 || t == R_X86_64_DTPOFF32 || t >= R_X86_64_GOTPC32_TLSDESC) return true;
    else return false;
}
static bool recognize_rela_type(elf64_rela const& r)
{
    elf_rel_type t = r.r_info.type;
    if(t == R_X86_64_64 || t == R_X86_64_RELATIVE || t == R_X86_64_DTPOFF64 || t == R_X86_64_JUMP_SLOT || t == R_X86_64_GLOB_DAT) return true;
    else return false;
}
void elf64_dynamic_object::apply_relocations()
{
    reloc_sym_resolve reloc_symbol_fn = std::bind(&elf64_dynamic_object::resolve_rela_sym, this, std::placeholders::_1, std::placeholders::_2);
    reloc_tar_resolve reloc_target_fn = std::bind(&elf64_dynamic_object::resolve_rela_target, this, std::placeholders::_1);
    // Assuming everything works as planned (decently tall ask), the relocation value will be calculated here for each relocation.
    for(elf64_relocation const& r : relocations)
    {
        if(is_dynamic_relocation(r.rela_entry)) continue; // these will be resolved later
        reloc_result result = r(reloc_symbol_fn, reloc_target_fn);
        addr_t phys_target  = get_frame()->translate(result.target);
        if(phys_target && result.value) phys_target.assign(result.value);
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
    default:
        return 0UL;
    }
}
bool elf64_dynamic_object::xload()
{
    // allocate the array to have enough space for all indices, but the non-load segments will be zeroed
    bool success = true;
    process_headers();
    if(!load_segments()) { panic("object contains no loadable segments"); success = false; }
    else if(!load_syms()) { panic("failed to load symbols"); success = false; }
    else if(!post_load_init()) { panic("failed to initialize program image"); success = false; }
    // other segments and sections, if/when needed, can be handled here; free the rest up
    cleanup();
    return success;
}
void elf64_dynamic_object::process_dt_relas()
{
    size_t rela_offs        = 0, rela_sz = 0;
    size_t unrec_rela_ct    = 0;
    for(size_t i = 0; i < num_dyn_entries; i++)
    {
        if(dyn_entries[i].d_tag == DT_RELA)         rela_offs   = dyn_entries[i].d_ptr;
        else if(dyn_entries[i].d_tag == DT_RELASZ)  rela_sz     = dyn_entries[i].d_val;
        if(rela_sz && rela_offs) break;
    }
    if(rela_sz && rela_offs)
    {
        elf64_rela* rela    = img_ptr(to_image_offset(rela_offs));
        size_t n            = rela_sz / sizeof(elf64_rela);
        for(size_t i = 0; i < n; i++)
        {
            if(is_object_rela(rela[i])) { object_relas.push_back(rela[i]); }
            else if(is_dynamic_relocation(rela[i])) continue; // these are computed / applied by the dynamic linker
            else if(!recognize_rela_type(rela[i])) { unrec_rela_ct++; }
            else if(rela[i].r_info.sym_index) { elf64_sym const* s = symtab + rela[i].r_info.sym_index; if(s) relocations.emplace_back(*s, rela[i]); }
            else relocations.emplace_back(rela[i]);
        }
    }
    if(unrec_rela_ct) { xklog("W: " + std::to_string(unrec_rela_ct) + " unrecognized relocation types"); debug_print_num(to_image_offset(rela_offs)); }
}
bool elf64_dynamic_object::post_load_init()
{
    apply_relocations();
    if(got_vaddr)
    {
        addr_t* got = get_frame()->translate(global_offset_table());
        if(__builtin_expect(got != nullptr, true)) { got[1] = this; }
        else { panic("GOT pointer is non-null but is invalid"); return false; }
    }
    try
    {
        std::vector<addr_t> fini_reverse_array{};
        if(__builtin_expect(!load_preinit(), false)) return false;
        if(init_fn) { init_array.push_back(resolve(init_fn)); }
        if(fini_fn) { fini_reverse_array.push_back(resolve(fini_fn)); }
        if(init_array_size && init_array_ptr) 
        {
            addr_t init_ptrs_vaddr  = resolve(init_array_ptr);
            uintptr_t* init_ptrs    = get_frame()->translate(init_ptrs_vaddr);
            if(__builtin_expect(!init_ptrs, false)) { panic("initialization array pointer is non-null but is invalid"); return false; }
            for(size_t i = 0; i < init_array_size; i++) { init_array.push_back(addr_t(init_ptrs[i])); }
        }
        if(fini_array_size && fini_array_ptr)
        {
            addr_t fini_ptrs_vaddr  = resolve(fini_array_ptr);
            uintptr_t* fini_ptrs    = get_frame()->translate(fini_ptrs_vaddr);
            if(__builtin_expect(!fini_ptrs, false)) { panic("finalization array pointer is non-null but is invalid"); return false; }
            for(size_t i = 0; i < fini_array_size; i++) { fini_reverse_array.push_back(addr_t(fini_ptrs[i])); } 
        }
        if(!fini_reverse_array.empty()) { fini_array.push_back(fini_reverse_array.rend(), fini_reverse_array.rbegin()); }
        return true;
    }
    catch(std::exception& e) { panic(e.what()); return false; }
}
void elf64_dynamic_object::process_dyn_entry(size_t i)
{
    switch(dyn_entries[i].d_tag)
    {
    case DT_INIT:
        init_fn         = dyn_entries[i].d_ptr;
        break;
    case DT_FINI:
        fini_fn         = dyn_entries[i].d_ptr;
        break;
    case DT_INIT_ARRAY:
        init_array_ptr  = dyn_entries[i].d_ptr;
        break;
    case DT_FINI_ARRAY:
        fini_array_ptr  = dyn_entries[i].d_ptr;
        break;
    case DT_INIT_ARRAYSZ:
        init_array_size = (dyn_entries[i].d_val / sizeof(addr_t));
        break;
    case DT_FINI_ARRAYSZ:
        fini_array_size = (dyn_entries[i].d_val / sizeof(addr_t));
        break;
    case DT_NEEDED:
        dependencies.emplace_back(symstrtab[dyn_entries[i].d_val]);
        break;
    case DT_RUNPATH:
        ld_paths        = std::move(std::ext::split(std::forward<std::string>(symstrtab[dyn_entries[i].d_val]), ':'));
        break;
    default:
        break;
    }
}
bool elf64_dynamic_object::load_syms()
{
    if(!elf64_object::load_syms()) { panic("no symbol table present"); return false; }
    bool have_dyn = false;
    for(size_t n = 0; n < ehdr().e_phnum && !have_dyn; n++)
    {
        elf64_phdr const& ph = phdr(n);
        if(is_dynamic(ph))
        {
            num_dyn_entries = ph.p_filesz / sizeof(elf64_dyn);
            dyn_entries     = dynseg_alloc.allocate(num_dyn_entries);
            array_copy<elf64_dyn>(dyn_entries, segment_ptr(n), num_dyn_entries);
            have_dyn        = true;
            dyn_segment_idx = n;
        }
    }
    if(!have_dyn) { panic("no dynamic segment present"); return false; }
    process_dt_relas();
    bool have_ht = false;
    for(size_t i = 0; i < num_dyn_entries; i++)
    {
        if(dyn_entries[i].d_tag == DT_GNU_HASH)
        {
            addr_t ht_addr          = img_ptr(to_image_offset(dyn_entries[i].d_ptr));
            elf64_gnu_htbl::hdr* h  = ht_addr;
            uint64_t* bloom_filter  = q_alloc.allocate(h->maskwords);
            uint64_t* og_filter     = ht_addr.plus(sizeof(elf64_gnu_htbl::hdr));
            uint32_t* buckets       = w_alloc.allocate(h->nbucket);
            uint32_t* og_buckets    = addr_t(og_filter).plus(h->maskwords * sizeof(uint64_t));
            size_t n_hvals          = static_cast<size_t>((symtab.total_size / symtab.entry_size) - h->symndx);
            uint32_t* hval_array    = w_alloc.allocate(n_hvals);
            uint32_t* og_hvals      = addr_t(og_buckets).plus(h->nbucket * sizeof(uint32_t));
            array_copy(bloom_filter, og_filter, h->maskwords);
            array_copy(buckets, og_buckets, h->nbucket);
            array_copy(hval_array, og_hvals, n_hvals);
            new(std::addressof(symbol_index.htbl)) elf64_gnu_htbl
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
            have_ht = true;
        }
        else process_dyn_entry(i);
    }
    if(__builtin_expect((!init_array_ptr ^ !init_array_size) || (!fini_array_ptr ^ !fini_array_size), false)) { panic("mismatched init and/or fini array entries"); return false; }
    if(have_ht) return process_got();
    panic("Symbol hash data missing"); 
    return false;
}
std::pair<elf64_sym, addr_t> elf64_dynamic_object::resolve_by_name(std::string const& symbol) const
{
    if(__builtin_expect(!segments || !num_seg_descriptors || !symbol_index, false)) { panic("cannot load symbols from an uninitialized object"); return std::make_pair(elf64_sym(), nullptr); }
    elf64_sym const* sym = symbol_index[symbol];
    return sym ? std::make_pair(*sym, resolve(*sym)) : std::make_pair(elf64_sym(), nullptr);
}
bool elf64_dynamic_object::process_got()
{
    size_t got_offs = 0, rela_offs = 0, rela_sz = 0;
    for(size_t i = 0; i < num_dyn_entries; i++)
    {
        if(dyn_entries[i].d_tag == DT_JMPREL)           rela_offs   = dyn_entries[i].d_ptr;
        else if(dyn_entries[i].d_tag == DT_PLTGOT)      got_offs    = dyn_entries[i].d_ptr;
        else if(dyn_entries[i].d_tag == DT_PLTRELSZ)    rela_sz     = dyn_entries[i].d_val;
        if(got_offs && rela_offs && rela_sz) break;
    }
    if(got_offs && rela_offs && rela_sz) 
    {
        got_vaddr           = got_offs;
        num_plt_relas       = rela_sz / sizeof(elf64_rela);
        plt_relas           = r_alloc.allocate(num_plt_relas);
        if(__builtin_expect(!plt_relas, false)) { panic("failed to allocate rela array"); return false; }
        elf64_rela* rela    = img_ptr(to_image_offset(rela_offs));
        array_copy(plt_relas, rela, num_plt_relas);
        return true;
    }
    else
    {
        for(size_t i = 0; i < ehdr().e_shnum; i++)
        {
            const char* name    = shstrtab[shdr(i).sh_name];
            if(std::strncmp(name, ".got.plt", 8) != 0) continue;
            got_vaddr           = shdr(i).sh_addr;
            return true; 
        }
    }
    got_vaddr = 0UL;
    return true;
}
void elf64_dynamic_object::set_resolver(addr_t ptr)
{
    if(got_vaddr)
    {
        addr_t got_table = get_frame()->translate(global_offset_table());
        if(!got_table) return;
        got_table.plus(sizeof(addr_t) * 2Z).assign(ptr);
    }
}
elf64_dynamic_object::elf64_dynamic_object(elf64_dynamic_object const& that) : 
    elf64_object        { that },
    num_dyn_entries     { that.num_dyn_entries },
    dyn_entries         { dynseg_alloc.allocate(num_dyn_entries) },
    num_plt_relas       { that.num_plt_relas },
    plt_relas           { that.plt_relas ? r_alloc.allocate(that.num_plt_relas) : nullptr },
    got_vaddr           { that.got_vaddr },
    dyn_segment_idx     { that.dyn_segment_idx },
    relocations         { that.relocations },
    dependencies        { that.dependencies },
    ld_paths            { that.ld_paths },
    init_array          { that.init_array },
    fini_array          { that.fini_array },
    init_fn             { that.init_fn },
    fini_fn             { that.fini_fn },
    init_array_ptr      { that.init_array_ptr },
    fini_array_ptr      { that.fini_array_ptr },
    init_array_size     { that.init_array_size },
    fini_array_size     { that.fini_array_size },
    symbol_index        { symstrtab, symtab, elf64_gnu_htbl{ .header{ that.symbol_index.htbl.header } } }
{
    symbol_index.htbl.bloom_filter_words    = q_alloc.allocate(symbol_index.htbl.header.maskwords);
    symbol_index.htbl.buckets               = w_alloc.allocate(symbol_index.htbl.header.nbucket);
    size_t nhash                            = static_cast<size_t>((symtab.total_size / symtab.entry_size) - symbol_index.htbl.header.symndx);
    symbol_index.htbl.hash_value_array      = w_alloc.allocate(nhash);
    array_copy(symbol_index.htbl.bloom_filter_words, that.symbol_index.htbl.bloom_filter_words, symbol_index.htbl.header.maskwords);
    array_copy(symbol_index.htbl.buckets, that.symbol_index.htbl.buckets, symbol_index.htbl.header.nbucket);
    array_copy(symbol_index.htbl.hash_value_array, that.symbol_index.htbl.hash_value_array, nhash);
    if(that.plt_relas) array_copy(this->plt_relas, that.plt_relas, num_plt_relas);
}
elf64_dynamic_object::elf64_dynamic_object(elf64_dynamic_object&& that) : 
    elf64_object        { std::move(that) },
    num_dyn_entries     { that.num_dyn_entries },
    dyn_entries         { that.dyn_entries },
    num_plt_relas       { that.num_plt_relas },
    plt_relas           { that.plt_relas },
    got_vaddr           { that.got_vaddr },
    dyn_segment_idx     { that.dyn_segment_idx },
    relocations         { std::move(that.relocations) },
    object_relas        { std::move(that.object_relas) },
    dependencies        { std::move(that.dependencies) },
    ld_paths            { std::move(that.ld_paths) },
    init_array          { std::move(that.init_array) },
    fini_array          { std::move(that.fini_array) },
    init_fn             { that.init_fn },
    fini_fn             { that.fini_fn },
    init_array_ptr      { that.init_array_ptr },
    fini_array_ptr      { that.fini_array_ptr },
    init_array_size     { that.init_array_size },
    fini_array_size     { that.fini_array_size },
    symbol_index        { symstrtab, symtab, elf64_gnu_htbl{ std::move(that.symbol_index.htbl) } }
{
    that.symbol_index.htbl.bloom_filter_words   = nullptr;
    that.symbol_index.htbl.buckets              = nullptr;
    that.symbol_index.htbl.hash_value_array     = nullptr;
    that.plt_relas                              = nullptr;
}