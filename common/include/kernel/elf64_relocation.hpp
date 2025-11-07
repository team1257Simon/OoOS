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
	elf64_sym symbol;
	elf64_rela rela_entry;
	elf64_relocation(elf64_sym const& sym, elf64_rela const& relaent);
	elf64_relocation(elf64_rela const& relaent);
	reloc_result operator()(reloc_sym_resolve const& sym_resolve, reloc_tar_resolve const& target_resolve) const;
};
#endif