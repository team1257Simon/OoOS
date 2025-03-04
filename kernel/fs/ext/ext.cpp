#include "fs/ext.hpp"
#include "fs/hda_ahci.hpp"
#include "sys/errno.h"
#include "bitmap.hpp"
static std::alignval_allocator<char, std::align_val_t(PAGESIZE)> buff_alloc{};
static std::allocator<ext_superblock> sb_alloc{};
static std::allocator<block_group_descriptor> bg_alloc{};
void ext_block_group::decrement_inode_ct() { dword inode_ct(descr->free_inodes, descr->free_inodes_hi); inode_ct--; descr->free_inodes = inode_ct.lo; descr->free_inodes_hi = inode_ct.hi; }
void ext_block_group::increment_inode_ct() { dword inode_ct(descr->free_inodes, descr->free_inodes_hi); inode_ct++; descr->free_inodes = inode_ct.lo; descr->free_inodes_hi = inode_ct.hi; }
bool ext_block_group::has_available_inode() { return descr->free_inodes || descr->free_inodes_hi; }
bool ext_block_group::has_available_blocks() { return descr->unallocated_blocks || descr->free_blocks_hi; }
bool ext_block_group::has_available_blocks(size_t n) { return dword(descr->unallocated_blocks, descr->free_blocks_hi) >= n; }
void ext_block_group::alter_available_blocks(int64_t diff) { dword num_blocks(descr->unallocated_blocks, descr->free_blocks_hi); num_blocks += diff; descr->unallocated_blocks = num_blocks.lo; descr->free_blocks_hi = num_blocks.hi; }
size_t extfs::block_size() { return 1024UL << sb->block_size_shift; }
size_t extfs::inodes_per_block() { return (1024UL << sb->block_size_shift) / sb->inode_size; }
size_t extfs::inodes_per_group() { return sb->inodes_per_group; }
size_t extfs::sectors_per_block() { return (1024UL / physical_block_size) << sb->block_size_shift; }
uint64_t extfs::block_to_lba(uint64_t block) { return static_cast<uint64_t>(superblock_lba - sb_off) + sectors_per_block() * static_cast<uint64_t>(block); }
directory_node *extfs::get_root_directory() { return root_dir; }
ext_jbd2_mode extfs::journal_mode() const { return ordered; /* TODO get this from mount options */ }
bool extfs::read_hd(void* dest, uint64_t lba_src, size_t sectors) { return ahci_hda::is_initialized() && ahci_hda::read(static_cast<char*>(dest), lba_src, sectors); }
bool extfs::write_hd(uint64_t lba_dest, const void* src, size_t sectors) { return ahci_hda::is_initialized() && ahci_hda::write(lba_dest, static_cast<char const*>(src), sectors); }
bool extfs::read_from_disk(disk_block& bl) { return read_hd(bl.data_buffer, block_to_lba(bl.block_number), sectors_per_block() * std::max(bl.chain_len, 1UL)); }
bool extfs::write_to_disk(disk_block const& bl) { return write_hd(block_to_lba(bl.block_number), bl.data_buffer, sectors_per_block() * std::max(bl.chain_len, 1UL)); }
bool extfs::persist(ext_directory_vnode* n) { return fs_journal.create_txn(n); /* directory entry data must go through the journal */ }
size_t extfs::blocks_per_group() { return sb->blocks_per_group; }
void extfs::allocate_block_buffer(disk_block& bl) { if(bl.data_buffer) { free_block_buffer(bl); } bl.data_buffer = buff_alloc.allocate(block_size() * bl.chain_len); array_zero(bl.data_buffer, block_size() * bl.chain_len); }
char *extfs::allocate_block_buffer() { char* result = buff_alloc.allocate(block_size()); array_zero(result, block_size()); return result; }
void extfs::free_block_buffer(disk_block& bl) { buff_alloc.deallocate(bl.data_buffer, block_size() * bl.chain_len); }
off_t extfs::inode_block_offset(uint32_t inode) { return off_t(inode_pos_in_group(inode) * sb->inode_size); }
uint64_t extfs::group_num_for_inode(uint32_t inode) { return ((static_cast<size_t>(inode - 1)) / sb->inodes_per_group) >> sb->groups_per_flex_shift; }
uint32_t extfs::inode_pos_in_group(uint32_t inode_num) { return (inode_num - 1) % (sb->inodes_per_group << sb->groups_per_flex_shift); }
dev_t extfs::xgdevid() const noexcept { return sb->fs_uuid.data_a; }
void extfs::syncdirs() { if(!this->fs_journal.execute_pending_txns()) panic("failed to execute transaction(s)"); }
extfs::extfs(uint64_t volume_start_lba) : 
    filesystem          {}, 
    file_nodes          {}, 
    dir_nodes           {},
    block_groups        {},
    sb                  { sb_alloc.allocate(1UL) }, 
    blk_group_descs     {}, 
    root_dir            {}, 
    num_blk_groups      { 0UL }, 
    superblock_lba      { volume_start_lba + sb_off },
    fs_journal          {}
                        {}
