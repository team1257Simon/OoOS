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