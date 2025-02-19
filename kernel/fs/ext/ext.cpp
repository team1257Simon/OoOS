#include "fs/ext.hpp"
#include "fs/hda_ahci.hpp"
#include "sys/errno.h"
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
bool jbd2_journal::need_escape(disk_block const &bl) { return ((__be32(reinterpret_cast<uint32_t const*>(bl.data_buffer)[0])) == jbd2_magic); }
off_t jbd2_journal::desc_tag_create(disk_block const& bl, void* where, uint32_t seq, bool is_first, bool is_last)
{
    off_t result = static_cast<off_t>(desc_tag_size(true));
    uint32_t fl = (is_first ? same_uuid : 0) | (is_last ? last_block : 0) | (need_escape(bl) ? escape : 0);
    if(sb->required_features & csum_v3) { new (static_cast<jbd2_block_tag3*>(where)) jbd2_block_tag3{ .block_number = __be32((bl.block_number) & 0xFFFFFFFF), .flags = __be32(fl), .block_number_hi = __be32((bl.block_number >> 32) & 0xFFFFFFFF) }; }
    else { new (static_cast<jbd2_block_tag*>(where)) jbd2_block_tag{ .block_number = __be32((bl.block_number) & 0xFFFFFFFF), .flags = __be16(fl), .block_number_hi = __be32(sb->required_features & x64_support ? ((bl.block_number >> 32) & 0xFFFFFFFF) : 0) }; }
    if(is_first) { uint8_t* uuid_pos = reinterpret_cast<uint8_t*>(where) + result; result += 16; __builtin_memcpy(uuid_pos, sb->uuid.data_bytes, 16); }
    // TODO: compute checksums
    return result;
}
size_t jbd2_journal::desc_tag_size(bool same_uuid) { return (sb->required_features & csum_v3 ? 16 : (sb->required_features & x64_support ? 12 : 8)) + (same_uuid ? 0 : 16); }
bool jbd2_journal::create_txn(ext_vnode *changed_node)
{
    std::vector<disk_block> dirty_blocks{};
    for(std::vector<disk_block>::iterator i = changed_node->block_data.begin(); i != changed_node->block_data.end(); i++) { if(i->dirty) { i->dirty = false; dirty_blocks.push_back(*i); } }
    size_t tags_per_block = (sb->journal_block_size - sizeof(jbd2_header) - desc_tag_size(false) - (sb->required_features & (csum_v2 | csum_v3) ? 4 : 0)) / desc_tag_size(true);
    size_t n_blocks = div_roundup(dirty_blocks.size(), tags_per_block);
    disk_block tb{ first_open_block++, std::allocator<char>{}.allocate(sb->journal_block_size) };
    uint32_t s = 0;
    uint64_t j = tb.block_number;
    size_t k = 0;
    off_t o;
    for(std::vector<disk_block>::iterator i = dirty_blocks.begin(); i < dirty_blocks.end(); i++, j++)
    {
        if(!k)
        {
            array_zero(tb.data_buffer, sb->journal_block_size);
            o = static_cast<off_t>(sizeof(jbd2_header));
            new(reinterpret_cast<jbd2_header*>(tb.data_buffer)) jbd2_header{ .blocktype = __be32(descriptor), .sequence = __be32(s++) };
        }
        o += desc_tag_create(*i, tb.data_buffer + o, s, !k, (k + 1 == tags_per_block || i + 1 == dirty_blocks.end()));
        bool esc = need_escape(*i);
        if(esc) *reinterpret_cast<uint32_t*>(i->data_buffer) = 0;
        if(!ahci_hda::get_instance()->write(parent_fs->block_to_lba(j), tb.data_buffer, parent_fs->sectors_per_block())) { panic("disk write failed"); sb->journal_errno = __be32(EPIPE); if(esc) { *reinterpret_cast<uint32_t*>(i->data_buffer) = jbd2_magic; } return false; }
        if(esc) { *reinterpret_cast<uint32_t*>(i->data_buffer) = jbd2_magic; }
        k = (k + 1) % tags_per_block;
        if(!k || i + 1 == dirty_blocks.end())
        {
            if(!ahci_hda::get_instance()->write(parent_fs->block_to_lba(tb.block_number), tb.data_buffer, sb->journal_block_size / physical_block_size)) { panic("disk write failed"); sb->journal_errno = __be32(EPIPE); return false; }
            first_open_block = j;
            tb.block_number = first_open_block++;
        }
    }
    array_zero(tb.data_buffer, sb->journal_block_size);
    new(reinterpret_cast<jbd2_commit_header*>(tb.data_buffer)) jbd2_commit_header{}; // TODO: figure out these values, checksum etc
    active_transactions.put_txn(std::move(dirty_blocks), jbd2_commit_header(*reinterpret_cast<jbd2_commit_header const*>(tb.data_buffer)));
    if(!ahci_hda::get_instance()->write(parent_fs->block_to_lba(tb.block_number), tb.data_buffer, sb->journal_block_size / physical_block_size)) { panic("disk write failed"); sb->journal_errno = __be32(EPIPE); return false; }
    std::allocator<char>{}.deallocate(tb.data_buffer, sb->journal_block_size);
    return true;
}
jbd2_txn_queue::reference jbd2_txn_queue::put_txn(std::vector<disk_block>&& blocks, jbd2_commit_header&& h) { return this->emplace(this->size(), std::move(blocks), std::move(h)); }