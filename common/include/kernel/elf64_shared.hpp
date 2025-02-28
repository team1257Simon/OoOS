#ifndef __ELF64_SHARED
#define __ELF64_SHARED
#include "elf64_object.hpp"
#include "bits/functional_hash.hpp"
struct shared_segment_descriptor
{
    addr_t absolute_addr;       // The global address of the segment's start. If the segment is not loaded (e.g. not a loadable segment) this will be zero.
    off_t obj_offset;           // The p_vaddr value from the segment's program header in the object file containing the segment's data.
    size_t size;                // The p_memsz from the segment's program header in the object file containing the segment's data.
    elf_segment_prot perms;     // The permission values as determined from the program header's flags (the three lowest bits only)
};
struct elf64_gnu_htbl
{
    struct hdr
    {
        uint32_t nbucket;
        uint32_t symndx;
        uint32_t maskwords;
        uint32_t shift2;
    } header;
    uint64_t* bloom_filter_words;
    uint32_t* buckets;
    uint32_t* hash_value_array;
};
struct elf64_string_table
{
    size_t total_size;
    const char* data;
    constexpr const char* operator[](size_t n) const { return data + n; }
};
struct elf64_sym_table
{
    size_t total_size;
    size_t entry_size;
    addr_t data;
    constexpr elf64_sym const* operator+(size_t n) const { return static_cast<elf64_sym const*>(data.plus(entry_size * n)); }
    constexpr elf64_sym const& operator[](size_t n) const { return *this->operator+(n); }
};
struct elf64_dynsym_index
{
    elf64_string_table strtab;
    elf64_sym_table symtab;
    elf64_gnu_htbl htbl;
    void destroy_if_present();
    elf64_sym const* operator[](std::string const& str) const;
    constexpr operator bool() const noexcept { return htbl.bloom_filter_words && htbl.buckets && htbl.hash_value_array; }
};
class elf64_shared_object : public elf64_object
{
    size_t __num_seg_descriptors{ 0UL };
    shared_segment_descriptor* __segments{ nullptr };
    size_t __num_dyn_entries{ 0UL };
    elf64_dyn* __dyn_entries{ nullptr };
    elf64_dynsym_index __symbol_index{};
protected:
    virtual bool xload() override;
    virtual bool xvalidate() override;
public:
    elf64_shared_object(file_node* n);
    shared_segment_descriptor const* segment_of(elf64_sym const* sym) const;
    shared_segment_descriptor const* segment_of(std::string const& symbol) const;
    addr_t resolve(std::string const& symbol) const;
    virtual ~elf64_shared_object();
};
#endif