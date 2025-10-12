#include "elf64_kernel_object.hpp"
#include "kdebug.hpp"
using ooos_kernel_module::abstract_module_base;
using ooos_kernel_module::module_takedown;
static inline abstract_module_base* init_mod(abstract_module_base* mod) { if(mod) mod->initialize(); return mod; }
uframe_tag* elf64_kernel_object::get_frame() const { return nullptr; }
void elf64_kernel_object::frame_enter() {}
void elf64_kernel_object::set_frame(uframe_tag* ft) {}
addr_t elf64_kernel_object::translate_in_frame(addr_t addr) { return addr; }
elf64_kernel_object::elf64_kernel_object(file_node* file) : elf64_object(file), elf64_dynamic_object(file), load_base{}, load_align{}, entry{}, module_object{} {}
elf64_kernel_object::~elf64_kernel_object() { if(module_object) module_takedown(module_object); if(load_base) ::operator delete(load_base, load_align); }
void elf64_kernel_object::on_load_failed() { if(load_base) ::operator delete(load_base, load_align); load_base = nullptr; }
void elf64_kernel_object::process_headers()
{
    elf64_dynamic_object::process_headers();
    size_t seg_base = 0UZ, extent = 0UZ, needed_align = 0UZ;
    size_t n        = ehdr().e_phnum;
    for(size_t i = 0; i < n; i++)
    {
        elf64_phdr const& ph    = phdr(i);
        if(!is_load(ph)) continue;
        needed_align            = std::max(needed_align, ph.p_align);
        seg_base                = addr_t(std::max(extent, ph.p_vaddr)).alignup(ph.p_align);
        extent                  = seg_base + ph.p_memsz;
    }
    load_align      = static_cast<std::align_val_t>(needed_align);
    load_base       = ::operator new(extent, load_align);
}
bool elf64_kernel_object::load_segments()
{
    size_t i        = 0;
    size_t n        = ehdr().e_phnum;
    bool have_loads = false;
    for(size_t j = 0; j < n; j++)
    {
        elf64_phdr const& ph = phdr(j);
        if(!is_load(ph) || !ph.p_memsz) continue;
        addr_t addr     = load_base.plus(ph.p_vaddr);
        addr_t img_dat  = img_ptr(ph.p_offset);
        array_copy<uint8_t>(addr, img_dat, ph.p_filesz);
        if(ph.p_memsz > ph.p_filesz) array_zero<uint8_t>(addr.plus(ph.p_filesz), static_cast<size_t>(ph.p_memsz - ph.p_filesz));
        new(std::addressof(segments[i++])) program_segment_descriptor
        {
            .absolute_addr  = addr,
            .virtual_addr   = addr,
            .obj_offset     = static_cast<off_t>(ph.p_offset),
            .size           = ph.p_memsz,
            .seg_align      = ph.p_align,
            .perms          = PV_RWX
        };
        have_loads      = true;
    }
    return have_loads;
}
bool elf64_kernel_object::load_syms()
{
    if(__unlikely(!elf64_dynamic_object::load_syms())) return false;
    uint64_t e_entry    = ehdr().e_entry;
    if(__unlikely(!e_entry)) { panic("[KO/LOADER] no module setup function found"); return false; }
    entry               = load_base.plus(e_entry);
    return true;
}
bool elf64_kernel_object::xvalidate()
{
    if(__unlikely(ehdr().e_machine != EM_AMD64 || ehdr().e_ident[elf_ident_enc_idx] != ED_LSB || ehdr().e_ident[elf_ident_class_idx] != EC_64)) { panic("[KO/LOADER] not a valid kernel module object"); return false; }
    if(__unlikely(!ehdr().e_phnum)) { panic("[KO/LOADER] no program headers present"); return false; }
    return true;
}
abstract_module_base* elf64_kernel_object::load_module()
{
    if(__unlikely(module_object != nullptr)) return module_object;
    if(__unlikely(!load())) return nullptr;
    module_object = entry.invoke<abstract_module_base*(ooos_kernel_module::kernel_api*)>(ooos_kernel_module::get_api_instance());
    if(module_object) module_object->initialize();
    return module_object;
}