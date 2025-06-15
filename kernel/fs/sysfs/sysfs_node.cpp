#include "fs/sysfs.hpp"
#include "sys/errno.h"
sysfs_inode& sysfs_vnode::inode() { return parent_fs.get_inode(ino); }
sysfs_inode const& sysfs_vnode::inode() const { return parent_fs.get_inode(ino); }
sysfs& sysfs_vnode::parent() { return parent_fs; }
uint32_t sysfs_vnode::inode_number() const { return ino; }
std::streamsize sysfs_vnode::sector_size() const { return sysfs_data_block_size; }
std::streamsize sysfs_vnode::on_overflow(std::streamsize n)
{
    size_t target = inode().size_bytes + n;
    if(target > extent_tree.total_extent() * sector_size())
    {
        size_t over = target - extent_tree.total_extent() * sector_size();
        try { extent_tree.push(std::max(1US, static_cast<uint16_t>(over / sector_size()))); }
        catch(std::exception& e) { panic(e.what()); return 0UZ; }
    }
    inode().size_bytes += n;
    if(__grow_buffer(up_to_nearest(n, sector_size()))) return n;
    throw std::bad_alloc();
}
std::streamsize sysfs_vnode::xsputn(char const* s, std::streamsize n)
{
    std::streamsize st_block    = sector_of(static_cast<std::streamsize>(tell()));
    std::streamsize result      = __base::xsputn(s, n);
    if(__unlikely(!result)) return 0UZ;
    std::streamsize max_block   = st_block + result / sector_size();
    do { dirty_blocks.push_back(st_block); } while(st_block++ < max_block);
    inode().size_bytes          = std::max(inode().size_bytes, static_cast<size_t>(tell()));
    on_modify();
    return result;
}
int sysfs_vnode::write_dev()
{
    if(dirty_blocks.empty()) return 0;
    try
    {
        for(size_t i : dirty_blocks)
        {
            uint32_t actual = extent_tree[i].start;
            for(size_t j = 0; i < extent_tree[i].length; j++, actual++)
                parent_fs.write_data(actual, sector_ptr(actual), sector_size());
        }
    }
    catch(std::exception& e) { panic(e.what()); return -EINVAL; }
    dirty_blocks.clear();
    return 0;
}
void sysfs_vnode::init()
{
    if(size_t ext = extent_tree.total_extent())
    {
        if(!__grow_buffer(extent_tree.total_extent() * sector_size())) throw std::bad_alloc();
        for(size_t i = 0; i < ext; i++)
        {
            uint32_t actual = extent_tree[i].start;
            for(size_t j = 0; i < extent_tree[i].length; j++, actual++)
                parent_fs.read_data(sector_ptr(actual), actual, sector_size());
        }
        __setc(inode().size_bytes);
        sync_ptrs();
    }
    else
    {
        extent_tree.push(1US);
        if(!__grow_buffer(sector_size())) throw std::bad_alloc();
        array_zero(__beg(), sector_size());
        sync_ptrs();
    }
}
sysfs_vnode::sysfs_vnode(sysfs& sysfs_parent, uint32_t inode_num) : 
    parent_fs       { sysfs_parent },
    ino             { inode_num },
    extent_tree     { *this },
    dirty_blocks    {}
                    { init(); }