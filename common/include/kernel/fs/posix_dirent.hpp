#ifndef __POSIX_DIRENT
#define __POSIX_DIRENT
#include "sys/dirent.h"
#include "fs/fs.hpp"
#include "kernel_mm.hpp"
struct directory_buffer
{
    addr_t buffer_start;
    size_t buffer_size;
    DIR cstruct_dir;
};
class posix_directory
{
    uframe_tag* __owning_frame;
    addr_t __buffer_mapped_vaddr;
    directory_buffer* __my_dir_buffer;
public:
    posix_directory(directory_vnode* dirnode, uframe_tag* calling_tag);
    posix_directory(uframe_tag* owning_tag, addr_t buffer_vaddr);
    ~posix_directory();
    posix_directory(posix_directory&& that);
    posix_directory(posix_directory const& that);
    posix_directory& operator=(posix_directory&& that);
    posix_directory& operator=(posix_directory const& that);
    constexpr addr_t get_base_vaddr() const { return __buffer_mapped_vaddr; }
    constexpr addr_t get_dir_struct_vaddr() const { return __buffer_mapped_vaddr.plus(offsetof(directory_buffer, cstruct_dir)); }
    constexpr directory_buffer* get_buffer_struct() { return __my_dir_buffer; }
};
#endif