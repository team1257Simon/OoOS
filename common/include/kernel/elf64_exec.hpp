#ifndef __EXECUTABLE
#define __EXECUTABLE
#include "kernel/elf64_object.hpp"
#include "kernel/kernel_mm.hpp"
class elf64_executable : public virtual elf64_object
{
protected:
    size_t stack_size;
    size_t tls_size;
    addr_t frame_base;
    addr_t frame_extent;
    addr_t stack_base;
    addr_t tls_base;
    addr_t entry;
    uframe_tag* frame_tag;
    elf64_program_descriptor program_descriptor;
    virtual void process_headers() override;
    virtual bool load_segments() override;
    virtual bool xvalidate() override;
    virtual void on_load_failed() override;
    virtual addr_t segment_vaddr(size_t n) const;
    virtual void frame_enter() override;
public:
    virtual ~elf64_executable();
    elf64_executable(file_node* n, size_t stack_sz = S04, size_t tls_sz = S04);
    elf64_program_descriptor const& describe() const noexcept;
    elf64_executable(elf64_executable const&) = delete;
    elf64_executable(elf64_executable&& that);
    elf64_executable operator=(elf64_executable const&) = delete;
};
#endif