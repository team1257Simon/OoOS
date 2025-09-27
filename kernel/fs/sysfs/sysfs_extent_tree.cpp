#include "fs/sysfs.hpp"
static size_t total_extents(sysfs& s, uint32_t ino);
sysfs_extent_tree::sysfs_extent_tree(sysfs_vnode& n) : __managed_vnode{ n }, __total_extent{ total_extents(n.parent_fs, n.ino) } {}
size_t sysfs_extent_tree::total_extent() const { return __total_extent; }
sysfs_inode& sysfs_extent_tree::__inode() { return __managed_vnode.inode(); }
sysfs_inode const& sysfs_extent_tree::__inode() const { return __managed_vnode.inode(); }
sysfs_extent_branch& sysfs_extent_tree::__root() { sysfs_inode& n = __inode(); return __managed_vnode.parent_fs.get_extent_branch(n.extent_root.start); }
sysfs_extent_entry& sysfs_extent_tree::__root_first() { return addr_t(std::addressof(__inode())).plus(offsetof(sysfs_inode, extent_root)).ref<sysfs_extent_entry>(); }
sysfs_extent_entry const& sysfs_extent_tree::__root_first() const { return addr_t(std::addressof(__inode())).plus(offsetof(sysfs_inode, extent_root)).ref<sysfs_extent_entry const>(); }
sysfs_extent_branch const& sysfs_extent_tree::__root() const { sysfs_inode const& n = __inode(); return __managed_vnode.parent_fs.get_extent_branch(n.extent_root.start); }
static size_t total_extents(sysfs& s, sysfs_extent_branch const& b)
{
    size_t result = 0UZ;
    if(b.depth)
        for(size_t i = 0; i < sysfs_extent_branch::num_entries; i++)
            result += total_extents(s, s.get_extent_branch(b[i].start));
    else for(size_t i = 0; i < sysfs_extent_branch::num_entries; i++) result += b[i].length;
    return result;
}
static size_t total_extents(sysfs& s, uint32_t ino)
{
    sysfs_inode& node = s.get_inode(ino);
    if(!node.root_depth) return node.extent_root.length;
    return total_extents(s, s.get_extent_branch(node.extent_root.start));
}
sysfs_extent_entry const& sysfs_extent_tree::__find_from(sysfs_extent_branch const& b, size_t idx, size_t pos, size_t incr) const
{
    if(!incr) 
        return b.depth ? __find_from(__managed_vnode.parent_fs.get_extent_branch(b[pos].start), idx) : b[pos];
    else if(b[pos].ordinal > idx || !b[pos].start)
        return __find_from(b, idx, pos - incr, incr / 2);
    else
        return __find_from(b, idx, pos + incr, incr / 2);
}
void sysfs_extent_tree::__overflow_root()
{
    uint32_t next           = __managed_vnode.parent_fs.add_extent_branch();
    if(__unlikely(!next)) throw std::bad_alloc();
    sysfs_extent_entry& r   = __root_first();
    sysfs_extent_branch* br = new(std::addressof(__managed_vnode.parent_fs.get_extent_branch(next))) sysfs_extent_branch
    { 
        .depth      { __managed_vnode.inode().root_depth }, 
        .entries    { r },
        .checksum   { 0U }
    };
    uint32_t csum           = crc32c_x86_3way(~0U, reinterpret_cast<uint8_t const*>(br), offsetof(sysfs_extent_branch, checksum));
    br->checksum            = csum;
    r.start                 = next;
    __managed_vnode.inode().root_depth++;
}
sysfs_extent_branch& sysfs_extent_tree::__next_leaf_branch()
{
    __stored_leaf_index                             = 0UZ;
    std::pair<sysfs_extent_branch*, size_t> avail   = __managed_vnode.parent_fs.next_available_extent_entry(__inode().extent_root.start);
    if(avail.first)
    {
        sysfs_extent_branch& b = *avail.first;
        if(b.depth) 
            return __managed_vnode.parent_fs.extend_to_leaf(b[avail.second].start, static_cast<uint32_t>(__total_extent));
        __stored_leaf_index = avail.second;
        return b;
    }
    __overflow_root();
    uint32_t added_ext          = __managed_vnode.parent_fs.add_extent_branch();
    if(__unlikely(!added_ext)) throw std::runtime_error("[FS/SYSFS/EXTENT] failed to expand extents file");
    sysfs_extent_branch& root   = __root();
    new(std::addressof(root[1])) sysfs_extent_entry
    {
        .ordinal    { static_cast<uint32_t>(__total_extent) },
        .length     {},
        .start      { added_ext }
    };
    uint32_t csum               = crc32c_x86_3way(~0U, reinterpret_cast<uint8_t const*>(std::addressof(root)), offsetof(sysfs_extent_branch, checksum));
    root.checksum               = csum;
    return __managed_vnode.parent_fs.extend_to_leaf(added_ext, static_cast<uint32_t>(__total_extent));
}
sysfs_extent_entry const& sysfs_extent_tree::operator[](size_t i) const
{
    if(__unlikely(i >= __total_extent))
        throw std::out_of_range("[FS/SYSFS/EXTENT] block ordinal " + std::to_string(i) + " is out of range for size " + std::to_string(__total_extent));
    if(!__inode().root_depth)
        return __root_first();
    return __find_from(__root(), i);
}
void sysfs_extent_tree::push(uint16_t n_blocks)
{
    uint32_t added_start = __managed_vnode.parent_fs.add_blocks(n_blocks);
    if(__unlikely(!added_start)) throw std::runtime_error("[FS/SYSFS/EXTENT] failed to expand data file");
    sysfs_inode& node = __managed_vnode.inode();
    if(!node.root_depth) 
    {
        __overflow_root();
        sysfs_extent_branch& root = __root();
        new(std::addressof(root[0])) sysfs_extent_entry
        {
            .ordinal    { static_cast<uint32_t>(__total_extent) },
            .length     { n_blocks },
            .start      { added_start }
        };
        uint32_t csum           = crc32c_x86_3way(~0U, reinterpret_cast<uint8_t const*>(std::addressof(root)), offsetof(sysfs_extent_branch, checksum));
        root.checksum           = csum;
    }
    else
    {
        sysfs_extent_branch& leaf_br = __next_leaf_branch();
        new(std::addressof(leaf_br[__stored_leaf_index])) sysfs_extent_entry
        {
            .ordinal    { static_cast<uint32_t>(__total_extent) },
            .length     { n_blocks },
            .start      { added_start }
        };
        uint32_t csum       = crc32c_x86_3way(~0U, reinterpret_cast<uint8_t const*>(std::addressof(leaf_br)), offsetof(sysfs_extent_branch, checksum));
        leaf_br.checksum    = csum;
    }
    __total_extent += n_blocks;
}