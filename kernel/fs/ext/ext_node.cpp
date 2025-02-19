#include "fs/ext.hpp"
ext_vnode::ext_vnode(extfs *parent, uint32_t inode_number) : 
    vfs_filebuf_base<char>      {}, 
    parent_fs                   { parent }, 
    on_disk_node                { parent->read_inode(inode_number) }, 
    block_data                  {},
    indirect_block_data         {},
    doubly_indirect_block_data  {},
    triply_indirect_block_data  { nullptr }
                                {}
ext_vnode::~ext_vnode() 
{ 
    if(on_disk_node) std::allocator<ext_inode>{}.deallocate(on_disk_node, 1UL); 
    if(triply_indirect_block_data) { std::allocator<char>{}.deallocate(triply_indirect_block_data->data_buffer, parent_fs->block_size()); std::allocator<disk_block>{}.deallocate(triply_indirect_block_data, 1UL); }
    for(disk_block& b : doubly_indirect_block_data) std::allocator<char>{}.deallocate(b.data_buffer, parent_fs->block_size());
    for(disk_block& b : indirect_block_data) std::allocator<char>{}.deallocate(b.data_buffer, parent_fs->block_size());
}
void ext_vnode::add_block(uint64_t block_number, char *data_ptr) { block_data.emplace_back(block_number, data_ptr); }
int ext_vnode::__ddwrite()
{
    return 0;
}
std::streamsize ext_vnode::__overflow(std::streamsize n)
{
    return std::streamsize();
}
std::streamsize ext_file_vnode::__ddread(std::streamsize n)
{
    return std::streamsize();
}
std::streamsize ext_file_vnode::__ddrem()
{
    return std::streamsize();
}
ext_file_vnode::size_type ext_file_vnode::write(const_pointer src, size_type n)
{
    return size_type();
}
ext_file_vnode::size_type ext_file_vnode::read(pointer dest, size_type n)
{
    return size_type();
}
ext_file_vnode::pos_type ext_file_vnode::seek(off_type off, std::ios_base::seekdir way)
{
    return pos_type();
}
ext_file_vnode::pos_type ext_file_vnode::seek(pos_type pos)
{
    return pos_type();
}
bool ext_file_vnode::fsync()
{
    return false;
}
uint64_t ext_file_vnode::size() const noexcept
{
    return 0;
}
ext_file_vnode::pos_type ext_file_vnode::tell() const
{
    return pos_type();
}
ext_file_vnode::ext_file_vnode(extfs *parent, uint32_t inode_number, int fd) : ext_vnode{ parent, inode_number }, file_node{ "", fd, inode_number } {}
std::streamsize ext_directory_vnode::__ddread(std::streamsize n)
{
    return std::streamsize();
}
std::streamsize ext_directory_vnode::__ddrem()
{
    return std::streamsize();
}
tnode *ext_directory_vnode::find(std::string const &)
{
    return nullptr;
}
bool ext_directory_vnode::link(tnode *original, std::string const &target)
{
    return false;
}
tnode *ext_directory_vnode::add(fs_node *n)
{
    return nullptr;
}
bool ext_directory_vnode::unlink(std::string const &name)
{
    return false;
}
uint64_t ext_directory_vnode::num_files() const noexcept
{
    return 0;
}
uint64_t ext_directory_vnode::num_subdirs() const noexcept
{
    return 0;
}
std::vector<std::string> ext_directory_vnode::lsdir() const
{
    return std::vector<std::string>();
}
bool ext_directory_vnode::fsync()
{
    return false;
}
ext_directory_vnode::ext_directory_vnode(extfs *parent, uint32_t inode_number) : ext_vnode{ parent, inode_number }, directory_node{ "", inode_number } {}