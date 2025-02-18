#include "fs/ext.hpp"
#include "fs/hda_ahci.hpp"
size_t extfs::block_size() { return 1024UL << sb->block_size_shift; }
size_t extfs::inodes_per_block() { return sb->inode_size / block_size(); }
size_t extfs::sectors_per_block() { return block_size() / physical_block_size; }
uint64_t extfs::block_to_lba(uint64_t block) { return static_cast<uint64_t>(superblock_lba - sb_off) + sectors_per_block() * static_cast<uint64_t>(block); }
directory_node *extfs::get_root_directory() { return root_dir; }
bool extfs::read_block(disk_block& blk) { return ahci_hda::read(blk.data_buffer, block_to_lba(blk.block_number), sectors_per_block()); }
extfs::~extfs() { if(sb) { std::allocator<ext_superblock>{}.deallocate(sb, 1UL); } if(blk_groups) { std::allocator<block_group_descriptor>{}.deallocate(blk_groups, num_blk_groups); } }
extfs::extfs(uint64_t volume_start_lba) : 
    filesystem          {}, 
    file_nodes          {}, 
    dir_nodes           {}, 
    sb                  { std::allocator<ext_superblock>{}.allocate(1UL) }, 
    blk_groups          { nullptr }, 
    root_dir            { nullptr }, 
    num_blk_groups      { 0UL }, 
    superblock_lba      { volume_start_lba + sb_off },
    fs_journal          {}
                        {}
void extfs::initialize()
{
    if(!(sb && ahci_hda::is_initialized() && ahci_hda::read(reinterpret_cast<char*>(sb), superblock_lba, sb_sectors))) throw std::runtime_error{ "failed to read superblock" };
    uint64_t block_cnt = qword(sb->block_count, sb->block_count_hi);
    uint64_t group_count_by_blocks = div_roundup(block_cnt, sb->blocks_per_group);
    uint64_t group_count_by_inodes = div_roundup(sb->inode_count, sb->inodes_per_group);
    if(group_count_by_blocks != group_count_by_inodes) { throw std::logic_error{ "inode block group count of " + std::to_string(group_count_by_inodes) + " does not match block group count of " + std::to_string(group_count_by_blocks) }; }
    num_blk_groups = group_count_by_blocks;
    if(!(num_blk_groups && blk_groups && ahci_hda::read(reinterpret_cast<char*>(blk_groups), block_to_lba(1UL), (num_blk_groups * sizeof(block_group_descriptor)) / physical_block_size))) throw std::runtime_error{ "failed to read block group table" };
}                       
uint64_t extfs::inode_to_block(uint32_t inode)
{
    size_t grp = (static_cast<size_t>(inode - 1)) / sb->inodes_per_group;
    size_t idx = (static_cast<size_t>(inode - 1)) % sb->inodes_per_group;
    return qword(blk_groups[grp].inode_table_start_block, blk_groups[grp].inode_table_start_block_hi) + (static_cast<uint64_t>(idx * sb->inode_size) / block_size());
}
ext_inode *extfs::read_inode(uint32_t inode_num)
{
    if(!ahci_hda::is_initialized()) return nullptr;
    ext_inode* result = std::allocator<ext_inode>{}.allocate(1);
    if(!ahci_hda::read_objects(result, block_to_lba(inode_to_block(inode_num)), 1UL)) { std::allocator<ext_inode>{}.deallocate(result, 1); return nullptr; }
    return result;
}