#include <elf64_shared.hpp>
#include <frame_manager.hpp>
#include <stdexcept>
const char* empty_name		= "";
static const char* find_so_name(addr_t, file_vnode*);
addr_t elf64_shared_object::resolve(uint64_t offs) const { return virtual_load_base.plus(offs); }
addr_t elf64_shared_object::resolve(elf64_sym const& sym) const { return sym.st_shndx != SHN_UNDEF ? resolve(sym.st_value) : nullptr; }
void elf64_shared_object::frame_enter() { kmm.enter_frame(frame_tag); }
void elf64_shared_object::set_frame(uframe_tag* ft) { frame_tag = ft; }
uframe_tag* elf64_shared_object::get_frame() const { return frame_tag; }
bool elf64_shared_object::is_position_relocated() const noexcept { return true; }
elf64_shared_object::~elf64_shared_object() = default;
elf64_shared_object::elf64_shared_object(file_vnode* n, uframe_tag* frame) : elf64_object(n), elf64_dynamic_object(n),
	soname					{ find_so_name(img_ptr(), n) },
	virtual_load_base		{ frame->dynamic_extent },
	frame_tag				{ frame }
							{}
elf64_shared_object::elf64_shared_object(elf64_shared_object&& that) : elf64_object(std::move(that)), elf64_dynamic_object(std::move(that)),
	soname					{ std::move(that.soname) },
	virtual_load_base		{ that.virtual_load_base },
	total_segment_size		{ that.total_segment_size },
	frame_tag				{ that.frame_tag },
	ref_count				{ that.ref_count },
	sticky					{ that.sticky },
	symbolic				{ that.symbolic },
	global					{ that.global },
	entry					{ that.entry }
							{ that.frame_tag = nullptr; }
elf64_shared_object::elf64_shared_object(elf64_shared_object const& that) : elf64_object(that), elf64_dynamic_object(that),
	soname					{ that.soname },
	virtual_load_base		{ that.virtual_load_base },
	total_segment_size		{ that.total_segment_size },
	frame_tag				{ that.frame_tag },
	ref_count				{ that.ref_count },
	sticky					{ that.sticky },
	symbolic				{ that.symbolic },
	global					{ that.global },
	entry					{ that.entry }
							{}
