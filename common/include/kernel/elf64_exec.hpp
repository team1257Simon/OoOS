#ifndef __EXECUTABLE
#define __EXECUTABLE
#include "kernel/elf64_object.hpp"
#include "kernel/kernel_mm.hpp"
class elf64_executable : public virtual elf64_object
{
    size_t __image_total_size;
    size_t __tgt_stack_size;
    size_t __tgt_tls_size;
    addr_t __process_frame_base{ nullptr };
    addr_t __process_frame_extent{ nullptr };
    addr_t __process_stack_base{ nullptr };
    addr_t __process_tls_base{ nullptr };
    addr_t __process_entry_ptr{ nullptr };
    uframe_tag* __process_frame_tag{ nullptr };
    elf64_program_descriptor __descr{};
protected:
    virtual void process_headers() override;
    virtual bool load_segments() override;
    virtual bool xload() override;
    virtual bool xvalidate() override;
public:
    virtual ~elf64_executable();
    elf64_executable(file_node* n, size_t stack_sz = S04, size_t tls_sz = S04);
    elf64_program_descriptor const& describe() const noexcept;
    elf64_executable(elf64_executable const&) = delete;
    elf64_executable(elf64_executable&&);
    elf64_executable operator=(elf64_executable const&) = delete;
};
#endif