#ifndef __ELF_RELOCATION
#define __ELF_RELOCATION
#include "elf64.h"
#include "functional"
#include "tuple"
struct reloc_result { addr_t target; uint64_t value; };
typedef std::function<uint64_t(elf64_sym const&, elf64_rela const&)> reloc_sym_resolve;
typedef std::function<addr_t(elf64_rela const&)> reloc_tar_resolve;
struct elf64_relocation
{
    elf64_sym symbol{};
    elf64_rela rela_entry;
    reloc_sym_resolve sym_resolve;
    reloc_tar_resolve target_resolve;
    elf64_relocation(elf64_sym const& sym, elf64_rela const& relaent, reloc_sym_resolve const& sres, reloc_tar_resolve const& tres);
    elf64_relocation(elf64_rela const& relaent, reloc_sym_resolve const& sres, reloc_tar_resolve const& tres);
    reloc_result operator()() const;
};
#endif