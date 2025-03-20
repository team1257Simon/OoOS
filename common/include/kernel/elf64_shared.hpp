#ifndef __ELF64_SHARED
#define __ELF64_SHARED
#include "elf64_dynamic.hpp"
#include "kernel_mm.hpp"
class elf64_shared_object : public elf64_dynamic_object
{
protected:
    std::string soname;
    addr_t virtual_load_base;
    uframe_tag* frame_tag; // Shared address spaces (for SOs and the like) will have their own frames; dynamic linking will map those segments to the process as well
    virtual bool load_segments() override;
    virtual bool xvalidate() override;
    virtual bool xload() override;
public:
    virtual addr_t resolve(uint64_t offs) const override;
    virtual addr_t resolve(elf64_sym const& sym) const override;
    addr_t resolve_by_name(std::string const& symbol) const;
    constexpr std::string const& get_soname() const { return soname; }
    program_segment_descriptor const* segment_of(addr_t symbol_vaddr) const;
    elf64_shared_object(file_node* n, uframe_tag* frame);
    elf64_shared_object(elf64_shared_object&& that);
    virtual ~elf64_shared_object();
};
#endif