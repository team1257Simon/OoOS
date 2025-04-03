#ifndef __ELF64_DYN_EXEC
#define __ELF64_DYN_EXEC
#include "kernel/elf64_exec.hpp"
#include "elf64_dynamic.hpp"
class elf64_dynamic_executable : public elf64_executable, public elf64_dynamic_object
{
protected:
    addr_t virtual_load_base;
    std::vector<addr_t> preinit_array;
    uintptr_t preinit_array_ptr;
    uintptr_t preinit_array_size;
    virtual bool post_load_init() override;
    virtual void process_dyn_entry(size_t i) override;
    virtual addr_t segment_vaddr(size_t n) const override;
    virtual bool load_segments() override;
    virtual void process_headers() override;
public:
    virtual addr_t resolve(uint64_t offs) const override;
    virtual addr_t resolve(elf64_sym const& sym) const override;
    virtual ~elf64_dynamic_executable();
    elf64_dynamic_executable(file_node* n, uintptr_t base_offset, size_t stack_sz = S04, size_t tls_sz = S04);
    constexpr std::vector<addr_t> const& get_preinit() const noexcept { return preinit_array; }
};
#endif