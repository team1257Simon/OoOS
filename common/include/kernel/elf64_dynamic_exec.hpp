#ifndef __ELF64_DYN_EXEC
#define __ELF64_DYN_EXEC
#include <elf64_exec.hpp>
#include <elf64_dynamic.hpp>
class elf64_dynamic_executable : public elf64_executable, public elf64_dynamic_object
{
protected:
	addr_t virtual_load_base;
	std::vector<addr_t> preinit_array{};
	uintptr_t preinit_array_ptr{};
	uintptr_t preinit_array_size{};
	virtual bool xload() override;
	virtual bool load_preinit() override;
	virtual void process_dyn_entry(size_t i) override;
	virtual addr_t segment_vaddr(size_t n) const override;
	virtual bool load_segments() override;
	virtual bool load_syms() override;
	virtual void process_headers() override;
public:
	constexpr std::vector<addr_t> const& get_preinit() const noexcept { return preinit_array; }
	virtual addr_t resolve(uintptr_t offs) const override;
	virtual addr_t resolve(elf64_sym const& sym) const override;
	virtual ~elf64_dynamic_executable();
	elf64_dynamic_executable(file_vnode* n, size_t stack_sz = S04, uintptr_t base_offset = 0UL);
	elf64_dynamic_executable(addr_t start, size_t size, size_t stack_sz = S04, uintptr_t base_offset = 0UL);
	elf64_dynamic_executable(elf64_dynamic_executable const& that);
};
#endif