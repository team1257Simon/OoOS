#include "fs/ext.hpp"
#include "fs/hda_ahci.hpp"
#include "sys/errno.h"
size_t extfs::block_size() { return 1024UL << sb->block_size_shift; }
size_t extfs::inodes_per_block() { return sb->inode_size / block_size(); }
size_t extfs::inodes_per_group() { return sb->inodes_per_group; }
size_t extfs::sectors_per_block() { return block_size() / physical_block_size; }
uint64_t extfs::block_to_lba(uint64_t block) { return static_cast<uint64_t>(superblock_lba - sb_off) + sectors_per_block() * static_cast<uint64_t>(block); }
directory_node *extfs::get_root_directory() { return root_dir; }
ext_jbd2_mode extfs::journal_mode() const { return ordered; /* TODO get this from mount options */ }
bool extfs::read_hd(void *dest, uint64_t lba_src, size_t sectors) { return ahci_hda::is_initialized() && ahci_hda::read(static_cast<char*>(dest), lba_src, sectors); }
bool extfs::write_hd(uint64_t lba_dest, const void *src, size_t sectors) { return ahci_hda::is_initialized() && ahci_hda::write(lba_dest, static_cast<char const*>(src), sectors); }
bool extfs::read_from_disk(disk_block &bl) { return read_hd(bl.data_buffer, block_to_lba(bl.block_number), sectors_per_block() * std::max(bl.chain_len, 1UL)); }
bool extfs::write_to_disk(disk_block const &bl) { return write_hd(block_to_lba(bl.block_number), bl.data_buffer, sectors_per_block() * std::max(bl.chain_len, 1UL)); }
bool extfs::persist(ext_directory_vnode *n) { return fs_journal.create_txn(n); /* directory entry data must go through the journal */ }
size_t extfs::blocks_per_group() { return sb->blocks_per_group; }
char *extfs::allocate_block_buffer() { char* result = std::allocator<char>{}.allocate(block_size()); array_zero(result, block_size()); return result; }
void extfs::free_block_buffer(disk_block& bl) { std::allocator<char>{}.deallocate(bl.data_buffer, block_size()); }
off_t extfs::inode_block_offset(uint32_t inode) { return off_t((inode % inodes_per_block()) * sb->inode_size); }
uint64_t extfs::group_num_for_inode(uint32_t inode) { return (static_cast<size_t>(inode - 1)) / sb->inodes_per_group; }
dev_t extfs::xgdevid() const noexcept { return sb->fs_uuid.data_a; }
extfs::extfs(uint64_t volume_start_lba) : 
    filesystem          {}, 
    file_nodes          {}, 
    dir_nodes           {},
    block_groups        {},
    sb                  { std::allocator<ext_superblock>{}.allocate(1UL) }, 
    blk_group_descs     {}, 
    root_dir            {}, 
    num_blk_groups      { 0UL }, 
    superblock_lba      { volume_start_lba + sb_off },
    fs_journal          {}
                        {}
extfs::~extfs() 
{ 
    if(sb) { std::allocator<ext_superblock>{}.deallocate(sb, 1UL); } 
    block_groups.clear(); // destruct these now to avoid dangling pointer shenanigans
    if(blk_group_descs) { std::allocator<block_group_descriptor>{}.deallocate(blk_group_descs, num_blk_groups); } 
}
ext_block_group::ext_block_group(extfs *parent, block_group_descriptor *desc) : 
    parent_fs       { parent }, 
    descr           { desc }, 
    inode_usage_bmp { qword(desc->inode_usage_bitmap_block_idx, desc->inode_usage_bitmap_block_idx_hi), parent->allocate_block_buffer() },
    blk_usage_bmp   { qword(desc->block_usage_bitmap_block_idx, desc->block_usage_bitmap_block_idx_hi), parent->allocate_block_buffer() },
    inode_blocks    {}
                    {}
ext_block_group::~ext_block_group()
{
    parent_fs->free_block_buffer(inode_usage_bmp);
    parent_fs->free_block_buffer(blk_usage_bmp);
    for(std::vector<disk_block>::iterator i = inode_blocks.begin(); i != inode_blocks.end(); i++) parent_fs->free_block_buffer(*i);
}
void extfs::initialize()
{
    if(!(sb && read_hd(sb, superblock_lba, sb_sectors))) throw std::runtime_error{ "failed to read superblock" };
    uint64_t block_cnt = qword(sb->block_count, sb->block_count_hi);
    uint64_t group_count_by_blocks = div_roundup(block_cnt, sb->blocks_per_group);
    uint64_t group_count_by_inodes = div_roundup(sb->inode_count, sb->inodes_per_group);
    if(group_count_by_blocks != group_count_by_inodes) { throw std::logic_error{ "inode block group count of " + std::to_string(group_count_by_inodes) + " does not match block group count of " + std::to_string(group_count_by_blocks) }; }
    num_blk_groups = group_count_by_blocks;
    size_t inode_blocks_per_group = div_roundup(sb->inodes_per_group, inodes_per_block());
    if(!(num_blk_groups && blk_group_descs && read_hd(blk_group_descs, block_to_lba(1UL), (num_blk_groups * sizeof(block_group_descriptor)) / physical_block_size))) throw std::runtime_error{ "failed to read block group table" };
    for(size_t i = 0; i < num_blk_groups; i++)
    {
        ext_block_group bg(this, blk_group_descs + i);
        if(!read_from_disk(bg.inode_usage_bmp) || !read_from_disk(bg.blk_usage_bmp)) throw std::runtime_error{ "failed to read block group" };
        uint64_t inode_table_start = qword(bg.descr->inode_table_start_block, bg.descr->inode_table_start_block_hi);
        for(size_t j = 0; j < inode_blocks_per_group; j++)
        {
            disk_block ibl{ inode_table_start + j, allocate_block_buffer() };
            if(!read_from_disk(ibl)) { free_block_buffer(ibl); throw std::runtime_error{ "failed to read inode table" }; }
            bg.inode_blocks.push_back(ibl);
        }
        block_groups.push_back(bg);
    }
}

