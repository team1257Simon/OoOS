#include <elf64_exec.hpp>
#include <frame_manager.hpp>
#include <stdexcept>
constexpr std::allocator<const char*> strptr_alloc{};
constexpr static size_t min_blk_sz = S04;
elf64_program_descriptor const& elf64_executable::describe() const noexcept { return program_descriptor; }
void elf64_executable::frame_enter() { kmm.enter_frame(frame_tag); }
addr_t elf64_executable::segment_vaddr(size_t n) const { return addr_t(phdr(n).p_vaddr); }
elf64_executable::~elf64_executable() { if(program_descriptor.ld_path) strptr_alloc.deallocate(program_descriptor.ld_path, program_descriptor.ld_path_count); }
void elf64_executable::on_load_failed() { fm.destroy_frame(*frame_tag); frame_tag = nullptr; kmm.exit_frame(); }
void elf64_executable::set_frame(uframe_tag* ft) { frame_tag = ft; program_descriptor.frame_ptr = ft; }
uframe_tag* elf64_executable::get_frame() const { return frame_tag; }
elf64_executable::elf64_executable(addr_t start, size_t size, size_t stack_sz) : elf64_object(start, size),
	stack_size			{ stack_sz },
	frame_base			{ nullptr },
	frame_extent		{ nullptr },
	stack_base			{ nullptr },
	entry				{ ehdr().e_entry },
	frame_tag			{ nullptr },
	program_descriptor	{}
						{}
elf64_executable::elf64_executable(file_vnode*n, size_t stack_sz) : elf64_object(n),
	stack_size			{ stack_sz },
	frame_base			{ nullptr },
	frame_extent		{ nullptr },
	stack_base			{ nullptr },
	entry				{ ehdr().e_entry },
	frame_tag			{ nullptr },
	program_descriptor	{}
						{}
elf64_executable::elf64_executable(elf64_executable&& that) : elf64_object(std::move(that)),
	stack_size			{ that.stack_size },
	frame_base			{ std::move(that.frame_base) },
	frame_extent		{ std::move(that.frame_extent) },
	stack_base			{ std::move(that.stack_base) },
	entry				{ std::move(that.entry) },
	frame_tag			{ std::move(that.frame_tag) },
	program_descriptor	{ std::move(that.program_descriptor) }
{
	that.frame_base						= nullptr;
	that.frame_extent					= nullptr;
	that.stack_base						= nullptr;
	that.tls_base						= nullptr;
	that.entry							= nullptr;
	that.frame_tag						= nullptr;
	program_descriptor.object_handle	= this;
	that.program_descriptor.ld_path		= nullptr;
}
elf64_executable::elf64_executable(elf64_executable const& that) : elf64_object(that),
	stack_size			{ that.stack_size },
	frame_base			{ that.frame_base },
	frame_extent		{ that.frame_extent },
	stack_base			{ that.stack_base },
	entry				{ that.entry },
	frame_tag			{ that.frame_tag },
	program_descriptor	{ that.program_descriptor }
	{
		program_descriptor.object_handle = this;
		if(program_descriptor.ld_path)
		{
			const char** old			= program_descriptor.ld_path;
			program_descriptor.ld_path	= strptr_alloc.allocate(program_descriptor.ld_path_count);
			array_copy(program_descriptor.ld_path, old, program_descriptor.ld_path_count);
		}
	}
