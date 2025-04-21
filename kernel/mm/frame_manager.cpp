#include "frame_manager.hpp"
#include "stdexcept"
#include "stdlib.h"
frame_manager frame_manager::__instance{};
frame_manager::frame_manager() : __shared_blocks(128) {}
frame_manager& frame_manager::get() { return __instance; }
void frame_manager::__release_block(block_descr& blk)
{
    if(auto i = __shared_blocks.find(blk.virtual_start); i != __shared_blocks.end())
    {
        i->second.num_refs--;
        if(i->second.num_refs != 0)
            return;
    }
    kmm.deallocate_block(blk.virtual_start, blk.size, blk.virtual_start != blk.physical_start);
}
void frame_manager::destroy_frame(uframe_tag& ft)
{
    if(!contains(ft)) throw std::out_of_range{ "invalid frame tag" };
    kmm.enter_frame(std::addressof(ft));
    for(block_descr& bd : ft.usr_blocks) kmm.deallocate_block(bd.virtual_start, bd.size, false);
    for(block_descr* bd : ft.shared_blocks) { __release_block(*bd); }
    kmm.exit_frame();
    for(addr_t addr : ft.kernel_allocated_blocks) { free(addr); }
    erase(ft);
}
uframe_tag& frame_manager::create_frame(addr_t start_base, addr_t start_extent)
{
    paging_table pt = kmm.allocate_pt();
    if(!pt) throw std::runtime_error{ "could not initialize paging tables" };
    else if(!kmm.copy_kernel_mappings(pt)) throw std::runtime_error{ "could not initialize page mappings" };
    return *emplace(pt, start_base, start_extent).first;
}
uframe_tag& frame_manager::fork_frame(uframe_tag* old_frame)
{
    uframe_tag& result = create_frame(old_frame->base, old_frame->extent);
    for(block_descr& bd : old_frame->usr_blocks)
    {
        block_descr* allocated = result.add_block(bd.size, bd.virtual_start, bd.align, bd.write, bd.execute);
        if(!allocated) throw std::bad_alloc();
        addr_t target = result.translate(bd.virtual_start);
        array_copy<uint8_t>(target, bd.physical_start, bd.size);
    }
    for(block_descr* bd : old_frame->shared_blocks)
    {
        kmm.enter_frame(std::addressof(result));
        kmm.map_to_current_frame(*bd);
        kmm.exit_frame();
        result.shared_blocks.push_back(bd); 
        __shared_blocks[bd->virtual_start].num_refs++;
    }
    result.mapped_max = old_frame->mapped_max;
    return result;
}
block_descr* frame_manager::get_shared(uframe_tag* tag, size_t size, addr_t start, size_t align, bool execute)
{
    auto result_it = __shared_blocks.find(start);
    if(result_it == __shared_blocks.end()) 
    {
        kmm.enter_frame(tag);
        addr_t allocated = kmm.allocate_user_block(size, start, align, false, execute);
        kmm.exit_frame();
        if(!allocated) return nullptr;
        result_it = __shared_blocks.insert(std::make_pair(start, shared_block{ allocated, start, size, align, false, execute, 1UL })).first;
    }
    else
    {
        kmm.enter_frame(tag);
        kmm.map_to_current_frame(result_it->second);
        kmm.exit_frame();
        result_it->second.num_refs++;
    }
    block_descr* result = std::addressof(result_it->second);
    tag->shared_blocks.push_back(result);
    return result;
}