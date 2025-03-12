#include "fs/ext.hpp"
#include "fs/hda_ahci.hpp"
#include "sys/errno.h"
#include "immintrin.h"
#include "bitmap.hpp"
#include "kdebug.hpp"
static std::alignval_allocator<char, std::align_val_t(physical_block_size)> buff_alloc{};
static std::allocator<ext_superblock> sb_alloc{};
static std::allocator<block_group_descriptor> bg_alloc{};
void ext_block_group::increment_dir_ct() { dword dir_ct(descr->num_directories, descr->num_directories_hi); dir_ct++; descr->num_directories = dir_ct.lo; descr->num_directories_hi = dir_ct.hi; }
void ext_block_group::decrement_dir_ct() { dword dir_ct(descr->num_directories, descr->num_directories_hi); dir_ct--; descr->num_directories = dir_ct.lo; descr->num_directories_hi = dir_ct.hi; }
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
bool extfs::read_unbuffered(disk_block& bl) { if(bl.data_buffer && bl.block_number) return ahci_hda::read_direct(bl.data_buffer, block_to_lba(bl.block_number), sectors_per_block() * bl.chain_len); else return false; }
bool extfs::read_from_disk(disk_block& bl) { if(bl.data_buffer && bl.block_number) return read_hd(bl.data_buffer, block_to_lba(bl.block_number), sectors_per_block() * bl.chain_len); else return false; }
bool extfs::write_unbuffered(disk_block const& bl) { if(bl.data_buffer && bl.block_number) return ahci_hda::write_direct(block_to_lba(bl.block_number), bl.data_buffer, sectors_per_block() * bl.chain_len); else return false; }
bool extfs::write_to_disk(disk_block const& bl) { if(bl.data_buffer && bl.block_number) return write_hd(block_to_lba(bl.block_number), bl.data_buffer, sectors_per_block() * bl.chain_len); else return false; }
size_t extfs::blocks_per_group() { return sb->blocks_per_group; }
void extfs::allocate_block_buffer(disk_block& bl) { if(bl.data_buffer) { free_block_buffer(bl); } size_t s = block_size() * bl.chain_len; bl.data_buffer = buff_alloc.allocate(s); array_zero(bl.data_buffer, s); }
char *extfs::allocate_block_buffer() { size_t bs = block_size(); char* result = buff_alloc.allocate(bs); array_zero(result, bs); return result; }
void extfs::free_block_buffer(disk_block& bl) { if(bl.data_buffer && bl.chain_len) buff_alloc.deallocate(bl.data_buffer, block_size() * bl.chain_len); }
off_t extfs::inode_block_offset(uint32_t inode) { return static_cast<off_t>(inode_pos_in_group(inode) * sb->inode_size); }
uint64_t extfs::group_num_for_inode(uint32_t inode) { return ((static_cast<size_t>(inode - 1)) / sb->inodes_per_group); }
uint32_t extfs::inode_pos_in_group(uint32_t inode_num) { return (inode_num - 1) % (sb->inodes_per_group); }
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
ext_block_group::ext_block_group(extfs* parent, block_group_descriptor* desc) : 
    parent_fs       { parent }, 
    descr           { desc },
    inode_usage_bmp { qword(desc->inode_usage_bitmap_block_idx, desc->inode_usage_bitmap_block_idx_hi), nullptr },
    blk_usage_bmp   { qword(desc->block_usage_bitmap_block_idx, desc->block_usage_bitmap_block_idx_hi), nullptr },
    inode_block     { qword(desc->inode_table_start_block, desc->inode_table_start_block_hi), nullptr }
                    {}
ext_block_group::ext_block_group(ext_block_group&& that) :
    parent_fs       { that.parent_fs },
    descr           { that.descr },
    inode_usage_bmp { that.inode_usage_bmp },
    blk_usage_bmp   { that.blk_usage_bmp },
    inode_block     { that.inode_block }
                    { that.inode_usage_bmp.data_buffer = that.inode_block.data_buffer = that.blk_usage_bmp.data_buffer = nullptr; }