bool elf64_executable::xvalidate()
{
	if(__unlikely(ehdr().e_machine != EM_AMD64 || ehdr().e_ident[elf_ident_enc_idx] != ED_LSB)) { panic("[PRG/EXEC] not an object for the correct machine"); return false; }
	if(__unlikely(ehdr().e_ident[elf_ident_class_idx] != EC_64)) { panic("[PRG/EXEC] 32-bit executables are not yet supported"); return false; }
	if(__unlikely(ehdr().e_type != ET_EXEC)) { panic("[PRG/EXEC] object is not an executable"); return false; }
	if(__unlikely(!ehdr().e_phnum)) { panic("[PRG/EXEC] no program headers present"); return false; }
	return true;
}
void elf64_executable::process_headers()
{
	if(!segments) elf64_object::process_headers();
	stack_size				= std::max(stack_size, min_blk_sz);
	size_t tls_seg			= 0UZ;
	for(size_t n = 0; n < ehdr().e_phnum; n++)
	{
		elf64_phdr const& h = phdr(n);
		uintptr_t addr		= h.p_vaddr;
		if(is_tls(h)) {
			tls_seg			= n;
			tls_align		= h.p_align;
			continue;
		}
		if(!is_load(h)) continue;
		if(!frame_base || frame_base > addr)
			frame_base		= addr_t(addr);
		frame_extent		= std::max(frame_extent, addr_t(addr + h.p_memsz));
	}
	frame_base				= frame_base.page_aligned();
	stack_base				= frame_extent.next_page_aligned();
	frame_extent			= stack_base.plus(stack_size).next_page_aligned();
	if(tls_seg) {
		tls_base			= frame_extent.alignup(tls_align);
		frame_extent		= tls_base.plus(phdr(tls_seg).p_memsz).next_page_aligned();
	}
}
bool elf64_executable::load_segments()
{
	frame_tag				= std::addressof(fm.create_frame(frame_base, frame_extent));
	if(__unlikely(!frame_tag)) { panic("[PRG/EXEC] failed to allocate frame"); return false; }
	for(size_t n = 0; n < ehdr().e_phnum; n++)
	{
		elf64_phdr const& h = phdr(n);
		if(!is_load(h) || !h.p_memsz) continue;
		addr_t addr			= is_tls(h) ? tls_base : segment_vaddr(n);
		addr_t target		= addr.trunc(h.p_align);
		size_t full_size	= h.p_memsz + (addr - target);
		addr_t img_dat		= segment_ptr(n);
		block_descriptor* b	= frame_tag->add_block(full_size, target, h.p_align, is_write(h), is_exec(h));
		if(__unlikely(!b)) {
			panic("[PRG/EXEC] failed allocate blocks for executable");
			return false;
		}
		if(is_tls(h)) {
			tls_base		= b->virtual_start;
			tls_size		= b->size;
		}
		addr_t idmap		= frame_tag->translate(addr);
		array_copy<uint8_t>(idmap, img_dat, h.p_filesz);
		if(h.p_memsz > h.p_filesz) array_zero<uint8_t>(idmap.plus(h.p_filesz), static_cast<size_t>(h.p_memsz - h.p_filesz));
		program_segment_descriptor desc
		{
			.absolute_addr	= idmap,
			.virtual_addr	= addr,
			.obj_offset		= static_cast<off_t>(h.p_offset),
			.size			= h.p_memsz,
			.seg_align		= h.p_align,
			.perms			= static_cast<elf_segment_prot>(0b100UC | (is_write(h) ? 0b010UC : 0) | (is_exec(h) ? 0b001UC : 0))
		};
		segments.push_back(desc);
	}
	block_descriptor* s		= frame_tag->add_block(stack_size, stack_base, page_size, true, false);
	if(__unlikely(!s)) { panic("[PRG/EXEC] failed to allocate block for stack"); return false; }
	frame_tag->extent		= frame_extent;
	frame_tag->mapped_max	= frame_extent;
	new(std::addressof(program_descriptor)) elf64_program_descriptor
	{
		.frame_ptr			= frame_tag,
		.prg_stack			= s->virtual_start,
		.stack_size			= s->size,
		.prg_tls			= tls_base,
		.tls_size			= tls_size,
		.tls_align			= tls_align,
		.entry				= entry,
		.ld_path			= nullptr,
		.ld_path_count		= 0UZ,
		.object_handle		= this
	};
	return true;
}