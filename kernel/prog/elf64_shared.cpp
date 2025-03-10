#include "elf64_shared.hpp"
#include "stdlib.h"
#include "stdexcept"
#include "kernel_mm.hpp"
constexpr size_t mw_bits = sizeof(uint64_t) * CHAR_BIT;
static std::elf64_gnu_hash __hash{};
static std::allocator<shared_segment_descriptor> sd_alloc{};
static std::allocator<uint32_t> w_alloc{};
static std::allocator<uint64_t> q_alloc{};
static std::alignval_allocator<elf64_dyn, std::align_val_t(PAGESIZE)> dynseg_alloc;
elf64_shared_object::elf64_shared_object(file_node *n) : elf64_object{ n } {}
elf64_shared_object::~elf64_shared_object()
{
    __symbol_index.destroy_if_present();
    if(__segments && __num_seg_descriptors) { for(size_t i = 0; i < __num_seg_descriptors; i++) { kernel_memory_mgr::get().deallocate_block(__segments[i].absolute_addr, __segments[i].size); } sd_alloc.deallocate(__segments, __num_seg_descriptors); } 
    if(__dyn_entries) dynseg_alloc.deallocate(__dyn_entries, __num_dyn_entries);
}
elf64_sym const* elf64_dynsym_index::operator[](std::string const& str) const
{
    if(!*this) return nullptr;
    uint32_t hash = __hash(str.c_str());
    uint32_t i = htbl.buckets[hash % htbl.header.nbucket];
    // TODO get bloom filter to actually work
    if(!i) return nullptr;
    uint32_t hash_val = hash & ~1U;
    uint32_t other_hash = htbl.hash_value_array[i - htbl.header.symndx];
    for(elf64_sym const* sym = symtab + i; ; ++i, sym = symtab + i, other_hash = htbl.hash_value_array[i - htbl.header.symndx]) { if(hash_val == (other_hash & ~1U) && !std::strcmp(str.c_str(), strtab[sym->st_name])) return sym; if(other_hash & 1U) break; }
    return nullptr;
}
void elf64_dynsym_index::destroy_if_present()
{
    if(htbl.bloom_filter_words) free(htbl.bloom_filter_words);
    if(htbl.buckets) free(htbl.buckets);
    if(htbl.hash_value_array) free(htbl.hash_value_array);
    if(symtab.data) free(symtab.data);
    if(strtab.data) free(addr_t(strtab.data));
}
bool elf64_shared_object::xvalidate()
{
    if(ehdr_ptr()->e_machine != EM_AMD64 || ehdr_ptr()->e_ident[elf_ident_encoding_idx] != ED_LSB) { panic("not an object for the correct machine"); return false; }
    if(ehdr_ptr()->e_ident[elf_ident_class_idx] != EC_64 ) { panic("32-bit object files are not yet supported"); return false; }
    if(ehdr_ptr()->e_type != ET_DYN) { panic("object is not a shared library"); return false; }
    if(!ehdr_ptr()->e_phnum) { panic("no program headers present"); return false; }
    return true;
}
bool elf64_shared_object::xload()
{
    // allocate the array to have enough space for all indices, but the non-load segments will be zeroed
    this->__num_seg_descriptors = ehdr().e_phnum;
    this->__segments = sd_alloc.allocate(__num_seg_descriptors);
    bool have_dyn = false;
    for(size_t n = 0; n < ehdr().e_phnum; n++)
    {
        if(!phdr(n).p_memsz) continue;
        if(is_load(phdr(n)))
        {
            addr_t blk = kernel_memory_mgr::get().allocate_user_block(phdr(n).p_memsz, nullptr, phdr(n).p_align, is_write(phdr(n)), is_exec(phdr(n)));
            addr_t img_dat = segment_ptr(n);    
            if(!blk) { throw std::bad_alloc{}; }
            array_copy<uint8_t>(blk, img_dat, phdr(n).p_filesz);
            elf_segment_prot perms = static_cast<elf_segment_prot>(0b100 | (is_write(phdr(n)) ? 0b010 : 0) | (is_exec(phdr(n)) ? 0b001 : 0));
            if(phdr(n).p_memsz > phdr(n).p_filesz) { size_t diff = static_cast<size_t>(phdr(n).p_memsz - phdr(n).p_filesz); array_zero<uint8_t>(blk.plus(phdr(n).p_filesz), diff); }
            new (std::addressof(__segments[n])) shared_segment_descriptor{ blk, static_cast<off_t>(phdr(n).p_vaddr), phdr(n).p_memsz, perms };
        }
        else if(is_dynamic(phdr(n)))
        {
            this->__num_dyn_entries = phdr(n).p_filesz / sizeof(elf64_dyn);
            this->__dyn_entries = dynseg_alloc.allocate(__num_dyn_entries);
            array_copy<elf64_dyn>(__dyn_entries, segment_ptr(n), __num_dyn_entries);
            have_dyn = true;
        }
    }
    if(!have_dyn) { panic("no dynamic segment present"); return false; }
    elf64_shdr const* strtab_shdr = nullptr;
    elf64_shdr const* symtab_shdr = nullptr;
    for(size_t i = 0; i < ehdr().e_shnum; i++)
    {
        if(shdr(i).sh_type == ST_DYNSYM)
        {
            symtab_shdr = shdr_ptr(i);
            strtab_shdr = shdr_ptr(symtab_shdr->sh_link);
            break;
        }
    }
    if(!symtab_shdr) { panic("no dynamic symbol table"); return false; }
    bool have_index = false;
    for(size_t i = 0; i < __num_dyn_entries && !have_index; i++)
    {
        if(__dyn_entries[i].d_tag == DT_GNU_HASH)
        {
            addr_t ht_addr = img_ptr(__dyn_entries[i].d_ptr);
            elf64_gnu_htbl::hdr* h = ht_addr;
            uint64_t* bloom_filter = q_alloc.allocate(h->maskwords);
            uint64_t* og_filter = ht_addr.plus(sizeof(elf64_gnu_htbl::hdr));
            uint32_t* buckets = w_alloc.allocate(h->nbucket);
            uint32_t* og_buckets = addr_t(og_filter).plus(h->maskwords * sizeof(uint64_t));
            size_t n_hvals = static_cast<size_t>((symtab_shdr->sh_size / symtab_shdr->sh_entsize) - h->symndx);
            uint32_t* hval_array = w_alloc.allocate(n_hvals);
            uint32_t* og_hvals = addr_t(og_buckets).plus(h->nbucket * sizeof(uint32_t));
            array_copy(bloom_filter, og_filter, h->maskwords);
            array_copy(buckets, og_buckets, h->nbucket);
            array_copy(hval_array, og_hvals, n_hvals);
            addr_t strtab_data = aligned_malloc(strtab_shdr->sh_size, strtab_shdr->sh_addralign);
            addr_t symtab_data = aligned_malloc(symtab_shdr->sh_size, symtab_shdr->sh_addralign);
            array_copy<char>(strtab_data, img_ptr(strtab_shdr->sh_offset), strtab_shdr->sh_size);
            array_copy<char>(symtab_data, img_ptr(symtab_shdr->sh_offset), symtab_shdr->sh_size);
            new (std::addressof(__symbol_index)) elf64_dynsym_index
            {
                .strtab { .total_size{ strtab_shdr->sh_size }, .data{ strtab_data } },
                .symtab { .total_size{ symtab_shdr->sh_size }, .entry_size{ symtab_shdr->sh_entsize }, .data{ symtab_data } },
                .htbl   
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
                }
            };
            have_index = true;
        }
    }
    if(!have_index) { panic("required data missing"); return false; }
    // other segments and sections, if/when needed, can be handled here; free the rest up
    cleanup();
    return true;
}
shared_segment_descriptor const *elf64_shared_object::segment_of(elf64_sym const *sym) const { for(size_t i = 0; i < __num_seg_descriptors; i++) { if(static_cast<uintptr_t>(__segments[i].obj_offset) <= sym->st_value && sym->st_value < static_cast<uintptr_t>(__segments[i].obj_offset + __segments[i].size)) return __segments + i; } return nullptr; }
shared_segment_descriptor const *elf64_shared_object::segment_of(std::string const& symbol) const { if(!__segments || !__num_seg_descriptors || !__symbol_index) { panic("cannot load symbols from an uninitialized object"); return nullptr; } return segment_of(__symbol_index[symbol]); }
addr_t elf64_shared_object::resolve(std::string const& symbol) const
{
    if(!__segments || !__num_seg_descriptors || !__symbol_index) { panic("cannot load symbols from an uninitialized object"); return nullptr; }
    elf64_sym const* sym = __symbol_index[symbol];
    shared_segment_descriptor const* sd = segment_of(sym);
    return (sd && sym) ? sd->absolute_addr.plus(static_cast<ptrdiff_t>(sym->st_value - sd->obj_offset)) : nullptr;
}