uint64_t extfs::inode_to_block(uint32_t inode)
{
    size_t grp = group_num_for_inode(inode);
    size_t idx = (static_cast<size_t>(inode - 1)) % sb->inodes_per_group;
    return qword(blk_group_descs[grp].inode_table_start_block, blk_group_descs[grp].inode_table_start_block_hi) + (static_cast<uint64_t>(idx * sb->inode_size) / block_size());
}
ext_inode* extfs::read_inode(uint32_t inode_num)
{
    if(group_num_for_inode(inode_num) >= block_groups.size()) throw std::out_of_range{ "invalid inode group" };
    if(((inode_num % sb->inodes_per_group) / inodes_per_block()) >= block_groups[group_num_for_inode(inode_num)].inode_blocks.size()) throw std::out_of_range{ "invalid inode index" };
    ext_inode* result = reinterpret_cast<ext_inode*>((block_groups[group_num_for_inode(inode_num)].inode_blocks[(inode_num % sb->inodes_per_group) / inodes_per_block()].data_buffer + inode_block_offset(inode_num)));
    return result;
}
file_node *extfs::open_fd(tnode* fd)
{
    file_node* n = fd->as_file();
    if(ext_file_vnode* exfn = dynamic_cast<ext_file_vnode*>(n)) { exfn->initialize(); }
    return n;
}
directory_node *extfs::mkdirnode(directory_node* parent, std::string const& name)
{
    return nullptr; // TODO
}
file_node *extfs::mkfilenode(directory_node* parent, std::string const& name)
{
    return nullptr; // TODO
}
void extfs::syncdirs()
{
    // TODO
}
void extfs::dldirnode(directory_node* dd)
{
    // TODO
}
void extfs::dlfilenode(file_node* fd)
{
    // TODO
}
disk_block extfs::claim_blocks(ext_vnode *requestor, size_t how_many)
{
    return disk_block(); // TODO
}
bool extfs::persist(ext_file_vnode* n) 
{ 
    if(journal_mode() != ordered) { return fs_journal.create_txn(n); } 
    for(std::vector<disk_block>::iterator i = n->block_data.begin(); i != n->block_data.end(); i++) 
    { 
        if(!i->dirty || !i->data_buffer) continue; 
        if(!write_to_disk(*i)) { panic("write failed"); sb->last_errno = EPIPE; return false; } 
        i->dirty = false;
    }
    std::vector<disk_block> dirty_metadata{};
    for(std::vector<disk_block>::iterator i = n->cached_metadata.begin(); i != n->cached_metadata.end(); i++) { if(i->data_buffer && i->dirty) { dirty_metadata.push_back(*i); i->dirty = false; } }
    return fs_journal.create_txn(dirty_metadata);
}
bool extfs::persist_group_metadata(size_t group_num)
{
    if(group_num < block_groups.size())
    {
        std::vector<disk_block> blks{};
        blks.push_back(block_groups[group_num].inode_usage_bmp);
        blks.push_back(block_groups[group_num].blk_usage_bmp);
        size_t dsc_blk = (group_num * sizeof(block_group_descriptor)) / block_size();
        blks.emplace_back(dsc_blk + 1, reinterpret_cast<char*>(blk_group_descs) + dsc_blk * block_size(), true, 1U);
        return fs_journal.create_txn(blks);
    }
    panic("block group number out of range");
    return false;
}
bool extfs::persist_inode(uint32_t inode_num)
{
    if(group_num_for_inode(inode_num) >= block_groups.size()) { panic("invalid group number"); return false; }
    if(((inode_num % sb->inodes_per_group) / inodes_per_block()) >= block_groups[group_num_for_inode(inode_num)].inode_blocks.size()) { panic("invalid inode index"); return false; }
    std::vector<disk_block> blks{};
    blks.push_back(block_groups[group_num_for_inode(inode_num)].inode_blocks[(inode_num % sb->inodes_per_group) / inodes_per_block()]);
    return fs_journal.create_txn(blks);
}
fs_node* extfs::dirent_to_vnode(ext_dir_entry* de)
{
    uint32_t idx = de->inode_idx;
    if(sb->required_features & dirent_type)
    {
        if(de->type_ind == dti_dir) return dir_nodes.emplace(this, idx).first.base();
        else 
        { 
            fs_node* result = file_nodes.emplace(this, idx, std::max(3, static_cast<int>(file_nodes.size() + device_nodes.size()))).first.base();
            next_fd = std::max(3, static_cast<int>(file_nodes.size() + device_nodes.size())); 
            return result;
        }
    }
    else
    { 
        ext_inode* n = read_inode(idx);
        if(n->mode.is_directory()) return dir_nodes.emplace(this, idx, n).first.base();
        else 
        { 
            fs_node* result = file_nodes.emplace(this,idx, n, std::max(3, static_cast<int>(file_nodes.size() + device_nodes.size()))).first.base(); 
            next_fd = std::max(3, static_cast<int>(file_nodes.size() + device_nodes.size()));
            return result; 
        }
    }
}