#include "frame_manager.hpp"
#include "stdexcept"
frame_manager frame_manager::__inst{};
uframe_tag &frame_manager::create_frame(vaddr_t start_base, vaddr_t start_extent)
{
    paging_table pt = heap_allocator::get().allocate_pt();
    if(!pt) throw std::runtime_error{ "could not initialize paging tables" };
    else if(!heap_allocator::get().copy_kernel_mappings(pt)) throw std::runtime_error{ "could not initialize page mappings" };
    return this->emplace_back(pt, start_base, start_extent);
}
uframe_tag &frame_manager::duplicate_frame(uframe_tag const &t)
{
    uframe_tag& result = create_frame(t.base, t.extent);
    heap_allocator::get().enter_frame(&result);
    for(std::vector<block_descr>::const_iterator i = t.usr_blocks.begin(); i != t.usr_blocks.end(); i++)
    {
        vaddr_t nblk = heap_allocator::get().duplicate_user_block(i->size, i->start, i->write, i->execute);
        if(!nblk) throw std::runtime_error{ "out of memory" };
        result.usr_blocks.emplace_back(nblk, i->size, i->write, i->execute);
    }
    heap_allocator::get().exit_frame();
    return result;
}
void frame_manager::destroy_frame(uframe_tag &ft) { if(this->__out_of_range(&ft)) throw std::out_of_range{ "invalid frame tag" }; this->erase(const_iterator{ &ft }); }
frame_manager& frame_manager::get() { return __inst; }