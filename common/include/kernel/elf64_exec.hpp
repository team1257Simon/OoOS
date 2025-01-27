#ifndef __EXECUTABLE
#define __EXECUTABLE
#include "kernel/kernel_defs.h"
#include "kernel/elf64.h"
#include "kernel/heap_allocator.hpp"
class elf64_executable
{
    vaddr_t __image_start;
    size_t __image_total_size;
    size_t __tgt_stack_size;
    size_t __tgt_tls_size;
    vaddr_t __process_frame_base{ nullptr };
    vaddr_t __process_frame_extent{ nullptr };
    vaddr_t __process_stack_base{ nullptr };
    vaddr_t __process_tls_base{ nullptr };
    vaddr_t __process_entry_ptr{ nullptr };
    uframe_tag* __process_frame_tag{ nullptr };
    bool __validated{ false };
    constexpr elf64_ehdr* __get_ehdr_ptr() noexcept { return __image_start; }
    constexpr elf64_ehdr const* __get_ehdr_ptr() const noexcept { return __image_start; }
    constexpr elf64_ehdr& __get_ehdr() noexcept { return *__get_ehdr_ptr(); }
    constexpr elf64_ehdr const& __get_ehdr() const noexcept { return *__get_ehdr_ptr(); }
    constexpr bool __is_loaded() const noexcept { return __validated && __process_frame_tag; }
protected:
    void xload();
public:
    constexpr elf64_executable(vaddr_t image, size_t sz, size_t stack_sz = S04, size_t tls_sz = S04) noexcept : __image_start{ image }, __image_total_size{ sz }, __tgt_stack_size{ stack_sz }, __tgt_tls_size{ tls_sz } {}
    constexpr elf64_desc describe() const noexcept { return { __process_frame_tag, __process_stack_base, __tgt_stack_size, __process_tls_base, __tgt_tls_size, __process_entry_ptr }; }
    bool validate() noexcept;
    bool load() noexcept;
};
#endif