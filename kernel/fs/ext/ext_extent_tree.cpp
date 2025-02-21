#include "fs/ext.hpp"
bool ext_node_extent_tree::parse_legacy()
{
    // blocks 0-11 are direct
    for(int i = 0; i < 12; i++)
    {
        uint64_t b = tracked_node->on_disk_node->block_info.legacy_extent.direct_blocks[i];
        if(!b) return true; // if the block is not present we're done
        disk_block* bptr = std::addressof(tracked_node->block_data.emplace_back(b, nullptr)); // the block data buffer will be allocated if and when it is needed
        cached_extent_node* nptr = std::addressof(tracked_extents.emplace_back(bptr, tracked_node, 0));
        base_extent_level.insert_or_assign(static_cast<uint64_t>(i), nptr);
        total_extent++;
    }
    uint64_t ind1 = tracked_node->on_disk_node->block_info.legacy_extent.singly_indirect_block;
    if(!ind1) return true; // no indirect block pointer means we're done
    disk_block* single_ptr_block = std::addressof(tracked_node->cached_metadata.emplace_back(ind1, tracked_node->parent_fs->allocate_block_buffer(), false, 1));
    if(!tracked_node->parent_fs->read_from_disk(*single_ptr_block)) { panic("read on single pointer block failed"); return false; }
    uint64_t cur_file_block = 12;
    std::map<uint64_t, cached_extent_node*>::iterator exnode = base_extent_level.insert(std::make_pair(cur_file_block, std::addressof(tracked_extents.emplace_back(single_ptr_block, tracked_node, 1)))).first;
    try 
    {
        cur_file_block = exnode->second->nl_recurse_legacy(this, exnode->first);
        // the function will return 0 to indicate having reached the end of the file's extent; otherwise it will return the next file node. A failure will throw an exception
        if(!cur_file_block) return true;
        // if we get here, there are more blocks to parse in the doubly-indirect pointers
        uint64_t ind2 = tracked_node->on_disk_node->block_info.legacy_extent.doubly_indirect_block;
        disk_block* di_pointer_block = std::addressof(tracked_node->cached_metadata.emplace_back(ind2, tracked_node->parent_fs->allocate_block_buffer(), false, 1));
        if(!tracked_node->parent_fs->read_from_disk(*di_pointer_block)) { panic("read on double pointer block failed"); return false; }
        exnode = base_extent_level.insert(std::make_pair(cur_file_block, std::addressof(tracked_extents.emplace_back(di_pointer_block, tracked_node, 2)))).first;
        cur_file_block = exnode->second->nl_recurse_legacy(this, exnode->first);
        if(!cur_file_block) return true;
        // if we made it all the way here, there are even more blocks, this time in triply-indirect pointers
        uint64_t ind3 = tracked_node->on_disk_node->block_info.legacy_extent.triply_indirect_block;
        disk_block* tri_pointer_block = std::addressof(tracked_node->cached_metadata.emplace_back(ind3, tracked_node->parent_fs->allocate_block_buffer(), false, 1));
        exnode = base_extent_level.insert(std::make_pair(cur_file_block, std::addressof(tracked_extents.emplace_back(tri_pointer_block, tracked_node, 3)))).first;
        if(!tracked_node->parent_fs->read_from_disk(*tri_pointer_block)) { panic("read on triple pointer block failed"); return false; }
        exnode->second->nl_recurse_legacy(this, exnode->first);
        return true;
    } 
    catch(std::exception& e) { panic(e.what()); }
    return false;
}
bool ext_node_extent_tree::parse_ext4()
{
    ext_extent_header* h = std::addressof(tracked_node->on_disk_node->block_info.ext4_extent.header);
    if(h->magic != ext_extent_magic) { panic("invalid extent tree header"); return false; }
    ext_extent_node* nodes = tracked_node->on_disk_node->block_info.ext4_extent.root_nodes;
    size_t num = std::min(size_t(h->entries), 4UL);
    uint64_t cur_file_block;
    for(size_t i = 0; i < num; i++)
    {
        if(h->depth)
        {
            cur_file_block = nodes[i].idx.file_node_start;
            uint64_t blknum = qword(nodes[i].idx.next_level_block_lo, uint32_t(nodes[i].idx.next_level_block_hi));
            if(!blknum) continue;
            disk_block* blk = std::addressof(tracked_node->cached_metadata.emplace_back(blknum, tracked_node->parent_fs->allocate_block_buffer(), false, 1));
            if(!tracked_node->parent_fs->read_from_disk(*blk)) { panic("metadata block read failed"); return false; }
            std::map<uint64_t, cached_extent_node*>::iterator exnode = base_extent_level.insert(std::make_pair(cur_file_block, std::addressof(tracked_extents.emplace_back(blk, tracked_node, h->depth)))).first;
            if(!exnode->second->nl_recurse_ext4(this, exnode->first)) return false;
        }
        else 
        {
            cur_file_block = nodes[i].leaf.file_node_start;
            size_t ext_sz = nodes[i].leaf.extent_size % 0x8000;
            uint64_t blknum = qword(nodes[i].leaf.extent_start_lo, uint32_t(nodes[i].leaf.extent_start_hi));
            disk_block* blk = std::addressof(tracked_node->block_data.emplace_back(blknum, nullptr, false, ext_sz));
            base_extent_level.insert_or_assign(cur_file_block, std::addressof(tracked_extents.emplace_back(blk, tracked_node, 0)));
            total_extent += ext_sz;
        }
    }
    return true;
}
cached_extent_node::cached_extent_node(disk_block *bptr, ext_vnode *node, uint16_t d) : blk_offset{ bptr - (d ? node->cached_metadata.data() : node->block_data.data()) }, tracked_node{ node }, depth{ d } {}
disk_block *cached_extent_node::block() { return ((depth ? tracked_node->cached_metadata.begin() : tracked_node->block_data.begin()) + blk_offset).base(); }
size_t cached_extent_node::nl_recurse_legacy(ext_node_extent_tree* parent, uint64_t start_file_block)
{
    if(!depth) return start_file_block + 1; // if this is a direct block, the file block for the next extent node is simply the subsequent one
    uint32_t* blk_ptrs = reinterpret_cast<uint32_t*>(block()->data_buffer);
    size_t total_dwords = tracked_node->parent_fs->block_size() / 4;
    uint64_t cur_file_block = start_file_block;
    for(size_t i = 0; i < total_dwords; i++, parent->total_extent++)
    {
        if(!blk_ptrs[i]) return 0; // if we hit an empty pointer we're done
        disk_block* blk = depth == 1 ? std::addressof(tracked_node->block_data.emplace_back(blk_ptrs[i], nullptr, false, 1)) : std::addressof(tracked_node->cached_metadata.emplace_back(blk_ptrs[i], tracked_node->parent_fs->allocate_block_buffer(), false, 1));
        if(blk->data_buffer && !tracked_node->parent_fs->read_from_disk(*blk)) { throw std::runtime_error{ "failed to read disk block" }; }
        std::map<uint64_t, cached_extent_node*>::iterator exnode = next_level_extents.insert_or_assign(cur_file_block, std::addressof(parent->tracked_extents.emplace_back(blk, tracked_node, uint16_t(depth - 1)))).first;
        cur_file_block = exnode->second->nl_recurse_legacy(parent, exnode->first);
        if(!cur_file_block) return 0;
    }
    return cur_file_block;
}
bool cached_extent_node::nl_recurse_ext4(ext_node_extent_tree* parent, uint64_t start_file_block)
{
    if(!depth) return true;
    ext_extent_header* h = reinterpret_cast<ext_extent_header*>(block()->data_buffer);
    if(h->magic != ext_extent_magic) { panic("invalid extent tree header"); return false; }
    size_t n_nodes = h->entries;
    ext_extent_node* nodes = reinterpret_cast<ext_extent_node*>(block()->data_buffer + sizeof(ext_extent_header));
    for(size_t i = 0; i < n_nodes; i++)
    {
        if(h->depth)
        {
            uint64_t cur_file_block = nodes[i].idx.file_node_start;
            uint64_t blknum = qword(nodes[i].idx.next_level_block_lo, uint32_t(nodes[i].idx.next_level_block_hi));
            if(!blknum) continue;
            disk_block* blk = std::addressof(tracked_node->cached_metadata.emplace_back(blknum, tracked_node->parent_fs->allocate_block_buffer(), false, 1));
            if(!tracked_node->parent_fs->read_from_disk(*blk)) { panic("metadata block read failed"); return false; }
            std::map<uint64_t, cached_extent_node*>::iterator exnode = next_level_extents.insert(std::make_pair(cur_file_block, std::addressof(parent->tracked_extents.emplace_back(blk, tracked_node, h->depth)))).first;
            if(!exnode->second->nl_recurse_ext4(parent, exnode->first)) return false;
        }
        else
        {
            uint64_t cur_file_block = nodes[i].leaf.file_node_start;
            size_t ext_sz = nodes[i].leaf.extent_size % 0x8000;
            uint64_t blknum = qword(nodes[i].leaf.extent_start_lo, uint32_t(nodes[i].leaf.extent_start_hi));
            disk_block* blk = std::addressof(tracked_node->block_data.emplace_back(blknum, nullptr, false, ext_sz));
            next_level_extents.insert_or_assign(cur_file_block, std::addressof(parent->tracked_extents.emplace_back(blk, tracked_node, 0)));
            parent->total_extent += ext_sz;
        }
    }
    return true;
}
uint64_t cached_extent_node::get_disk_blocknum_recurse(uint64_t file_block)
{
    std::map<uint64_t, cached_extent_node*>::iterator i = next_level_extents.lower_bound(file_block);
    if(i == next_level_extents.end()) i--;
    if(!i->second) return 0;
    if(i->second->depth > 1) return i->second->get_disk_blocknum_recurse(file_block);
    disk_block* b = i->second->block();
    if(file_block < b->chain_len + i->first) return b->block_number + (file_block % i->first);
    return 0;
}
uint64_t ext_node_extent_tree::get_disk_blocknum(uint64_t file_block)
{
    std::map<uint64_t, cached_extent_node*>::iterator i = base_extent_level.lower_bound(file_block);
    if(i == base_extent_level.end()) i--;
    if(!i->second) return 0;
    if(i->second->depth > 1) return i->second->get_disk_blocknum_recurse(file_block);
    disk_block* b = i->second->block();
    if(file_block < b->chain_len + i->first) return b->block_number + (file_block % i->first);
    return 0;
}
disk_block *cached_extent_node::get_block_recurse(uint64_t file_block)
{
    std::map<uint64_t, cached_extent_node*>::iterator i = next_level_extents.lower_bound(file_block);
    if(i == next_level_extents.end()) i--;
    if(!i->second) return nullptr;
    if(i->second->depth > 1) return i->second->get_block_recurse(file_block);
    disk_block* b = i->second->block();
    if(file_block < b->chain_len + i->first) return b;
    return nullptr;
}
disk_block *ext_node_extent_tree::get_extent_block(uint64_t file_block)
{
    std::map<uint64_t, cached_extent_node*>::iterator i = base_extent_level.lower_bound(file_block);
    if(i == base_extent_level.end()) i--;
    if(!i->second) return nullptr;
    if(i->second->depth > 1) return i->second->get_block_recurse(file_block);
    disk_block* b = i->second->block();
    if(file_block < b->chain_len + i->first) return b;
    return nullptr;
}