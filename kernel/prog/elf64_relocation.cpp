#include "elf64_relocation.hpp"
elf64_relocation::elf64_relocation(elf64_rela const& relaent) : rela_entry{ relaent } {}
elf64_relocation::elf64_relocation(elf64_sym const& sym, elf64_rela const& relaent) : symbol{ sym }, rela_entry{ relaent } {}
reloc_result elf64_relocation::operator()(reloc_sym_resolve const& sym_resolve, reloc_tar_resolve const& target_resolve) const { return reloc_result(target_resolve(rela_entry), sym_resolve(symbol, rela_entry)); }