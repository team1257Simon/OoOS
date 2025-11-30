#ifndef __ELF64_SHARED
#define __ELF64_SHARED
#include <elf64_dynamic.hpp>
class elf64_shared_object : public elf64_dynamic_object
{
	friend class shared_object_map;
protected:
	std::string soname;
	addr_t virtual_load_base;
	size_t total_segment_size{};
	uframe_tag* frame_tag;
	size_t ref_count = 1UZ;
	bool sticky{};
	bool symbolic{};
	bool global{};
	addr_t entry{};
	virtual void process_headers() override;
	virtual bool load_segments() override;
	virtual bool xvalidate() override;
	virtual void xrelease() override;
	virtual void frame_enter() override;
	virtual void process_flags(elf_dyn_flags flags) override;
	virtual void process_dyn_entry(size_t i) override;
	virtual void set_frame(uframe_tag* ft) override;
	virtual uframe_tag* get_frame() const override;
	virtual bool is_position_relocated() const noexcept override;
public:
	constexpr std::string const& get_soname() const { return soname; }
	constexpr void incref() noexcept { ref_count++; }
	constexpr void decref() noexcept { ref_count--; }
	constexpr size_t refs() const noexcept { return ref_count; }
	constexpr void set_sticky(bool value = true) noexcept { sticky = value; }
	constexpr void set_global(bool value = true) noexcept { global = value; }
	constexpr bool is_sticky() const noexcept { return sticky; }
	constexpr bool is_symbolic() const noexcept { return symbolic; }
	constexpr bool is_global() const noexcept { return global; }
	constexpr addr_t get_load_offset() const noexcept { return virtual_load_base; }
	constexpr addr_t entry_point() const { return entry; }
	constexpr bool could_contain(addr_t addr) const noexcept { return addr >= virtual_load_base && virtual_load_base.plus(total_segment_size) > addr; }
	virtual addr_t resolve(uint64_t offs) const override;
	virtual addr_t resolve(elf64_sym const& sym) const override;
	const char* sym_lookup(addr_t addr) const;
	program_segment_descriptor const* segment_of(addr_t symbol_vaddr) const;
	elf64_shared_object(file_vnode* n, uframe_tag* frame);
	elf64_shared_object(elf64_shared_object&& that);
	elf64_shared_object(elf64_shared_object const& that);
	virtual ~elf64_shared_object();
};
#endif