#ifndef __ELF_DYNAMIC
#define __ELF_DYNAMIC
#include "elf64_object.hpp"
#include "elf64_relocation.hpp"
#include "vector"
class elf64_dynamic_object : public virtual elf64_object
{
protected:
    size_t num_dyn_entries;
    elf64_dyn* dyn_entries;
    size_t num_plt_relas;
    elf64_rela* plt_relas;
    size_t got_vaddr;
    size_t dyn_segment_idx;
    std::vector<elf64_relocation> relocations;
    std::vector<const char*> dependencies;
    std::vector<std::string> ld_paths;
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
    virtual bool process_got();
    virtual bool post_load_init() = 0;
    virtual void process_dyn_entry(size_t i);
    bool load_syms();
    void process_dynamic_relas();
    uint64_t resolve_rela_sym(elf64_sym const& s, elf64_rela const& r) const;
public:
    addr_t resolve_rela_target(elf64_rela const& r) const;
    elf64_dynamic_object(file_node* n);
    elf64_dynamic_object(elf64_dynamic_object const& that);
    elf64_dynamic_object(elf64_dynamic_object&& that);
    virtual ~elf64_dynamic_object();
    virtual void apply_relocations();
    addr_t resolve_by_name(std::string const& symbol) const;
    addr_t global_offset_table() const;
    addr_t dyn_segment_ptr() const;
    constexpr size_t dyn_segment_len() const noexcept { return num_dyn_entries; }
    constexpr std::vector<addr_t> const& get_init() const noexcept { return init_array; }
    constexpr std::vector<addr_t> const& get_fini() const noexcept { return fini_array; }
    constexpr std::vector<const char*> const& get_dependencies() const noexcept { return dependencies; }
    constexpr std::vector<std::string> const& get_ld_paths() const noexcept { return ld_paths; }
    constexpr elf64_rela const& get_plt_rela(unsigned idx) const noexcept { return plt_relas[idx]; }
};
extern "C"
{
    addr_t syscall_dlinit(elf64_dynamic_object* obj_handle);    // void (**dlinit(void* handle))(int argc, char** argv, char** env);
    addr_t syscall_dlpreinit(elf64_dynamic_object* obj_handle); // void (**dlpreinit(void* handle))(int argc, char** argv, char** env);
    addr_t syscall_dlfini(elf64_dynamic_object* obj_handle);    // void (**dlfini(void* handle))();
    addr_t syscall_depends(elf64_dynamic_object* obj_handle);   // char** depends(void* handle);
}
#endif