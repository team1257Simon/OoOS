#include "elf64_relocation.hpp"
elf64_relocation::elf64_relocation(elf64_rela const& relaent, reloc_sym_resolve const& sres, reloc_tar_resolve const& tres) : rela_entry{ relaent }, sym_resolve{ sres }, target_resolve{ tres } {}
elf64_relocation::elf64_relocation(elf64_sym const& sym, elf64_rela const& relaent, reloc_sym_resolve const& sres, reloc_tar_resolve const& tres) : symbol{ sym }, rela_entry{ relaent }, sym_resolve{ sres }, target_resolve{ tres } {}
reloc_result elf64_relocation::operator()() const { return reloc_result(target_resolve(rela_entry), sym_resolve(symbol, rela_entry)); }