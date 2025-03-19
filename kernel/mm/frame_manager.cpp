#include "frame_manager.hpp"
#include "stdexcept"
frame_manager frame_manager::__inst{};
void frame_manager::destroy_frame(uframe_tag& ft) { if(!contains(ft)) throw std::out_of_range{ "invalid frame tag" }; erase(ft); }
frame_manager& frame_manager::get() { return __inst; }
uframe_tag& frame_manager::create_frame(addr_t start_base, addr_t start_extent)
{
    paging_table pt = kernel_memory_mgr::get().allocate_pt();
    if(!pt) throw std::runtime_error{ "could not initialize paging tables" };
    else if(!kernel_memory_mgr::get().copy_kernel_mappings(pt)) throw std::runtime_error{ "could not initialize page mappings" };
    uframe_tag* result = emplace(pt, start_base, start_extent).first.base();
    kernel_memory_mgr::get().enter_frame(result);
    if(!kernel_memory_mgr::get().identity_map_to_user(pt, PT_LEN, true, false)) { destroy_frame(*result); throw std::runtime_error{ "could not initialize page mappings" }; }
    kernel_memory_mgr::get().exit_frame();
    return *result;
}
uframe_tag& frame_manager::duplicate_frame(uframe_tag const& t)
{
    uframe_tag& result = create_frame(t.base, t.extent);
    kernel_memory_mgr::get().enter_frame(std::addressof(result));
    for(std::vector<block_descr>::const_iterator i = t.usr_blocks.begin(); i != t.usr_blocks.end(); i++)
    {
        addr_t nblk = kernel_memory_mgr::get().duplicate_user_block(i->size, i->start, i->write, i->execute);
        if(!nblk) { throw std::runtime_error{ "failed to allocate new block" }; }
        result.usr_blocks.emplace_back(nblk, i->size, i->write, i->execute);
    }
    kernel_memory_mgr::get().exit_frame();
    return result;
}