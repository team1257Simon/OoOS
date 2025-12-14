#ifndef __ELF64_OBJ
#define __ELF64_OBJ
#include <kernel_mm.hpp>
#include <elf64_index.hpp>
#include <fs/fs.hpp>
class elf64_object
{
	bool __validated{};
	bool __loaded{};
	addr_t __image_start;
	size_t __image_size;
protected:
	size_t num_seg_descriptors{};
	std::vector<program_segment_descriptor> segments{};
	elf64_sym_table symtab{};
	elf64_string_table symstrtab{};
	elf64_string_table shstrtab{};
	addr_t tls_base{};
	size_t tls_size{};
	size_t tls_align{};
	size_t tls_mod_idx{};
	constexpr elf64_ehdr const& ehdr() const noexcept { return __image_start.deref<elf64_ehdr>(); }
	constexpr elf64_phdr const& phdr(size_t n) const noexcept { return __image_start.plus(ehdr().e_phoff + n * ehdr().e_phentsize).deref<elf64_phdr>(); }
	constexpr elf64_shdr const& shdr(size_t n) const noexcept { return __image_start.plus(ehdr().e_shoff + n * ehdr().e_shentsize).deref<elf64_shdr>(); }
	constexpr addr_t img_ptr(size_t offs = 0UZ) const noexcept { return __image_start.plus(offs); }
	constexpr addr_t segment_ptr(size_t n) const noexcept { return __image_start.plus(phdr(n).p_offset); }
	constexpr addr_t section_ptr(size_t n) const noexcept { return __image_start.plus(shdr(n).sh_offset); }
	virtual void process_headers();
	virtual void xrelease();
	virtual bool xload();
	virtual bool load_syms();
	virtual void on_load_failed();
	virtual bool load_segments()			= 0;
	virtual bool xvalidate()				= 0;
	virtual void frame_enter()				= 0;
	virtual void set_frame(uframe_tag*)		= 0;
	virtual uframe_tag* get_frame() const	= 0;
	virtual bool is_position_relocated() const noexcept;
	void release_segments();
	off_t segment_index(size_t offset) const;
	off_t segment_index(elf64_sym const* sym) const;
	void cleanup();
public:
	constexpr elf64_sym const& get_sym(size_t idx) const noexcept { return symtab[idx]; }
	constexpr addr_t module_tls() const noexcept { return tls_base; }
	constexpr size_t module_tls_size() const noexcept { return tls_size; }
	constexpr size_t module_tls_align() const noexcept { return tls_align; }
	constexpr size_t module_tls_index() const noexcept { return tls_mod_idx; }
	constexpr void module_tls_index(size_t idx) noexcept { tls_mod_idx = idx; }
	elf64_object(file_vnode* n);
	elf64_object(addr_t start, size_t size);
	elf64_object(elf64_object const&);
	elf64_object(elf64_object&&);
	virtual addr_t resolve(uint64_t offs) const;
	virtual addr_t resolve(elf64_sym const& sym) const;
	virtual ~elf64_object();
	std::vector<block_descriptor> segment_blocks() const;
	bool validate() noexcept;
	bool load() noexcept;
	void on_copy(uframe_tag* new_frame);
};
#endif