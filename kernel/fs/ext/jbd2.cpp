#include "fs/ext.hpp"
#include "sys/errno.h"
jbd2_transaction_queue::reference jbd2_transaction_queue::put_txn(std::vector<disk_block>&& blocks, jbd2_commit_header&& h) { return this->emplace(static_cast<transaction_id>(this->size()), std::move(blocks), std::move(h)); }
bool jbd2_transaction::execute_and_complete(extfs *fs_ptr) { for(std::vector<disk_block>::iterator i = data_blocks.begin(); i != data_blocks.end(); i++) { if(!fs_ptr->write_to_disk(*i)) { panic("write failed"); return false; } } return true; }
bool jbd2::need_escape(disk_block const &bl) { return ((__be32(reinterpret_cast<uint32_t const*>(bl.data_buffer)[0])) == jbd2_magic); }
size_t jbd2::desc_tag_size(bool same_uuid) { return (sb->required_features & csum_v3 ? 16 : (sb->required_features & x64_support ? 12 : 8)) + (same_uuid ? 0 : 16); }
size_t jbd2::tags_per_block() { return (sb->journal_block_size - sizeof(jbd2_header) - desc_tag_size(false) - (sb->required_features & (csum_v2 | csum_v3) ? 4 : 0)) / desc_tag_size(true); }
char *jbd2::allocate_block_buffer() { char* result = std::allocator<char>{}.allocate(sb->journal_block_size); array_zero(result, sb->journal_block_size); return result; }
void jbd2::free_buffers(std::vector<disk_block> &bufs) { for(std::vector<disk_block>::iterator i = bufs.begin(); i < bufs.end(); i++) std::allocator<char>{}.deallocate(i->data_buffer, sb->journal_block_size); bufs.clear(); }
off_t jbd2::desc_tag_create(disk_block const& bl, void* where, uint32_t seq, bool is_first, bool is_last)
{
    off_t result = static_cast<off_t>(desc_tag_size(true)) * bl.chain_len;
    uint32_t fl = (is_first ? same_uuid : 0) | (is_last ? last_block : 0) | (((__be32(reinterpret_cast<uint32_t const*>(bl.data_buffer)[0])) == jbd2_magic) ? escape : 0);
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
    disk_block tb{ first_open_block++, allocate_block_buffer(), false, 1U };
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
            if(!parent_fs->write_to_disk(db)) { panic("disk write failed"); sb->journal_errno = __be32(EIO); if(esc) { *reinterpret_cast<uint32_t*>(db.data_buffer) = jbd2_magic; } return false; }
            if(esc) { *reinterpret_cast<uint32_t*>(db.data_buffer) = jbd2_magic; }
        }
        k = (k + 1) % tpb;
        if(!k || i + 1 == txn_blocks.end())
        {
            if(!parent_fs->write_to_disk(tb)) { panic("disk write failed"); sb->journal_errno = __be32(EIO); return false; }
            tb.block_number = j++;
            first_open_block = j;
        }
    }
    array_zero(tb.data_buffer, sb->journal_block_size);
    jbd2_commit_header* ch = new(reinterpret_cast<jbd2_commit_header*>(tb.data_buffer)) jbd2_commit_header{};
    ch->checksum_size = 4;
    ch->checksum_type = 4; // crc32c
    uint64_t timestamp = syscall_time(0);
    ch->commit_nanos = __be32((timestamp % 1000U) * 1000U);
    ch->commit_seconds = __be64(timestamp / 1000UL);
    uint32_t csum = blk_crc32(tb, sb->journal_block_size, &(sb->uuid));
    BARRIER; // can never be too safe with this
    ch->checksum[0] = __be32(csum);
    active_transactions.put_txn(std::vector<disk_block>(txn_blocks), std::move(*ch));
    if(!parent_fs->write_to_disk(tb)) { panic("disk write failed"); sb->journal_errno = __be32(EIO); return false; }
    std::allocator<char>{}.deallocate(tb.data_buffer, sb->journal_block_size);
    return true;
}
bool jbd2::clear_log()
{
    disk_block zb{ sb->start_block, allocate_block_buffer(), false, 1 };
    array_zero(zb.data_buffer, sb->journal_block_size);
    for(size_t i = 0; i < sb->journal_block_count; i++, zb.block_number++) { if(!parent_fs->write_to_disk(zb)) { panic("write failed"); sb->journal_errno = __be32(EIO); return false; } } // TODO: better error handling :)
    std::allocator<char>{}.deallocate(zb.data_buffer, sb->journal_block_size);
    for(std::vector<disk_block>::iterator i = replay_blocks.begin(); i != replay_blocks.end(); i++) { if(i->data_buffer) { std::allocator<char>{}.deallocate(i->data_buffer, sb->journal_block_size); } }
    replay_blocks.clear();
    return true;
}
bool jbd2::execute_pending_txns()
{
    try
    {
        while(!active_transactions.at_end()) 
        { 
            if(!active_transactions.pop().execute_and_complete(parent_fs)) 
            { 
                // TODO: better error handling :)
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
    char* blk_buffer = allocate_block_buffer();
    disk_block blk{ sb->start_block, blk_buffer, false, 1 };
    bool success = true;
    try { while(blk.block_number < sb->start_block + sb->journal_block_count) parse_next_log_entry(blk); }
    catch(std::exception& e) { panic(e.what()); success = false; }
    std::allocator<char>{}.deallocate(blk_buffer, sb->journal_block_size);
    return success;
}
void jbd2::parse_next_log_entry(disk_block& blk)
{
    if(!parent_fs->read_from_disk(blk)) throw std::runtime_error{ "disk read failed on journal block" };
    jbd2_header* h = reinterpret_cast<jbd2_header*>(blk.data_buffer);
    if(h->magic != jbd2_magic) { blk.block_number++; }
    else if(h->blocktype == descriptor)
    {
        size_t tpb = tags_per_block();
        std::vector<disk_block> txn_data_blocks{};
        bool have_commit = false;
        bool inval = false;
        jbd2_commit_header ch;
        while(!have_commit)
        {
            if(h->magic != jbd2_magic) { blk.block_number++; continue; }
            if((have_commit = (h->blocktype == commit))) new(&ch) jbd2_commit_header(*reinterpret_cast<jbd2_commit_header const*>(blk.data_buffer));
            else if(!inval)
            {
                char* current_pos = blk.data_buffer + sizeof(jbd2_header);
                uint32_t dbnum = blk.block_number + 1;
                bool last_entry = false;
                for(size_t i = 0; i < tpb && !last_entry; i++)
                {
                    uint64_t target_block;
                    bool esc = false;
                    bool s_uuid;
                    uint32_t stored_csum, block_csum;
                    if(sb->required_features & csum_v3)
                    {
                        jbd2_block_tag3* tag = reinterpret_cast<jbd2_block_tag3*>(current_pos);
                        target_block = qword(tag->block_number, tag->block_number_hi);
                        last_entry = tag->flags & last_block;
                        esc = tag->flags & escape;
                        s_uuid = tag->flags & same_uuid;
                        stored_csum = tag->checksum;
                    }
                    else
                    {
                        jbd2_block_tag* tag = reinterpret_cast<jbd2_block_tag*>(current_pos);
                        target_block = qword(tag->block_number, (sb->required_features & x64_support) ? tag->block_number_hi : 0U);
                        last_entry = tag->flags & last_block;
                        esc = tag->flags & escape;
                        s_uuid = tag->flags & same_uuid;
                        stored_csum = tag->checksum;
                    }
                    current_pos += desc_tag_size(s_uuid);
                    disk_block& next = txn_data_blocks.emplace_back(target_block, allocate_block_buffer(), true, 1U);
                    disk_block read_tar{ dbnum++, next.data_buffer, false, 1 };
                    if(!parent_fs->read_from_disk(read_tar)) { free_buffers(txn_data_blocks); throw std::runtime_error{ "disk read failed on data block" }; }
                    block_csum = blk_crc32(read_tar, sb->journal_block_size, &(sb->uuid), &(h->sequence));
                    if(!(sb->required_features & csum_v3)) block_csum &= 0xFFFF; // if the v3 flag is not set, the stored hecksum is only the low-order 16 bits
                    if(block_csum != stored_csum) { xdirect_writeln("W: checksum of " + std::to_string(block_csum) + "did not match stored value of " + std::to_string(stored_csum)); free_buffers(txn_data_blocks); inval = true; continue; }
                    if(esc) { *reinterpret_cast<uint32_t*>(read_tar.data_buffer) = jbd2_magic; }
                }
                blk.block_number = dbnum;
                if(!parent_fs->read_from_disk(blk)) { free_buffers(txn_data_blocks); throw std::runtime_error{ "disk read failed on journal block" }; }
                h = reinterpret_cast<jbd2_header*>(blk.data_buffer);
            }
        }
        if(!inval)
        {
            for(std::vector<disk_block>::iterator i = txn_data_blocks.begin(); i != txn_data_blocks.end(); i++) { replay_blocks.push_back(*i); }
            active_transactions.put_txn(std::move(txn_data_blocks), std::move(ch));
        }
        blk.block_number++;
    }
    // TODO handle revocation blocks
}
uint32_t jbd2::calculate_sb_checksum()
{
    uint32_t sb_cs_val = sb->checksum;
    sb->checksum = __be32(0U);
    uint32_t result = crc32c(sb);
    sb->checksum = __be32(sb_cs_val);
    return result;
}