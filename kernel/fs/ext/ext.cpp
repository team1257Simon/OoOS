#include "fs/ext.hpp"
#include "fs/hda_ahci.hpp"
#include "sys/errno.h"
#include "immintrin.h"
#include "algorithm"
#include "bitmap.hpp"
#include "kdebug.hpp"
using std::addressof;
constexpr static std::alignval_allocator<char, std::align_val_t(physical_block_size)> buff_alloc{};
constexpr static std::allocator<ext_superblock> sb_alloc{};
constexpr static std::allocator<block_group_descriptor> bg_alloc{};
constexpr static uint16_t isize_val = sizeof(ext_inode) - offsetof(ext_inode, extra_isize);
void ext_block_group::increment_dir_ct() { dword dir_ct(descr->num_directories, descr->num_directories_hi); dir_ct++; descr->num_directories = dir_ct.lo; descr->num_directories_hi = dir_ct.hi; }
void ext_block_group::decrement_dir_ct() { dword dir_ct(descr->num_directories, descr->num_directories_hi); dir_ct--; descr->num_directories = dir_ct.lo; descr->num_directories_hi = dir_ct.hi; }
bool ext_block_group::has_available_inode() { return descr->free_inodes || descr->free_inodes_hi; }
bool ext_block_group::has_available_blocks() { return descr->free_blocks_lo || descr->free_blocks_hi; }
bool ext_block_group::has_available_blocks(size_t n) { return dword(descr->free_blocks_lo, descr->free_blocks_hi) >= n; }
size_t extfs::block_size() { return 1024UL << sb->block_size_shift; }
size_t extfs::inodes_per_block() { return (1024UL << sb->block_size_shift) / sb->inode_size; }
size_t extfs::inodes_per_group() { return sb->inodes_per_group; }
size_t extfs::sectors_per_block() { return (1024UL / physical_block_size) << sb->block_size_shift; }
uint64_t extfs::block_to_lba(uint64_t block) { return static_cast<uint64_t>(superblock_lba - sb_off) + sectors_per_block() * static_cast<uint64_t>(block); }
directory_node* extfs::get_root_directory() { return root_dir; }
ext_jbd2_mode extfs::journal_mode() const { return ordered; /* TODO get this from mount options */ }
bool extfs::read_hd(void* dest, uint64_t lba_src, size_t sectors) { return hda_ahci::is_initialized() && hda_ahci::read(static_cast<char*>(dest), lba_src, sectors); }
bool extfs::read_hd(disk_block& bl) { if(bl.data_buffer && bl.block_number) return read_hd(bl.data_buffer, block_to_lba(bl.block_number), sectors_per_block() * bl.chain_len); else return false; }
bool extfs::write_hd(uint64_t lba_dest, const void* src, size_t sectors) { return hda_ahci::is_initialized() && hda_ahci::write(lba_dest, src, sectors); }
bool extfs::write_hd(disk_block const& bl) { if(bl.data_buffer && bl.block_number) return write_hd(block_to_lba(bl.block_number), bl.data_buffer, sectors_per_block() * bl.chain_len); else return false; }
size_t extfs::blocks_per_group() { return sb->blocks_per_group; }
void extfs::allocate_block_buffer(disk_block& bl) { if(bl.data_buffer) { free_block_buffer(bl); } size_t s = block_size() * bl.chain_len; bl.data_buffer = buff_alloc.allocate(s); array_zero(bl.data_buffer, s); }
char *extfs::allocate_block_buffer() { size_t bs = block_size(); char* result = buff_alloc.allocate(bs); array_zero(result, bs); return result; }
void extfs::free_block_buffer(disk_block& bl) { if(bl.data_buffer && bl.chain_len) buff_alloc.deallocate(bl.data_buffer, block_size() * bl.chain_len); }
off_t extfs::inode_block_offset(uint32_t inode) { return static_cast<off_t>(inode_pos_in_group(inode) * sb->inode_size); }
uint64_t extfs::group_num_for_inode(uint32_t inode) { return ((static_cast<size_t>(inode - 1)) / sb->inodes_per_group); }
uint32_t extfs::inode_pos_in_group(uint32_t inode_num) { return (inode_num - 1) % (sb->inodes_per_group); }
dev_t extfs::xgdevid() const noexcept { return sb->fs_uuid.data_a; }
void extfs::syncdirs() { if(!fs_journal.execute_pending_txns()) panic("failed to execute transaction(s)"); }
bool extfs::update_free_inode_count(int diff) { sb->unallocated_inodes += diff; return persist_sb(); }
file_node* extfs::on_open(tnode* node) { return on_open(node, std::ios_base::in | std::ios_base::out); }
fs_node* extfs::dirent_to_vnode(ext_dir_entry* de) { return inode_to_vnode(de->inode_idx, static_cast<ext_dirent_type>(de->type_ind)); }
size_t extfs::inode_size() { return sb->inode_size; }
uint32_t extfs::__sb_checksum() const { return crc32c_x86_3way(~0U, reinterpret_cast<uint8_t const*>(sb), offsetof(ext_superblock, checksum)); }
extfs::extfs(uint64_t volume_start_lba) : 
    filesystem          {}, 
    file_nodes          {}, 
    dir_nodes           {},
    dev_nodes           {},
    block_groups        {},
    dev_linked_nodes    {},
    named_pipes         { 256UZ },
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
ext_pipe_pair& extfs::__init_pipes(uint32_t inode_num, std::string const& name)
{
    int first_fd = next_fd;
    while(current_open_files.contains(first_fd)) first_fd++;
    int second_fd = first_fd + 1;
    while(current_open_files.contains(second_fd)) second_fd++;
    std::pair<named_pipe_map::iterator, bool> result = named_pipes.emplace(this, inode_num, name, first_fd, second_fd);
    if(result.second) { register_fd(addressof(result.first->in)); register_fd(addressof(result.first->out)); }
    return *result.first;
}
bool ext_block_group::decrement_inode_ct()
{
    dword inode_ct(descr->free_inodes, descr->free_inodes_hi);
    inode_ct--;
    descr->free_inodes = inode_ct.lo;
    descr->free_inodes_hi = inode_ct.hi;
    return parent_fs->update_free_inode_count(-1);
}
bool ext_block_group::increment_inode_ct()
{
    dword inode_ct(descr->free_inodes, descr->free_inodes_hi); 
    inode_ct++;
    descr->free_inodes = inode_ct.lo;
    descr->free_inodes_hi = inode_ct.hi;
    return parent_fs->update_free_inode_count(+1);
}
bool ext_block_group::alter_available_blocks(int diff)
{
    uint32_t num_blocks = dword(descr->free_blocks_lo, descr->free_blocks_hi);
    num_blocks += diff;
    dword dw_num_blocks(num_blocks);
    descr->free_blocks_lo = dw_num_blocks.lo;
    descr->free_blocks_hi = dw_num_blocks.hi;
    return parent_fs->update_free_block_count(diff);
}
void ext_block_group::compute_checksums(size_t group_num)
{
    const dword gn(static_cast<uint32_t>(group_num));
    size_t bs = parent_fs->block_size();
    if(parent_fs->sb->read_only_optional_features & metadata_csum)
    {
        const uint32_t cs_seed      = crc32c(parent_fs->uuid_csum, gn);
        dword bbmp_cs               = crc32c_blk(cs_seed, blk_usage_bmp, bs);
        dword ibmp_cs               = crc32c_blk(cs_seed, inode_usage_bmp, bs);
        barrier();
        descr->block_usage_bmp_checkum      = bbmp_cs.lo;
        descr->block_usage_bmp_checkum_hi   = bbmp_cs.hi;
        descr->inode_usage_bmp_checksum     = ibmp_cs.lo;
        descr->inode_usage_bmp_checksum_hi  = ibmp_cs.hi;
        descr->group_checksum               = 0;
        barrier();
        dword cs_full                       = crc32c(cs_seed, *descr);
        barrier();
        descr->group_checksum               = cs_full.lo;
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
    uint32_t checkval = __sb_checksum();
    if(sb->checksum != checkval)
        throw std::logic_error{ "superblock checksum of " + std::to_string(sb->checksum, std::ext::hex) + " does not match calculated value of " + std::to_string(checkval, std::ext::hex) };
    uuid_csum = crc32c(~0U, sb->fs_uuid);
    uint64_t block_cnt = qword(sb->block_count, sb->block_count_hi);
    uint64_t group_count_by_blocks = div_round_up(block_cnt, sb->blocks_per_group);
    uint64_t group_count_by_inodes = div_round_up(sb->inode_count, sb->inodes_per_group);
    if(group_count_by_blocks != group_count_by_inodes) { throw std::logic_error{ "inode block group count of " + std::to_string(group_count_by_inodes) + " does not match block group count of " + std::to_string(group_count_by_blocks) }; }
    num_blk_groups                  = group_count_by_blocks;
    blk_group_descs                 = bg_alloc.allocate(num_blk_groups);
    size_t bgsz                     = up_to_nearest(num_blk_groups * sizeof(block_group_descriptor), block_size());
    char* bg_buffer                 = buff_alloc.allocate(bgsz);
    size_t inode_blocks_per_group   = div_round_up(sb->inodes_per_group, inodes_per_block());
    bg_table_block.data_buffer      = bg_buffer;
    bg_table_block.chain_len        = div_round_up(bgsz, block_size());
    if(!(num_blk_groups && blk_group_descs && read_hd(bg_table_block))) throw std::runtime_error{ "failed to read block group table" };
    blk_group_descs = reinterpret_cast<block_group_descriptor*>(bg_buffer);
    for(size_t i = 0; i < num_blk_groups; i++)
    {
        ext_block_group& bg             = block_groups.emplace_back(this, blk_group_descs + i);
        bg.inode_usage_bmp.chain_len    = div_round_up(sb->blocks_per_group, block_size() * CHAR_BIT);
        bg.blk_usage_bmp.chain_len      = div_round_up(sb->inodes_per_group, block_size() * CHAR_BIT);
        bg.inode_block.chain_len        = inode_blocks_per_group;
        allocate_block_buffer(bg.inode_usage_bmp);
        allocate_block_buffer(bg.blk_usage_bmp);
        allocate_block_buffer(bg.inode_block);
        if(!read_hd(bg.inode_usage_bmp) || !read_hd(bg.blk_usage_bmp)) throw std::runtime_error{ "failed to read block group" };
        if(!read_hd(bg.inode_block)) { throw std::runtime_error{ "failed to read inode table" }; }
        uint16_t cs                         = blk_group_descs[i].group_checksum;
        blk_group_descs[i].group_checksum   = 0;
        uint32_t cs0                        = crc32c(uuid_csum, dword(i));
        dword dw_cs                         = crc32c(cs0, blk_group_descs[i]);
        if(dw_cs.lo != cs) throw std::runtime_error{ "checksum calculated value of " + std::to_string(dw_cs.lo, std::ext::hex) + " did not match expected " + std::to_string(cs, std::ext::hex) };
        blk_group_descs[i].group_checksum   = cs;
    }
    std::construct_at(std::addressof(fs_journal), this, sb->journal_inode);
    if(!fs_journal.initialize()) throw std::runtime_error{ "journal init failed" };
    ext_directory_vnode* rdnode = dir_nodes.emplace(this, 2U, next_fd++).first.base();
    if(!rdnode->initialize()) throw std::runtime_error{ "root dir init failed" };
    root_dir                = rdnode;
    qword tstamp            = sys_time(nullptr);
    sb->last_mount_time     = tstamp.lo;
    sb->last_mount_time_hi  = tstamp.hi.lo.lo;
    if(!persist_sb()) throw std::runtime_error{ "superblock write failed" };
    initialized = true;
}
bool extfs::persist_sb()
{
    qword tstamp            = sys_time(nullptr);
    sb->last_write_time     = tstamp.lo;
    sb->last_write_time_hi  = tstamp.hi.lo.lo;
    sb->checksum            = __sb_checksum();
    return write_hd(superblock_lba, sb, sb_sectors);
}
bool extfs::update_free_block_count(int diff)
{
    uint64_t num_blocks = qword(sb->unallocated_blocks_lo, sb->unallocated_blocks_hi);
    num_blocks          += diff;
    qword qwblocks(num_blocks);
    sb->unallocated_blocks_lo = qwblocks.lo;
    sb->unallocated_blocks_hi = qwblocks.hi;
    return persist_sb();
}
uint32_t extfs::claim_inode(bool dir)
{
    for(size_t i = 0; i < block_groups.size(); i++)
    {
        if(block_groups[i].has_available_inode())
        {
            unsigned long* bmp  = reinterpret_cast<unsigned long*>(block_groups[i].inode_usage_bmp.data_buffer);
            off_t avail         = bitmap_scan_single_zero(bmp, (block_size() * block_groups[i].inode_usage_bmp.chain_len) / sizeof(unsigned long));
            uint32_t result     = static_cast<uint32_t>(avail + (sb->inodes_per_group * i) + 1U);
            bitmap_set_bit(bmp, avail);
            if(dir) block_groups[i].increment_dir_ct();
            if(!(block_groups[i].decrement_inode_ct())) return 0U;
            if(!persist_group_metadata(i)) return 0U;
            return result; 
        }
    }
    return 0U;
}
bool extfs::release_inode(uint32_t num, bool dir)
{
    if(!num) return false;
    size_t i            = num / sb->inodes_per_group;
    off_t bit_off       = static_cast<off_t>(num % sb->inodes_per_group);
    unsigned long* bmp  = reinterpret_cast<unsigned long*>(block_groups[i].inode_usage_bmp.data_buffer);
    bitmap_clear_bit(bmp, bit_off);
    if(dir) block_groups[i].decrement_dir_ct();
    if(!block_groups[i].increment_inode_ct()) return false;
    return persist_group_metadata(i);
}
void extfs::release_blocks(uint64_t start, size_t num)
{
    size_t i            = start / sb->blocks_per_group;
    off_t off           = static_cast<off_t>(start % sb->blocks_per_group);
    unsigned long* bmp  = reinterpret_cast<unsigned long*>(block_groups[i].blk_usage_bmp.data_buffer);
    bitmap_clear_chain_bits(bmp, off, num);
    block_groups[i].alter_available_blocks(static_cast<int>(+num));
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
file_node* extfs::on_open(tnode* fd, std::ios_base::openmode mode)
{
    if(fd->is_pipe())
    {
        ext_pipe_pair* p = dynamic_cast<ext_pipe_pair*>(fd->ptr());
        if(!p) return fd->as_file();
        if(mode.in) return addressof(p->in);
        return addressof(p->out);
    }
    else if(ext_file_vnode* vn = dynamic_cast<ext_file_vnode*>(fd->ptr())) { vn->initialize(); return vn; }
    return fd->as_file();
}
filesystem::target_pair extfs::get_parent(directory_node* start, std::string const& path, bool create)
{
    std::vector<std::string> pathspec = std::ext::split(path, path_separator());
    if(pathspec.empty()) throw std::logic_error{ "empty path" };
    directory_node* cur = start;
    for(size_t i = 0; i < pathspec.size() - 1; i++)
    {
        if(pathspec[i].empty()) continue;
        tnode* node = cur->find(pathspec[i]);
        if(!node) 
        {
            if(create) 
            {
                directory_node* created = mkdirnode(cur, pathspec[i]);
                if(!created) throw std::runtime_error{ "failed to create " + pathspec[i] };
                cur = created;
            } 
            else { throw std::out_of_range{ "path " + pathspec[i] + " does not exist (use open_directory(\".../" + pathspec[i] + "\", true) to create it)" }; } 
        }
        else if(node->is_directory()) cur = node->as_directory();
        else throw std::invalid_argument{ "path is invalid because entry " + pathspec[i] + " is a file" };
    }
    return target_pair(std::piecewise_construct, std::forward_as_tuple(cur), std::forward_as_tuple(pathspec.back()));
}
file_node* extfs::open_file(std::string const& path, std::ios_base::openmode mode, bool create)
{
    target_pair parent  = filesystem::get_parent(path, false);
    tnode* node         = parent.first->find(parent.second);
    if(node && node->is_directory()) throw std::logic_error{ "path " + path + " exists and is a directory" };
    file_node* result;
    bool pipe = false;
    if(!node)
    {
        if(!create) throw std::out_of_range{ "file not found: " + path }; 
        if(file_node* created = mkfilenode(parent.first, parent.second)) result = created;
        else throw std::runtime_error{ "failed to create file: " + path };
    }
    else result = on_open(node, mode);
    if(ext_file_vnode* exfn = dynamic_cast<ext_file_vnode*>(result)) { exfn->initialize(); }
    if(!current_open_files.contains(result->vid())) { register_fd(result); }
    if(!pipe) result->current_mode = mode;
    return result;
}
directory_node* extfs::open_directory(std::string const& path, bool create)
{
    if(path.empty()) return get_root_directory(); // empty path or "/" refers to root directory
    target_pair parent  = filesystem::get_parent(path, create);
    tnode* node         = parent.first->find(parent.second);
    directory_node* result;
    if(!node)
    {
        if(create) 
        {
            directory_node* cn = mkdirnode(parent.first, parent.second);
            if(!cn) throw std::runtime_error{ "failed to create " + path };
            result = cn;
        } 
        else throw std::out_of_range{ "path " + path + " does not exist (use open_directory(\"" + path + "\", true) to create it)" }; 
    }
    else if(node->is_file()) throw std::invalid_argument{ "path " + path + " exists and is a file" };
    else { result = node->as_directory(); }
    if(!current_open_files.contains(result->vid())) { register_fd(result); }
    return result;
}
directory_node* extfs::mkdirnode(directory_node* parent, std::string const& name)
{
    qword tstamp            = sys_time(nullptr);
    uint8_t extrabits       = (tstamp.hi.hi >> 4) & 0x03UC;
    if(uint32_t inode_num   = claim_inode(true); __builtin_expect(inode_num != 0, true)) try
    {
        ext_inode* inode = new(static_cast<void*>(get_inode(inode_num))) ext_inode
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
                        .entries        { 0US },
                        .max_entries    { 4US },
                        .depth          { 0US }
                    } 
                } 
            },
            .size_hi            { 0U },
            .extra_isize        { isize_val },
            .changed_time_hi    { extrabits },
            .mod_time_extra     { extrabits },
            .created_time       { tstamp.lo },
            .created_time_hi    { extrabits }
        };
        ext_directory_vnode* vnode      = dir_nodes.emplace(this, inode_num, inode, next_fd++).first.base();
        ext_directory_vnode& exparent   = dynamic_cast<ext_directory_vnode&>(*parent);
        if(exparent.add_dir_entry(vnode, dti_dir, name.data(), name.size())) 
        {
            if(!vnode->init_dir_blank(std::addressof(exparent))) { panic("failed to initialize directory"); return nullptr; }
            if(vnode->fsync()) return vnode;
            else panic("failed to persist directory node");
        }
        else panic("failed to add directory entry");
    }
    catch(std::exception& e) { panic(e.what()); }
    else panic("failed to get inode");
    return nullptr;
}
file_node* extfs::mkfilenode(directory_node* parent, std::string const& name)
{
    qword tstamp        = sys_time(nullptr);
    uint8_t extrabits   = (tstamp.hi.hi >> 4) & 0x03;
    if(uint32_t inode_num = claim_inode(false); __builtin_expect(inode_num != 0, true)) try
    {
        ext_inode* inode = new(static_cast<void*>(get_inode(inode_num))) ext_inode
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
                        .entries        { 0US },
                        .max_entries    { 4US },
                        .depth          { 0US } 
                    } 
                } 
            },
            .size_hi            { 0U },
            .extra_isize        { isize_val },
            .changed_time_hi    { extrabits },
            .mod_time_extra     { extrabits },
            .created_time       { tstamp.lo },
            .created_time_hi    { extrabits }
        };
        ext_file_vnode* vnode           = file_nodes.emplace(this, inode_num, inode, next_fd++).first.base();
        ext_directory_vnode& exparent   = dynamic_cast<ext_directory_vnode&>(*parent);
        if(__builtin_expect(exparent.add_dir_entry(vnode, dti_regular, name.data(), name.size()), true)) 
        {
            if(__builtin_expect(!vnode->initialize(), false)) { panic("failed to initialize node"); return nullptr; };
            vnode->fsync();
            return vnode;
        }
        else panic("failed to add directory entry");
    }
    catch(std::exception& e) { panic(e.what()); }
    else panic("failed to get inode");
    return nullptr;
}
device_node* extfs::mkdevnode(directory_node* parent, std::string const& name, dev_t id, int fd)
{
    device_stream* dev = dreg[id];
    if(!dev) { throw std::invalid_argument{"no device found with that id"}; }
    qword tstamp        = sys_time(nullptr);
    uint8_t extrabits   = (tstamp.hi.hi >> 4) & 0x03;
    if(uint32_t inode_num = claim_inode(false); __builtin_expect(inode_num != 0, true)) try
    {
        ext_inode* inode = new(static_cast<void*>(get_inode(inode_num))) ext_inode
        {
            .mode               { 0020666U },
            .size_lo            { 0U },
            .changed_time       { tstamp.lo },
            .modified_time      { tstamp.lo },
            .flags              { use_extents },
            .device_hardlink_id { id },
            .size_hi            { 0U },
            .extra_isize        { isize_val },
            .changed_time_hi    { extrabits },
            .mod_time_extra     { extrabits },
            .created_time       { tstamp.lo },
            .created_time_hi    { extrabits }
        };
        array_copy(inode->block_info.link_target, name.c_str(), std::min(name.size(), 60UZ));
        ext_device_vnode* vnode         = dev_nodes.emplace(this, inode_num, dev, fd).first.base();
        ext_directory_vnode& exparent   = dynamic_cast<ext_directory_vnode&>(*parent);
        if(exparent.add_dir_entry(vnode, dti_chardev, name.data(), name.size()) && vnode->update_inode()) { return vnode; }
        else panic("failed to add directory entry");
    }
    catch(std::exception& e) { panic(e.what()); }
    else panic("failed to get inode");
    return nullptr;
}
pipe_pair extfs::mkpipe(directory_node* parent, std::string const& name)
{
    qword tstamp = sys_time(nullptr);
    uint8_t extrabits = (tstamp.hi.hi >> 4) & 0x03;
    if(uint32_t inode_num = claim_inode(false); __builtin_expect(inode_num != 0, true)) try
    {
        ext_inode* inode = new(static_cast<void*>(get_inode(inode_num))) ext_inode
        {
            .mode               { 0010666U },
            .size_lo            { 0U },
            .changed_time       { tstamp.lo },
            .modified_time      { tstamp.lo },
            .flags              { use_extents },
            .size_hi            { 0U },
            .extra_isize        { isize_val },
            .changed_time_hi    { extrabits },
            .mod_time_extra     { extrabits },
            .created_time       { tstamp.lo },
            .created_time_hi    { extrabits }
        };
        array_copy(inode->block_info.link_target, name.c_str(), std::min(name.size(), 60UZ));
        ext_directory_vnode& exparent   = dynamic_cast<ext_directory_vnode&>(*parent);
        ext_pipe_pair& result           = __init_pipes(inode_num, name);
        if(__builtin_expect(exparent.add_dir_entry(addressof(result.in), dti_fifo, name.c_str(), name.size()) && result.in.update_inode(), true)) { return { addressof(result.in), addressof(result.out) }; }
        else panic("failed to add directory entry");
    }
    catch(std::exception& e) { panic(e.what()); }
    else panic("failed to get inode");
    return { nullptr, nullptr };
}
void extfs::release_all(ext_vnode& extn)
{
    for(size_t i = 0; i < extn.cached_metadata.size(); i++) { free_block_buffer(extn.cached_metadata[i]); release_blocks(extn.cached_metadata[i].block_number, extn.cached_metadata[i].chain_len); }
    for(size_t i = 0; i < extn.block_data.size(); i++) { release_blocks(extn.block_data[i].block_number, extn.block_data[i].chain_len); }
    extn.extents.base_extent_level.clear();
    extn.extents.tracked_extents.clear();
    if(extn.on_disk_node->flags & use_extents) { array_zero(extn.on_disk_node->block_info.ext4_extent.root_nodes, 4); }
    else array_zero(reinterpret_cast<uint32_t*>(std::addressof(extn.on_disk_node->block_info.legacy_extent)), 15);
}
void extfs::dldirnode(directory_node* dd)
{
    if(!dd->is_empty()) { throw std::logic_error{ std::string{ "cannot delete non-empty directory " } + dd->name() }; }
    uint64_t cid                = dd->cid();
    ext_directory_vnode& exdn   = dynamic_cast<ext_directory_vnode&>(*dd);
    uint32_t inode_num          = exdn.inode_number;
    release_all(exdn);
    release_inode(inode_num, true);
    array_zero(reinterpret_cast<char*>(exdn.on_disk_node), sb->inode_size);
    persist_inode(inode_num);
    dir_nodes.erase(cid);
}
void extfs::dlfilenode(file_node* fd)
{
    uint64_t cid            = fd->cid();
    ext_file_vnode& exfn    = dynamic_cast<ext_file_vnode&>(*fd);
    uint32_t inode_num      = exfn.inode_number;
    release_all(exfn);
    release_inode(inode_num, false);
    array_zero(reinterpret_cast<char*>(exfn.on_disk_node), sb->inode_size);
    persist_inode(inode_num);
    file_nodes.erase(cid);
}
void extfs::dldevnode(device_node* dd)
{
    ext_device_vnode& exdn  = dynamic_cast<ext_device_vnode&>(*dd);
    dev_t dn                = dd->get_device_id();
    uint32_t inode_num      = exdn.inode_number;
    release_inode(inode_num, false);
    array_zero(reinterpret_cast<char*>(exdn.on_disk_node), sb->inode_size);
    persist_inode(inode_num);
    dev_linked_nodes.erase(dn);
}
void extfs::dlpipenode(fs_node* fn)
{
    if(ext_pipe_pair* pipes = dynamic_cast<ext_pipe_pair*>(fn))
    {
        uint32_t inode_num = pipes->in.inode_number;
        release_inode(inode_num, false);
        array_zero(reinterpret_cast<char*>(pipes->in.on_disk_node), sb->inode_size);
        persist_inode(inode_num);
        named_pipes.erase(pipes->concrete_name);
    }
    else filesystem::dlpipenode(fn);
}
disk_block* extfs::claim_blocks(ext_vnode* requestor, size_t how_many)
{
    if(!how_many) return nullptr;
    for(size_t i = 0; i < block_groups.size(); i++)
    {
        if(block_groups[i].has_available_blocks(how_many))
        {
            unsigned long* bmp  = reinterpret_cast<unsigned long*>(block_groups[i].blk_usage_bmp.data_buffer);
            off_t avail         = bitmap_scan_chain_zeroes(bmp, (block_size() * block_groups[i].blk_usage_bmp.chain_len) / sizeof(unsigned long), how_many);
            if(avail < 0) continue;
            bitmap_set_chain_bits(bmp, avail, how_many);
            if(!block_groups[i].alter_available_blocks(static_cast<int>(-how_many))) return nullptr;
            uint64_t result = i * sb->blocks_per_group + avail;
            disk_block* blk = std::addressof(requestor->block_data.emplace_back(result, nullptr, false, how_many));
            if(!((requestor->on_disk_node->flags & use_extents) ? requestor->extents.push_extent_ext4(blk) : requestor->extents.push_extent_legacy(blk))) return nullptr;
            dword blcnt(requestor->on_disk_node->blocks_count_lo, requestor->on_disk_node->blocks_count_hi);
            blcnt                                       += how_many;
            requestor->on_disk_node->blocks_count_lo    = blcnt.lo;
            requestor->on_disk_node->blocks_count_hi    = blcnt.hi;
            if(!persist_group_metadata(i)) return nullptr;
            return blk;
        }
    }
    panic("out of space");
    return nullptr;
}
disk_block* extfs::claim_metadata_block(ext_node_extent_tree* requestor)
{
    for(size_t i = 0; i < block_groups.size(); i++)
    {
        if(block_groups[i].has_available_blocks())
        {
            unsigned long* bmp  = reinterpret_cast<unsigned long*>(block_groups[i].blk_usage_bmp.data_buffer);
            off_t avail         = bitmap_scan_single_zero(bmp, (block_size() * block_groups[i].blk_usage_bmp.chain_len) / sizeof(unsigned long));
            bitmap_set_chain_bits(bmp, avail, 1);
            if(!block_groups[i].alter_available_blocks(-1)) return nullptr; // this only gets called from push extent, which in turn gets called from claim blocks which calls persist metadata
            uint64_t result = i * sb->blocks_per_group + avail;
            dword blcnt(requestor->tracked_node->on_disk_node->blocks_count_lo, requestor->tracked_node->on_disk_node->blocks_count_hi);
            blcnt++;
            requestor->tracked_node->on_disk_node->blocks_count_lo = blcnt.lo;
            requestor->tracked_node->on_disk_node->blocks_count_hi = blcnt.hi;
            return std::addressof(requestor->tracked_node->cached_metadata.emplace_back(result, allocate_block_buffer(), false, 1U));
        }
    }
    panic("out of space");
    return nullptr;
}
bool extfs::persist(ext_vnode* n) 
{ 
    if(journal_mode() != ordered) { return fs_journal.create_txn(n); } 
    for(disk_block& db : n->block_data) 
    {
		if(!(db.dirty && db.block_number && db.data_buffer)) { continue; }
        if(!write_hd(db)) { panic("write failed"); return false; }
        db.dirty = false;
    }
    std::vector<disk_block> dirty_metadata{};
    for(disk_block& mdb : n->cached_metadata) { if(mdb.dirty) { dirty_metadata.push_back(mdb); mdb.dirty = false; } }
    if(dirty_metadata.empty()) return true; // vacuous truth if nothing to do
    return fs_journal.create_txn(dirty_metadata) && persist_sb();
}
bool extfs::persist_group_metadata(size_t group_num)
{
    if(__builtin_expect(group_num < block_groups.size(), true))
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
    size_t grp = group_num_for_inode(inode_num);
    if(__builtin_expect(grp >= block_groups.size(), false)) { panic("invalid group number"); return false; }
    std::vector<disk_block> blks{};
    size_t bs       = block_size();
    uint64_t iblk   = inode_to_block(inode_num);
    off_t ipos      = inode_block_offset(inode_num);
    blks.emplace_back(iblk, block_groups[grp].inode_block.data_buffer + (ipos - (ipos % bs)), true, 1UL);
    return fs_journal.create_txn(blks);
}
static ext_dirent_type mode_to_dirent_type(file_mode mode)
{
    if(mode.is_directory()) return dti_dir;
    else if(mode.is_chardev()) return dti_blockdev;
    else if(mode.is_chardev()) return dti_chardev;
    else if(mode.is_socket()) return dti_socket;
    else if(mode.is_fifo()) return dti_fifo;
    else if(mode.is_symlink()) return dti_symlink;
    else if(mode.is_regular()) return dti_regular;
    else return dti_unknown;
}
fs_node* extfs::inode_to_vnode(uint32_t idx, ext_dirent_type type)
{
    ext_inode* n = get_inode(idx);
    if(!(sb->required_features & dirent_type)) type = mode_to_dirent_type(n->mode);
    if(type == dti_dir) return dir_nodes.emplace(this, idx, next_fd++).first.base();
    else if(type == dti_fifo)
    {
        std::string name(n->block_info.link_target, std::strnlen(n->block_info.link_target, 60UZ));
        if(named_pipes.contains(name)) return named_pipes.find(name).base();
        return addressof(__init_pipes(idx, name));
    }
    else if(type == dti_chardev || type == dti_blockdev)
    { 
        dev_t id = n->device_hardlink_id;
        if(dev_linked_nodes.contains(id)) return dev_linked_nodes[id];
        device_stream* dev = dreg[id];
        return (dev_linked_nodes[id] = dev_nodes.emplace(this, idx, dev, next_fd++).first.base());
    }
    else 
    { 
        next_fd         = std::max(3, static_cast<int>(file_nodes.size() + device_nodes.size()));
        fs_node* result = file_nodes.emplace(this, idx, next_fd++).first.base();
        return result;
    }
}
device_node* extfs::lndev(const std::string& where, int fd, dev_t id, bool create_parents)
{
    target_pair parent = filesystem::get_parent(where, create_parents);
    if(parent.first->find(parent.second)) throw std::logic_error{ "cannot create link " + parent.second + " because it already exists" };
    ext_directory_vnode& exparent = dynamic_cast<ext_directory_vnode&>(*parent.first);
    if(dev_linked_nodes.contains(id))
    {
        ext_device_vnode* node = dev_linked_nodes[id];
        if(exparent.add_dir_entry(node, dti_chardev, parent.second.data(), parent.second.size()) && node->update_inode()) return node;
        throw std::runtime_error{ "failed to create directory entry" };
    }
    device_node* result = mkdevnode(parent.first, parent.second, id, fd);
    register_fd(result);
    return result; 
}
bool extfs::truncate_node(ext_vnode* n)
{
    if(__builtin_expect(!n->on_disk_node->size_hi && !n->on_disk_node->size_lo, false)) return true;
    release_all(*n);
    n->truncate_buffer();
    return n->update_inode();
}
tnode* extfs::resolve_symlink(ext_directory_vnode* from, std::string const& link, std::set<fs_node*>& checked)
{
    if(!from) from = dynamic_cast<ext_directory_vnode*>(root_dir);
    std::vector<std::string> pathspec = std::ext::split(link, path_separator());
    if(pathspec.empty()) throw std::logic_error{ "empty path" };
    for(size_t i = 0; i < pathspec.size() - 1; i++)
    {
        if(pathspec[i].empty()) continue;
        tnode* node = from->find_r(pathspec[i], checked);
        if(!node) throw std::out_of_range{ "broken link" };
        else if(node->is_directory()) { from = dynamic_cast<ext_directory_vnode*>(node->as_directory()); if(!from) throw std::bad_cast(); }
        else throw std::invalid_argument{ "symlink path component .../" + pathspec[i] + "/ is a file" };
    }
    return from->find_r(pathspec.back(), checked);
}