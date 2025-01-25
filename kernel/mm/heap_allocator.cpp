#include "kernel/heap_allocator.hpp"
#include "direct_text_render.hpp"
#include "errno.h"
extern "C"
{
    extern unsigned char __start;
    extern unsigned char __end;
    kframe_tag* __kernel_frame_tag = reinterpret_cast<kframe_tag*>(&__end);
}
constexpr ptrdiff_t bt_offset{ sizeof(block_tag) };
static uint8_t __heap_allocator_data_loc[sizeof(heap_allocator)];
constexpr uint32_t get_block_exp(uint64_t size) { if(size < (1ull << MIN_BLOCK_EXP)) return  MIN_BLOCK_EXP; for(size_t j =  MIN_BLOCK_EXP; j < MAX_BLOCK_EXP; j++) if(static_cast<uint64_t>(1ull << j) > size) return j; return MAX_BLOCK_EXP - 1; }
constexpr uint64_t div_roundup(size_t num, size_t denom) { return (num % denom == 0) ? (num / denom) : (1 + (num / denom)); }
constexpr block_size nearest(size_t sz)  { return sz <= S04 ? S04 : sz <= S08 ? S08 : sz <= S16 ? S16 : sz <= S32 ? S32 : sz <= S64 ? S64 : sz <= S128 ? S128 : sz <= S256 ? S256 : S512; }
constexpr size_t how_many_status_arrays(size_t mem_size) { return div_roundup(mem_size, GIGABYTE); }
constexpr uint64_t truncate(uint64_t n, uint64_t unit) { return (n % unit == 0) ? n : n - (n % unit); }
constexpr uint64_t up_to_nearest(uint64_t n, uint64_t unit) { return (n % unit == 0) ? n : truncate(n + unit, unit); }
constexpr size_t region_size_for(size_t sz) { return sz > S512 ? (truncate(sz, S512) + nearest(sz % S512)) : nearest(sz); }
constexpr size_t add_align_size(vaddr_t tag, size_t align) { return align > 1 ? (up_to_nearest(tag + bt_offset, align) - ptrdiff_t(uintptr_t(tag) + bt_offset)) : 0; }
static uintptr_t block_offset(uintptr_t addr, block_idx idx) 
{
    switch(idx)
    {
        case I0:
            addr += S128;
        case I1:
            addr += S64;
        case I2:
            addr += S32;
        case I3:
            addr += S16;
        case I4:
            addr += S08;
        case I5:
            addr += S04;
        case I6:
            addr += S04;
        case I7:
        default:
            return addr;
    }
}
static bool check_multi(block_size sz, status_byte sb)
{
    bool result = true;
    switch(sz)
    {
        case S512:
            result &= sb[I0];
        case S256:
            result &= sb[I1];
        case S128:
            result &= sb[I2];
        case S64:
            result &= sb[I3];
        case S32:
            result &= sb[I4];
        case S16:
            result &= sb[I5];
        case S08:
            result &= sb[I6];
        default:
            result &= sb[I7];
            break;
    }
    return result;
}
static paging_table __build_new_pt(paging_table in, uint16_t idx, bool write_thru)
{
    paging_table result = heap_allocator::get().allocate_pt();
    if(result)
    {
        in[idx].present = true;
        in[idx].write = true;
        if(write_thru) in[idx].write_thru = true;
        in[idx].physical_address = std::bit_cast<uintptr_t>(result) >> 12;
    }
    return result;
}
static paging_table __get_table(vaddr_t const& of_page, bool write_thru, paging_table pml4)
{
    if (pml4[of_page.pml4_idx].present)
    {
        if(write_thru) pml4[of_page.pml4_idx].write_thru = true;
        paging_table pdp = vaddr_t{ pml4[of_page.pml4_idx].physical_address << 12 };
        if(pdp[of_page.pdp_idx].present) 
        {
            if(write_thru) pdp[of_page.pdp_idx].write_thru = true;
            paging_table pd = vaddr_t { pdp[of_page.pdp_idx].physical_address << 12 };
            if(pd[of_page.pd_idx].present) { if(write_thru) pd[of_page.pd_idx].write_thru = true; return vaddr_t{ pd[of_page.pd_idx].physical_address << 12 }; }
            else return __build_new_pt(pd, of_page.pd_idx, write_thru);
        }
        else 
        {
            paging_table pd = __build_new_pt(pdp, of_page.pdp_idx, write_thru);
            if(pd) return __build_new_pt(pd, of_page.pd_idx, write_thru);
        }
    }
    else
    {
        paging_table pdp = __build_new_pt(pml4, of_page.pml4_idx, write_thru);
        if(pdp)
        {
            paging_table pd = __build_new_pt(pdp, of_page.pdp_idx, write_thru);
            if(pd) return __build_new_pt(pd, of_page.pd_idx, write_thru);
        }
    }
    return nullptr;
}
static paging_table __get_table(vaddr_t const& of_page, bool write_thru) { return __get_table(of_page, write_thru, get_cr3()); }
static paging_table __find_table(vaddr_t const& of_page)
{
    paging_table pml4 = get_cr3();
    if (pml4[of_page.pml4_idx].present)
    {
        paging_table pdp = vaddr_t{ pml4[of_page.pml4_idx].physical_address << 12 };
        if(pdp[of_page.pdp_idx].present) 
        {
            paging_table pd = vaddr_t{ pdp[of_page.pdp_idx].physical_address << 12 };
            if(pd[of_page.pd_idx].present) return vaddr_t{ pd[of_page.pd_idx].physical_address << 12 };
        }
    }
    return nullptr;
}
static vaddr_t __skip_mmio(vaddr_t start, size_t pages)
{
    vaddr_t curr { start };
    vaddr_t ed = start + ptrdiff_t(pages * PAGESIZE);
    for(size_t i = 0; i < pages; i++, curr += PAGESIZE)
    {
        paging_table pt = __get_table(start, false);
        if(!pt) { tlb_flush(); return nullptr; }
        if (pt[curr.page_idx].present && (pt[curr.page_idx].write_thru || pt[curr.page_idx].cache_disable)) i = 0;
    }
    vaddr_t c_ed = curr + ptrdiff_t(pages * PAGESIZE);
    if(uintptr_t(c_ed) > uintptr_t(ed)) curr = curr + ptrdiff_t(uintptr_t(c_ed) - uintptr_t(ed));
    return curr;
}
static void __set_kernel_global(uintptr_t max)
{
    paging_table pt = nullptr;
    for(vaddr_t addr{ &__start }; uintptr_t(addr) < max; addr += PAGESIZE)
    {
        if(!pt || !addr.page_idx) pt = __find_table(addr);
        if(pt) pt[addr.page_idx].global = true;
    }
}
static vaddr_t __map_kernel_pages(vaddr_t start, size_t pages)
{
    if(!start) return nullptr; 
    vaddr_t curr{ __skip_mmio(start, pages) };
    if(!curr) return nullptr;
    start = curr;
    uintptr_t phys = curr;
    paging_table pt = __get_table(curr, false);
    if(!pt) { return nullptr; }
    for(size_t i = 0; i < pages; i++, curr += PAGESIZE, phys += PAGESIZE)
    {
        if(i != 0 && curr.page_idx == 0) pt = __get_table(curr, false);
        if(pt[curr.page_idx].present && pt[curr.page_idx].global) continue;
        pt[curr.page_idx].present = true;
        pt[curr.page_idx].global = true;
        pt[curr.page_idx].write = true;
        pt[curr.page_idx].physical_address = phys >> 12;
    }
    tlb_flush();
    return start;
}
static vaddr_t __map_mmio_pages(vaddr_t start, size_t pages)
{
    if(!start) return nullptr;
    vaddr_t curr{ start };
    paging_table pt = __get_table(curr, true);
    if(!pt) return nullptr;
    for(size_t i = 0; i < pages; i++, curr += PAGESIZE)
    {
        if(i != 0 && curr.page_idx == 0) pt = __get_table(curr, true);
        pt[curr.page_idx].present = true;
        pt[curr.page_idx].global = true;
        pt[curr.page_idx].write = true;
        pt[curr.page_idx].write_thru = true;
        pt[curr.page_idx].physical_address = uint64_t(curr) >> 12;
    }
    tlb_flush();
    return start;
}
static vaddr_t __map_user_pages(vaddr_t start_vaddr, uintptr_t start_paddr, size_t pages, paging_table pml4, bool write, bool execute)
{
    if(start_vaddr && start_paddr && pages && pml4)
    {
        vaddr_t curr{ __skip_mmio(start_vaddr, pages) };
        if(!curr) return nullptr;
        start_vaddr = curr;
        uintptr_t phys = start_paddr;
        paging_table pt = __get_table(curr, false, pml4);
        if(!pt) return nullptr;
        for(size_t i = 0; i < pages; i++, curr += PAGESIZE, phys += PAGESIZE)
        {
            if(i != 0 && curr.page_idx == 0) pt = __get_table(curr, false, pml4);
            pt[curr.page_idx].present = true;
            pt[curr.page_idx].user_access = true;
            pt[curr.page_idx].write = write;
            pt[curr.page_idx].execute_disable = !execute;
            pt[curr.page_idx].physical_address = phys >> 12;
        }
        tlb_flush();
        return start_vaddr;
    }
    else return nullptr;
}
static void __unmap_user_pages(vaddr_t start, size_t pages)
{
    if(start && pages)
    {
        vaddr_t curr{ start };
        paging_table pt = nullptr;
        for(size_t i = 0; i < pages; i++, curr += PAGESIZE)
        {
            if(!pt || curr.page_idx == 0) pt = __find_table(curr);
            if(pt && pt[curr.page_idx].user_access)
            {
                pt[curr.page_idx].present = false;
                pt[curr.page_idx].user_access = false;
                pt[curr.page_idx].execute_disable = false;
                pt[curr.page_idx].physical_address = 0u;
                asm volatile("invlpg (%0)" :: "r"(uintptr_t(curr)) : "memory");
            }
        }
    }
}
heap_allocator* heap_allocator::__instance{ reinterpret_cast<heap_allocator*>(__heap_allocator_data_loc) };
heap_allocator &heap_allocator::get() { return *__instance; }
void heap_allocator::__lock() { lock(&__heap_mutex); __suspend_frame(); }
void heap_allocator::__unlock() { release(&__heap_mutex); __resume_frame(); }
void heap_allocator::__mark_used(uintptr_t addr_start, size_t num_regions) { for(size_t i = 0; i < num_regions; i++, addr_start += REGION_SIZE) __get_sb(addr_start)->set_used(ALL); }
void heap_allocator::__suspend_frame() noexcept { vaddr_t cur_cr3{ get_cr3() }; if(cur_cr3 != __kernel_cr3) { __suspended_cr3 = cur_cr3; set_cr3(__kernel_cr3); } }
void heap_allocator::__resume_frame() noexcept { if(__suspended_cr3) { set_cr3(__suspended_cr3); __suspended_cr3 = nullptr; } }
uintptr_t heap_allocator::__find_claim_avail_region(size_t sz)
{
    uintptr_t addr = up_to_nearest(__physical_open_watermark, REGION_SIZE);
    uintptr_t result = 0;
    size_t regions = 0;
    bool multi = (sz > S512);
    for(size_t rem = sz; status_byte::gb_of(addr) < __num_status_bytes && rem > 0; addr += REGION_SIZE)
    {
        block_size bs = nearest(rem);
        status_byte& sb = *__get_sb(addr);
        if(!(multi ? check_multi(bs, sb) : sb[bs]))
        {
            // If we're looking for a block bigger than one region, we need to start over
            result = 0;
            rem = sz;
            regions = 0;
        }
        else 
        {
            if(bs == S04)
            {
                // Special case: there are two blocks of this size per region
                if(sb[I7]) { sb.set_used(I7); if(!multi) { return block_offset(addr, I7); } }
                else { sb.set_used(I6); if(!multi) { return block_offset(addr, I6); } }
                __mark_used(result, regions);
                return result;
            }
            else if (bs == S512)
            {
                // Either we're allocating a full region, or more than a full region.
                if(result == 0) result = addr;
                regions++;
                if(rem <= S512) { __mark_used(result, regions); return result; }
                rem -= S512;
            }
            else 
            {
                if(!multi)
                {
                    block_idx idx = bs == S08 ? I5 : (bs == S16 ? I4 : (bs == S32 ? I3 : (bs == S64 ? I2 : (bs == S128 ? I1 : I0))));
                    sb.set_used(idx);
                    return block_offset(addr, idx);
                }
                // For multi-region blocks, if the block contains a non-full-region part larger than 4 pages, the indices are computed in reverse.
                // As a result, the entire block remains contiguous (subregion 7 is at the lowest physical address).
                // These special sub-blocks span a cumulative set of subregions such that the total capacity equals the required remainder.
                // For instance, a 16-page remainder sub-block would consist of sub-regions 5 (8 pages), 6 (4 pages), and 7 (4 pages), while the actual 16-page sub-block at index 4 remains open.
                switch(bs)
                {
                    case S256:
                        sb.set_used(I1);
                    case S128:
                        sb.set_used(I2);
                    case S64:
                        sb.set_used(I3);
                    case S32:
                        sb.set_used(I4);
                    case S16:
                        sb.set_used(I5);
                    case S08:
                        sb.set_used(I6);
                        sb.set_used(I7);
                    default:
                        break;
                }
                __mark_used(result, regions);
                return result;
            }
        }
    }
    if(status_byte::gb_of(addr) < __num_status_bytes) return result;
    return 0;
}
void heap_allocator::__release_claimed_region(size_t sz, uintptr_t start)
{
    block_size bs = nearest(sz);
    bool multi = (sz > S512);
    for(size_t rem = sz; rem > 0; rem -= (bs > rem ? rem : bs), start += S512)
    {
        bs = nearest(rem);
        if(bs == S04) { if(start % REGION_SIZE == 0) __status(start).set_free(I7); else __status(start).set_free(I6); }
        else if(bs == S512) __status(start).set_free(ALL);
        else if(multi)
        {
            status_byte sb = __status(start);
            switch(bs)
            {
                case S256:
                    sb.set_free(I1);
                case S128:
                    sb.set_free(I2);
                case S64:
                    sb.set_free(I3);
                case S32:
                    sb.set_free(I4);
                case S16:
                    sb.set_free(I5);
                case S08:
                    sb.set_free(I6);
                    sb.set_free(I7);
                default:
                    break;
            }
        }
        else __status(start).set_free((bs == S08 ? I5 : (bs == S16 ? I4 : (bs == S32 ? I3 : (bs == S64 ? I2 : (bs == S128 ? I1 : I0))))));
    }
}
vaddr_t heap_allocator::allocate_mmio_block(size_t sz)
{
    __lock();
    vaddr_t result{ nullptr };
    if(uintptr_t phys = __find_claim_avail_region(sz)) { result = __map_mmio_pages(vaddr_t{ phys }, div_roundup(region_size_for(sz), PAGESIZE)); __physical_open_watermark = std::max(phys, __physical_open_watermark); }
    __unlock();
    return result;
}
vaddr_t heap_allocator::allocate_user_block(size_t sz, vaddr_t start, bool write, bool execute)
{
    vaddr_t pml4{ get_cr3() };
    __lock();
    vaddr_t result{ nullptr };
    if(uintptr_t phys = __find_claim_avail_region(sz)) { result = __map_user_pages(start, phys, div_roundup(region_size_for(sz), PAGESIZE), pml4, write, execute); }
    __unlock();
    return result;
}
void heap_allocator::init_instance(mmap_t *mmap)
{
    gb_status* __the_status_bytes{ vaddr_t{ &__end } + ptrdiff_t(sizeof(kframe_tag)) };
    size_t num_status_bytes{ how_many_status_arrays(mmap->total_memory) };
    uintptr_t heap{ vaddr_t{ &__end } + ptrdiff_t(sizeof(kframe_tag) + num_status_bytes * sizeof(gb_status)) };
    new (__the_status_bytes) gb_status[num_status_bytes];
    new (__kernel_frame_tag) kframe_tag{};
    new (heap_allocator::__instance) heap_allocator{ __the_status_bytes, num_status_bytes, heap, get_cr3() };
    heap_allocator::__instance->__mark_used(0, div_roundup(heap, REGION_SIZE));
    for(size_t i = 0; i < mmap->num_entries; i++) { if(mmap->entries[i].type != AVAILABLE) { heap_allocator::__instance->__mark_used(mmap->entries[i].addr, div_roundup(mmap->entries[i].len, PT_LEN)); if(mmap->entries[i].type == MMIO) __map_mmio_pages(vaddr_t{ mmap->entries[i].addr }, mmap->entries[i].len); } }
    __instance->__physical_open_watermark = heap;
    __set_kernel_global(vaddr_t{ &__end } + ptrdiff_t(sizeof(kframe_tag)));
}
paging_table heap_allocator::allocate_pt()
{
    const size_t pt_size{ sizeof(pt_entry) * PT_LEN };
    uint32_t exp{ get_block_exp(REGION_SIZE) - MIN_BLOCK_EXP };
    block_tag* tag{ nullptr };
    for(tag = __kernel_frame_tag->available_blocks[exp]; bool(tag); tag = tag->next)
    {
        tag->align_bytes = add_align_size(tag, PAGESIZE);
        if (tag->available_size() >= pt_size)
        {
            __kernel_frame_tag->remove_block(tag);
            if(!tag->left_split && !tag->right_split) __kernel_frame_tag->complete_pages[exp]--;
            break;
        }
        tag->align_bytes = 0ul;
    }
    if(!tag)
    {
        vaddr_t allocated{ __find_claim_avail_region(S512) };
        if(!allocated) return nullptr;
        if(!translate_vaddr(allocated)) { __map_kernel_pages(allocated, REGION_SIZE / PAGESIZE); }
        tag = new (allocated) block_tag{ REGION_SIZE - bt_offset, 0 };
        __physical_open_watermark = std::max(uintptr_t(allocated), __physical_open_watermark);
    }
    tag->held_size = pt_size;
    vaddr_t result = tag->actual_start();
    if(tag->available_size() - bt_offset >= (1 << MIN_BLOCK_EXP)) __kernel_frame_tag->insert_block(tag->split(), -1);
    if(result && __active_frame) __active_frame->pt_blocks.push_back(result);
    return result;
}
vaddr_t heap_allocator::allocate_kernel_block(size_t sz)
{
    __lock();
    vaddr_t phys{ __find_claim_avail_region(sz) };
    vaddr_t result{ nullptr };
    if(phys) { result = __map_kernel_pages(phys, div_roundup(region_size_for(sz), PAGESIZE)); __physical_open_watermark = std::max(uintptr_t(phys), __physical_open_watermark); }
    __unlock();
    return result;
}
void heap_allocator::deallocate_block(vaddr_t const& base, size_t sz, bool should_unmap) { __lock(); if(uintptr_t phys{ translate_vaddr(base) }) { __release_claimed_region(sz, phys); if(should_unmap) __unmap_user_pages(base, div_roundup(sz, PAGESIZE)); __physical_open_watermark = std::min(phys, __physical_open_watermark); } __unlock(); }
void kframe_tag::__lock() { lock(&__my_mutex); }
void kframe_tag::__unlock() { release(&__my_mutex); }
void heap_allocator::enter_frame(uframe_tag *ft) noexcept { this->__active_frame = ft; }
void kframe_tag::insert_block(block_tag* blk, int idx) { blk->index = idx < 0 ? (get_block_exp(blk->block_size) - MIN_BLOCK_EXP) : idx; if (available_blocks[blk->index]) { blk->next = available_blocks[blk->index]; available_blocks[blk->index]->previous = blk; } available_blocks[blk->index] = blk; }
void kframe_tag::remove_block(block_tag* blk)
{
    if(available_blocks[blk->index] == blk) available_blocks[blk->index] = blk->next;
    if(blk->previous) blk->previous->next = blk->next;
    if(blk->next) blk->next->previous = blk->previous;
    blk->next = nullptr;
    blk->previous = nullptr;
    blk->index = -1;
}
/**
 * Basically implements the standard ::operator new(std::size_t, std::align_val) in the frame given by *this.
 * In the kernel, allocations invoke this on the kernel frame tag, which is located directly after the kernel in memory and will be accessible from the kernel's GS base.
 * Similarly, in userspace, the current process' frame tag will be accessible from that process' GS base.
 */
