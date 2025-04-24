#include "fs/ext.hpp"
#include "sys/errno.h"
#include "kdebug.hpp"
std::allocator<jbd2_superblock> sb_alloc{};
jbd2::jbd2(extfs* parent, uint32_t inode) : ext_vnode{ parent, inode } {}
jbd2::~jbd2() { if(sb) sb_alloc.deallocate(sb, 1); }
bool jbd2_transaction::execute_and_complete(extfs* fs_ptr) { for(disk_block& db : data_blocks) { if(!db.block_number || !db.data_buffer) continue; if(!fs_ptr->write_hd(db)) { panic("write failed"); return false; } } return true; }
bool jbd2::need_escape(disk_block const& bl) { return (((reinterpret_cast<__be32 const*>(bl.data_buffer)[0])) == jbd2_magic); }
size_t jbd2::desc_tag_size(bool same_uuid) { return (sb->required_features & csum_v3 ? 16 : (sb->required_features & x64_support ? 12 : 8)) + (same_uuid ? 0 : 16); }
size_t jbd2::tags_per_block() { return 1 + (parent_fs->block_size() - sizeof(jbd2_header) - desc_tag_size(false) - (sb->required_features & (csum_v2 | csum_v3) ? 4 : 0)) / desc_tag_size(true); }
off_t jbd2::desc_tag_create(disk_block const& bl, void* where, uint32_t seq, bool is_first, bool is_last)
{
    off_t result = static_cast<off_t>(desc_tag_size(true));
    uint32_t fl = (!is_first ? same_uuid : 0) | (is_last ? last_block : 0) | (need_escape(bl) ? escape : 0);
    __be32 be_seq(seq);
    uint32_t csum = crc32c(uuid_checksum, seq);
    csum = crc32c_blk(csum, bl, parent_fs->block_size());
    if(sb->required_features & csum_v3) { std::construct_at<jbd2_block_tag3>(static_cast<jbd2_block_tag3*>(where), __be32((bl.block_number) & 0xFFFFFFFF), __be32(fl), __be32((bl.block_number & 0xFFFFFFFF00000000) >> 32), __be32(csum)); }
    else { std::construct_at<jbd2_block_tag>(static_cast<jbd2_block_tag*>(where), __be32((bl.block_number) & 0xFFFFFFFF), __be16(uint16_t(csum & 0xFFFF)), __be16(fl), __be32(sb->required_features & x64_support ? (bl.block_number & 0xFFFFFFFF00000000) >> 32 : 0)); }
    if(is_first) { uint8_t* uuid_pos = static_cast<uint8_t*>(where) + result; result += 16; array_copy(uuid_pos, sb->uuid.data_bytes, sizeof(guid_t)); }
    return result;
}
bool jbd2::create_txn(ext_vnode* changed_node)
{
    std::vector<disk_block> dirty_blocks{};
    for(std::vector<disk_block>::iterator i = changed_node->block_data.begin(); i != changed_node->block_data.end(); i++) { if(i->dirty && i->data_buffer) { i->dirty = false; dirty_blocks.push_back(*i); } }
    for(std::vector<disk_block>::iterator i = changed_node->cached_metadata.begin(); i != changed_node->cached_metadata.end(); i++) { if(i->dirty) { i->dirty = false; dirty_blocks.push_back(*i); } }
    if(dirty_blocks.empty()) return true; // vacuous success; nothing to do
    return create_txn(dirty_blocks);
}
bool jbd2::create_txn(std::vector<disk_block> const& txn_blocks)
{
    if(txn_blocks.empty()) return true; // vacuous success; nothing to do
    size_t tpb = tags_per_block();
    size_t bs = parent_fs->block_size();
    uint64_t txn_st_block = first_open_block + sb->start_block;
    std::vector<disk_block> actual_blocks{};
    for(disk_block const& b : txn_blocks) { if(b.chain_len > 1) { for(size_t i = 0; i < b.chain_len; i++) { actual_blocks.emplace_back(b.block_number + i, b.data_buffer + i * bs, b.dirty, 1UL); } } else actual_blocks.push_back(b); }
    unsigned seq = static_cast<unsigned>(active_transactions.size());
    char* pos = __cur();
    char* dblk_tar = pos + bs;
    std::construct_at<jbd2_header>(reinterpret_cast<jbd2_header*>(pos), jbd2_magic, __be32(descriptor), __be32(seq));
    pos += sizeof(jbd2_header);
    mark_write(pos);
    size_t total = 0, k = tpb;
    bool first = true;
    for(size_t i = 0; i < actual_blocks.size(); i++, total++)
    {
        pos += desc_tag_create(actual_blocks[i], pos, seq, first, k == 1 || !(i + 1 < actual_blocks.size()));
        first = false;
        array_copy(dblk_tar, actual_blocks[i].data_buffer, bs);
        mark_write(dblk_tar);
        actual_blocks[i].data_buffer = dblk_tar;
        dblk_tar += bs;
        if(!(--k))
        {
            pos = __cur() + (bs - 4);
            if(sb->required_features & (csum_v2 | csum_v3)) std::construct_at(reinterpret_cast<__be32*>(pos), crc32c(uuid_checksum, __cur(), bs));
            __setc(dblk_tar);
            dblk_tar += bs;
            pos = __cur();
            std::construct_at<jbd2_header>(reinterpret_cast<jbd2_header*>(pos), jbd2_magic, __be32(descriptor), __be32(seq));
            pos += sizeof(jbd2_header);
            mark_write(pos);
            total++;
            first = true;
        }
    }
    pos = __cur() + (bs - 4);
    if(sb->required_features & (csum_v2 | csum_v3)) std::construct_at(reinterpret_cast<__be32*>(pos), crc32c(uuid_checksum, __cur(), bs));
    total++;
    __setc(dblk_tar);
    disk_block ch_block(txn_st_block + total++, dblk_tar, false, 1U);
    uint64_t timestamp = sys_time(nullptr);
    jbd2_commit_header* ch = new(static_cast<void*>(dblk_tar)) jbd2_commit_header
    { 
        .checksum_type = 4,
        .checksum_size = 4,
        .commit_seconds = __be64(timestamp / 1000UL),
        .commit_nanos = __be32((timestamp % 1000U) * 1000U)
    };
    mark_write(dblk_tar);
    uint32_t csum = crc32c_blk(uuid_checksum, ch_block, bs);
    barrier();
    ch->checksum[0] = csum;
    first_open_block = txn_st_block + total;
    __bumpc(bs);
    if(!ddwrite()) return false;
    barrier();
    active_transactions.emplace(actual_blocks, static_cast<int>(seq));
    return true;
}
bool jbd2::ddread()
{
    disk_block full_blk(block_data[0].block_number, __beg(), false, extents.total_extent);
    if(!parent_fs->read_hd(full_blk)) { panic("failed to read journal"); return false; }
    return true;
}
bool jbd2::ddwrite()
{
    disk_block full_blk(block_data[0].block_number, __beg(), false, extents.total_extent);
    if(!parent_fs->write_hd(full_blk)) { panic("failed to write journal"); return false; }
    return true;
}
bool jbd2::clear_log()
{
    size_t bs = parent_fs->block_size();
    array_zero(__beg() + bs, static_cast<size_t>(__capacity() - bs));
    replay_blocks.clear();
    return ddwrite();
}
bool jbd2::execute_pending_txns()
{
    try { while(!active_transactions.at_end()) { jbd2_transaction* t = std::addressof(active_transactions.pop()); if(!t->execute_and_complete(parent_fs)) { active_transactions.restart(); return false; } } return true; }
    catch(std::exception& e) { panic(e.what()); }
    return false;
}
uint32_t jbd2::calculate_sb_checksum()
{
    uint32_t sb_cs_val = sb->checksum;
    sb->checksum = __be32(0U);
    uint32_t result = crc32c(*sb);
    sb->checksum = __be32(sb_cs_val);
    return result;
}
bool jbd2::initialize()
{
    if(has_init) return true;
    if(!init_extents()) return false;
    size_t bs = parent_fs->block_size();
    size_t s_req = extents.total_extent * bs;
    if(!__grow_buffer(s_req)) { panic("failed to allocate buffer"); return false; }
    update_block_ptrs();
    if(!ddread()) return false;
    __bumpc(bs);
    sb = reinterpret_cast<jbd2_superblock*>(block_data[0].data_buffer);
    if(sb->header.magic != jbd2_magic) { uintptr_t num = sb->header.magic; std::string errstr = "superblock invalid; expected magic number of 0x99B3030C but found magic number of " + std::to_string(reinterpret_cast<void*>(num)); panic(errstr.c_str()); return false; }
    uuid_checksum = crc32c(sb->uuid);
    return (has_init = true);
}