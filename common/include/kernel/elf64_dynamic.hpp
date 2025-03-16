#ifndef __ELF_DYNAMIC
#define __ELF_DYNAMIC
#include "elf64_object.hpp"
class elf64_dynamic_object : public elf64_object
{
protected:
    size_t num_dyn_entries{ 0UL };
    elf64_dyn* dyn_entries{ nullptr };
    addr_t* got_entry_ptrs{ nullptr };
    elf64_dynsym_index symbol_index;
    virtual bool xload() override;
    virtual bool load_dynamic_syms();
    virtual bool process_got();
public:
    elf64_dynamic_object(file_node* n);
    virtual ~elf64_dynamic_object();
};
#endif