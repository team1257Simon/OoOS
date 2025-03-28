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
    std::unordered_map<iterator, std::string> __obj_paths;
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
    void set_path(iterator obj, std::string const& path);
    const char* get_path(iterator obj) const;
    static shared_object_map& get_globals();
};
extern "C"
{
    addr_t syscall_dlopen(const char* name, int flags);                             // void* dlopen(const char* name, int flags);
    int syscall_dlclose(addr_t handle);                                             // int dlclose(void* hadle);
    addr_t syscall_dlsym(addr_t handle, const char* name);                          // void* dlsym(void* restrict handle, const char* restrict name);
    addr_t syscall_getsym(const char* name);                                        // void* getsym(const char* name); shortcut to syscall_dlsym(syscall_dlopen(0, RTLD_NOW), name)
    int syscall_dlpath(const char* path_str);                                       // int dlpath(const char* path_str); add a colon-separated list of strings, possibly terminated with a semicolon, to the list of search paths for shared objects
    int syscall_dlorigin(addr_t handle, const char** str_out, size_t* sz_out);      // int dlorigin(void* restrict handle, const char** restrict str_out, size_t* restrict sz_out); write the full absolute path of the given SO to the file given by fdout; if sz_out is nonnull, it receives the total number of characters written
}
#endif