#include "frame_manager.hpp"
#include "stdexcept"
frame_manager frame_manager::__instance{};
void frame_manager::destroy_frame(uframe_tag& ft) { if(!contains(ft)) throw std::out_of_range{ "invalid frame tag" }; erase(ft); }
frame_manager& frame_manager::get() { return __instance; }
uframe_tag& frame_manager::create_frame(addr_t start_base, addr_t start_extent)
{
    paging_table pt = kmm.allocate_pt();
    if(!pt) throw std::runtime_error{ "could not initialize paging tables" };
    else if(!kmm.copy_kernel_mappings(pt)) throw std::runtime_error{ "could not initialize page mappings" };
    return *emplace(pt, start_base, start_extent).first;
}
uframe_tag& frame_manager::duplicate_frame(uframe_tag const& t)
{
    uframe_tag& result = create_frame(t.base, t.extent);
    kmm.enter_frame(std::addressof(result));
    for(std::vector<block_descr>::const_iterator i = t.usr_blocks.begin(); i != t.usr_blocks.end(); i++)
    {
        addr_t nblk = kmm.duplicate_user_block(i->size, i->virtual_start, i->write, i->execute);
        if(!nblk) { throw std::runtime_error{ "failed to allocate new block" }; }
        result.usr_blocks.emplace_back(nblk, i->virtual_start, i->size, i->write, i->execute);
    }
    kmm.exit_frame();
    return result;
}