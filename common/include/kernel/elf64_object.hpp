#ifndef __ELF64_OBJ
#define __ELF64_OBJ
#include "kernel/kernel_defs.h"
#include "kernel/kernel_mm.hpp"
#include "kernel/elf64.h"
#include "kernel/fs/fs.hpp"
class elf64_object
{
    bool __validated;
    bool __loaded;
    addr_t __image_start;
    size_t __image_size;
protected:
    size_t num_seg_descriptors;
    program_segment_descriptor* segments;
    elf64_sym_table symtab;
    elf64_string_table symstrtab;
    elf64_string_table shstrtab;
    constexpr elf64_ehdr const& ehdr() const noexcept { return __image_start.ref<elf64_ehdr>(); }
    constexpr elf64_phdr const& phdr(size_t n) const noexcept { return __image_start.plus(ehdr().e_phoff + n * ehdr().e_phentsize).ref<elf64_phdr>(); }
    constexpr elf64_shdr const& shdr(size_t n) const noexcept { return __image_start.plus(ehdr().e_shoff + n * ehdr().e_shentsize).ref<elf64_shdr>(); }
    constexpr addr_t img_ptr(size_t offs = 0UZ) const noexcept { return __image_start.plus(offs); }
    constexpr addr_t segment_ptr(size_t n) const noexcept { return __image_start.plus(phdr(n).p_offset); }
    constexpr addr_t section_ptr(size_t n) const noexcept { return __image_start.plus(shdr(n).sh_offset); }
    virtual void process_headers();
    virtual void xrelease();
    virtual bool xload();
    virtual bool load_syms();
    virtual void on_load_failed();
    virtual bool load_segments() = 0;
    virtual bool xvalidate() = 0;
    virtual void frame_enter() = 0;
    virtual void set_frame(uframe_tag*) = 0;
    virtual uframe_tag* get_frame() const = 0;
    void release_segments();
    off_t segment_index(size_t offset) const;
    off_t segment_index(elf64_sym const* sym) const;
    void cleanup();
public:
    elf64_object(file_node* n);
    elf64_object(addr_t start, size_t size);
    elf64_object(elf64_object const&);
    elf64_object(elf64_object&&);
    virtual addr_t resolve(uint64_t offs) const;    
    virtual addr_t resolve(elf64_sym const& sym) const;
    virtual ~elf64_object();
    bool validate() noexcept;
    bool load() noexcept;
    void on_copy(uframe_tag* new_frame);
    std::vector<block_descriptor> segment_blocks() const;
    constexpr elf64_sym const& get_sym(size_t idx) const noexcept { return symtab[idx]; }
};
#endif