#ifndef __ELF_DYNAMIC
#define __ELF_DYNAMIC
#include "elf64_object.hpp"
#include "elf64_relocation.hpp"
#include "vector"
#include <tuple>
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
    std::vector<elf64_rela> object_relas;
    std::vector<std::string> dependencies;
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
    virtual bool load_preinit();
    virtual void process_dyn_entry(size_t i);
    bool load_syms();
    void process_dt_relas();
    bool post_load_init();
    uint64_t resolve_rela_sym(elf64_sym const& s, elf64_rela const& r) const;
    size_t to_image_offset(size_t offs);
public:
    addr_t resolve_rela_target(elf64_rela const& r) const;
    elf64_dynamic_object(file_node* n);
    elf64_dynamic_object(addr_t start, size_t size);
    elf64_dynamic_object(elf64_dynamic_object const& that);
    elf64_dynamic_object(elf64_dynamic_object&& that);
    virtual ~elf64_dynamic_object();
    virtual void apply_relocations();
    std::pair<elf64_sym, addr_t> resolve_by_name(std::string const& symbol) const;
    addr_t global_offset_table() const;
    addr_t dyn_segment_ptr() const;
    constexpr size_t dyn_segment_len() const noexcept { return num_dyn_entries; }
    constexpr std::vector<addr_t> const& get_init() const noexcept { return init_array; }
    constexpr std::vector<addr_t> const& get_fini() const noexcept { return fini_array; }
    constexpr std::vector<std::string> const& get_dependencies() const noexcept { return dependencies; }
    constexpr std::vector<std::string> const& get_ld_paths() const noexcept { return ld_paths; }
    constexpr bool has_plt_relas() const noexcept { return plt_relas != nullptr; }
    constexpr elf64_rela const& get_plt_rela(unsigned idx) const noexcept { return plt_relas[idx]; }
    constexpr const char* symbol_name(elf64_sym const& sym) const noexcept { return symstrtab[sym.st_name]; }
    constexpr std::vector<elf64_rela> const& get_object_relas() const noexcept { return object_relas; }
};
extern "C"
{
    addr_t syscall_dlinit(addr_t handle);    // void (**dlinit(void* handle))(int argc, char** argv, char** env);
    addr_t syscall_dlpreinit(addr_t handle); // void (**dlpreinit(void* handle))(int argc, char** argv, char** env);
    addr_t syscall_dlfini(addr_t handle);    // void (**dlfini(void* handle))();
    addr_t syscall_depends(addr_t handle);   // char** depends(void* handle);
}
#endif