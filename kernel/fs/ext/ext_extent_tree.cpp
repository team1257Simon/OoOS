#include "fs/ext.hpp"
typedef std::map<uint64_t, off_t>::iterator offset_iterator;
ext_node_extent_tree::ext_node_extent_tree(ext_vnode* tracked) : tracked_node{ tracked } {}
ext_node_extent_tree::ext_node_extent_tree()    = default;
ext_node_extent_tree::~ext_node_extent_tree()   = default;
off_t ext_node_extent_tree::cached_node_pos(cached_extent_node const* n) { return n - tracked_extents.begin().base(); }
off_t ext_node_extent_tree::cached_node_pos(cached_extent_node const& n) { return cached_node_pos(std::addressof(n)); }
cached_extent_node* ext_node_extent_tree::get_cached(off_t which) { return (tracked_extents.begin() + which).base(); }
cached_extent_node::cached_extent_node(disk_block* bptr, ext_vnode* node, uint16_t d) : blk_offset{ bptr - (d ? node->cached_metadata.data() : node->block_data.data()) }, tracked_node{ node }, depth{ d }, next_level_extents{} {}
disk_block* cached_extent_node::block() { return ((depth ? tracked_node->cached_metadata.begin() : tracked_node->block_data.begin()) + blk_offset).base(); }
static void populate_leaf(ext_extent_leaf& leaf, disk_block* blk, uint64_t fn_start)
{
	qword blknum(blk->block_number);
	leaf.extent_start_lo    = blknum.lo;
	leaf.extent_start_hi    = blknum.hi.lo;
	leaf.extent_size        = blk->chain_len;
	leaf.file_node_start    = fn_start;
}
static void populate_index(ext_extent_index& index, uint64_t next, uint64_t fn_start)
{
	qword blknum(next);
	index.next_level_block_lo   = blknum.lo;
	index.next_level_block_hi   = blknum.hi.lo;
	index.file_node_start       = fn_start;
}
bool ext_node_extent_tree::parse_legacy()
{
	if(has_init) return true;
	// blocks 0-11 are direct
	for(int i = 0; i < 12; i++)
	{
		uint64_t b			= tracked_node->on_disk_node->block_info.legacy_extent.direct_blocks[i];
		if(!b) return (has_init = true); // if the block is not present we're done
		disk_block* bptr	= tracked_node->block_data.insert(tracked_node->block_data.begin() + i, std::move(disk_block(b, nullptr))).base(); // the block data buffer will be allocated if and when it is needed
		tracked_extents.emplace_back(bptr, tracked_node, 0US);
		total_extent++;
	}
	uint64_t ind1					= tracked_node->on_disk_node->block_info.legacy_extent.singly_indirect_block;
	if(!ind1)
		return (has_init			= true); // no indirect block pointer means we're done
	base_depth++;
	disk_block* single_ptr_block	= std::addressof(tracked_node->cached_metadata.emplace_back(ind1, tracked_node->parent_fs->allocate_block_buffer(), false, 1U));
	if(!tracked_node->parent_fs->read_block(*single_ptr_block)) { panic("[FS/EXT4/EXTENT] read on single pointer block failed"); return false; }
	uint64_t cur_file_block    		= 12;
	cached_extent_node* base    	= std::addressof(tracked_extents.emplace_back(single_ptr_block, tracked_node, 1US));
	offset_iterator exnode			= base_extent_level.insert(std::move(std::make_pair(cur_file_block, cached_node_pos(base)))).first;
	try
	{
		cur_file_block					= base->nl_recurse_legacy(this, exnode->first);
		// the function will return 0 to indicate having reached the end of the file's extent; otherwise it will return the next file node. A failure will throw an exception
		if(!cur_file_block)
			return (has_init			= true);
		base_depth++;
		// if we get here, there are more blocks to parse in the doubly-indirect pointers
		uint64_t ind2                   = tracked_node->on_disk_node->block_info.legacy_extent.doubly_indirect_block;
		disk_block* di_pointer_block    = std::addressof(tracked_node->cached_metadata.emplace_back(ind2, tracked_node->parent_fs->allocate_block_buffer(), false, 1U));
		if(!tracked_node->parent_fs->read_block(*di_pointer_block)) { panic("[FS/EXT4/EXTENT] read on double pointer block failed"); return false; }
		base            = std::addressof(tracked_extents.emplace_back(di_pointer_block, tracked_node, 2US));
		exnode          = base_extent_level.insert(std::move(std::make_pair(cur_file_block, cached_node_pos(base)))).first;
		cur_file_block  = base->nl_recurse_legacy(this, exnode->first);
		if(!cur_file_block)
			return (has_init			= true);
		base_depth++;
		// if we made it all the way here, there are even more blocks, this time in triply-indirect pointers
		uint64_t ind3                   = tracked_node->on_disk_node->block_info.legacy_extent.triply_indirect_block;
		disk_block* tri_pointer_block   = std::addressof(tracked_node->cached_metadata.emplace_back(ind3, tracked_node->parent_fs->allocate_block_buffer(), false, 1U));
		base                            = std::addressof(tracked_extents.emplace_back(tri_pointer_block, tracked_node, 3US));
		exnode                          = base_extent_level.insert(std::move(std::make_pair(cur_file_block, cached_node_pos(base)))).first;
		if(!tracked_node->parent_fs->read_block(*tri_pointer_block)) { panic("[FS/EXT4/EXTENT] read on triple pointer block failed"); return false; }
		base->nl_recurse_legacy(this, exnode->first);
		return (has_init				= true);
	}
	catch(std::exception& e) { panic(e.what()); }
	return false;
}
bool ext_node_extent_tree::parse_ext4()
{
	if(has_init) return true;
	ext_extent_header* h	= std::addressof(tracked_node->on_disk_node->block_info.ext4_extent.header);
	if(h->magic != ext_extent_magic) { panic("[FS/EXT4/EXTENT] invalid extent tree header"); return false; }
	base_depth				= h->depth;
	if(!h->entries)
		return (has_init	= true); // no entries means we're done (empty file, or newly created)
	ext_extent_node* nodes  = tracked_node->on_disk_node->block_info.ext4_extent.root_nodes;
	size_t num              = std::min(size_t(h->entries), 4UL);
	uint64_t cur_file_block;
	for(size_t i = 0; i < num; i++)
	{
		if(base_depth)
		{
			cur_file_block  = nodes[i].idx.file_node_start;
			uint64_t blknum = qword(nodes[i].idx.next_level_block_lo, uint32_t(nodes[i].idx.next_level_block_hi));
			if(!blknum) continue;
			disk_block* blk	= std::addressof(tracked_node->cached_metadata.emplace_back(blknum, tracked_node->parent_fs->allocate_block_buffer(), false, 1U));
			if(!tracked_node->parent_fs->read_block(*blk)) { panic("[FS/EXT4/EXTENT] metadata block read failed"); return false; }
			cached_extent_node* base    = std::addressof(tracked_extents.emplace_back(blk, tracked_node, base_depth));
			offset_iterator exnode		= base_extent_level.insert(std::move(std::make_pair(cur_file_block, cached_node_pos(base)))).first;
			if(!base->nl_recurse_ext4(this, exnode->first)) return false;
		}
		else
		{
			cur_file_block  = nodes[i].leaf.file_node_start;
			size_t ext_sz   = nodes[i].leaf.extent_size % 0x8000US;
			uint64_t blknum = qword(nodes[i].leaf.extent_start_lo, uint32_t(nodes[i].leaf.extent_start_hi));
			tracked_node->block_data.reserve(tracked_node->block_data.size() + ext_sz);
			disk_block* blk	= tracked_node->block_data.insert(tracked_node->block_data.begin() + cur_file_block++, std::move(disk_block(blknum, nullptr, false, 1U))).base();
			for(size_t k = 1; k < ext_sz; k++) { tracked_node->block_data.insert(tracked_node->block_data.begin() + cur_file_block++, std::move(disk_block(blknum + k, nullptr, false, 1U))); }
			base_extent_level.insert_or_assign(cur_file_block, cached_node_pos(tracked_extents.emplace_back(blk, tracked_node, base_depth)));
			total_extent	+= ext_sz;
		}
	}
	return has_init			= true;
}
size_t cached_extent_node::nl_recurse_legacy(ext_node_extent_tree* parent, uint64_t start_file_block)
{
	if(!depth) return start_file_block + 1; // if this is a direct block, the file block for the next extent node is simply the subsequent one
	uint32_t* blk_ptrs      = reinterpret_cast<uint32_t*>(block()->data_buffer);
	size_t total_dwords     = tracked_node->parent_fs->block_size() / 4;
	uint64_t cur_file_block = start_file_block;
	for(size_t i = 0; i < total_dwords; i++, parent->total_extent++)
	{
		if(!blk_ptrs[i]) return 0; // if we hit an empty pointer we're done
		disk_block* blk = depth == 1 ? tracked_node->block_data.insert(tracked_node->block_data.begin() + cur_file_block, std::move(disk_block(blk_ptrs[i], nullptr, false, 1U))).base() : std::addressof(tracked_node->cached_metadata.emplace_back(blk_ptrs[i], tracked_node->parent_fs->allocate_block_buffer(), false, 1U));
		if(blk->data_buffer && !tracked_node->parent_fs->read_block(*blk)) { throw std::runtime_error("[FS/EXT4/EXTENT] failed to read disk block"); }
		cached_extent_node* base    = std::addressof(parent->tracked_extents.emplace_back(blk, tracked_node, static_cast<uint16_t>(depth - 1)));
		offset_iterator exnode		= next_level_extents.insert_or_assign(cur_file_block, parent->cached_node_pos(base)).first;
		cur_file_block              = base->nl_recurse_legacy(parent, exnode->first);
		if(!cur_file_block) return 0;
	}
	return cur_file_block;
}
bool cached_extent_node::nl_recurse_ext4(ext_node_extent_tree* parent, uint64_t start_file_block)
{
	if(!depth) return true; // nothing to do for leaf nodes; return vacuous success
	disk_block* my_block	= block();
	ext_extent_header* h	= reinterpret_cast<ext_extent_header*>(my_block->data_buffer);
	if(h->magic != ext_extent_magic) { panic("[FS/EXT4/EXTENT] invalid extent tree header"); return false; }
	size_t n_nodes          = h->entries;
	ext_extent_node* nodes  = reinterpret_cast<ext_extent_node*>(my_block->data_buffer + sizeof(ext_extent_header));
	for(size_t i = 0; i < n_nodes; i++)
	{
		uint16_t depth				= h->depth;
		if(depth)
		{
			uint64_t cur_file_block	= nodes[i].idx.file_node_start;
			uint64_t blknum         = qword(nodes[i].idx.next_level_block_lo, uint32_t(nodes[i].idx.next_level_block_hi));
			if(!blknum) continue;
			disk_block* blk = std::addressof(tracked_node->cached_metadata.emplace_back(blknum, tracked_node->parent_fs->allocate_block_buffer(), false, 1U));
			if(!tracked_node->parent_fs->read_block(*blk)) { panic("[FS/EXT4/EXTENT] metadata block read failed"); return false; }
			cached_extent_node* base    = std::addressof(parent->tracked_extents.emplace_back(blk, tracked_node, depth));
			offset_iterator exnode		= next_level_extents.insert(std::move(std::make_pair(cur_file_block, parent->cached_node_pos(base)))).first;
			if(!base->nl_recurse_ext4(parent, exnode->first)) return false;
		}
		else
		{
			uint64_t cur_file_block = nodes[i].leaf.file_node_start;
			size_t ext_sz           = nodes[i].leaf.extent_size % 0x8000;
			uint64_t blknum         = qword(nodes[i].leaf.extent_start_lo, static_cast<uint32_t>(nodes[i].leaf.extent_start_hi));
			disk_block* blk         = tracked_node->block_data.insert(tracked_node->block_data.begin() + cur_file_block++, std::move(disk_block{ blknum, nullptr, false, 1U })).base();
			for(size_t k = 1; k < ext_sz; k++) { tracked_node->block_data.insert(tracked_node->block_data.begin() + cur_file_block++, std::move(disk_block{ blknum + k, nullptr, false, 1U })); }
			next_level_extents.insert_or_assign(cur_file_block, parent->cached_node_pos(parent->tracked_extents.emplace_back(blk, tracked_node, depth)));
			parent->total_extent += ext_sz;
		}
	}
	return tracked_node->parent_fs->persist_inode(tracked_node->inode_number);
}
bool ext_node_extent_tree::push_extent_legacy(disk_block* blk)
{
	if(!has_init && !parse_legacy()) return false;
	for(int i = 0; i < 12; i++)
	{
		if(!tracked_node->on_disk_node->block_info.legacy_extent.direct_blocks[i])
		{
			tracked_node->on_disk_node->block_info.legacy_extent.direct_blocks[i] = static_cast<uint32_t>(blk->block_number);
			tracked_extents.emplace_back(blk, tracked_node, 0US);
			total_extent++;
			return tracked_node->parent_fs->persist_inode(tracked_node->inode_number);
		}
	}
	offset_iterator last	= base_extent_level.max();
	if(last == base_extent_level.end())
	{
		disk_block* nl_blk	= tracked_node->parent_fs->claim_metadata_block(this);
		if(nl_blk)
		{
			cached_extent_node* base	= std::addressof(tracked_extents.emplace_back(nl_blk, tracked_node, 1US));
			base_extent_level.insert_or_assign(total_extent, cached_node_pos(base));
			return base->push_extent_recurse_legacy(this, blk);
		}
		else return false;
	}
	else try
	{
		cached_extent_node* base		= get_cached(last->second);
		if(!base->push_extent_recurse_legacy(this, blk))
		{
			if(base->depth < 3)
			{
				uint16_t nd                     = base->depth + 1;
				if(nd > base_depth) base_depth  = nd;
				disk_block* nl_blk              = tracked_node->parent_fs->claim_metadata_block(this);
				if(nl_blk)
				{
					cached_extent_node* base = std::addressof(tracked_extents.emplace_back(nl_blk, tracked_node, nd));
					base_extent_level.insert_or_assign(total_extent, cached_node_pos(base));
					return base->push_extent_recurse_legacy(this, blk);
				}
			}
			return false;
		}
		else return true;
	}
	catch(std::exception& e) { panic(e.what()); }
	return false;
}
bool ext_node_extent_tree::push_extent_ext4(disk_block* blk)
{
	if(!has_init && !parse_ext4()) return false;
	if(tracked_node->on_disk_node->block_info.ext4_extent.header.depth == 0)
	{
		if(tracked_node->on_disk_node->block_info.ext4_extent.header.entries < tracked_node->on_disk_node->block_info.ext4_extent.header.max_entries) // should be 4
		{
			uint16_t idx	= tracked_node->on_disk_node->block_info.ext4_extent.header.entries++;
			populate_leaf(tracked_node->on_disk_node->block_info.ext4_extent.root_nodes[idx].leaf, blk, total_extent);
			base_extent_level.insert_or_assign(total_extent, cached_node_pos(tracked_extents.emplace_back(blk, tracked_node, 0US)));
			total_extent	+= blk->chain_len;
			return true;
		}
		if(!ext4_root_overflow()) return false;
	}
	offset_iterator i			= base_extent_level.max();
	if(i == base_extent_level.end()) { panic("[FS/EXT4/EXTENT] illegal extent tree state"); return false; }
	cached_extent_node* base	= get_cached(i->second);
	try
	{
		if(!base->push_extent_recurse_ext4(this, blk))
		{
			if(tracked_node->on_disk_node->block_info.ext4_extent.header.entries < tracked_node->on_disk_node->block_info.ext4_extent.header.max_entries)
			{
				uint16_t idx        = tracked_node->on_disk_node->block_info.ext4_extent.header.entries;
				disk_block* nl_blk  = tracked_node->parent_fs->claim_metadata_block(this);
				if(!nl_blk) { panic("[FS/EXT4/EXTENT] failed to claim metadata block"); return false; }
				tracked_node->on_disk_node->block_info.ext4_extent.header.entries++;
				populate_index(tracked_node->on_disk_node->block_info.ext4_extent.root_nodes[idx].idx, nl_blk->block_number, total_extent);
				uint16_t d                  = tracked_node->on_disk_node->block_info.ext4_extent.header.depth;
				cached_extent_node* base    = std::addressof(tracked_extents.emplace_back(nl_blk, tracked_node, d));
				base_extent_level.insert_or_assign(total_extent, cached_node_pos(base));
				return base->push_extent_recurse_ext4(this, blk);
			}
			if(!ext4_root_overflow()) return false;
			i = base_extent_level.max();
			return get_cached(i->second)->push_extent_recurse_ext4(this, blk);
		}
		return true;
	}
	catch(std::exception& e) { panic(e.what()); }
	return false;
}
bool ext_node_extent_tree::ext4_root_overflow()
{
	if(__builtin_expect(base_depth > 4, false)) { panic("[FS/EXT4/EXTENT] max file extent reached"); return false; }
	disk_block* nl_blk = tracked_node->parent_fs->claim_metadata_block(this);
	if(!nl_blk) { panic("[FS/EXT4/EXTENT] failed to claim metadata block"); return false; }
	size_t bs = tracked_node->parent_fs->block_size();
	base_depth++;
	tracked_node->on_disk_node->block_info.ext4_extent.header.depth++;
	ext_extent_header* header       = new(static_cast<void*>(nl_blk->data_buffer)) ext_extent_header{ .magic{ ext_extent_magic }, .max_entries{ static_cast<uint16_t>(((bs - sizeof(ext_extent_header)) / sizeof(ext_extent_node))) }, .depth{ base_depth } };
	size_t root_entries             = std::min(4US, tracked_node->on_disk_node->block_info.ext4_extent.header.max_entries);
	ext_extent_node* extent_nodes   = reinterpret_cast<ext_extent_node*>(nl_blk->data_buffer + sizeof(ext_extent_header));
	array_copy(extent_nodes, tracked_node->on_disk_node->block_info.ext4_extent.root_nodes, root_entries);
	header->entries = root_entries;
	qword nblk_qw(nl_blk->block_number);
	array_zero(std::addressof(tracked_node->on_disk_node->block_info.ext4_extent.root_nodes[0]), root_entries);
	new(static_cast<void*>(std::addressof(tracked_node->on_disk_node->block_info.ext4_extent.root_nodes[0]))) ext_extent_node{ .idx{ .next_level_block_lo{ nblk_qw.lo }, .next_level_block_hi{ nblk_qw.hi.lo } } };
	nl_blk->dirty               = true;
	cached_extent_node* base    = std::addressof(tracked_extents.emplace_back(nl_blk, tracked_node, base_depth));
	base->next_level_extents.swap(base_extent_level);
	base_extent_level.insert_or_assign(0UL, cached_node_pos(base));
	tracked_node->on_disk_node->block_info.ext4_extent.header.entries = 1US;
	return true;
}
bool cached_extent_node::push_extent_recurse_legacy(ext_node_extent_tree* parent, disk_block* blk)
{
	disk_block* my_blk  = block();
	size_t bs           = tracked_node->parent_fs->block_size();
	size_t ndwords      = bs / 4;
	if(depth < 2)
	{
		uint32_t* bptrs = reinterpret_cast<uint32_t*>(my_blk->data_buffer);
		for(size_t i = 0; i < ndwords; i++)
		{
			if(!bptrs[i])
			{
				bptrs[i]        = static_cast<uint32_t>(blk->block_number);
				next_level_extents.insert_or_assign(parent->total_extent++, parent->cached_node_pos(parent->tracked_extents.emplace_back(blk, tracked_node, 0US)));
				my_blk->dirty   = true;
				return true;
			}
		}
		return false;
	}
	uint32_t* bptrs								= reinterpret_cast<uint32_t*>(my_blk->data_buffer);
	uint32_t* target							= nullptr;
	uint16_t nd									= depth - 1;
	offset_iterator last						= next_level_extents.max();
	if(last == next_level_extents.end() || !parent->get_cached(last->second)->push_extent_recurse_legacy(parent, blk))
	{
		for(size_t i = 0; i < ndwords; i++) { if(!bptrs[i]) { target = bptrs + i; break; } }
		if(!target) return false;
		disk_block* nl_blk			= tracked_node->parent_fs->claim_metadata_block(parent);
		if(!nl_blk) throw std::runtime_error("[FS/EXT4/EXTENT] failed to allocate block");
		cached_extent_node* base	= std::addressof(parent->tracked_extents.emplace_back(nl_blk, tracked_node, nd));
		next_level_extents.insert(std::move(std::make_pair(parent->total_extent, parent->cached_node_pos(base))));
		*target						= nl_blk->block_number;
		my_blk->dirty				= true;
		return base->push_extent_recurse_legacy(parent, blk);
	}
	return true;
}
bool cached_extent_node::push_extent_recurse_ext4(ext_node_extent_tree* parent, disk_block* blk)
{
	disk_block* my_blk				= block();
	ext_extent_header* hdr			= reinterpret_cast<ext_extent_header*>(my_blk->data_buffer);
	if(hdr->magic != ext_extent_magic) { throw std::runtime_error("[FS/EXT4/EXTENT] invalid header"); }
	if(depth < 2)
	{
		if(hdr->entries < hdr->max_entries)
		{
			ext_extent_node* node	= reinterpret_cast<ext_extent_node*>(my_blk->data_buffer + sizeof(ext_extent_header)) + hdr->entries;
			populate_leaf(node->leaf, blk, parent->total_extent);
			parent->total_extent	+= blk->chain_len;
			my_blk->dirty			= true;
			csum_update();
			return true;
		}
		return false;
	}
	uint16_t nd							= depth - 1US;
	offset_iterator last				= next_level_extents.max();
	if(last == next_level_extents.end() || !parent->get_cached(last->second)->push_extent_recurse_ext4(parent, blk))
	{
		if(hdr->entries < hdr->max_entries)
		{
			disk_block* nl_blk			= tracked_node->parent_fs->claim_metadata_block(parent);
			if(!nl_blk) throw std::runtime_error("[FS/EXT4/EXTENT] failed to allocate block");
			ext_extent_node* node		= reinterpret_cast<ext_extent_node*>(my_blk->data_buffer + sizeof(ext_extent_header)) + hdr->entries;
			cached_extent_node* base	= std::addressof(parent->tracked_extents.emplace_back(nl_blk, tracked_node, nd));
			next_level_extents.insert(std::move(std::make_pair(parent->total_extent, parent->cached_node_pos(base))));
			populate_index(node->idx, nl_blk->block_number, parent->total_extent);
			my_blk->dirty = true;
			csum_update();
			return base->push_extent_recurse_ext4(parent, blk);
		}
		return false;
	}
	return true;
}
void cached_extent_node::csum_update()
{
	size_t bs			= tracked_node->parent_fs->block_size();
	uint32_t csum		= tracked_node->parent_fs->get_uuid_csum();
	csum				= crc32c(csum, tracked_node->inode_number);
	csum				= crc32c(csum, tracked_node->on_disk_node->version_lo);
	csum				= crc32c(csum, tracked_node->on_disk_node->version_hi);
	uint32_t* target	= reinterpret_cast<uint32_t*>(block()->data_buffer + bs - 4);
	*target				= 0U;
	csum				= crc32c_blk(csum, *block(), bs);
	*target				= csum;
}