static const char* find_so_name(addr_t image_start, file_vnode* so_file)
{
	elf64_ehdr const& eh		= image_start.deref<elf64_ehdr>();
	for(size_t i = 0UZ; i < eh.e_phnum; i++)
	{
		elf64_phdr const* phptr = image_start.plus(eh.e_phoff + i * eh.e_phentsize);
		elf64_phdr const& ph	= *phptr;
		if(is_dynamic(ph))
		{
			elf64_dyn* dyn_ent	= image_start.plus(ph.p_offset);
			size_t n			= ph.p_filesz / sizeof(elf64_dyn);
			size_t strtab_off	= 0UZ, name_off = 0UZ;
			for(size_t j = 0UZ; j < n; j++)
			{
				if(dyn_ent[j].d_tag == DT_STRTAB) strtab_off	= dyn_ent[j].d_ptr;
				else if(dyn_ent[j].d_tag == DT_SONAME) name_off	= dyn_ent[j].d_val;
				if(strtab_off && name_off) break;
			}
			if(!(strtab_off && name_off)) return so_file->name();
			else return image_start.plus(strtab_off + name_off);
		}
	}
	return empty_name;
}
void elf64_shared_object::process_dyn_entry(size_t i)
{
	if(dyn_entries[i].d_tag == DT_SYMBOLIC)
		symbolic	= true;
	elf64_dynamic_object::process_dyn_entry(i);
}
void elf64_shared_object::process_flags(elf_dyn_flags flags)
{
	if((flags & DF_SYMBOLIC) != 0)
		symbolic	= true;
	elf64_dynamic_object::process_flags(flags);
}
const char* elf64_shared_object::sym_lookup(addr_t addr) const
{
	if(__unlikely(!could_contain(addr))) return nullptr;
	size_t nsym = symtab.entries();
	for(size_t i = 0UZ; i < nsym; i++)
	{
		elf64_sym const& sym	= symtab[i];
		addr_t sym_base			= resolve(sym);
		if(addr >= sym_base && sym_base.plus(sym.st_size) > addr) return symstrtab[sym.st_name];
	}
	return nullptr;
}
program_segment_descriptor const* elf64_shared_object::segment_of(addr_t symbol_vaddr) const
{
	off_t seg_idx = segment_index(symbol_vaddr - virtual_load_base);
	if(__unlikely(seg_idx < 0)) return nullptr;
	return std::addressof(segments[seg_idx]);
}
void elf64_shared_object::process_headers()
{
	elf64_dynamic_object::process_headers();
	size_t total_size = 0UZ, tls_idx = 0UZ;
	for(size_t n = 0UZ; n < ehdr().e_phnum; n++)
	{
		elf64_phdr const& h			= phdr(n);
		if(is_tls(h)) tls_idx 		= n;
		else if(is_load(h))
			total_size				= addr_t(total_size).alignup(h.p_align).plus(h.p_memsz);
	}
	if(tls_idx) tls_base			= virtual_load_base.plus(total_size).alignup(phdr(tls_idx).p_align);
}
bool elf64_shared_object::xvalidate()
{
	if(ehdr().e_machine != EM_AMD64 || ehdr().e_ident[elf_ident_enc_idx] != ED_LSB) panic("[PRG/EXEC] not an object for the correct machine");
	else if(ehdr().e_ident[elf_ident_class_idx] != EC_64) panic("[PRG/EXEC] 32-bit object files are not supported");
	else if(ehdr().e_type != ET_DYN) panic("[PRG/EXEC] not a shared object");
	else if(!ehdr().e_phnum) panic("[PRG/EXEC] no program headers present");
	else return true;
	return false;
}
bool elf64_shared_object::load_segments()
{
	bool have_loads	= false;
	if(ehdr().e_entry)
		entry		= virtual_load_base.plus(ehdr().e_entry);
	for(size_t n	= 0UZ; n < ehdr().e_phnum; n++)
	{
		elf64_phdr const& h			= phdr(n);
		if(!h.p_memsz) continue;
		if(is_load(h))
		{
			addr_t addr				= is_tls(h) ? tls_base : virtual_load_base.plus(h.p_vaddr);
			addr_t target			= addr.trunc(h.p_align);
			size_t full_size		= h.p_memsz + (addr - target);
			block_descriptor* bd	= frame_tag->add_block(full_size, target, h.p_align, is_write(h), is_exec(h), is_global());
			if(!bd) throw std::bad_alloc{};
			if(is_tls(h))
			{
				tls_base			= bd->virtual_start;
				tls_size			= bd->size;
				tls_align			= bd->align;
			}
			addr_t idmap			= frame_tag->translate(addr);
			size_t actual_size		= kernel_memory_mgr::aligned_size(target, full_size);
			if(fm.count_references(bd->virtual_start) < 2)
			{
				addr_t img_dat		= segment_ptr(n);
				array_copy<uint8_t>(idmap, img_dat, h.p_filesz);
				if(h.p_memsz > h.p_filesz) array_zero<uint8_t>(idmap.plus(h.p_filesz), static_cast<size_t>(h.p_memsz - h.p_filesz));
			}
			program_segment_descriptor desc
			{
				.absolute_addr	= idmap,
				.virtual_addr	= addr,
				.obj_offset		= static_cast<off_t>(h.p_offset),
				.size			= h.p_memsz,
				.seg_align		= h.p_align,
				.perms			= static_cast<elf_segment_prot>(0b100UC | (is_write(h) ? 0b010UC : 0UC) | (is_exec(h) ? 0b001UC : 0UC))
			};
			segments.push_back(desc);
			frame_tag->dynamic_extent	= std::max(frame_tag->dynamic_extent, target.plus(actual_size).next_page_aligned());
			total_segment_size			+= actual_size;
			have_loads					= true;
		}
	}
	return have_loads;
}