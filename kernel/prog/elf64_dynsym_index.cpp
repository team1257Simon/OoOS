#include <elf64_index.hpp>
#include <ranges>
constexpr static gnu_string_hash __gnu_hash{};
constexpr static size_t bloom_bits	= static_cast<int>(sizeof(uint64_t) * __CHAR_BIT__);
elf64_sym const* elf64_dynsym_index::operator[](std::string const& str) const
{
	if(__unlikely(!*this)) return nullptr;
	uint32_t hash		= __gnu_hash(str.c_str());
	uint64_t bloom		= htbl.bloom_filter_words[(hash / bloom_bits) % htbl.header.maskwords];
	uint64_t mask		= ((1UL << hash % bloom_bits) | (1UL << (hash >> htbl.header.shift2) % bloom_bits));
	if((bloom & mask) != mask) return nullptr;
	uint32_t i			= htbl.buckets[hash % htbl.header.nbucket];
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
void elf64_requisites_index::build(addr_t verneed_section, elf64_string_table const& strtab)
{
	addr_t current_entry			= verneed_section;
	requisites.emplace_back();
	requisites.emplace_back(true);
	for(size_t i = 0UZ; i < verneed_num; i++)
	{
		elf64_verneed const& ent	= current_entry.deref<elf64_verneed>();
		addr_t vernaux_entry		= current_entry.plus(ent.vn_aux);
		current_entry				+= ent.vn_next;
		std::vector<uint16_t>& vec	= by_object.emplace(std::piecewise_construct, std::forward_as_tuple(std::move(std::string(strtab[ent.vn_file]))), std::tuple<>()).first->second;
		for(size_t j = 0UZ; j < ent.vn_cnt; j++)
		{
			elf64_vernaux const& va			= vernaux_entry.deref<elf64_vernaux>();
			vernaux_entry					+= va.vna_next;
			uint16_t target					= va.vna_other;
			while(requisites.size() <= static_cast<size_t>(target)) requisites.emplace_back();
			elf64_sym_version& req			= requisites[target];
			req.base_or_weak				= (va.vna_flags & VER_FLG_WEAK) != 0US;
			req.name_hash					= va.vna_hash;
			req.name						= std::move(std::string(strtab[va.vna_name]));
			vec.push_back(target);
		}
	}
}
void elf64_version_index::build(addr_t verdef_section, elf64_string_table const& strtab)
{
	addr_t current_entry		= verdef_section;
	definitions.emplace_back();
	for(size_t i = 0UZ; i < verdef_num; i++)
	{
		elf64_verdef const& vd	= current_entry.deref<elf64_verdef>();
		elf64_verdaux const& va	= current_entry.plus(vd.vd_aux).deref<elf64_verdaux>();
		current_entry			+= vd.vd_next;
		uint16_t target			= vd.vd_ndx;
		while(definitions.size() <= static_cast<size_t>(target)) definitions.emplace_back();
		elf64_sym_version& ver	= definitions[target];
		ver.base_or_weak		= (vd.vd_flags & VER_FLG_BASE) != 0US;
		ver.name_hash			= vd.vd_hash;
		ver.name				= std::move(std::string(strtab[va.vda_name]));
	}
}
bool elf64_version_index::check(elf64_sym_version const& ver) const
{
	if(__unlikely(ver.base_or_weak)) return true;
	auto match = [&ver](elf64_sym_version const& v) -> bool { return ver == v; };
	return !std::ranges::empty(definitions | std::views::filter(match));
}