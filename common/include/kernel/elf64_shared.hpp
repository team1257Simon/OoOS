#ifndef __ELF64_SHARED
#define __ELF64_SHARED
#include "elf64_dynamic.hpp"
#include "kernel_mm.hpp"
class elf64_shared_object : public elf64_dynamic_object
{
    friend class shared_object_map;
protected:
    uint64_t so_handle_magic;
    std::string soname;
    addr_t virtual_load_base;
    uframe_tag* frame_tag; // Shared address spaces (for SOs and the like) will have their own frames; dynamic linking will map those segments to the process as well
    size_t ref_count;
    bool sticky;
    virtual bool load_segments() override;
    virtual bool post_load_init() override;
    virtual bool xvalidate() override;
    virtual void xrelease() override;
public:
    virtual addr_t resolve(uint64_t offs) const override;
    virtual addr_t resolve(elf64_sym const& sym) const override;
    constexpr std::string const& get_soname() const { return soname; }
    constexpr void incref() noexcept { ref_count++; }
    constexpr void decref() noexcept { ref_count--; }
    constexpr size_t refs() const noexcept { return ref_count; }
    constexpr uint64_t magic() const noexcept { return so_handle_magic; }
    constexpr void set_sticky(bool value = true) noexcept { sticky = value; }
    constexpr bool is_sticky() const noexcept { return sticky; }
    program_segment_descriptor const* segment_of(addr_t symbol_vaddr) const;
    elf64_shared_object(file_node* n, uframe_tag* frame);
    elf64_shared_object(elf64_shared_object&& that);
    virtual ~elf64_shared_object();
    friend bool is_valid_handle(elf64_shared_object const& so);
};
#endif