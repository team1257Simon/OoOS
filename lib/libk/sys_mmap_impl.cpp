#include "kernel/libk_decls.h"
#include "kernel/heap_allocator.hpp"

static paging_table __get_table(vaddr_t const& of_page)
{
    paging_table pml4 = __sysinternal_get_cr3();
    // TODO
    return 0;
}

vaddr_t sys_mmap(vaddr_t const& start, uintptr_t phys, size_t pages)
{
    // TODO
    return{};
}