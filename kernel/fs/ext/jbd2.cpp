#include "fs/ext.hpp"
#include "sys/errno.h"
std::allocator<char> bl_alloc{};
std::allocator<jbd2_superblock> sb_alloc{};
jbd2::jbd2(extfs *parent, uint32_t inode) : ext_vnode{ parent, inode } {}
jbd2::~jbd2() { if(sb) sb_alloc.deallocate(sb, 1); }
jbd2_transaction_queue::reference jbd2_transaction_queue::put_txn(std::vector<disk_block>&& blocks, jbd2_commit_header&& h) { return this->emplace(static_cast<transaction_id>(this->size()), blocks, h); }
bool jbd2_transaction::execute_and_complete(extfs *fs_ptr) { for(std::vector<disk_block>::iterator i = data_blocks.begin(); i != data_blocks.end(); i++) { if(!i->block_number) continue; /* Zeroed blocknums are revoked blocks */ if(!fs_ptr->write_to_disk(*i)) { panic("write failed"); return false; } } return true; }
bool jbd2::need_escape(disk_block const &bl) { return (((reinterpret_cast<__be32 const*>(bl.data_buffer)[0])) == jbd2_magic); }
size_t jbd2::desc_tag_size(bool same_uuid) { return (sb->required_features & csum_v3 ? 16 : (sb->required_features & x64_support ? 12 : 8)) + (same_uuid ? 0 : 16); }
size_t jbd2::tags_per_block() { return (sb->journal_block_size - sizeof(jbd2_header) - desc_tag_size(false) - (sb->required_features & (csum_v2 | csum_v3) ? 4 : 0)) / desc_tag_size(true); }
char *jbd2::allocate_block_buffer() { char* result = bl_alloc.allocate(sb->journal_block_size); array_zero(result, sb->journal_block_size); return result; }
void jbd2::free_buffers(std::vector<disk_block> &bufs) { for(std::vector<disk_block>::iterator i = bufs.begin(); i < bufs.end(); i++) bl_alloc.deallocate(i->data_buffer, sb->journal_block_size); bufs.clear(); }
bool jbd2::write_block(disk_block const &bl) { if(!parent_fs->write_to_disk(bl)) { panic("disk write failed"); sb->journal_errno = __be32(EIO); return false; } return true; }  // TODO: better error handling :)
off_t jbd2::desc_tag_create(disk_block const& bl, void* where, uint32_t seq, bool is_first, bool is_last)
{
    off_t result = static_cast<off_t>(desc_tag_size(true));
    uint32_t fl = (is_first ? same_uuid : 0) | (is_last ? last_block : 0) | ((((reinterpret_cast<__be32 const*>(bl.data_buffer)[0])) == jbd2_magic) ? escape : 0);
    __be32 be_seq(seq);
    uint32_t csum = blk_crc32(bl, sb->journal_block_size, &be_seq, &(sb->uuid));
    if(sb->required_features & csum_v3) { new (static_cast<jbd2_block_tag3*>(where)) jbd2_block_tag3{ .block_number = __be32((bl.block_number) & 0xFFFFFFFF), .flags = __be32(fl), .block_number_hi = __be32((bl.block_number >> 32) & 0xFFFFFFFF), .checksum = __be32(csum) }; }
    else { new (static_cast<jbd2_block_tag*>(where)) jbd2_block_tag{ .block_number = __be32((bl.block_number) & 0xFFFFFFFF), .checksum = __be16(uint16_t(csum & 0xFFFF)), .flags = __be16(fl), .block_number_hi = __be32(sb->required_features & x64_support ? ((bl.block_number >> 32) & 0xFFFFFFFF) : 0) }; }
    if(is_first) { uint8_t* uuid_pos = reinterpret_cast<uint8_t*>(where) + result; result += 16; arraycopy(uuid_pos, sb->uuid.data_bytes, 16); }
    return result;
}
bool jbd2::create_txn(ext_vnode *changed_node)
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
    size_t n = 0, total = 0;
    char* pos = this->__get_ptr(txn_st_block * bs);
    for(size_t i = 0; i < txn_blocks.size(); i++)
    {
        disk_block const& b = txn_blocks[i];
        size_t j = 0;
        while(j < b.chain_len)
        {
            char* cur_tgt = pos + bs;
            array_zero(pos, bs);
            new (reinterpret_cast<jbd2_header*>(pos)) jbd2_header{ .blocktype = __be32(descriptor), .sequence = __be32(n++) };
            pos += sizeof(jbd2_header);
            total++;
            for(size_t k = j; k < tpb && j < b.chain_len; k++, j++, total++)
            {
                char* bl_pos = b.data_buffer + bs * j;
                disk_block sub{ b.block_number + j, bl_pos, false, 1U };
                pos += desc_tag_create(sub, pos, n, !k, (k + 1 == tpb) || (j + 1 == b.chain_len));
                bool esc = (reinterpret_cast<__be32 const*>(bl_pos)[0] == jbd2_magic);
                if(esc) reinterpret_cast<uint32_t*>(bl_pos)[0] = 0;
                arraycopy(cur_tgt, sub.data_buffer, bs);
                cur_tgt += bs;
                total++;
                if(esc) reinterpret_cast<__be32*>(bl_pos)[0] = jbd2_magic;
            }
            pos = cur_tgt;
        }
    }
    array_zero(pos, bs);
    disk_block ch_block{ txn_st_block + total++, pos, false, 1U };
    uint64_t timestamp = syscall_time(0);
    jbd2_commit_header* ch = new(reinterpret_cast<jbd2_commit_header*>(pos)) jbd2_commit_header
    { 
        .checksum_type = 4,
        .checksum_size = 4,
        .commit_seconds = __be64(timestamp / 1000UL),
        .commit_nanos = __be32((timestamp % 1000U) * 1000U)
    };
    uint32_t csum = blk_crc32(ch_block, bs, std::addressof(sb->uuid));
    BARRIER;
    ch->checksum[0] = __be32(csum);
    first_open_block = txn_st_block + total;
    active_transactions.put_txn(std::vector<disk_block>(txn_blocks), jbd2_commit_header(*ch));
    for(std::vector<disk_block>::iterator i = block_data.begin(); i != block_data.end(); i++) { if(!write_block(*i)) return false; }
    return true;
}
bool jbd2::clear_log()
{
    size_t bs = parent_fs->block_size();
    for(std::vector<disk_block>::iterator i = block_data.begin(); i != block_data.end(); i++) { if(i->data_buffer) { array_zero(i->data_buffer, bs * i->chain_len); if(!write_block(*i)) return false; } }
    for(std::vector<disk_block>::iterator i = replay_blocks.begin(); i != replay_blocks.end(); i++) { if(i->data_buffer) { bl_alloc.deallocate(i->data_buffer, bs); } }
    replay_blocks.clear();
    return true;
}
bool jbd2::execute_pending_txns()
{
    try
    {
        while(!active_transactions.at_end()) 
        { 
            jbd2_transaction* t = std::addressof(active_transactions.pop());
            if(!t->execute_and_complete(parent_fs)) 
            {
                active_transactions.restart(); 
                sb->journal_errno = __be32(EIO); 
                return false; 
            } 
        } 
        return clear_log();
    }
    catch(std::exception& e) { panic(e.what()); }
    return false;
}
bool jbd2::read_log()
{
    for(std::vector<disk_block>::iterator i = block_data.begin(); i != block_data.end(); i++) { if(!parent_fs->read_from_disk(*i)) { panic("journal block read failed"); return false; } }
    bool success = true;
    try { for(std::vector<disk_block>::iterator i = block_data.begin(); i != block_data.end(); i++) { parse_next_log_entry(i); } }
    catch(std::exception& e) { panic(e.what()); success = false; sb->journal_errno = __be32(EIO); }
    return success;
}
void jbd2::parse_next_log_entry(std::vector<disk_block>::iterator& i)
{
    size_t bs = parent_fs->block_size();
    size_t j = 0;    
    while(i != block_data.end() && j < i->chain_len)
    {
        char* pos = i->data_buffer + bs * j;
        jbd2_header* h = reinterpret_cast<jbd2_header*>(pos);
        if(h->magic == jbd2_magic)
        {
            uint32_t type = h->blocktype;
            if(type == descriptor)
            {
                size_t tpb = tags_per_block();
                std::vector<disk_block> txn_data_blocks{};
                bool have_commit = false;
                bool inval = false;
                jbd2_commit_header* ch = nullptr;
                while(!have_commit)
                {
                    if(h->magic == jbd2_magic)
                    {
                        if((have_commit = (h->blocktype == commit))) ch = reinterpret_cast<jbd2_commit_header*>(pos);
                        else if(!inval)
                        {
                            char* desc_pos = pos;
                            pos += bs;
                            bool last_entry = false;
                            for(size_t k = 0; k < tpb && !last_entry; k++)
                            {
                                uint64_t disk_target_block;
                                bool esc = false;
                                bool is_same_uuid;
                                uint32_t stored_csum, block_csum;
                                if(sb->required_features & csum_v3)
                                {
                                    jbd2_block_tag3* tag = reinterpret_cast<jbd2_block_tag3*>(desc_pos);
                                    disk_target_block = qword(tag->block_number, tag->block_number_hi);
                                    last_entry = tag->flags & last_block;
                                    esc = tag->flags & escape;
                                    is_same_uuid = tag->flags & same_uuid;
                                    stored_csum = tag->checksum;
                                }
                                else
                                {
                                    jbd2_block_tag* tag = reinterpret_cast<jbd2_block_tag*>(desc_pos);
                                    disk_target_block = qword(tag->block_number, (sb->required_features & x64_support) ? tag->block_number_hi : 0U);
                                    last_entry = tag->flags & last_block;
                                    esc = tag->flags & escape;
                                    is_same_uuid = tag->flags & same_uuid;
                                    stored_csum = tag->checksum;
                                }
                                desc_pos += desc_tag_size(is_same_uuid);
                                disk_block& next = txn_data_blocks.emplace_back(disk_target_block, parent_fs->allocate_block_buffer(), true, 1U);
                                arraycopy(next.data_buffer, pos, bs);
                                pos += bs;
                                block_csum = blk_crc32(next, bs, std::addressof(sb->uuid), std::addressof(h->sequence));
                                if(!(sb->required_features & csum_v3)) block_csum &= 0xFFFF; // only store 16 bits for v2 checksum
                                if(block_csum != stored_csum) { klog("(WARN) ignoring invalid descriptor block"); free_buffers(txn_data_blocks); inval = true; continue; }
                                if(esc) { reinterpret_cast<__be32*>(next.data_buffer)[0] = jbd2_magic; }
                            }
                        }
                    }
                    if(++j == i->chain_len) { i++; j = 0; if(i == block_data.end()) break; pos = i->data_buffer; }
                }
                if(!inval) { replay_blocks.push_back(txn_data_blocks.begin(), txn_data_blocks.end()); active_transactions.put_txn(std::move(txn_data_blocks), jbd2_commit_header(*ch)); }
            }
            else if(h->blocktype == revocation)
            {
                jbd2_revoke_header* rh = reinterpret_cast<jbd2_revoke_header*>(pos);
                size_t bytes = rh->block_bytes_used;
                bool inval = false;
                if((sb->required_features & csum_v2) || (sb->required_features & csum_v3)) 
                {
                    bytes -= 4;
                    jbd2_block_tail* tail = reinterpret_cast<jbd2_block_tail*>(pos + bytes);
                    uint32_t og_checksum = tail->block_checksum;
                    tail->block_checksum = 0;
                    uint32_t calculated = blk_crc32(*i, bs, std::addressof(sb->uuid));
                    tail->block_checksum = og_checksum;
                    inval = (og_checksum != calculated);
                }
                if(!inval)
                {
                    if(sb->required_features & x64_support)
                    {
                        uint64_t* block_nums = reinterpret_cast<uint64_t*>(pos + sizeof(jbd2_revoke_header));
                        size_t num_blocks = bytes / sizeof(uint64_t);
                        for(jbd2_transaction_queue::iterator i = active_transactions.begin(); i != active_transactions.end(); i++) { for(std::vector<disk_block>::iterator d =  i->data_blocks.begin(); d != i->data_blocks.end(); d++) { for(size_t k = 0; k < num_blocks; k++) { if(d->block_number == block_nums[k]) { d->block_number = 0UL; break; } } } }
                    }
                    else
                    {
                        uint32_t* block_nums = reinterpret_cast<uint32_t*>(pos + sizeof(jbd2_revoke_header));
                        size_t num_blocks = bytes / sizeof(uint32_t);
                        for(jbd2_transaction_queue::iterator i = active_transactions.begin(); i != active_transactions.end(); i++) { for(std::vector<disk_block>::iterator d =  i->data_blocks.begin(); d != i->data_blocks.end(); d++) { for(size_t k = 0; k < num_blocks; k++) { if(d->block_number == block_nums[k]) { d->block_number = 0UL; break; } } } }
                    }
                }
                else { klog("(WARN) ignoring invalid revocation block"); }
                j++;
            }
        }
    }
}
uint32_t jbd2::calculate_sb_checksum()
{
    uint32_t sb_cs_val = sb->checksum;
    sb->checksum = __be32(0U);
    uint32_t result = crc32c(sb);
    sb->checksum = __be32(sb_cs_val);
    return result;
}
bool jbd2::initialize()
{
    if(!init_extents()) return false;
    size_t s_req = extents.total_extent * parent_fs->block_size();
    if(!this->__grow_buffer(s_req)) { panic("failed to allocate buffer");  return false; }
    else { this->__setc(this->__max()); }
    uint64_t d_bl = block_data[0].block_number;
    disk_block tmp_db{ d_bl, parent_fs->allocate_block_buffer() };
    if(!parent_fs->read_from_disk(tmp_db)) { panic("failed to read superblock"); return false; }
    sb = sb_alloc.allocate(1);
    new (sb) jbd2_superblock(*reinterpret_cast<jbd2_superblock*>(tmp_db.data_buffer));
    parent_fs->free_block_buffer(tmp_db);
    this->update_block_ptrs();
    return true;
}