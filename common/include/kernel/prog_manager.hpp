#ifndef __PROG_MANAGER
#define __PROG_MANAGER
#include "kernel/elf64_dynamic_exec.hpp"
#include "list"
class prog_manager : std::list<elf64_executable>, std::list<elf64_dynamic_executable>
{
    using __static_base = std::list<elf64_executable>;
    using __dynamic_base = std::list<elf64_dynamic_executable>;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
    constexpr static off_t __static_node_offset = offsetof(typename __static_base::__node_type, __data);
    constexpr static off_t __dynamic_node_offset = offsetof(typename __dynamic_base::__node_type, __data);
#pragma GCC diagnostic pop
    static prog_manager __instance;
    elf64_executable* __add(addr_t img_start, size_t img_size, size_t stack_sz, size_t tls_sz);
public:
    prog_manager();
    elf64_executable* add(file_node* exec_file, size_t stack_sz = S04, size_t tls_sz = S04);
    void remove(elf64_executable* e);
    static prog_manager& get_instance();
};
#endif