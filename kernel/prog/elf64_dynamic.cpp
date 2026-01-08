#include <elf64_dynamic.hpp>
#include <kernel_mm.hpp>
#include <stdlib.h>
#include <algorithm>
constexpr static bool is_object_rela(elf64_rela const& r) { return r.r_info.type == R_X86_64_GLOB_DAT; }
bool elf64_dynamic_object::load_preinit() { return true; /* stub; only applicable to executables */ }
addr_t elf64_dynamic_object::resolve_rela_target(elf64_rela const& r) const { return resolve(r.r_offset); }
addr_t elf64_dynamic_object::global_offset_table() const { return got_vaddr ? resolve(got_vaddr) : nullptr; }
addr_t elf64_dynamic_object::dyn_segment_ptr() const { return dyn_segment_idx ? resolve(phdr(dyn_segment_idx).p_vaddr) : nullptr; }
addr_t elf64_dynamic_object::translate_in_frame(addr_t addr) { return get_frame()->translate(addr); }
void elf64_dynamic_object::process_flags(elf_dyn_flags flags) { if(flags & DF_STATIC_TLS) static_tls = true; }
void elf64_dynamic_object::process_flags(elf_dyn_flags_1 flags1) { if(flags1 & DF_1_NOW) bind_now = true; }
elf64_dynamic_object::elf64_dynamic_object(addr_t start, size_t size) :	elf64_object(start, size), symbol_index(symstrtab, symtab) {}
elf64_dynamic_object::elf64_dynamic_object(file_vnode* n) : elf64_object(n), symbol_index(symstrtab, symtab) {}
elf64_dynamic_object::elf64_dynamic_object(elf64_dynamic_object const& that) :
	elf64_object	{ that },
	num_dyn_entries	{ that.num_dyn_entries },
	dyn_entries		{ that.dyn_entries },
	num_plt_relas	{ that.num_plt_relas },
	plt_relas		{ that.plt_relas },
	got_vaddr		{ that.got_vaddr },
	dyn_segment_idx	{ that.dyn_segment_idx },
	relocations		{ that.relocations },
	object_relas	{ that.object_relas },
	tls_relas		{ that.tls_relas },
	dependencies	{ that.dependencies },
	ld_paths		{ that.ld_paths },
	init_array		{ that.init_array },
	fini_array		{ that.fini_array },
	init_fn			{ that.init_fn },
	fini_fn			{ that.fini_fn },
	init_array_ptr	{ that.init_array_ptr },
	fini_array_ptr	{ that.fini_array_ptr },
	init_array_size	{ that.init_array_size },
	fini_array_size	{ that.fini_array_size },
	symbol_index
	{
		.strtab		{ symstrtab },
		.symtab		{ symtab },
		.htbl		{ that.symbol_index.htbl },
		.versym		{ that.symbol_index.versym },
		.verdef		{ that.symbol_index.verdef },
		.verneed	{ that.symbol_index.verneed }
	}
	{}
elf64_dynamic_object::elf64_dynamic_object(elf64_dynamic_object&& that) :
	elf64_object	{ std::move(that) },
	num_dyn_entries	{ that.num_dyn_entries },
	dyn_entries		{ std::move(that.dyn_entries) },
	num_plt_relas	{ that.num_plt_relas },
	plt_relas		{ std::move(that.plt_relas) },
	got_vaddr		{ that.got_vaddr },
	dyn_segment_idx	{ that.dyn_segment_idx },
	relocations		{ std::move(that.relocations) },
	object_relas	{ std::move(that.object_relas) },
	tls_relas		{ std::move(that.tls_relas) },
	dependencies	{ std::move(that.dependencies) },
	ld_paths		{ std::move(that.ld_paths) },
	init_array		{ std::move(that.init_array) },
	fini_array		{ std::move(that.fini_array) },
	init_fn			{ that.init_fn },
	fini_fn			{ that.fini_fn },
	init_array_ptr	{ that.init_array_ptr },
	fini_array_ptr	{ that.fini_array_ptr },
	init_array_size	{ that.init_array_size },
	fini_array_size	{ that.fini_array_size },
	symbol_index
	{
		.strtab		{ symstrtab },
		.symtab		{ symtab },
		.htbl		{ std::move(that.symbol_index.htbl) },
		.versym		{ std::move(that.symbol_index.versym) },
		.verdef		{ std::move(that.symbol_index.verdef) },
		.verneed	{ std::move(that.symbol_index.verneed) }
	}
	{}
