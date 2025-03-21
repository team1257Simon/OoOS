#ifndef __ELF_DYNAMIC
#define __ELF_DYNAMIC
#include "elf64_object.hpp"
#include "elf64_relocation.hpp"
#include "vector"
class elf64_dynamic_object : public elf64_object
{
protected:
    size_t num_dyn_entries;
    elf64_dyn* dyn_entries;
    size_t num_plt_got_slots;
    elf64_rela* plt_got_slots;
    size_t got_seg_index;
    std::vector<elf64_relocation> relocations;
    elf64_dynsym_index symbol_index;
    virtual bool xload() override;
    virtual bool load_dynamic_syms();
    virtual bool process_got();
    void process_dynamic_relas();
    addr_t resolve_rela_target(elf64_rela const& r) const;
    uint64_t resolve_rela_sym(elf64_sym const& s, elf64_rela const& r) const;
public:
    elf64_dynamic_object(file_node* n);
    elf64_dynamic_object(elf64_dynamic_object const& that);
    elf64_dynamic_object(elf64_dynamic_object&& that);
    virtual ~elf64_dynamic_object();
    virtual void apply_relocations();
};
#endif