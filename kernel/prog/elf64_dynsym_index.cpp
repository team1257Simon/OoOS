#include "elf64.h"
#include "stdlib.h"
#include "bits/functional_hash.hpp"
static std::elf64_gnu_hash __hash{};
elf64_sym const* elf64_dynsym_index::operator[](std::string const& str) const
{
	if(__unlikely(!*this)) return nullptr;
	uint32_t hash	= __hash(str.c_str());
	uint32_t i		= htbl.buckets[hash % htbl.header.nbucket];
	// TODO get bloom filter to actually work
	if(__unlikely(!i)) return nullptr;
	uint32_t hash_val	= hash & ~1U;
	uint32_t other_hash	= htbl.hash_value_array[i - htbl.header.symndx];
	for(elf64_sym const* sym = symtab + i; ; ++i, sym = symtab + i, other_hash = htbl.hash_value_array[i - htbl.header.symndx]) 
	{ 
		if(hash_val == (other_hash & ~1U) && !std::strcmp(str.c_str(), strtab[sym->st_name])) 
			return sym;
		if(other_hash & 1U) 
			break;
	}
	return nullptr;
}
void elf64_dynsym_index::destroy_if_present()
{
	if(htbl.bloom_filter_words) free(htbl.bloom_filter_words);
	if(htbl.buckets)            free(htbl.buckets);
	if(htbl.hash_value_array)   free(htbl.hash_value_array);
}