elf64_dynamic_object::~elf64_dynamic_object() = default;
constexpr static bool is_tls_rela(elf64_rela const& r)
{
	elf_rel_type t = r.r_info.type;
	if(t == R_X86_64_DPTMOD64 || t == R_X86_64_DTPOFF64 || t == R_X86_64_TPOFF64 || t == R_X86_64_DTPOFF32 || t == R_X86_64_TPOFF32) return true;
	else return false;
}
static bool is_dynamic_relocation(elf64_rela const& r)
{
	elf_rel_type t = r.r_info.type;
	if(t == R_X86_64_JUMP_SLOT || t == R_X86_64_GLOB_DAT || t == R_X86_64_TPOFF64 || t == R_X86_64_TPOFF32 || t == R_X86_64_DPTMOD64 || t == R_X86_64_DTPOFF64 || t == R_X86_64_DTPOFF32 || t >= R_X86_64_GOTPC32_TLSDESC) return true;
	else return false;
}
static bool recognize_rela_type(elf64_rela const& r)
{
	elf_rel_type t = r.r_info.type;
	if(t == R_X86_64_64 || t == R_X86_64_RELATIVE || t == R_X86_64_DTPOFF64 || t == R_X86_64_DPTMOD64 || t == R_X86_64_JUMP_SLOT || t == R_X86_64_GLOB_DAT) return true;
	else return false;
}
void elf64_dynamic_object::apply_relocations()
{
	reloc_sym_resolve reloc_symbol_fn = std::bind(&elf64_dynamic_object::resolve_rela_sym, this, std::placeholders::_1, std::placeholders::_2);
	reloc_tar_resolve reloc_target_fn = std::bind(&elf64_dynamic_object::resolve_rela_target, this, std::placeholders::_1);
	// Assuming everything works as planned (decently tall ask), the relocation value will be calculated here for each relocation.
	for(elf64_relocation const& r : relocations)
	{
		if(is_dynamic_relocation(r.rela_entry)) continue; // these will be resolved later
		reloc_result result	= r(reloc_symbol_fn, reloc_target_fn);
		addr_t phys_target	= translate_in_frame(result.target);
		if(phys_target && result.value) phys_target.assign(result.value);
		else
		{
			klog("[PRG/DYN] W: invalid relocation");
			xklog("[PRG/DYN] D: relocation offset was " + std::to_string(r.rela_entry.r_offset, std::ext::hex));
			xklog("[PRG/DYN] D: relocation addend was " + std::to_string(r.rela_entry.r_addend, std::ext::hex));
			if(result.target) xklog("[PRG/DYN] D: relocation target was " + std::to_string(result.target.full, std::ext::hex));
			if(result.value) xklog("[PRG/DYN] D: relocation value was " + std::to_string(result.value, std::ext::hex));
		}
	}
}
uint64_t elf64_dynamic_object::resolve_rela_sym(elf64_sym const& s, elf64_rela const& r) const
{
	switch(r.r_info.type)
	{
	case R_X86_64_64:
		return resolve(s.st_value + r.r_addend);
	case R_X86_64_RELATIVE:
		return resolve(r.r_addend);
	default:
		klog("[PRG/DYN] W: invalid or unrecognized relocation");
		return 0UL;
	}
}
int64_t elf64_dynamic_object::resolve_tls_rela(elf64_sym const& s, elf64_rela const& r, std::vector<ptrdiff_t> const& mod_offsets)
{
	if(mod_offsets.size() > tls_mod_idx)
	{
		switch(r.r_info.type)
		{
		case R_X86_64_DTPOFF64:
			return static_cast<int64_t>(r.r_addend + s.st_value);
		case R_X86_64_DPTMOD64:
			return static_cast<int64_t>(tls_mod_idx);
		case R_X86_64_TPOFF64:
			return static_cast<int64_t>(s.st_value) - mod_offsets[tls_mod_idx];
		case R_X86_64_TPOFF32:
			return static_cast<int64_t>(static_cast<int>(s.st_value) - static_cast<int>(mod_offsets[tls_mod_idx]));
		default:
			klog("[PRG/DYN] W: invalid or unrecognized TLS relocation");
			return 0L;
		}
	}
	else throw std::out_of_range("[PRG/DYN] no TLS offset present for this object");
}
bool elf64_dynamic_object::xload()
{
	// allocate the array to have enough space for all indices, but the non-load segments will be zeroed
	bool success = true;
	process_headers();
	if(__unlikely(!load_segments())) { success = (panic("[PRG/DYN] object contains no loadable segments"), false); }
	else if(__unlikely(!load_syms())) { success = (panic("[PRG/DYN] failed to load symbols"), false); }
	else if(__unlikely(!post_load_init())) { success = (panic("[PRG/DYN] failed to initialize program image"), false); }
	// other segments and sections, if/when needed, can be handled here; free the rest up
	cleanup();
	return success;
}
void elf64_dynamic_object::process_relas(elf64_rela* rela_array, size_t num_relas)
{
	size_t unrec_rela_ct	= 0UZ;
	for(size_t i			= 0UZ; i < num_relas; i++)
	{
		if(is_object_rela(rela_array[i])) { object_relas.push_back(rela_array[i]); }
		else if(is_tls_rela(rela_array[i])) { tls_relas.push_back(rela_array[i]); }
		else if(is_dynamic_relocation(rela_array[i])) continue; // these are computed / applied by the dynamic linker
		else if(!recognize_rela_type(rela_array[i])) { unrec_rela_ct++; }
		else if(rela_array[i].r_info.sym_index) { elf64_sym const* s = symtab + rela_array[i].r_info.sym_index; if(s) relocations.emplace_back(*s, rela_array[i]); }
		else relocations.emplace_back(rela_array[i]);
	}
	if(unrec_rela_ct) { xklog("[PRG/DYN] W: " + std::to_string(unrec_rela_ct) + " unrecognized relocation types"); }
}
void elf64_dynamic_object::find_and_process_relas()
{
	size_t n_sections				= ehdr().e_shnum;
	for(size_t i = 0UZ; i < n_sections; i++)
	{
		elf64_shdr const& section	= shdr(i);
		if(section.sh_type == SHT_RELA)
			process_relas(img_ptr(section.sh_offset), section.sh_size / section.sh_entsize);
	}
}
bool elf64_dynamic_object::post_load_init()
{
	apply_relocations();
	if(got_vaddr)
	{
		addr_t* got = translate_in_frame(global_offset_table());
		if(__builtin_expect(got != nullptr, true)) { got[1] = this; }
		else return panic("[PRG/DYN] GOT pointer is non-null but is invalid"), false;
	}
	try
	{
		std::vector<addr_t> fini_reverse_array{};
		if(__unlikely(!load_preinit())) return false;
		if(init_fn) { init_array.push_back(resolve(init_fn)); }
		if(fini_fn) { fini_reverse_array.push_back(resolve(fini_fn)); }
		if(init_array_size && init_array_ptr)
		{
			addr_t init_ptrs_vaddr	= resolve(init_array_ptr);
			uintptr_t* init_ptrs	= translate_in_frame(init_ptrs_vaddr);
			if(__unlikely(!init_ptrs)) return panic("[PRG] initialization array pointer is non-null but is invalid"), false;
			for(size_t i = 0UZ; i < init_array_size; i++) init_array.push_back(addr_t(init_ptrs[i]));
		}
		if(fini_array_size && fini_array_ptr)
		{
			addr_t fini_ptrs_vaddr	= resolve(fini_array_ptr);
			uintptr_t* fini_ptrs	= translate_in_frame(fini_ptrs_vaddr);
			if(__unlikely(!fini_ptrs)) return panic("[PRG] finalization array pointer is non-null but is invalid"), false;
			for(size_t i = 0UZ; i < fini_array_size; i++) fini_reverse_array.push_back(addr_t(fini_ptrs[i]));
		}
		if(!fini_reverse_array.empty()) { fini_array.push_back(fini_reverse_array.rend(), fini_reverse_array.rbegin()); }
		return true;
	}
	catch(std::exception& e) { panic(e.what()); return false; }
}
void elf64_dynamic_object::process_dyn_entry(size_t i)
{
	switch(dyn_entries[i].d_tag)
	{
	case DT_INIT:
		init_fn								= dyn_entries[i].d_ptr;
		break;
	case DT_FINI:
		fini_fn								= dyn_entries[i].d_ptr;
		break;
	case DT_INIT_ARRAY:
		init_array_ptr						= dyn_entries[i].d_ptr;
		break;
	case DT_FINI_ARRAY:
		fini_array_ptr						= dyn_entries[i].d_ptr;
		break;
	case DT_INIT_ARRAYSZ:
		init_array_size						= (dyn_entries[i].d_val / sizeof(addr_t));
		break;
	case DT_FINI_ARRAYSZ:
		fini_array_size						= (dyn_entries[i].d_val / sizeof(addr_t));
		break;
	case DT_RUNPATH:
		ld_paths							= std::move(std::ext::split(std::forward<std::string>(symstrtab[dyn_entries[i].d_val]), ':'));
		break;
	case DT_JMPREL:
		plt_rela_offs						= dyn_entries[i].d_ptr;
		break;
	case DT_PLTRELSZ:
		num_plt_relas						= dyn_entries[i].d_val  / sizeof(elf64_rela);
		break;
	case DT_PLTGOT:
		got_vaddr							= dyn_entries[i].d_ptr;
		break;
	case DT_VERDEFNUM:
		symbol_index.verdef.verdef_num		= dyn_entries[i].d_val;
		break;
	case DT_VERNEEDNUM:
		symbol_index.verneed.verneed_num	= dyn_entries[i].d_val;
		break;
	case DT_NEEDED:
		dependencies.emplace_back(symstrtab[dyn_entries[i].d_val]);
		break;
	case DT_FLAGS:
		process_flags(static_cast<elf_dyn_flags>(dyn_entries[i].d_val));
		break;
	case DT_FLAGS_1:
		process_flags(static_cast<elf_dyn_flags_1>(dyn_entries[i].d_val));
		break;
	default:
		break;
	}
}
bool elf64_dynamic_object::load_syms()
{
	if(__unlikely(!elf64_object::load_syms())) return panic("[PRG/DYN] no symbol table present"), false;
	bool have_dyn		= false;
	elf64_ehdr const& e	= ehdr();
	for(size_t n = 0UZ; n < e.e_phnum && !have_dyn; n++)
	{
		elf64_phdr const& ph	= phdr(n);
		if(is_dynamic(ph))
		{
			num_dyn_entries		= ph.p_filesz / sizeof(elf64_dyn);
			elf64_dyn const* d	= segment_ptr(n);
			dyn_entries			= std::move(std::vector<elf64_dyn>(d, d + num_dyn_entries));
			have_dyn			= true;
			dyn_segment_idx		= n;
		}
	}
	find_and_process_relas();
	if(have_dyn) process_dynamic();
	if(__unlikely((!init_array_ptr ^ !init_array_size) || (!fini_array_ptr ^ !fini_array_size)))
		return panic("[PRG/DYN] mismatched init and/or fini array entries"), false;
	if(__unlikely(!(symbol_index.verdef || symbol_index.verneed))) return process_plt_got();
	for(size_t n = 0UZ; n < e.e_shnum; n++)
	{
		elf64_shdr const& sh	= shdr(n);
		elf64_shdr const& sl	= shdr(sh.sh_link);
		addr_t begin			= img_ptr(sh.sh_offset);
		switch(sh.sh_type)
		{
		case SHT_VERSYM:
			symbol_index.versym	= std::move(std::vector<uint16_t>(begin.as<uint16_t>(), begin.plus(sh.sh_size).as<uint16_t>()));
			continue;
		case SHT_VERDEF:
			if(__unlikely(!symbol_index.verdef)) return panic("[PRG/DYN] mismatched verdef entries"), false;
			symbol_index.verdef.build(begin, elf64_string_table(sl.sh_size, img_ptr(sl.sh_offset)));
			continue;
		case SHT_VERNEED:
			if(__unlikely(!symbol_index.verneed)) return panic("[PRG/DYN] mismatched verneed entries"), false;
			symbol_index.verneed.build(begin, elf64_string_table(sl.sh_size, img_ptr(sl.sh_offset)));
			continue;
		default:
			continue;
		}
	}
	return process_plt_got();
}
void elf64_dynamic_object::process_dynamic()
{
	for(size_t i = 0UZ; i < num_dyn_entries; i++)
	{
		if(dyn_entries[i].d_tag == DT_GNU_HASH)
		{
			uframe_tag* frame		= get_frame();
			addr_t ht_addr			= frame ? frame->translate(resolve(dyn_entries[i].d_ptr)) : resolve(dyn_entries[i].d_ptr);
			elf64_gnu_htbl::hdr* h	= ht_addr;
			size_t n_hvals			= static_cast<size_t>((symtab.total_size / symtab.entry_size) - h->symndx);
			uint64_t* og_filter		= ht_addr.plus(sizeof(elf64_gnu_htbl::hdr));
			uint32_t* og_buckets	= addr_t(og_filter).plus(h->maskwords * sizeof(uint64_t));
			uint32_t* og_hvals		= addr_t(og_buckets).plus(h->nbucket * sizeof(uint32_t));
			new(std::addressof(symbol_index.htbl)) elf64_gnu_htbl
			{
				.header
				{
					.nbucket		{ h->nbucket },
					.symndx			{ h->symndx },
					.maskwords		{ h->maskwords },
					.shift2			{ h->shift2 }
				},
				.bloom_filter_words	{ og_filter, og_filter + h->maskwords },
				.buckets			{ og_buckets, og_buckets + h->nbucket },
				.hash_value_array	{ og_hvals, og_hvals + n_hvals }
			};
		}
		else process_dyn_entry(i);
	}
}
std::pair<elf64_sym, addr_t> elf64_dynamic_object::fallback_resolve(std::string const& symbol) const
{
	std::string read_name;
	for(elf64_sym const& sym : symtab)
	{
		read_name = symstrtab[sym.st_name];
		if(read_name == symbol)
			return std::make_pair(sym, sym.st_info.type == ST_TLS ? addr_t(static_cast<elf64_dynamic_object const*>(this)) : resolve(sym));
	}
	return std::make_pair(elf64_sym(), nullptr);
}
std::pair<elf64_sym, addr_t> elf64_dynamic_object::resolve_by_name(std::string const& symbol) const
{
	if(__unlikely(!segments || !num_seg_descriptors))
		return panic("[PRG/DYN] cannot load symbols from an uninitialized object"), std::make_pair(elf64_sym(), nullptr);
	elf64_sym const* sym = symbol_index[symbol];
	return sym ? std::make_pair(*sym, sym->st_info.type == ST_TLS ? addr_t(static_cast<elf64_dynamic_object const*>(this)) : resolve(*sym)) : std::make_pair(elf64_sym(), nullptr);
}
bool elf64_dynamic_object::process_plt_got()
{
	if(plt_rela_offs && num_plt_relas)
	{
		uframe_tag* frame		= get_frame();
		elf64_rela* rela		= frame ? frame->translate(resolve(plt_rela_offs)) : resolve(plt_rela_offs);
		plt_relas				= std::move(std::vector<elf64_rela>(rela, rela + num_plt_relas));
		bool need_got_fixup		= is_position_relocated() && got_vaddr;
		if(bind_now || need_got_fixup)
		{
			for(elf64_rela const& r : plt_relas)
			{
				addr_t target		= frame ? frame->translate(resolve_rela_target(r)) : resolve_rela_target(r);
				if(__unlikely(!target)) return panic("[PRG/DYN] virtual address fault"), false;
				addr_t sym_addr		= bind_now ? resolve(symtab[r.r_info.sym_index]) : nullptr;
				if(sym_addr) target.assign(sym_addr);
				else if(need_got_fixup) target.assign(resolve(target.deref<uintptr_t>()));
			}
		}
	}
	return true;
}
void elf64_dynamic_object::set_resolver(addr_t ptr)
{
	if(got_vaddr)
	{
		addr_t got_table	= translate_in_frame(global_offset_table());
		if(!got_table) return;
		got_table.plus(sizeof(addr_t) * 2Z).assign(ptr);
	}
}