#ifndef __ELF_DYNAMIC
#define __ELF_DYNAMIC
#include "elf64_object.hpp"
#include "elf64_relocation.hpp"
#include "vector"
class elf64_dynamic_object : public elf64_object
{
protected:
    size_t num_dyn_entries;
    elf64_dyn* dyn_entries;
    size_t num_plt_got_slots;
    elf64_rela* plt_got_slots;
    size_t got_seg_index;
    std::vector<elf64_relocation> relocations;
    std::vector<const char*> dependencies;
    std::vector<addr_t> init_array;
    std::vector<addr_t> fini_array;
    uintptr_t init_fn;
    uintptr_t fini_fn;
    uintptr_t init_array_ptr;
    uintptr_t fini_array_ptr;
    size_t init_array_size;
    size_t fini_array_size;
    elf64_dynsym_index symbol_index;
    virtual bool xload() override;
    virtual bool load_syms();
    virtual bool process_got();
    virtual bool post_load_init() = 0;
    void process_dynamic_relas();
    addr_t resolve_rela_target(elf64_rela const& r) const;
    uint64_t resolve_rela_sym(elf64_sym const& s, elf64_rela const& r) const;
public:
    elf64_dynamic_object(file_node* n);
    elf64_dynamic_object(elf64_dynamic_object const& that);
    elf64_dynamic_object(elf64_dynamic_object&& that);
    virtual ~elf64_dynamic_object();
    virtual void apply_relocations();
    addr_t resolve_by_name(std::string const& symbol) const;
    constexpr std::vector<addr_t> const& get_init() const noexcept { return init_array; }
    constexpr std::vector<addr_t> const& get_fini() const noexcept { return fini_array; }
    constexpr std::vector<const char*> const& get_dependencies() const noexcept { return dependencies; }
};
extern "C"
{
    addr_t syscall_dlinit(elf64_dynamic_object* obj_handle);    // void (**dlinit(void* handle))();
    addr_t syscall_dlfini(elf64_dynamic_object* obj_handle);    // void (**dlfini(void* handle))();
    addr_t syscall_depends(elf64_dynamic_object* obj_handle);   // char** depends(void* handle);
}
#endif