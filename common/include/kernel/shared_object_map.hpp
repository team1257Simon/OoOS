#ifndef __DYNAMIC_LINK_MAP
#define __DYNAMIC_LINK_MAP
#include "unordered_set"
#include "unordered_map"
#include "elf64_shared.hpp"
/* The MODE argument to `dlopen' contains one of the following: */
#define RTLD_LAZY       0x0001
#define RTLD_NOW        0x0002
#define RTLD_NOLOAD	    0x0004
#define RTLD_DEEPBIND   0x0008
#define RTLD_PREINIT    0x0010
#define RTLD_GLOBAL	    0x0100
#define RTLD_NODELETE   0x1000
class shared_object_map : protected std::hash_set<elf64_shared_object, std::string, std::hash<std::string>, std::equal_to<void>, std::allocator<elf64_shared_object>, decltype([](elf64_shared_object const& o) -> std::string const& { return o.get_soname(); })>
{
    using __base = std::hash_set<elf64_shared_object, std::string, std::hash<std::string>, std::equal_to<void>, std::allocator<elf64_shared_object>, __key_extract>;
    using typename __base::__node_ptr;
    using typename __base::__const_node_ptr;
    using typename __base::__node_type;
    static shared_object_map __globals;
    static iterator __ld_so;
    std::unordered_map<iterator, std::string> __obj_paths;
    static addr_t __global_dynamic_extent();
public:
    uframe_tag* shared_frame;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
    constexpr static off_t node_offset = offsetof(__node_type, __data.__my_storage);
#pragma GCC diagnostic pop
    using typename __base::iterator;
    using typename __base::const_iterator;
    using typename __base::reference;
    using typename __base::const_reference;
    using typename __base::pointer;
    using typename __base::const_pointer;
    using typename __base::size_type;
    using typename __base::difference_type;
    shared_object_map(uframe_tag* frame = nullptr, size_type init_ct = 128UL);
    ~shared_object_map();
    elf64_shared_object& operator[](std::string const& name);
    elf64_shared_object const& operator[](std::string const& name) const;
    bool contains(std::string const& name) const;
    iterator begin() noexcept;
    const_iterator begin() const noexcept;
    const_iterator cbegin() const noexcept;
    iterator end() noexcept;
    const_iterator end() const noexcept;
    const_iterator cend() const noexcept;
    iterator find(std::string const& what) noexcept;
    const_iterator find(std::string const& what) const noexcept;
    void clear();
    size_type size() const noexcept;
    iterator add(file_node* so_file);
    iterator get_if_resident(file_node* so_file);
    bool remove(iterator so_handle);
    iterator transfer(shared_object_map& that, iterator handle);
    void set_path(iterator obj, std::string const& path);
    const char* get_path(iterator obj) const;
    void copy(shared_object_map const& that);
    static shared_object_map& get_globals();
    static iterator get_ldso_object(filesystem* fs);
};
struct dl_addr_info
{
    const char* so_name;
    void*       so_vbase;
    const char* symbol_name;
    void*       actual_addr;
};
extern "C"
{
    addr_t syscall_dlopen(const char* name, int flags);                             // void* dlopen(const char* name, int flags);
    int syscall_dlclose(addr_t handle);                                             // int dlclose(void* hadle);
    addr_t syscall_dlsym(addr_t handle, const char* name);                          // void* dlsym(void* restrict handle, const char* restrict name);
    addr_t syscall_resolve(uint32_t sym_idx, addr_t got_offset_1);                  // void* dlresolve(int sym_idx); 
    int syscall_dlmap(elf64_dynamic_object* obj, elf64_dlmap_entry* ent);           // int dlmap(void* restrict handle, struct link_map* restrict ent);
    int syscall_dlpath(const char* path_str);                                       // int dlpath(const char* path_str); add a colon-separated list of strings, possibly terminated with a semicolon, to the list of search paths for shared objects
    int syscall_dladdr(addr_t sym_addr, dl_addr_info* info);                        // int dladdr(const void* addr, dl_info* info);
}
#ifndef INST_SOM
extern 
#endif
template class std::hash_set<elf64_shared_object, std::string, std::hash<std::string>, std::equal_to<void>, std::allocator<elf64_shared_object>, shared_object_map::__key_extract>;
#endif