ext_block_group::~ext_block_group()
{
    parent_fs->free_block_buffer(inode_usage_bmp);
    parent_fs->free_block_buffer(blk_usage_bmp);
    parent_fs->free_block_buffer(inode_block);
}
void ext_block_group::compute_checksums(size_t group_num)
{
    dword gn(static_cast<uint32_t>(group_num));
    size_t bs = parent_fs->block_size();
    if(parent_fs->sb->read_only_optional_features & metadata_csum)
    {
        uint32_t cs_seed = crc32c(parent_fs->sb->fs_uuid);
        cs_seed = crc32c(cs_seed, group_num);
        dword bbmp_cs = crc32c_blk(cs_seed, blk_usage_bmp, bs);
        dword ibmp_cs = crc32c_blk(cs_seed, inode_usage_bmp, bs);
        descr->block_usage_bmp_checkum = bbmp_cs.lo;
        descr->block_usage_bmp_checkum_hi = bbmp_cs.hi;
        descr->inode_usage_bmp_checksum = ibmp_cs.lo;
        descr->inode_usage_bmp_checksum_hi = ibmp_cs.hi;
        descr->group_checksum = 0;
        BARRIER;
        dword cs_full = crc32c(parent_fs->sb->fs_uuid);
        cs_full = crc32c(cs_full, static_cast<uint32_t>(group_num));
        cs_full = crc32c(cs_full, *descr);
        BARRIER;
        descr->group_checksum = cs_full.lo;
    }
    else if(parent_fs->sb->read_only_optional_features & gdt_csum)
    {
        constexpr auto off1 = offsetof(block_group_descriptor, group_checksum);
        constexpr auto off2 = offsetof(block_group_descriptor, block_usage_bitmap_block_idx_hi);
        descr->group_checksum = crc16_calc(reinterpret_cast<char*>(descr) + off2, static_cast<size_t>(sizeof(block_group_descriptor) - off2), crc16_calc(descr, static_cast<size_t>(off1), crc16_calc(std::addressof(parent_fs->sb->fs_uuid), sizeof(guid_t), crc16_calc(std::addressof(gn), 4UL))));
    }
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
    if(!(num_blk_groups && blk_group_descs && read_from_disk(bg_table_block))) throw std::runtime_error{ "failed to read block group table" };
    blk_group_descs = reinterpret_cast<block_group_descriptor*>(bg_buffer);
    for(size_t i = 0; i < num_blk_groups; i++)
    {
        ext_block_group& bg = block_groups.emplace_back(this, blk_group_descs + i);
        bg.inode_usage_bmp.chain_len = div_roundup(sb->blocks_per_group, block_size() * CHAR_BIT);
        bg.blk_usage_bmp.chain_len = div_roundup(sb->inodes_per_group, block_size() * CHAR_BIT);
        bg.inode_block.chain_len = inode_blocks_per_group;
        allocate_block_buffer(bg.inode_usage_bmp);
        allocate_block_buffer(bg.blk_usage_bmp);
        allocate_block_buffer(bg.inode_block);
        if(!read_unbuffered(bg.inode_usage_bmp) || !read_unbuffered(bg.blk_usage_bmp)) throw std::runtime_error{ "failed to read block group" };
        if(!read_unbuffered(bg.inode_block)) { throw std::runtime_error{ "failed to read inode table" }; }
        uint16_t cs = blk_group_descs[i].group_checksum;
        blk_group_descs[i].group_checksum = 0;
        dword dw_i(i);
        dword dw_cs = crc32c(sb->fs_uuid);
        dw_cs = crc32c(dw_cs, dw_i);
        dw_cs = crc32c(dw_cs, blk_group_descs[i]);
        if(dw_cs.lo != cs) throw std::runtime_error{ "checksums on block group did not match" };
        blk_group_descs[i].group_checksum = cs;
    }
    ext_directory_vnode* rdnode = dir_nodes.emplace(this, 2U).first.base();
    if(!rdnode->initialize()) throw std::runtime_error{ "root dir init failed" };
    root_dir = rdnode;
    std::construct_at(std::addressof(fs_journal), this, sb->journal_inode);
    if(!fs_journal.initialize()) throw std::runtime_error{ "journal init failed" };
    else initialized = true;
}
uint32_t extfs::claim_inode(bool dir)
{
    for(size_t i = 0; i < block_groups.size(); i++)
    {
        if(block_groups[i].has_available_inode())
        {
            unsigned long* bmp = reinterpret_cast<unsigned long*>(block_groups[i].inode_usage_bmp.data_buffer);
            off_t avail = bitmap_scan_sz(bmp, (block_size() * block_groups[i].inode_usage_bmp.chain_len) / sizeof(unsigned long));
            uint32_t result = static_cast<uint32_t>(avail + (sb->inodes_per_group * i) + 1U);
            bitmap_set_cbits(bmp, avail, 1UL);
            block_groups[i].decrement_inode_ct();
            if(dir) block_groups[i].increment_dir_ct();
            if(!persist_group_metadata(i)) return 0U;
            return result; 
        }
    }
    return 0U;
}
bool extfs::release_inode(uint32_t num, bool dir)
{
    if(!num) return false;
    size_t i = num / sb->inodes_per_group;
    off_t bit_off = static_cast<off_t>(num % sb->inodes_per_group);
    unsigned long* bmp = reinterpret_cast<unsigned long*>(block_groups[i].inode_usage_bmp.data_buffer);
    bitmap_clear_cbits(bmp, bit_off, 1UL);
    block_groups[i].increment_inode_ct();
    if(dir) block_groups[i].decrement_dir_ct();
    return persist_group_metadata(i);
}
void extfs::release_blocks(uint64_t start, size_t num)
{
    size_t i = start / sb->blocks_per_group;
    off_t off = static_cast<off_t>(start % sb->blocks_per_group);
    unsigned long* bmp = reinterpret_cast<unsigned long*>(block_groups[i].blk_usage_bmp.data_buffer);
    bitmap_clear_cbits(bmp, off, num);
    block_groups[i].alter_available_blocks(num);
    persist_group_metadata(i);
}
uint64_t extfs::inode_to_block(uint32_t inode)
{
    size_t grp = (static_cast<size_t>(inode - 1)) / sb->inodes_per_group;
    size_t idx = (static_cast<size_t>(inode - 1)) % sb->inodes_per_group;
    return qword(blk_group_descs[grp].inode_table_start_block, blk_group_descs[grp].inode_table_start_block_hi) + (static_cast<uint64_t>(idx * sb->inode_size) / block_size());
}
ext_inode* extfs::get_inode(uint32_t inode_num)
{
    size_t grp = group_num_for_inode(inode_num);
    if(grp >= block_groups.size()) throw std::out_of_range{ "invalid inode group" };
    return reinterpret_cast<ext_inode*>(block_groups[grp].inode_block.data_buffer + inode_block_offset(inode_num));
}
file_node *extfs::open_fd(tnode* fd)
{
    file_node* n = fd->as_file();
    if(ext_file_vnode* exfn = dynamic_cast<ext_file_vnode*>(n)) { exfn->initialize(); }
    return n;
}
directory_node *extfs::mkdirnode(directory_node* parent, std::string const& name)
{
    qword tstamp = syscall_time(nullptr);
    if(uint32_t inode_num = claim_inode(true)) try
    {
        ext_inode* inode = ::new (static_cast<void*>(get_inode(inode_num))) ext_inode
        {
            .mode               { 0040666U },
            .size_lo            { 0U },
            .changed_time       { tstamp.lo },
            .modified_time      { tstamp.lo },
            .flags              { use_extents },
            .block_info         
            { 
                .ext4_extent
                { 
                    .header
                    { 
                        .magic          { ext_extent_magic },
                        .entries        { 0 },
                        .max_entries    { 4 },
                        .depth          { 0 }
                    } 
                } 
            },
            .size_hi            { 0U },
            .changed_time_hi    { tstamp.hi },
            .mod_time_extra     { tstamp.hi },
            .created_time       { tstamp.lo },
            .created_time_hi    { tstamp.hi }
        };
        ext_directory_vnode* vnode = dir_nodes.emplace(this, inode_num, inode).first.base();
        ext_directory_vnode& exparent = dynamic_cast<ext_directory_vnode&>(*parent);
        if(exparent.add_dir_entry(vnode, dti_dir, name.data(), name.size()) && vnode->init_dir_blank(std::addressof(exparent))) 
        {
            word checksum = crc32c(*inode);
            checksum = crc32c(checksum, sb->fs_uuid);
            checksum = crc32c(checksum, inode_num);
            inode->checksum_lo = checksum.lo;
            inode->checksum_hi = checksum.hi;
            if(persist_inode(inode_num) && persist(vnode)) return vnode;
            else panic("failed to persist directory node");
        }
        else panic("failed to add directory entry");
    }
    catch(std::exception& e) { panic(e.what()); }
    else panic("failed to get inode");
    return nullptr;
}
file_node *extfs::mkfilenode(directory_node* parent, std::string const& name)
{
    qword tstamp = syscall_time(nullptr);
    if(uint32_t inode_num = claim_inode(false)) try
    {
        ext_inode* inode = ::new (static_cast<void*>(get_inode(inode_num))) ext_inode
        {
            .mode               { 0100666U },
            .size_lo            { 0U },
            .changed_time       { tstamp.lo },
            .modified_time      { tstamp.lo },
            .flags              { use_extents },
            .block_info         
            { 
                .ext4_extent
                { 
                    .header
                    { 
                        .magic          { ext_extent_magic },
                        .entries        { 0 },
                        .max_entries    { 4 },
                        .depth          { 0 } 
                    } 
                } 
            },
            .size_hi            { 0U },
            .changed_time_hi    { tstamp.hi },
            .mod_time_extra     { tstamp.hi },
            .created_time       { tstamp.lo },
            .created_time_hi    { tstamp.hi }
        };
        next_fd = std::max(3, static_cast<int>(file_nodes.size() + device_nodes.size()));
        ext_file_vnode* vnode = file_nodes.emplace(this, inode_num, inode, next_fd).first.base();
        ext_directory_vnode& exparent = dynamic_cast<ext_directory_vnode&>(*parent);
        if(exparent.add_dir_entry(vnode, dti_dir, name.data(), name.size()) && vnode->initialize()) 
        {
            dword checksum = crc32c(*inode);
            checksum = crc32c(checksum, sb->fs_uuid);
            checksum = crc32c(checksum, inode_num);
            inode->checksum_lo = checksum.lo;
            inode->checksum_hi = checksum.hi;
            if(persist_inode(inode_num)) return vnode;
            else panic("failed to add loop entries");
        }
        else panic("failed to add directory entry");
    }
    catch(std::exception& e) { panic(e.what()); }
    else panic("failed to get inode");
    return nullptr;
}
void extfs::dldirnode(directory_node* dd)
{
    if(!dd->is_empty()) { throw std::logic_error{ std::string{ "cannot delete non-empty directory " } + dd->name() }; }
    uint64_t cid = dd->cid();
    ext_directory_vnode& exdn = dynamic_cast<ext_directory_vnode&>(*dd);
    uint32_t inode_num = exdn.inode_number;
    release_inode(inode_num, true);
    array_zero(reinterpret_cast<char*>(exdn.on_disk_node), sb->inode_size);
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
    release_inode(inode_num, false);
    array_zero(reinterpret_cast<char*>(exfn.on_disk_node), sb->inode_size);
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
            dword blcnt(requestor->on_disk_node->blocks_count_lo, requestor->on_disk_node->blocks_count_hi);
            blcnt += how_many;
            requestor->on_disk_node->blocks_count_lo = blcnt.lo;
            requestor->on_disk_node->blocks_count_hi = blcnt.hi;
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
            dword blcnt(requestor->tracked_node->on_disk_node->blocks_count_lo, requestor->tracked_node->on_disk_node->blocks_count_hi);
            blcnt++;
            requestor->tracked_node->on_disk_node->blocks_count_lo = blcnt.lo;
            requestor->tracked_node->on_disk_node->blocks_count_hi = blcnt.hi;
            return std::addressof(requestor->tracked_node->cached_metadata.emplace_back(result, allocate_block_buffer(), false, 1U));
        }
    }
    return nullptr;
}
bool extfs::persist(ext_vnode* n) 
{ 
    if(journal_mode() != ordered) { return fs_journal.create_txn(n); } 
    for(disk_block& db : n->block_data) 
    {
		if(!(db.dirty && db.block_number && db.data_buffer)) { continue; }
        if(!write_unbuffered(db)) { panic("write failed"); sb->last_errno = EPIPE; return false; }
        db.dirty = false;
    }
    std::vector<disk_block> dirty_metadata{};
    for(disk_block& mdb : n->cached_metadata) { if(mdb.dirty) { dirty_metadata.push_back(mdb); mdb.dirty = false; } }
    if(dirty_metadata.empty()) return true; // vacuous truth if nothing to do
    return fs_journal.create_txn(dirty_metadata);
}
bool extfs::persist_group_metadata(size_t group_num)
{
    if(group_num < block_groups.size())
    {
        block_groups[group_num].compute_checksums(group_num);
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
    ext_inode* inode = get_inode(inode_num);
    inode->checksum_hi = inode->checksum_lo = 0;
    dword csum = crc32c(sb->fs_uuid);
    csum = crc32c(csum, inode_num);
    csum = crc32c(csum, *inode);
    inode->checksum_lo = csum.lo;
    inode->checksum_hi = csum.hi;
    size_t grp = group_num_for_inode(inode_num);
    if(grp >= block_groups.size()) { panic("invalid group number"); return false; }
    std::vector<disk_block> blks{};
    size_t bs = block_size();
    uint64_t iblk = inode_to_block(inode_num);
    blks.emplace_back(iblk, block_groups[grp].inode_block.data_buffer + bs * (iblk - block_groups[grp].inode_block.block_number), true, 1UL);
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
        ext_inode* n = get_inode(idx);
        if(n->mode.is_directory()) return dir_nodes.emplace(this, idx, n).first.base();
        else 
        {
            next_fd = std::max(3, static_cast<int>(file_nodes.size() + device_nodes.size()));
            fs_node* result = file_nodes.emplace(this, idx, n, next_fd++).first.base(); 
            return result; 
        }
    }
}