vaddr_t kframe_tag::allocate(size_t size, size_t align)
{
    if(!size) return nullptr;
    __lock();
    uint32_t idx = get_block_exp(size) - MIN_BLOCK_EXP;
    block_tag* tag{ nullptr };
    for(tag = available_blocks[idx]; bool(tag); tag = tag->next)
    {
        if(tag->available_size() >= size + add_align_size(tag, align))
        {
            remove_block(tag);
            tag->held_size = size;
            tag->align_bytes = add_align_size(tag, align);
            if(!tag->left_split && !tag->right_split) complete_pages[idx]--;
            break;
        }
    }
    if(!tag)
    {
        if(!(tag = __create_tag(size, align))) { __unlock(); return nullptr; }
        idx = get_block_exp(tag->allocated_size()) - MIN_BLOCK_EXP;
    }
    tag->index = idx;
    vaddr_t result{ tag->actual_start() };
    if(tag->available_size() - sizeof(block_tag) >= (1 << MIN_BLOCK_EXP)) insert_block(tag->split(), -1);
    __unlock();
    return result;
}
void kframe_tag::deallocate(vaddr_t ptr, size_t align)
{
    if(ptr)
    {
        __lock();
        block_tag* tag{ ptr - bt_offset };
        for(size_t i = 0; tag->magic != BLOCK_MAGIC; i++) tag = ptr - ptrdiff_t(bt_offset + i);
        if(tag->magic == BLOCK_MAGIC)
        {
            tag->held_size = 0;
            tag->align_bytes = 0;
            while(tag->left_split && (tag->left_split->index >= 0)) tag = __melt_left(tag);
            while(tag->right_split && (tag->right_split->index >= 0)) tag = __melt_right(tag);
            unsigned int idx = get_block_exp(tag->allocated_size()) - MIN_BLOCK_EXP;
            if((!tag->left_split && !tag->right_split) && complete_pages[idx] >= MAX_COMPLETE_PAGES) { heap_allocator::get().deallocate_block(tag, tag->block_size); __unlock(); return; }
            insert_block(tag, idx);
        }
        __unlock();
    }
}
vaddr_t kframe_tag::reallocate(vaddr_t ptr, size_t size, size_t align)
{
    if(!ptr) return allocate(size, align);
    if(!size) return nullptr;
    block_tag* tag{ ptr - bt_offset };
    for(size_t i = 0; tag->magic != BLOCK_MAGIC; i++) tag = ptr - ptrdiff_t(bt_offset + i);
    if(tag->magic == BLOCK_MAGIC && tag->allocated_size() >= size + add_align_size(tag, align))
    {
        tag->align_bytes = add_align_size(tag, align);
        size_t delta = size - (tag->held_size + tag->align_bytes);
        tag->held_size += delta;
        return tag->actual_start();
    }
    vaddr_t result{ allocate(size, align) };
    if(result) { __builtin_memcpy(result, ptr, tag->held_size > size ? size : tag->held_size); }
    deallocate(ptr);
    return result;
}
vaddr_t kframe_tag::array_allocate(size_t num, size_t size)
{
    vaddr_t result{ allocate(num * size, size) };
    if(result) __builtin_memset(result, 0, num * size);
    return result;
}
block_tag *kframe_tag::__create_tag(size_t size, size_t align)
{
    size_t actual_size = std::max(size + bt_offset, align) + align;
    vaddr_t allocated{ heap_allocator::get().allocate_kernel_block(actual_size) };
    if(!allocated) return nullptr;
    return new (allocated) block_tag{ region_size_for(actual_size), size, -1, add_align_size(allocated, align) };
}
block_tag *kframe_tag::__melt_left(block_tag *tag) noexcept
{
    if(!tag->left_split) return tag;
    block_tag* left{ tag->left_split };
    left->block_size += tag->block_size;
    left->right_split = tag->right_split;
    if(tag->right_split) tag->right_split->left_split = left;
    remove_block(tag);
    return left;
}
block_tag *kframe_tag::__melt_right(block_tag *tag) noexcept
{
    block_tag* right{ tag->right_split };
    tag->block_size += right->block_size;
    tag->right_split = right->right_split;
    if(right->right_split) right->right_split->left_split = tag;
    remove_block(right);
    return tag;
}
block_tag *block_tag::split()
{
    block_tag* that{ this->actual_start() + ptrdiff_t(this->held_size) };
    new (that) block_tag{ available_size(), 0, -1, this, this->right_split };
    if(that->right_split) that->right_split->left_split = that;
    this->right_split = that;
    this->block_size -= that->block_size;
    return that;
}
void uframe_tag::__lock() { lock(&__my_mutex); }
void uframe_tag::__unlock() { release(&__my_mutex); }
uframe_tag::~uframe_tag() { for(block_descr blk : usr_blocks) heap_allocator::get().deallocate_block(blk.start, blk.size, true); for(vaddr_t addr : pt_blocks) __kernel_frame_tag->deallocate(addr); }
bool uframe_tag::shift_extent(ptrdiff_t amount)
{
    if(amount == 0) return true; // nothing to do, vacuous success
    if(amount < 0)
    {
        uintptr_t amt_freed = -amount;
        if(extent > amt_freed)
        {
            vaddr_t target = extent + amount;
            std::vector<block_descr>::reverse_iterator i = usr_blocks.rend();
            while(i != usr_blocks.rbegin() && target < i->start) { heap_allocator::get().deallocate_block(i->start, i->size, true); i++; }
            bool nrem = (i != usr_blocks.rbegin());
            usr_blocks.erase(i.base(), usr_blocks.end());
            if(!nrem) { size_t nsz = region_size_for(uint64_t(target)); extent = base + ptrdiff_t(nsz); vaddr_t allocated = heap_allocator::get().allocate_user_block(nsz, base); if(!allocated) { extent = nullptr; return false; } usr_blocks.emplace_back(allocated, nsz); return true; }
            vaddr_t nst = usr_blocks.back().start + ptrdiff_t(usr_blocks.back().size);
            size_t nsz = region_size_for(target - nst);
            extent = nst;
            vaddr_t allocated = heap_allocator::get().allocate_user_block(nsz, nst);
            if(!allocated) return false;
            usr_blocks.emplace_back(allocated, nsz);
            extent += nsz;
            return true; 
        }
        else return false;
    }
    size_t added{ region_size_for(static_cast<size_t>(amount)) };
    vaddr_t allocated = heap_allocator::get().allocate_user_block(added, extent);
    if(allocated) { usr_blocks.emplace_back(allocated, added); extent += added; return true; }
    return false;
}
#include "sched/task.h"
extern "C"
{ 
    uintptr_t translate_vaddr(vaddr_t addr) { if(paging_table pt = __find_table(addr)) return (pt[addr.page_idx].physical_address << 12) | addr.offset; else return 0; }
    vaddr_t syscall_sbrk(ptrdiff_t incr)
    {
        uframe_tag* ctask_frame = current_active_task()->frame_ptr;
        if(ctask_frame->magic != UFRAME_MAGIC) return nullptr;
        heap_allocator::get().enter_frame(ctask_frame);
        vaddr_t result = ctask_frame->extent;
        if(ctask_frame->shift_extent(incr)) return result;
        else return vaddr_t{ uintptr_t(-1) };
    }
}