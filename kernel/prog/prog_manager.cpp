#include "prog_manager.hpp"
static std::alignas_allocator<char, elf64_ehdr> elf_alloc{};
prog_manager prog_manager::__instance{};
constexpr static elf64_phdr const& phdr(elf64_ehdr const& ehdr, size_t i) { return addr_t(std::addressof(ehdr)).plus(ehdr.e_phoff + i * ehdr.e_phentsize).ref<elf64_phdr>(); }
constexpr static bool is_dynamic_object(elf64_ehdr const& ehdr) { for(size_t i = 0; i < ehdr.e_phnum; i++) { if(phdr(ehdr, i).p_type == PT_DYNAMIC) return true; } return false; }
prog_manager::prog_manager() : __static_base(), __dynamic_base() {}
prog_manager& prog_manager::get_instance() { return __instance; }
elf64_executable* prog_manager::__add(addr_t img_start, size_t img_size, size_t stack_sz, size_t tls_sz)
{
    if(__builtin_memcmp(img_start, "\177ELF", 4) != 0) { panic("missing identifier; invalid object"); return nullptr; }
    if(is_dynamic_object(img_start.ref<elf64_ehdr>()))
    {
        __dynamic_base::iterator result = __dynamic_base::emplace_back(img_start, img_size, stack_sz, tls_sz, 0UL);
        if(__unlikely(!result->load())) { __dynamic_base::erase(result); return nullptr; }
        return result.base();
    }
    __static_base::iterator result = __static_base::emplace_back(img_start, img_size, stack_sz, tls_sz);
    if(__unlikely(!result->load())) { __static_base::erase(result); return nullptr; }
    return result.base();
}
elf64_executable* prog_manager::add(file_node* exec_file, size_t stack_sz, size_t tls_sz)
{
    size_t size     = static_cast<size_t>(exec_file->size() - exec_file->tell());
    addr_t start    = elf_alloc.allocate(size);
    if(__unlikely(!exec_file->read(start, size)))
    { 
        elf_alloc.deallocate(start, size); 
        panic("read failed"); 
        return nullptr;
    }
    if(elf64_executable* result = __add(start, size, stack_sz, tls_sz); !__unlikely(!result)) return result;
    else
    {
        elf_alloc.deallocate(start, size);
        panic("load failed");
        return nullptr;
    }
}
void prog_manager::remove(elf64_executable* e)
{
    if(elf64_dynamic_executable* dyn = dynamic_cast<elf64_dynamic_executable*>(e))
        __dynamic_base::erase(__dynamic_base::iterator(addr_t(dyn).minus(__dynamic_node_offset)));
    else
        __static_base::erase(__static_base::iterator(addr_t(e).minus(__static_node_offset)));
}
elf64_executable* prog_manager::clone(elf64_executable const* exec)
{
    try
    {
        if(elf64_dynamic_executable const* dyn = dynamic_cast<elf64_dynamic_executable const*>(exec))
            return __dynamic_base::emplace_back(*dyn).base();
        return __static_base::emplace_back(*exec).base();
    }
    catch(std::exception& e) { panic(e.what()); }
    return nullptr;
}