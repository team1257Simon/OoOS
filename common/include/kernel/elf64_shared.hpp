#ifndef __ELF64_SHARED
#define __ELF64_SHARED
#include "elf64_dynamic.hpp"
class elf64_shared_object : public elf64_dynamic_object
{
protected:
    virtual bool load_segments() override;
    virtual bool xvalidate() override;
public:
    addr_t resolve(std::string const& symbol) const;
    elf64_shared_object(file_node* n);
    virtual ~elf64_shared_object();
};
#endif