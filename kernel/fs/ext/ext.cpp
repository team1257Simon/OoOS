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
jbd2_txn_queue::reference jbd2_txn_queue::put_txn(std::vector<disk_block>&& blocks, jbd2_commit_header&& h) { return this->emplace(static_cast<transaction_id>(this->size()), std::move(blocks), std::move(h)); }
bool jbd2_journal_txn::execute_and_complete(extfs *fs_ptr) { for(std::vector<disk_block>::iterator i = data_blocks.begin(); i != data_blocks.end(); i++) { if(!fs_ptr->write_to_disk(*i)) { panic("write failed"); return false; } } return true; }
bool jbd2_journal::need_escape(disk_block const &bl) { return ((__be32(reinterpret_cast<uint32_t const*>(bl.data_buffer)[0])) == jbd2_magic); }
size_t jbd2_journal::desc_tag_size(bool same_uuid) { return (sb->required_features & csum_v3 ? 16 : (sb->required_features & x64_support ? 12 : 8)) + (same_uuid ? 0 : 16); }
size_t jbd2_journal::tags_per_block() { return (sb->journal_block_size - sizeof(jbd2_header) - desc_tag_size(false) - (sb->required_features & (csum_v2 | csum_v3) ? 4 : 0)) / desc_tag_size(true); }
char *jbd2_journal::allocate_block_buffer() { char* result = std::allocator<char>{}.allocate(sb->journal_block_size); array_zero(result, sb->journal_block_size); return result; }
void jbd2_journal::free_buffers(std::vector<disk_block> &bufs) { for(std::vector<disk_block>::iterator i = bufs.begin(); i < bufs.end(); i++) std::allocator<char>{}.deallocate(i->data_buffer, sb->journal_block_size); bufs.clear(); }
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
off_t jbd2_journal::desc_tag_create(disk_block const& bl, void* where, uint32_t seq, bool is_first, bool is_last)
{
    off_t result = static_cast<off_t>(desc_tag_size(true)) * bl.chain_len;
    uint32_t fl = (is_first ? same_uuid : 0) | (is_last ? last_block : 0) | (((__be32(reinterpret_cast<uint32_t const*>(bl.data_buffer)[0])) == jbd2_magic) ? escape : 0);
    if(sb->required_features & csum_v3) { new (static_cast<jbd2_block_tag3*>(where)) jbd2_block_tag3{ .block_number = __be32((bl.block_number) & 0xFFFFFFFF), .flags = __be32(fl), .block_number_hi = __be32((bl.block_number >> 32) & 0xFFFFFFFF) }; }
    else { new (static_cast<jbd2_block_tag*>(where)) jbd2_block_tag{ .block_number = __be32((bl.block_number) & 0xFFFFFFFF), .flags = __be16(fl), .block_number_hi = __be32(sb->required_features & x64_support ? ((bl.block_number >> 32) & 0xFFFFFFFF) : 0) }; }
    if(is_first) { uint8_t* uuid_pos = reinterpret_cast<uint8_t*>(where) + result; result += 16; arraycopy(uuid_pos, sb->uuid.data_bytes, 16); }
    // TODO: compute checksums
    return result;
}
bool jbd2_journal::create_txn(ext_vnode *changed_node)
{
    std::vector<disk_block> dirty_blocks{};
    for(std::vector<disk_block>::iterator i = changed_node->block_data.begin(); i != changed_node->block_data.end(); i++) { if(i->dirty && i->data_buffer) { i->dirty = false; dirty_blocks.push_back(*i); } }
    for(std::vector<disk_block>::iterator i = changed_node->cached_metadata.begin(); i != changed_node->cached_metadata.end(); i++) { if(i->dirty) { i->dirty = false; dirty_blocks.push_back(*i); } }
    return create_txn(dirty_blocks);
}
bool jbd2_journal::create_txn(std::vector<disk_block> const& txn_blocks)
{
    size_t tpb = tags_per_block();
    disk_block tb{ first_open_block++, allocate_block_buffer(), false, 1 };
    uint32_t s = 0;
    uint64_t j = tb.block_number;
    size_t k = 0;
    off_t o;
    for(std::vector<disk_block>::const_iterator i = txn_blocks.begin(); i < txn_blocks.end(); i++)
    {
        if(!i->data_buffer) continue; // skip empty file blocks if any made it this far
        if(!k)
        {
            array_zero(tb.data_buffer, sb->journal_block_size);
            o = static_cast<off_t>(sizeof(jbd2_header));
            new(reinterpret_cast<jbd2_header*>(tb.data_buffer)) jbd2_header{ .blocktype = __be32(descriptor), .sequence = __be32(s++) };
        }
        for(size_t n = 0; n < std::max(1UL, i->chain_len); n++)
        {
            disk_block db{ j++, i->data_buffer + n*parent_fs->block_size(), false, 1 };
            o += desc_tag_create(db, tb.data_buffer + o, s, !k, (k + 1 == tpb || i + 1 == txn_blocks.end()));
            bool esc = ((__be32(reinterpret_cast<uint32_t const*>(db.data_buffer)[0])) == jbd2_magic);
            if(esc) *reinterpret_cast<uint32_t*>(db.data_buffer) = 0;
            if(!parent_fs->write_to_disk(db)) { panic("disk write failed"); sb->journal_errno = __be32(EPIPE); if(esc) { *reinterpret_cast<uint32_t*>(db.data_buffer) = jbd2_magic; } return false; }
            if(esc) { *reinterpret_cast<uint32_t*>(db.data_buffer) = jbd2_magic; }
        }
        k = (k + 1) % tpb;
        if(!k || i + 1 == txn_blocks.end())
        {
            if(!parent_fs->write_to_disk(tb)) { panic("disk write failed"); sb->journal_errno = __be32(EPIPE); return false; }
            tb.block_number = j++;
            first_open_block = j;
        }
    }
    array_zero(tb.data_buffer, sb->journal_block_size);
    new(reinterpret_cast<jbd2_commit_header*>(tb.data_buffer)) jbd2_commit_header{}; // TODO: figure out these values, checksum etc
    active_transactions.put_txn(std::vector<disk_block>(txn_blocks), jbd2_commit_header(*reinterpret_cast<jbd2_commit_header const*>(tb.data_buffer)));
    if(!parent_fs->write_to_disk(tb)) { panic("disk write failed"); sb->journal_errno = __be32(EPIPE); return false; }
    std::allocator<char>{}.deallocate(tb.data_buffer, sb->journal_block_size);
    return true;
}
bool jbd2_journal::clear_log()
{
    disk_block zb{ sb->start_block, allocate_block_buffer(), false, 1 };
    array_zero(zb.data_buffer, sb->journal_block_size);
    for(size_t i = 0; i < sb->journal_block_count; i++, zb.block_number++) { if(!parent_fs->write_to_disk(zb)) { panic("write failed"); sb->journal_errno = __be32(EPIPE); return false; } } // TODO: better error handling :)
    std::allocator<char>{}.deallocate(zb.data_buffer, sb->journal_block_size);
    for(std::vector<disk_block>::iterator i = replay_blocks.begin(); i != replay_blocks.end(); i++) { if(i->data_buffer) { std::allocator<char>{}.deallocate(i->data_buffer, sb->journal_block_size); } }
    replay_blocks.clear();
    return true;
}
bool jbd2_journal::execute_pending_txns()
{
    try
    {
        while(!active_transactions.at_end()) 
        { 
            if(!active_transactions.pop().execute_and_complete(parent_fs)) 
            { 
                // TODO: better error handling :)
                active_transactions.restart(); 
                sb->journal_errno = __be32(EPIPE); 
                return false; 
            } 
        } 
        return clear_log();
    }
    catch(std::exception& e) { panic(e.what()); }
    return false;
}
bool jbd2_journal::read_log()
{
    char* blk_buffer = allocate_block_buffer();
    disk_block blk{ sb->start_block, blk_buffer, false, 1 };
    bool success = true;
    try { while(blk.block_number < sb->start_block + sb->journal_block_count) parse_next_log_entry(blk); }
    catch(std::exception& e) { panic(e.what()); success = false; }
    std::allocator<char>{}.deallocate(blk_buffer, sb->journal_block_size);
    return success;
}
void jbd2_journal::parse_next_log_entry(disk_block& blk)
{
    if(!parent_fs->read_from_disk(blk)) throw std::runtime_error{ "disk read failed on journal block" };
    jbd2_header* h = reinterpret_cast<jbd2_header*>(blk.data_buffer);
    if(h->magic != jbd2_magic) { blk.block_number++; }
    else if(h->blocktype == descriptor)
    {
        size_t tpb = tags_per_block();
        std::vector<disk_block> txn_data_blocks{};
        bool have_commit = false;
        jbd2_commit_header ch;
        while(!have_commit)
        {
            if(h->magic != jbd2_magic) { blk.block_number++; continue; }
            if((have_commit = (h->blocktype == commit))) new(&ch) jbd2_commit_header(*reinterpret_cast<jbd2_commit_header const*>(blk.data_buffer));
            else 
            {
                char* current_pos = blk.data_buffer + sizeof(jbd2_header);
                uint32_t dbnum = blk.block_number + 1;
                bool last_entry = false;
                for(size_t i = 0; i < tpb && !last_entry; i++)
                {
                    uint64_t target_block;
                    bool esc = false;
                    bool s_uuid;
                    if(sb->required_features & csum_v3)
                    {
                        jbd2_block_tag3* tag = reinterpret_cast<jbd2_block_tag3*>(current_pos);
                        // TODO verify checksum
                        target_block = qword(tag->block_number, tag->block_number_hi);
                        last_entry = tag->flags & last_block;
                        esc = tag->flags & escape;
                        s_uuid = tag->flags & same_uuid;
                    }
                    else
                    {
                        jbd2_block_tag* tag = reinterpret_cast<jbd2_block_tag*>(current_pos);
                        // TODO verify checksum
                        target_block = qword(tag->block_number, (sb->required_features & x64_support) ? tag->block_number_hi : 0U);
                        last_entry = tag->flags & last_block;
                        esc = tag->flags & escape;
                        s_uuid = tag->flags & same_uuid;
                    }
                    current_pos += desc_tag_size(s_uuid);
                    disk_block& next = txn_data_blocks.emplace_back(target_block, allocate_block_buffer(), true, 1U);
                    disk_block read_tar{ dbnum++, next.data_buffer, false, 1 };
                    if(!parent_fs->read_from_disk(read_tar)) { free_buffers(txn_data_blocks); throw std::runtime_error{ "disk read failed on data block" }; }
                    if(esc) { *reinterpret_cast<uint32_t*>(read_tar.data_buffer) = jbd2_magic; }
                }
                blk.block_number = dbnum;
                if(!parent_fs->read_from_disk(blk)) { free_buffers(txn_data_blocks); throw std::runtime_error{ "disk read failed on journal block" }; }
                h = reinterpret_cast<jbd2_header*>(blk.data_buffer);
            }
        }
        for(std::vector<disk_block>::iterator i = txn_data_blocks.begin(); i != txn_data_blocks.end(); i++) { replay_blocks.push_back(*i); }
        active_transactions.put_txn(std::move(txn_data_blocks), std::move(ch));
        blk.block_number++;
    }
    // TODO handle revocation blocks
}
file_node *extfs::open_fd(tnode* fd)
{
    file_node* n = fd->as_file();
    if(ext_file_vnode* exfn = dynamic_cast<ext_file_vnode*>(n)) exfn->initialize();
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
uint64_t extfs::claim_next_available_block()
{
    return 0; // TODO
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