#include <fs/ext.hpp>
#include <sys/errno.h>
#include <unordered_set>
typedef decltype([](qword const& qw) -> uint64_t const& { return *reinterpret_cast<uint64_t const*>(&qw); }) reint_qword;
typedef std::hash_set<qword, uint64_t, std::hash<uint64_t>, std::equal_to<void>, std::allocator<qword>, reint_qword> blocknum_set;
jbd2::jbd2() = default;
jbd2::~jbd2() = default;
jbd2::jbd2(extfs* parent, uint32_t inode) : ext_vnode(parent, inode) {}
bool jbd2::need_escape(disk_block const& bl) { return (((reinterpret_cast<__be32 const*>(bl.data_buffer)[0])) == jbd2_magic); }
size_t jbd2::desc_tag_size(bool same_uuid) { return (sb->required_features & csum_v3 ? 16 : (sb->required_features & x64_support ? 12 : 8)) + (same_uuid ? 0 : 16); }
size_t jbd2::tags_per_block() { return 1UZ + (parent_fs->block_size() - sizeof(jbd2_header) - desc_tag_size(false) - (sb->required_features & (csum_v2 | csum_v3) ? 4 : 0)) / desc_tag_size(true); }
bool jbd2_transaction::execute_and_complete(extfs* fs_ptr)
{
	for(disk_block& db : data_blocks)
	{
		if(!db.block_number || !db.data_buffer)
			continue;
		if(!fs_ptr->write_block(db))
			return false;
	}
	return true;
}
off_t jbd2::desc_tag_create(disk_block const& bl, void* where, uint32_t seq, bool is_first, bool is_last)
{
	off_t result	= static_cast<off_t>(desc_tag_size(true));
	uint32_t fl		= (!is_first ? same_uuid : 0U) | (is_last ? last_block : 0U) | (need_escape(bl) ? escape : 0U);
	uint32_t csum	= crc32c(uuid_checksum, seq);
	csum			= crc32c_blk(csum, bl, parent_fs->block_size());
	if(sb->required_features & csum_v3) { std::construct_at<jbd2_block_tag3>(static_cast<jbd2_block_tag3*>(where), __be32((bl.block_number) & 0xFFFFFFFFU), __be32(fl), __be32((bl.block_number & 0xFFFFFFFF00000000) >> 32), __be32(csum)); }
	else { std::construct_at<jbd2_block_tag>(static_cast<jbd2_block_tag*>(where), __be32((bl.block_number) & 0xFFFFFFFFU), __be16(static_cast<uint16_t>(csum & 0xFFFF)), __be16(fl), __be32(sb->required_features & x64_support ? (bl.block_number & 0xFFFFFFFF00000000) >> 32 : 0)); }
	if(is_first)
	{
		uint8_t* uuid_pos	= static_cast<uint8_t*>(where) + result;
		result				+= 16UZ;
		array_copy(uuid_pos, sb->uuid.data_bytes, sizeof(guid_t));
	}
	return result;
}
bool jbd2::create_txn(ext_vnode* changed_node)
{
	std::vector<disk_block> dirty_blocks{};
	for(std::vector<disk_block>::iterator i	= changed_node->block_data.begin(); i != changed_node->block_data.end(); i++) {
		if(i->dirty && i->data_buffer) {
			i->dirty = false;
			dirty_blocks.push_back(*i);
		}
	}
	for(std::vector<disk_block>::iterator i	= changed_node->cached_metadata.begin(); i != changed_node->cached_metadata.end(); i++) {
		if(i->dirty) {
			i->dirty = false;
			dirty_blocks.push_back(*i);
		}
	}
	if(dirty_blocks.empty()) return true; // vacuous success; nothing to do
	return create_txn(dirty_blocks);
}
bool jbd2::create_txn(std::vector<disk_block> const& txn_blocks)
{
	if(txn_blocks.empty()) return true; // vacuous success; nothing to do
	size_t tpb				= tags_per_block();
	size_t bs				= parent_fs->block_size();
	uint64_t txn_st_block	= first_open_block + sb->start_block;
	std::vector<disk_block> actual_blocks{};
	for(disk_block const& b : txn_blocks)
	{
		if(b.chain_len > 1UZ)
			for(size_t i = 0UZ; i < b.chain_len; i++)
				actual_blocks.emplace_back(b.block_number + i, b.data_buffer + i * bs, b.dirty, 1UL);
		else actual_blocks.push_back(b);
	}
	unsigned seq	= static_cast<unsigned>(active_transactions.size());
	char* pos		= __cur();
	char* dblk_tar	= pos + bs;
	std::construct_at<jbd2_header>(reinterpret_cast<jbd2_header*>(pos), jbd2_magic, __be32(descriptor), __be32(seq));
	pos				+= sizeof(jbd2_header);
	mark_write(pos);
	size_t total	= 0UZ, k = tpb;
	bool first		= true;
	for(size_t i	= 0UZ; i < actual_blocks.size(); i++, total++)
	{
		pos								+= desc_tag_create(actual_blocks[i], pos, seq, first, k == 1UZ || !(i + 1UZ < actual_blocks.size()));
		first							= false;
		array_copy(dblk_tar, actual_blocks[i].data_buffer, bs);
		mark_write(dblk_tar);
		actual_blocks[i].data_buffer	= dblk_tar;
		dblk_tar						+= bs;
		if(!(--k))
		{
			pos			= __cur() + (bs - 4Z);
			if(sb->required_features & (csum_v2 | csum_v3)) std::construct_at(reinterpret_cast<__be32*>(pos), crc32c(uuid_checksum, __cur(), bs));
			__setc(dblk_tar);
			dblk_tar	+= bs;
			pos			= __cur();
			std::construct_at<jbd2_header>(reinterpret_cast<jbd2_header*>(pos), jbd2_magic, __be32(descriptor), __be32(seq));
			pos			+= sizeof(jbd2_header);
			mark_write(pos);
			total++;
			first		= true;
		}
	}
	pos					= __cur() + (bs - 4);
	if(sb->required_features & (csum_v2 | csum_v3)) std::construct_at(reinterpret_cast<__be32*>(pos), crc32c(uuid_checksum, __cur(), bs));
	total++;
	__setc(dblk_tar);
	disk_block ch_block(txn_st_block + total++, dblk_tar, false, 1U);
	uint64_t timestamp		= sys_time(nullptr);
	jbd2_commit_header* ch	= new(dblk_tar) jbd2_commit_header
	{
		.checksum_type	= 4UC,
		.checksum_size	= 4UC,
		.commit_seconds	= __be64(timestamp / 1000UL),
		.commit_nanos	= __be32((timestamp % 1000U) * 1000U)
	};
	mark_write(dblk_tar);
	uint32_t csum		= crc32c_blk(uuid_checksum, ch_block, bs);
	barrier();
	ch->checksum[0]		= csum;
	first_open_block	= txn_st_block + total;
	__bumpc(bs);
	if(!ddwrite()) return false;
	barrier();
	active_transactions.emplace(actual_blocks, static_cast<int>(seq));
	return true;
}
bool jbd2::ddread()
{
	disk_block full_blk(block_data[0].block_number, __beg(), false, extents.total_extent);
	if(!parent_fs->read_block(full_blk)) { panic("[FS/EXT4/JBD2] failed to read journal"); return false; }
	return true;
}
bool jbd2::ddwrite()
{
	disk_block full_blk(block_data[0].block_number, __beg(), false, extents.total_extent);
	if(!parent_fs->write_block(full_blk)) { panic("[FS/EXT4/JBD2] failed to write journal"); return false; }
	return true;
}
bool jbd2::clear_log()
{
	size_t bs	= parent_fs->block_size();
	array_zero(reinterpret_cast<uint64_t*>(__beg() + bs), static_cast<size_t>(__capacity() - bs) / sizeof(uint64_t));
	replay_blocks.clear();
	__rst();
	log_seq		= 0;
	return ddwrite();
}
bool jbd2::execute_pending_txns() try
{
	while(!active_transactions.at_end())
	{
		jbd2_transaction* t = std::addressof(active_transactions.pop());
		if(!t->execute_and_complete(parent_fs))
		{
			panic("[FS/EXT4/JBD2] write failed");
			active_transactions.restart();
			return false;
		}
	}
	return clear_log();
}
catch(std::exception& e) {
	panic(e.what());
	return false;
}
uint32_t jbd2::calculate_sb_checksum()
{
	uint32_t sb_cs_val	= sb->checksum;
	sb->checksum		= 0UBE;
	uint32_t result		= crc32c(*sb);
	sb->checksum		= sb_cs_val;
	return result;
}
bool jbd2::on_open()
{
	if(has_init) return true;
	if(!init_extents()) return false;
	size_t bs			= parent_fs->block_size();
	size_t s_req		= extents.total_extent * bs;
	if(!__grow_buffer_exact(s_req)) { panic("[FS/EXT4/JBD2] failed to allocate buffer"); return false; }
	update_block_ptrs();
	if(!ddread()) return false;
	__bumpc(bs);
	sb						= reinterpret_cast<jbd2_superblock*>(block_data[0].data_buffer);
	if(sb->header.magic		!= jbd2_magic)
	{
		uintptr_t num		= sb->header.magic;
		std::string errstr	= "[FS/EXT4/JBD2] superblock invalid; expected magic number of 0x99B3030C but found magic number of " + std::to_string(reinterpret_cast<void*>(num));
		panic(errstr.c_str());
		return false;
	}
	uuid_checksum		= crc32c(sb->uuid);
	return (has_init	= read_log());
}
log_read_state jbd2::read_next_log_entry()
{
	char* entry_start	= __cur();
	jbd2_header* h		= reinterpret_cast<jbd2_header*>(entry_start);
	if(h->magic == 0 && h->blocktype == 0 && h->sequence == 0) return NREM;
	else if(h->magic != jbd2_magic) return SKIP;
	uint32_t type		= h->blocktype;
	try
	{
		if(type == revocation) { parse_revocation(); return VALID; }
		else if(type == descriptor) return read_log_transaction();
		else return SKIP;
	}
	catch(std::exception& e)
	{
		panic(e.what());
		__rst();
		return ERROR;
	}
}
void jbd2::parse_revocation()
{
	blocknum_set block_nums(64UZ);
	char* entry_start		= __cur();
	jbd2_revoke_header* rh	= reinterpret_cast<jbd2_revoke_header*>(entry_start);
	__be64* start			= reinterpret_cast<__be64*>(entry_start + sizeof(jbd2_revoke_header));
	__be64* end				= reinterpret_cast<__be64*>(entry_start + rh->block_bytes_used);
	block_nums.insert(start, end);
	for(jbd2_transaction& t : active_transactions)
	{
		std::vector<disk_block>::iterator i = t.data_blocks.begin();
		while(i		!= t.data_blocks.end())
		{
			if(block_nums.contains(i->block_number))
				i	= t.data_blocks.erase(i);
			else i++;
		}
	}
}
log_read_state jbd2::read_log_transaction()
{
	char* start				= __cur();
	char* pos				= start;
	char* end				= __max();
	size_t bs				= parent_fs->block_size();
	jbd2_commit_header* ch	= nullptr;
	while(!ch)
	{
		if(pos < end)
		{
			jbd2_header* bh	= reinterpret_cast<jbd2_header*>(pos);
			if(bh->magic == jbd2_magic && bh->blocktype == commit) { ch = reinterpret_cast<jbd2_commit_header*>(bh); }
			else { pos += bs; }
		}
		else return NREM;
	}
	std::vector<disk_block> txn_blocks{};
	transaction_id id			= static_cast<transaction_id>(log_seq);
	end							= reinterpret_cast<char*>(ch);
	char* block_st				= start;
	char* block_ed;
	uint32_t cb_csum_checkval	= ch->checksum[0];
	ch->checksum[0]				= 0UBE;
	uint32_t csum_base			= crc32c(uuid_checksum, ch->header.sequence);
	uint32_t cb_csum			= crc32c(csum_base, end, bs);
	ch->checksum[0]				= cb_csum_checkval;
	if(cb_csum != cb_csum_checkval) goto skip_txn;
	do {
		jbd2_header* h			= reinterpret_cast<jbd2_header*>(block_st);
		csum_base				= crc32c(uuid_checksum, h->sequence);
		if(h->magic != jbd2_magic) return SKIP;
		if(h->blocktype == revocation) parse_revocation();
		else
		{
			pos					= block_st + sizeof(jbd2_header);
			block_ed			= block_st + bs;
			__be32* db_csum_pos	= reinterpret_cast<__be32*>(block_ed - sizeof(__be32));
		   uint32_t checkval	= *db_csum_pos;
		   *db_csum_pos			= 0UBE;
		   uint32_t db_csum		= crc32c(csum_base, block_st, bs);
		   *db_csum_pos			= checkval;
			if(db_csum != checkval) goto skip_txn;
			while(pos < block_ed)
			{
				if(sb->required_features & csum_v3)
				{
					jbd2_block_tag3* tag	= reinterpret_cast<jbd2_block_tag3*>(pos);
					qword blocknum(tag->block_number, sb->required_features & x64_support ? tag->block_number_hi : 0UBE);
					__bumpc(bs);
					txn_blocks.emplace_back(blocknum, __cur(), true, 1);
					uint32_t csum			= crc32c(csum_base, __cur(), bs);
					uint32_t checkval		= tag->checksum;
					if(csum != checkval) goto skip_txn;
					if(tag->flags & last_block) pos = block_ed;
					else pos += desc_tag_size(tag->flags & same_uuid);
				}
				else
				{
					jbd2_block_tag* tag 	= reinterpret_cast<jbd2_block_tag*>(pos);
					qword blocknum(tag->block_number, sb->required_features & x64_support ? tag->block_number_hi : 0UBE);
					__bumpc(bs);
					uint32_t csum 			= crc32c(csum_base, __cur(), bs);
					uint16_t checkval		= tag->checksum;
					if(dword(csum).lo != checkval) goto skip_txn;
					txn_blocks.emplace_back(blocknum, __cur(), true, 1);
					if(tag->flags & last_block)
						pos					= block_ed;
					else pos				+= desc_tag_size(tag->flags & same_uuid);
				}
			}
		}
		__bumpc(bs);
		block_st							= __cur();
	} while(block_st < end);
	active_transactions.emplace(std::move(txn_blocks), id);
	return VALID;
skip_txn:
	klog("[FS/EXT4/JBD2] W: skipping corrupted transaction");
	__setc(end);
	return SKIP;
}
bool jbd2::read_log()
{
	char* end	= __max();
	size_t bs	= parent_fs->block_size();
	while(__cur() < end)
	{
		log_read_state state = read_log_transaction();
		switch(state)
		{
		case NREM:
			return true;
		case ERROR:
			return false;
		default:
			__bumpc(bs);
			log_seq++;
			break;
		}
	}
	if(active_transactions.empty()) return true;
	return execute_pending_txns();
}