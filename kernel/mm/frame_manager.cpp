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