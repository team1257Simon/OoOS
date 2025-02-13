#ifndef __ELF64_SHARED
#define __ELF64_SHARED
#include "elf64_object.hpp"
struct shared_segment_descriptor
{
    addr_t absolute_addr;       // The global address of the segment's start. As of right now this will be identity-mapped
    off_t obj_offset;           // The p_vaddr value from the segment's program header in the object file containing the segment's data.
    size_t size;                // The p_memsz from the segment's program header in the object file containing the segment's data.
    elf_segment_prot perms;     // The permission values as determined from the program header's flags (the three lowest bits only)
};
#endif