extfs::~extfs() 
{ 
    if(sb) { sb_alloc.deallocate(sb, 1UL); } 
    block_groups.clear(); // destruct these now to avoid dangling pointer shenanigans
    if(blk_group_descs) { bg_alloc.deallocate(blk_group_descs, num_blk_groups); } 
}
ext_block_group::ext_block_group(extfs *parent, block_group_descriptor *desc) : 
    parent_fs       { parent }, 
    descr           { desc }, 
    inode_usage_bmp { qword(desc->inode_usage_bitmap_block_idx, desc->inode_usage_bitmap_block_idx_hi) },
    blk_usage_bmp   { qword(desc->block_usage_bitmap_block_idx, desc->block_usage_bitmap_block_idx_hi) },
    inode_block     {}
                    {}
ext_block_group::~ext_block_group()
{
    parent_fs->free_block_buffer(inode_usage_bmp);
    parent_fs->free_block_buffer(blk_usage_bmp);
}
void extfs::initialize()
{
    if(initialized) return;
    if(!(sb && read_hd(sb, superblock_lba, sb_sectors))) throw std::runtime_error{ "failed to read superblock" };
    uint64_t block_cnt = qword(sb->block_count, sb->block_count_hi);
    uint64_t group_count_by_blocks = div_roundup(block_cnt, sb->blocks_per_group);
    uint64_t group_count_by_inodes = div_roundup(sb->inode_count, sb->inodes_per_group);
    if(group_count_by_blocks != group_count_by_inodes) { throw std::logic_error{ "inode block group count of " + std::to_string(group_count_by_inodes) + " does not match block group count of " + std::to_string(group_count_by_blocks) }; }
    num_blk_groups = group_count_by_blocks;
    blk_group_descs = bg_alloc.allocate(num_blk_groups);
    size_t bgsz = up_to_nearest(num_blk_groups * sizeof(block_group_descriptor), block_size());
    char* bg_buffer = buff_alloc.allocate(bgsz);
    size_t inode_blocks_per_group = div_roundup(sb->inodes_per_group, inodes_per_block());
    bg_table_block.data_buffer = bg_buffer;
    bg_table_block.chain_len = div_roundup(bgsz, block_size());
    if(!(num_blk_groups && blk_group_descs && read_hd(bg_buffer, block_to_lba(1UL), div_roundup(bgsz, physical_block_size)))) throw std::runtime_error{ "failed to read block group table" };
    blk_group_descs = reinterpret_cast<block_group_descriptor*>(bg_buffer);
    size_t num_group_structs = num_blk_groups;
    size_t group_incr;
    if(sb->required_features & flex_groups) { num_group_structs = std::max(num_blk_groups >> sb->groups_per_flex_shift, 1UL); group_incr = std::min(1UL << sb->groups_per_flex_shift, num_blk_groups); }
    else group_incr = 1;
    for(size_t i = 0; i < num_group_structs; i++)
    {
        ext_block_group bg(this, blk_group_descs + (i * group_incr));
        bg.inode_usage_bmp.chain_len = group_incr;
        bg.blk_usage_bmp.chain_len = group_incr;
        allocate_block_buffer(bg.inode_usage_bmp);
        allocate_block_buffer(bg.blk_usage_bmp);
        if(!read_from_disk(bg.inode_usage_bmp) || !read_from_disk(bg.blk_usage_bmp)) throw std::runtime_error{ "failed to read block group" };
        uint64_t inode_table_start = qword(bg.descr->inode_table_start_block, bg.descr->inode_table_start_block_hi);
        bg.inode_block.block_number = inode_table_start;
        bg.inode_block.chain_len = inode_blocks_per_group * group_incr;
        allocate_block_buffer(bg.inode_block);
        if(!read_from_disk(bg.inode_block)) { throw std::runtime_error{ "failed to read inode table" }; }
        block_groups.push_back(bg);
    }
    ext_directory_vnode* rdnode = dir_nodes.emplace(this, 2U).first.base();
    rdnode->initialize();
    root_dir = rdnode;
    new (std::addressof(fs_journal)) jbd2(this, sb->journal_inode);
    if(!fs_journal.initialize()) throw std::runtime_error{ "journal init failed" };
    else initialized = true;
}
uint32_t extfs::claim_inode()
{
    for(size_t i = 0; i < block_groups.size(); i++)
    {
        if(block_groups[i].has_available_inode())
        {
            unsigned long* bmp = reinterpret_cast<unsigned long*>(block_groups[i].inode_usage_bmp.data_buffer);
            off_t avail = bitmap_scan_sz(bmp, (block_size() * block_groups[i].inode_usage_bmp.chain_len) / sizeof(unsigned long));
            uint32_t result = static_cast<uint32_t>(avail + (sb->inodes_per_group * i));
            bitmap_set_cbits(bmp, avail, 1UL);
            block_groups[i].decrement_inode_ct();
            if(!persist_group_metadata(i)) return 0U;
            return result; 
        }
    }
    return 0U;
}
bool extfs::release_inode(uint32_t num)
{
    if(!num) return false;
    size_t i = num / sb->inodes_per_group;
    off_t bit_off = off_t(num % sb->inodes_per_group);
    unsigned long* bmp = reinterpret_cast<unsigned long*>(block_groups[i].inode_usage_bmp.data_buffer);
    bitmap_clear_cbits(bmp, bit_off, 1UL);
    block_groups[i].increment_inode_ct();
    return persist_group_metadata(i);
}
void extfs::release_blocks(uint64_t start, size_t num)
{
    size_t i = start / sb->blocks_per_group;
    off_t off = off_t(start % sb->blocks_per_group);
    unsigned long* bmp = reinterpret_cast<unsigned long*>(block_groups[i].blk_usage_bmp.data_buffer);
    bitmap_clear_cbits(bmp, off, num);
    block_groups[i].alter_available_blocks(+num);
    persist_group_metadata(i);
}
uint64_t extfs::inode_to_block(uint32_t inode)
{
    size_t grp = (static_cast<size_t>(inode - 1)) / sb->inodes_per_group;
    size_t idx = (static_cast<size_t>(inode - 1)) % sb->inodes_per_group;
    return qword(blk_group_descs[grp].inode_table_start_block, blk_group_descs[grp].inode_table_start_block_hi) + (static_cast<uint64_t>(idx * sb->inode_size) / block_size());
}
ext_inode* extfs::read_inode(uint32_t inode_num)
{
    size_t grp = group_num_for_inode(inode_num);
    if(grp >= block_groups.size()) throw std::out_of_range{ "invalid inode group" };
    return reinterpret_cast<ext_inode*>((block_groups[grp].inode_block.data_buffer + inode_block_offset(inode_num)));
}
file_node *extfs::open_fd(tnode* fd)
{
    file_node* n = fd->as_file();
    if(ext_file_vnode* exfn = dynamic_cast<ext_file_vnode*>(n)) { exfn->initialize(); }
    return n;
}
directory_node *extfs::mkdirnode(directory_node* parent, std::string const& name)
{
    qword tstamp = syscall_time(0);
    if(uint32_t inode_num = claim_inode()) try
    {
        ext_directory_vnode& exparent = dynamic_cast<ext_directory_vnode&>(*parent);
        ext_inode* inode = new(read_inode(inode_num)) ext_inode
        {
            .mode = 004777U,
            .changed_time = tstamp.lo,
            .modified_time = tstamp.lo,
            .flags = use_extents,
            .block_info = { .ext4_extent = { .header = { .magic = ext_extent_magic, .entries = 0, .max_entries = 4, .depth = 0 } } },
            .changed_time_hi = tstamp.hi,
            .mod_time_extra = tstamp.hi,
            .created_time = tstamp.lo,
            .created_time_hi = tstamp.hi
        };
        dword checksum = crc32c(inode, std::addressof(sb->fs_uuid), std::addressof(inode_num));
        inode->checksum_lo = checksum.lo;
        inode->checksum_hi = checksum.hi;
        next_fd = std::max(3, static_cast<int>(file_nodes.size() + device_nodes.size()));
        ext_directory_vnode* fn = dir_nodes.emplace(this, inode_num, inode).first.base();
        if(exparent.add_dir_entry(fn, dti_dir, name.c_str(), name.size()) && persist_inode(inode_num)) return fn;
        else panic("failed to add directory entry");
    }
    catch(std::exception& e) { panic(e.what()); }
    else panic("no available inodes");
    return nullptr;
}
file_node *extfs::mkfilenode(directory_node* parent, std::string const& name)
{
    qword tstamp = syscall_time(0);
    if(uint32_t inode_num = claim_inode()) try
    {
        ext_directory_vnode& exparent = dynamic_cast<ext_directory_vnode&>(*parent);
        ext_inode* inode = new(read_inode(inode_num)) ext_inode
        {
            .mode = 010777U,
            .changed_time = tstamp.lo,
            .modified_time = tstamp.lo,
            .flags = use_extents,
            .block_info = { .ext4_extent = { .header = { .magic = ext_extent_magic, .entries = 0, .max_entries = 4, .depth = 0 } } },
            .changed_time_hi = tstamp.hi,
            .mod_time_extra = tstamp.hi,
            .created_time = tstamp.lo,
            .created_time_hi = tstamp.hi
        };
        dword checksum = crc32c(inode, std::addressof(sb->fs_uuid), std::addressof(inode_num));
        inode->checksum_lo = checksum.lo;
        inode->checksum_hi = checksum.hi;
        next_fd = std::max(3, static_cast<int>(file_nodes.size() + device_nodes.size()));
        ext_file_vnode* fn = file_nodes.emplace(this, inode_num, inode, next_fd).first.base();
        if(exparent.add_dir_entry(fn, dti_regular, name.data(), name.size()) && persist_inode(inode_num)) return fn;
        else panic("failed to add directory entry");
    }
    catch(std::exception& e) { panic(e.what()); }
    else panic("no available inodes");
    return nullptr;
}
void extfs::dldirnode(directory_node* dd)
{
    if(!dd->is_empty()) { throw std::logic_error{ std::string{ "cannot delete non-empty directory " } + dd->name() }; }
    uint64_t cid = dd->cid();
    ext_directory_vnode& exdn = dynamic_cast<ext_directory_vnode&>(*dd);
    uint32_t inode_num = exdn.inode_number;
    release_inode(inode_num);
    new (exdn.on_disk_node) ext_inode{ .mode = 0, .block_info = { .link_target = {} } };
    persist_inode(inode_num);
    for(size_t i = 0; i < exdn.cached_metadata.size(); i++) { release_blocks(exdn.cached_metadata[i].block_number, exdn.cached_metadata[i].chain_len); }
    for(size_t i = 0; i < exdn.block_data.size(); i++) { release_blocks(exdn.block_data[i].block_number, exdn.block_data[i].chain_len); }
    dir_nodes.erase(cid);
}
void extfs::dlfilenode(file_node* fd)
{
    uint64_t cid = fd->cid();
    ext_file_vnode& exfn = dynamic_cast<ext_file_vnode&>(*fd);
    uint32_t inode_num = exfn.inode_number;
    release_inode(inode_num);
    new (exfn.on_disk_node) ext_inode{ .mode = 0, .block_info = { .link_target = {} } };
    persist_inode(inode_num);
    for(size_t i = 0; i < exfn.cached_metadata.size(); i++) { release_blocks(exfn.cached_metadata[i].block_number, exfn.cached_metadata[i].chain_len); }
    for(size_t i = 0; i < exfn.block_data.size(); i++) { release_blocks(exfn.block_data[i].block_number, exfn.block_data[i].chain_len); }
    file_nodes.erase(cid);
}
disk_block *extfs::claim_blocks(ext_vnode *requestor, size_t how_many)
{
    if(!how_many) return nullptr;
    for(size_t i = 0; i < block_groups.size(); i++)
    {
        if(block_groups[i].has_available_blocks(how_many))
        {
            unsigned long* bmp = reinterpret_cast<unsigned long*>(block_groups[i].blk_usage_bmp.data_buffer);
            off_t avail = bitmap_scan_cz(bmp, (block_size() * block_groups[i].blk_usage_bmp.chain_len) / sizeof(unsigned long), how_many);
            if(avail < 0) continue;
            bitmap_set_cbits(bmp, avail, how_many);
            block_groups[i].alter_available_blocks(-how_many);
            if(!persist_group_metadata(i)) return nullptr;
            uint64_t result = i * sb->blocks_per_group + avail;
            disk_block* blk = std::addressof(requestor->block_data.emplace_back(result, nullptr, false, how_many));
            if(!((requestor->on_disk_node->flags & use_extents) ? requestor->extents.push_extent_ext4(blk) : requestor->extents.push_extent_legacy(blk))) return nullptr;
            return blk;
        }
    }
    return nullptr;
}
disk_block *extfs::claim_metadata_block(ext_node_extent_tree *requestor)
{
    for(size_t i = 0; i < block_groups.size(); i++)
    {
        if(block_groups[i].has_available_blocks())
        {
            unsigned long* bmp = reinterpret_cast<unsigned long*>(block_groups[i].blk_usage_bmp.data_buffer);
            off_t avail = bitmap_scan_sz(bmp, (block_size() * block_groups[i].blk_usage_bmp.chain_len) / sizeof(unsigned long));
            bitmap_set_cbits(bmp, avail, 1);
            block_groups[i].alter_available_blocks(-1L);
            if(!persist_group_metadata(i)) return nullptr;
            uint64_t result = i * sb->blocks_per_group + avail;
            disk_block* blk = std::addressof(requestor->tracked_node->cached_metadata.emplace_back(result, allocate_block_buffer(), false, 1U));
            dword blcnt(requestor->tracked_node->on_disk_node->blocks_count_lo, requestor->tracked_node->on_disk_node->blocks_count_hi);
            blcnt++;
            requestor->tracked_node->on_disk_node->blocks_count_lo = blcnt.lo;
            requestor->tracked_node->on_disk_node->blocks_count_hi = blcnt.hi;
            return blk;
        }
    }
    return nullptr;
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
    if(dirty_metadata.empty()) return true; // vacuous truth if nothing to do
    return fs_journal.create_txn(dirty_metadata);
}
bool extfs::persist_group_metadata(size_t group_num)
{
    if(group_num < block_groups.size())
    {
        std::vector<disk_block> blks{};
        blks.push_back(block_groups[group_num].inode_usage_bmp);
        blks.push_back(block_groups[group_num].blk_usage_bmp);
        blks.push_back(bg_table_block);
        return fs_journal.create_txn(blks);
    }
    panic("block group number out of range");
    return false;
}
bool extfs::persist_inode(uint32_t inode_num)
{
    size_t grp = group_num_for_inode(inode_num);
    if(grp >= block_groups.size()) { panic("invalid group number"); return false; }
    std::vector<disk_block> blks{};
    size_t bs = block_size();
    off_t ioffs = truncate(inode_block_offset(inode_num), bs);
    blks.emplace_back(block_groups[grp].inode_block.block_number + ioffs / bs, block_groups[grp].inode_block.data_buffer + ioffs, true, 1UL);
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
            next_fd = std::max(3, static_cast<int>(file_nodes.size() + device_nodes.size())); 
            fs_node* result = file_nodes.emplace(this, idx, next_fd++).first.base();
            return result;
        }
    }
    else
    { 
        ext_inode* n = read_inode(idx);
        if(n->mode.is_directory()) return dir_nodes.emplace(this, idx, n).first.base();
        else 
        {
            next_fd = std::max(3, static_cast<int>(file_nodes.size() + device_nodes.size()));
            fs_node* result = file_nodes.emplace(this, idx, n, next_fd++).first.base(); 
            return result; 
        }
    }
}