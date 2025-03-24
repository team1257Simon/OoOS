#ifndef __DYNAMIC_LINK_MAP
#define __DYNAMIC_LINK_MAP
#include "unordered_set"
#include "unordered_map"
#include "elf64_shared.hpp"
/* The MODE argument to `dlopen' contains one of the following: */
#define RTLD_LAZY 0x0001
#define RTLD_NOW 0x0002
#define RTLD_NOLOAD	0x0004
#define RTLD_DEEPBIND 0x0008
#define RTLD_GLOBAL	0x0100
#define RTLD_NODELETE 0x1000
class shared_object_map : protected std::hash_set<elf64_shared_object, std::string, std::hash<std::string>, std::equal_to<void>, std::allocator<std::__impl::__hash_node<elf64_shared_object>>, decltype([](elf64_shared_object const& o) -> std::string const& { return o.get_soname(); })>
{
    using __base = std::hash_set<elf64_shared_object, std::string, std::hash<std::string>, std::equal_to<void>, std::allocator<std::__impl::__hash_node<elf64_shared_object>>, __key_extract>;
    using typename __base::__node_ptr;
    using typename __base::__const_node_ptr;
    static shared_object_map __global_shared_objects;
public:
    uframe_tag* shared_frame;
    using typename __base::iterator;
    using typename __base::const_iterator;
    using typename __base::reference;
    using typename __base::const_reference;
    using typename __base::pointer;
    using typename __base::const_pointer;
    using typename __base::size_type;
    using typename __base::difference_type;
    shared_object_map(uframe_tag* frame = nullptr, size_type init_ct = 128UL);
    shared_object_map(shared_object_map const& that, uframe_tag* nframe);
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
    static shared_object_map& get_globals();
};
// hashtable iterators can be passed/returned in registers (the assembly simply sees the integer value of the iterator's pointer); this means they're fine as handles.
extern "C"
{
    shared_object_map::iterator syscall_dlopen(const char* name, int flags);
    int syscall_dlclose(shared_object_map::iterator handle);
    addr_t syscall_dlsym(shared_object_map::iterator handle, const char* name);
}
#endif