#include "kernel/kernel_mm.hpp"
#include "direct_text_render.hpp"
#include "sched/task_ctx.hpp"
#include "errno.h"

extern "C"
{
    extern unsigned char __start;
    extern unsigned char __end;
    extern unsigned char __code;
    kframe_tag* __kernel_frame_tag = reinterpret_cast<kframe_tag*>(&__end);
}
constexpr ptrdiff_t bt_offset{ sizeof(block_tag) };
static uint8_t __heap_allocator_data_loc[sizeof(kernel_memory_mgr)];
constexpr uint32_t get_block_exp(uint64_t size) { if(size < (1ull << MIN_BLOCK_EXP)) return  MIN_BLOCK_EXP; for(size_t j =  MIN_BLOCK_EXP; j < MAX_BLOCK_EXP; j++) if(static_cast<uint64_t>(1ull << j) > size) return j; return MAX_BLOCK_EXP - 1; }
constexpr block_size nearest(size_t sz)  { return sz <= S04 ? S04 : sz <= S08 ? S08 : sz <= S16 ? S16 : sz <= S32 ? S32 : sz <= S64 ? S64 : sz <= S128 ? S128 : sz <= S256 ? S256 : S512; }
constexpr size_t how_many_status_arrays(size_t mem_size) { return div_roundup(mem_size, GIGABYTE); }
constexpr size_t region_size_for(size_t sz) { return sz > S512 ? (up_to_nearest(sz, S512)) : nearest(sz); }
constexpr size_t add_align_size(addr_t tag, size_t align) { return align > 1 ? (up_to_nearest(tag + bt_offset, align) - ptrdiff_t(uintptr_t(tag) + bt_offset)) : 0; }
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
static paging_table __build_new_pt(paging_table in, uint16_t idx, bool write_thru)
{
    paging_table result = kernel_memory_mgr::get().allocate_pt();
    if(result)
    {
        in[idx].present = true;
        in[idx].write = true;
        in[idx].user_access = true;
        if(write_thru) in[idx].write_thru = true;
        in[idx].physical_address = std::bit_cast<uintptr_t>(result) >> 12;
    }
    return result;
}
static paging_table __get_table(addr_t const& of_page, bool write_thru, paging_table pml4)
{
    if (pml4[of_page.pml4_idx].present)
    {
        if(write_thru) pml4[of_page.pml4_idx].write_thru = true;
        pml4[of_page.pml4_idx].user_access = true;
        paging_table pdp = addr_t{ pml4[of_page.pml4_idx].physical_address << 12 };
        if(pdp[of_page.pdp_idx].present) 
        {
            if(write_thru) pdp[of_page.pdp_idx].write_thru = true;
            pdp[of_page.pdp_idx].user_access = true; // this bit is controlled at the page-level only
            paging_table pd = addr_t { pdp[of_page.pdp_idx].physical_address << 12 };
            if(pd[of_page.pd_idx].present) { if(write_thru) pd[of_page.pd_idx].write_thru = true; pd[of_page.pd_idx].user_access = true; return addr_t{ pd[of_page.pd_idx].physical_address << 12 }; }
            else return __build_new_pt(pd, of_page.pd_idx, write_thru);
        }
        else { paging_table pd = __build_new_pt(pdp, of_page.pdp_idx, write_thru); if(pd) return __build_new_pt(pd, of_page.pd_idx, write_thru); }
    }
    else { paging_table pdp = __build_new_pt(pml4, of_page.pml4_idx, write_thru); if(pdp) { paging_table pd = __build_new_pt(pdp, of_page.pdp_idx, write_thru); if(pd) return __build_new_pt(pd, of_page.pd_idx, write_thru); } }
    return nullptr;
}
static paging_table __get_table(addr_t const& of_page, bool write_thru) { return __get_table(of_page, write_thru, get_cr3()); }
static paging_table __find_table(addr_t const& of_page, paging_table pml4) { if (pml4[of_page.pml4_idx].present) { paging_table pdp = addr_t{ pml4[of_page.pml4_idx].physical_address << 12 }; if(pdp[of_page.pdp_idx].present) { paging_table pd = addr_t{ pdp[of_page.pdp_idx].physical_address << 12 }; if(pd[of_page.pd_idx].present) return addr_t{ pd[of_page.pd_idx].physical_address << 12 }; } }  return nullptr; }
static paging_table __find_table(addr_t const& of_page) { return __find_table(of_page, get_cr3()); }
static void __set_kernel_page_settings(uintptr_t max) { paging_table pt = nullptr; for(addr_t addr{ &__start }; addr < max; addr += PAGESIZE) { if(!pt || !addr.page_idx) pt = __find_table(addr); if(pt) { pt[addr.page_idx].global = true; pt[addr.page_idx].write = true; pt[addr.page_idx].user_access = true; } } }
static addr_t __skip_mmio(addr_t start, size_t pages)
{
    addr_t curr { start };
    addr_t ed = start.plus(pages * PAGESIZE);
    for(size_t i = 0; i < pages; i++, curr += PAGESIZE)
    {
        paging_table pt = __get_table(start, false);
        if(!pt) { return nullptr; }
        if (pt[curr.page_idx].present && (pt[curr.page_idx].write_thru || pt[curr.page_idx].cache_disable)) i = 0;
    }
    addr_t c_ed = curr.plus(pages * PAGESIZE);
    if(c_ed > ed) curr = curr.plus(c_ed - ed);
    return curr;
}
static addr_t __map_kernel_pages(addr_t start, size_t pages, bool global)
{
    if(!start) return nullptr; 
    addr_t curr{ __skip_mmio(start, pages) };
    if(!curr) return nullptr;
    start = curr;
    uintptr_t phys = curr;
    paging_table pt = __get_table(curr, false);
    if(!pt) { return nullptr; }
    bool modified = false;
    for(size_t i = 0; i < pages; i++, curr += PAGESIZE, phys += PAGESIZE)
    {
        if(i != 0 && curr.page_idx == 0) pt = __get_table(curr, false);
        if(pt[curr.page_idx].present && (pt[curr.page_idx].global || pt[curr.page_idx].physical_address == phys >> 12)) continue;
        pt[curr.page_idx].present = true;
        pt[curr.page_idx].global = global;
        pt[curr.page_idx].write = true;
        pt[curr.page_idx].user_access = true; // userland access to pages will be controlled by having only the necessary pages mapped into the userland page frame
        pt[curr.page_idx].physical_address = phys >> 12;
        modified = true;
    }
    if(modified) tlb_flush();
    return start;
}
static addr_t __copy_kernel_page_mapping(addr_t start, size_t pages, paging_table pml4)
{
    addr_t curr{ start };
    paging_table pt = __get_table(curr, false);
    if(!pt) return nullptr;
    paging_table upt = __get_table(curr, false, pml4);
    if(!upt) return nullptr;
    for(size_t i = 0; i < pages; i++, curr += PAGESIZE)
    {
        if(i != 0 && curr.page_idx == 0) { pt = __get_table(curr, true); upt = __get_table(curr, false, pml4); }
        __builtin_memcpy(&upt[curr.page_idx], &pt[curr.page_idx], sizeof(pt_entry));
        upt[curr.page_idx].write = false;
        upt[curr.page_idx].user_access = true;
    }
    return start;
}
static addr_t __map_mmio_pages(addr_t start, size_t pages)
{
    if(!start) return nullptr;
    addr_t curr{ start };
    paging_table pt = __get_table(curr, true);
    if(!pt) return nullptr;
    for(size_t i = 0; i < pages; i++, curr += PAGESIZE)
    {
        if(i != 0 && curr.page_idx == 0) pt = __get_table(curr, true);
        pt[curr.page_idx].present = true;
        pt[curr.page_idx].global = true;
        pt[curr.page_idx].write = true;
        pt[curr.page_idx].write_thru = true;
        pt[curr.page_idx].user_access = true;
        pt[curr.page_idx].physical_address = uint64_t(curr) >> 12;
    }
    tlb_flush();
    return start;
}
static addr_t __map_user_pages(addr_t start_vaddr, uintptr_t start_paddr, size_t pages, paging_table pml4, bool write, bool execute)
{
    if(start_vaddr && start_paddr && pages && pml4)
    {
        addr_t curr = start_vaddr;
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
static void __unmap_pages(addr_t start, size_t pages, addr_t pml4 = get_cr3())
{
    if(start && pages)
    {
        addr_t curr{ start };
        paging_table pt = nullptr;
        for(size_t i = 0; i < pages; i++, curr += PAGESIZE)
        {
            if(!pt || curr.page_idx == 0) pt = __find_table(curr, pml4);
            if(pt && !pt[curr.page_idx].global)
            {
                pt[curr.page_idx].present = false;
                pt[curr.page_idx].user_access = false;
                pt[curr.page_idx].execute_disable = false;
                pt[curr.page_idx].physical_address = 0u;
                asm volatile("invlpg (%0)" :: "r"(curr.val()) : "memory");
            }
        }
    }
}
kernel_memory_mgr* kernel_memory_mgr::__instance{ reinterpret_cast<kernel_memory_mgr*>(__heap_allocator_data_loc) };
kernel_memory_mgr &kernel_memory_mgr::get() { return *__instance; }
uintptr_t kernel_memory_mgr::__claim_region(uintptr_t addr, block_idx idx) { __status(addr).set_used(idx); return block_offset(addr, idx); }
void kernel_memory_mgr::__lock() { lock(&__heap_mutex); __suspend_frame(); }
void kernel_memory_mgr::__unlock() { release(&__heap_mutex); __resume_frame(); }
void kernel_memory_mgr::__userlock() { lock(&__user_mutex); }
void kernel_memory_mgr::__userunlock() { release(&__user_mutex); }
void kernel_memory_mgr::__mark_used(uintptr_t addr_start, size_t num_regions) { for(size_t i = 0; i < num_regions; i++, addr_start += REGION_SIZE) __get_sb(addr_start)->set_used(ALL); }
void kernel_memory_mgr::__suspend_frame() noexcept { addr_t cur_cr3{ get_cr3() }; if(cur_cr3 != __kernel_cr3) { __suspended_cr3 = cur_cr3; set_cr3(__kernel_cr3); } }
void kernel_memory_mgr::__resume_frame() noexcept { if(__suspended_cr3) { set_cr3(__suspended_cr3); __suspended_cr3 = nullptr; } }
uintptr_t kernel_memory_mgr::__find_and_claim_available_region(size_t sz)
{
    uintptr_t addr = up_to_nearest(__physical_open_watermark, REGION_SIZE);
    if(sz > S256)
    {
        size_t num_regions = div_roundup(sz, S512);
        uintptr_t result = addr;
        for(size_t n = num_regions; status_byte::gb_of(addr) < __num_status_bytes && n > 0; addr += S512) { if(__status(addr)[S512]) { n--; if(!n) { for(uintptr_t i = result; i < addr; i += S512) __status(i).set_used(ALL); return result; } }  else { n = num_regions; result = addr + S512; } }
    }
    else 
    {
        for(block_size bs = nearest(sz); status_byte::gb_of(addr) < __num_status_bytes; addr += S512)
        {
            if(__status(addr)[bs])
            {
                switch(bs)
                {
                case S04:
                    return __claim_region(addr, __status(addr)[I7] ? I7 : I6);
                case S08:
                    return __claim_region(addr, I5);
                case S16:
                    return __claim_region(addr, I4);
                case S32:
                    return __claim_region(addr, I3);
                case S64:
                    return __claim_region(addr, I2);
                case S128:
                    return __claim_region(addr, I1);
                default:
                    return __claim_region(addr, I0);
                }
            }
        }
    }
    return 0;
}
void kernel_memory_mgr::__release_claimed_region(size_t sz, uintptr_t start)
{
    block_size bs = nearest(sz);
    if(sz > S256) { size_t n = div_roundup(sz, S512); for(size_t i = 0; i < n; i++, start += S512) { __status(start).set_free(ALL); } }
    else
    {
        switch(bs)
        {
            case S256:
                __status(start).set_free(I0);
                break;
            case S128:
                __status(start).set_free(I1);
                break;
            case S64:
                __status(start).set_free(I2);
                break;
            case S32:
                __status(start).set_free(I3);
                break;
            case S16:
                __status(start).set_free(I4);
                break;
            case S08:
                __status(start).set_free(I5);
                break;
            default:
                __status(start).set_free((start % REGION_SIZE == 0) ? I7 : I6);
                break;
        }
    }
}
size_t kernel_memory_mgr::page_aligned_region_size(addr_t start, size_t requested) { return size_t((start + ptrdiff_t(requested + PAGESIZE)).page_aligned() - start.page_aligned()); }
void kernel_memory_mgr::suspend_user_frame() { __instance->__suspend_frame(); }
void kernel_memory_mgr::resume_user_frame() { __instance->__resume_frame(); }
uintptr_t kernel_memory_mgr::translate_vaddr_in_current_frame(addr_t addr) { if(paging_table pt = __find_table(addr, __active_frame ? __active_frame->pml4 : paging_table(__kernel_cr3))) return (pt[addr.page_idx].physical_address << 12) | addr.offset; else return 0; }
void kernel_memory_mgr::deallocate_block(addr_t const& base, size_t sz, bool should_unmap) { uintptr_t phys{ translate_vaddr_in_current_frame(base) }; addr_t pml4 =  __active_frame ? addr_t{ __active_frame->pml4 } : __kernel_cr3; __lock(); if(phys) { __release_claimed_region(sz, phys); if(should_unmap) __unmap_pages(base, div_roundup(sz, PAGESIZE), pml4); __physical_open_watermark = std::min(phys, __physical_open_watermark); } __unlock(); }
void kernel_memory_mgr::init_instance(mmap_t *mmap)
{
    gb_status* __the_status_bytes{ addr_t{ &__end }.plus(sizeof(kframe_tag)) };
    size_t num_status_bytes{ how_many_status_arrays(mmap->total_memory) };
    uintptr_t heap{ addr_t{ &__end }.plus(sizeof(kframe_tag) + num_status_bytes * sizeof(gb_status)) };
    new (__the_status_bytes) gb_status[num_status_bytes];
    new (__kernel_frame_tag) kframe_tag{};
    new (kernel_memory_mgr::__instance) kernel_memory_mgr{ __the_status_bytes, num_status_bytes, heap, get_cr3() };
    kernel_memory_mgr::__instance->__mark_used(0, div_roundup(heap, REGION_SIZE));
    for(size_t i = 0; i < mmap->num_entries; i++) { if(mmap->entries[i].type != AVAILABLE) { kernel_memory_mgr::__instance->__mark_used(mmap->entries[i].addr, div_roundup(mmap->entries[i].len, PT_LEN)); if(mmap->entries[i].type == MMIO) __map_mmio_pages(addr_t{ mmap->entries[i].addr }, mmap->entries[i].len); } }
    __instance->__physical_open_watermark = heap;
    __set_kernel_page_settings(addr_t{ &__end }.plus(sizeof(kframe_tag)));
}
void kernel_memory_mgr::enter_frame(uframe_tag *ft) noexcept { this->__active_frame = ft; }
void kernel_memory_mgr::exit_frame() noexcept { this->__active_frame = nullptr; }
addr_t kernel_memory_mgr::copy_kernel_mappings(paging_table target) { return __copy_kernel_page_mapping(addr_t(&__start), div_roundup(static_cast<size_t>(&__end - &__start), PAGESIZE), target); }
addr_t kernel_memory_mgr::allocate_mmio_block(size_t sz)
{
    __lock();
    addr_t result{ nullptr };
    if(uintptr_t phys = __find_and_claim_available_region(sz)) { result = __map_mmio_pages(addr_t{ phys }, div_roundup(region_size_for(sz), PAGESIZE)); __physical_open_watermark = std::max(phys, __physical_open_watermark); }
    __unlock();
    return result;
}
addr_t kernel_memory_mgr::allocate_user_block(size_t sz, addr_t start, size_t align, bool write, bool execute)
{
    addr_t pml4{ __active_frame ? __active_frame->pml4 : get_cr3() };
    __userlock();
    size_t rsz = page_aligned_region_size(start, sz); // allocate to the end of page so the userspace doesn't see kernel data structures
    addr_t result = __kernel_frame_tag->allocate(rsz, align);
    if(!start) start = result;
    if(result && !__map_user_pages(start.page_aligned(), result, div_roundup(rsz, PAGESIZE), pml4, write, execute)) { __kernel_frame_tag->deallocate(result, align); result = nullptr; }
    __userunlock();
    return result;
}
paging_table kernel_memory_mgr::allocate_pt()
{
    const size_t pt_size{ sizeof(pt_entry) * PT_LEN };
    const size_t total_sz = up_to_nearest(pt_size + bt_offset, PAGESIZE);
    const size_t rsz = region_size_for(total_sz * 8);
    uint32_t exp{ get_block_exp(pt_size + bt_offset) - MIN_BLOCK_EXP };
    block_tag* tag{ nullptr };
    for(tag = __kernel_frame_tag->available_blocks[exp]; bool(tag); tag = tag->next)
    {
        tag->align_bytes = add_align_size(tag, PAGESIZE);
        if (tag->available_size() >= pt_size) { __kernel_frame_tag->remove_block(tag); if(!tag->left_split && !tag->right_split) __kernel_frame_tag->complete_pages[exp]--; break; }
        tag->align_bytes = 0ul;
    }
    if(!tag)
    {
        addr_t allocated{ __find_and_claim_available_region(rsz) };
        if(!allocated) return nullptr;
        if(!translate_vaddr(allocated)) { __map_kernel_pages(allocated, rsz / PAGESIZE, true); }
        tag = new (allocated) block_tag{ rsz, 0 };
        __physical_open_watermark = std::max(uintptr_t(allocated), __physical_open_watermark);
    }
    tag->held_size = pt_size;
    tag->align_bytes = add_align_size(tag, PAGESIZE);
    addr_t result = tag->actual_start();
    __builtin_memset(result, 0, PAGESIZE);
    if(tag->available_size() - bt_offset >= (1 << MIN_BLOCK_EXP)) __kernel_frame_tag->insert_block(tag->split(), -1);
    if(result && __active_frame) { bool lk = test_lock(&__heap_mutex); if(lk) __unlock(); __active_frame->pt_blocks.push_back(result); if(lk) __lock(); }
    return result;
}
addr_t kernel_memory_mgr::allocate_kernel_block(size_t sz)
{
    __lock();
    addr_t phys{ __find_and_claim_available_region(sz) };
    addr_t result{ nullptr };
    if(phys) { result = __map_kernel_pages(phys, div_roundup(region_size_for(sz), PAGESIZE), true); __physical_open_watermark = std::max(uintptr_t(phys), __physical_open_watermark); }
    __unlock();
    return result;
}
addr_t kernel_memory_mgr::duplicate_user_block(size_t sz, addr_t start, bool write, bool execute)
{
    addr_t id_map = identity_map_to_kernel(start, sz);
    if(!id_map) return nullptr;
    addr_t pml4{ __active_frame ? __active_frame->pml4 : get_cr3() };
    __lock();
    addr_t result{ nullptr };
    if(uintptr_t result_phys = __find_and_claim_available_region(sz)) { result = __map_user_pages(start, result_phys, div_roundup(region_size_for(sz), PAGESIZE), pml4, write, execute); __builtin_memcpy(result, id_map, sz); }
    __unlock();
    return result;
}
addr_t kernel_memory_mgr::identity_map_to_kernel(addr_t start, size_t sz)
{
    uintptr_t phys = translate_vaddr_in_current_frame(start);
    if(!phys) return nullptr;
    // Identity-mapping of pages in kernel space is on an as-needed basis; these mappings are never removed, but are not pinned globally as the kernel's space is
    __lock();
    uframe_tag* fr = __active_frame;
    if(fr) exit_frame();
    addr_t result{ __map_kernel_pages(addr_t{ phys }, div_roundup(sz, PAGESIZE), false) };
    if(fr) enter_frame(fr);
    __unlock();
    if(result) return addr_t(phys);
    return nullptr;
}
addr_t kernel_memory_mgr::identity_map_to_user(addr_t what, size_t sz, bool write, bool execute)
{
    if(!__active_frame) return nullptr;
    __lock();
    addr_t result = __map_user_pages(what, what, div_roundup(sz, PAGESIZE), __active_frame->pml4, write, execute);
    __unlock();
    return result;
}
void kframe_tag::__lock() { lock(&__my_mutex); }
void kframe_tag::__unlock() { release(&__my_mutex); }
block_tag *kframe_tag::__create_tag(size_t size, size_t align)
{
    size_t actual_size = std::max(size + bt_offset, align) + align;
    addr_t allocated{ kernel_memory_mgr::get().allocate_kernel_block(actual_size) };
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
addr_t kframe_tag::allocate(size_t size, size_t align)
{
    if(!size) { direct_writeln("W: size zero alloc"); return nullptr; }
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
    if(!tag) { if(!(tag = __create_tag(size, align))) { __unlock(); panic("out of memory"); debug_print_num(kernel_memory_mgr::get().open_wm()); debug_print_num(size); return nullptr; } idx = get_block_exp(tag->allocated_size()) - MIN_BLOCK_EXP; }
    tag->index = idx;
    if(tag->available_size() - sizeof(block_tag) >= (1 << MIN_BLOCK_EXP)) insert_block(tag->split(), -1);
    __unlock();
    return tag->actual_start();
}
void kframe_tag::deallocate(addr_t ptr, size_t align)
{
    if(ptr)
    {
        __lock();
        block_tag* tag{ ptr - bt_offset };
        for(size_t i = 0; tag && tag->magic != BLOCK_MAGIC; i++) tag = ptr - ptrdiff_t(bt_offset + i);
        if(tag->magic == BLOCK_MAGIC)
        {
            tag->held_size = 0;
            tag->align_bytes = 0;
            while(tag->left_split && (tag->left_split->index >= 0)) tag = __melt_left(tag);
            while(tag->right_split && (tag->right_split->index >= 0)) tag = __melt_right(tag);
            unsigned int idx = get_block_exp(tag->allocated_size()) - MIN_BLOCK_EXP;
            if((!tag->left_split && !tag->right_split) && complete_pages[idx] >= MAX_COMPLETE_PAGES) { kernel_memory_mgr::get().deallocate_block(tag, tag->block_size); __unlock(); return; }
            insert_block(tag, idx);
        }
        __unlock();
    }
}
addr_t kframe_tag::reallocate(addr_t ptr, size_t size, size_t align)
{
    if(!ptr) return allocate(size, align);
    if(!size) { direct_writeln("W: size zero alloc"); return nullptr; }
    block_tag* tag{ ptr - bt_offset };
    for(size_t i = 0; tag->magic != BLOCK_MAGIC; i++) tag = ptr.minus(bt_offset + i);
    if(tag->magic == BLOCK_MAGIC && tag->allocated_size() >= size + add_align_size(tag, align))
    {
        tag->align_bytes = add_align_size(tag, align);
        size_t delta = size - (tag->held_size + tag->align_bytes);
        tag->held_size += delta;
        return tag->actual_start();
    }
    addr_t result{ allocate(size, align) };
    if(result) { __builtin_memcpy(result, ptr, tag->held_size > size ? size : tag->held_size); }
    deallocate(ptr);
    return result;
}
addr_t kframe_tag::array_allocate(size_t num, size_t size)
{
    addr_t result{ allocate(num * size, size) };
    if(result) __builtin_memset(result, 0, num * size);
    return result;
}
block_tag *block_tag::split()
{
    block_tag* that{ this->actual_start().plus(this->held_size) };
    new (that) block_tag{ available_size(), 0, -1, this, this->right_split };
    if(that->right_split) that->right_split->left_split = that;
    this->right_split = that;
    this->block_size -= that->block_size;
    return that;
}
void uframe_tag::__lock() { lock(&__my_mutex); }
void uframe_tag::__unlock() { release(&__my_mutex); }
uframe_tag::~uframe_tag() { for(block_descr blk : usr_blocks) kernel_memory_mgr::get().deallocate_block(blk.start, blk.size, true); for(addr_t addr : pt_blocks) __kernel_frame_tag->deallocate(addr); }
bool uframe_tag::shift_extent(ptrdiff_t amount)
{
    if(amount == 0) return true; // nothing to do, vacuous success
    if(amount < 0)
    {
        uintptr_t amt_freed = -amount;
        if(extent > amt_freed)
        {
            addr_t target = extent + amount;
            std::vector<block_descr>::reverse_iterator i = usr_blocks.rend();
            while(i != usr_blocks.rbegin() && target < i->start) { kernel_memory_mgr::get().deallocate_block(i->start, i->size, true); i++; }
            bool nrem = (i != usr_blocks.rbegin());
            usr_blocks.erase(i.base(), usr_blocks.end());
            if(!nrem) { size_t nsz = region_size_for(uint64_t(target)); extent = base.plus(nsz); addr_t allocated = kernel_memory_mgr::get().allocate_user_block(nsz, base); if(!allocated) { extent = nullptr; return false; } usr_blocks.emplace_back(allocated, nsz); return true; }
            addr_t nst = usr_blocks.back().start.plus(usr_blocks.back().size);
            size_t nsz = region_size_for(target - nst);
            extent = nst;
            addr_t allocated = kernel_memory_mgr::get().allocate_user_block(nsz, nst);
            if(!allocated) return false;
            usr_blocks.emplace_back(allocated, nsz);
            extent += nsz;
            return true; 
        }
        else return false;
    }
    size_t added{ region_size_for(static_cast<size_t>(amount)) };
    addr_t allocated = kernel_memory_mgr::get().allocate_user_block(added, extent);
    if(allocated) { usr_blocks.emplace_back(allocated, added); extent += added; if(mapped_max < extent) mapped_max = extent; return true; }
    return false;
}
addr_t uframe_tag::mmap_add(addr_t addr, size_t len, bool write, bool exec)
{
    if(addr_t result = kernel_memory_mgr::get().allocate_user_block(len, addr.page_aligned(), PAGESIZE, write, exec)) 
    {
        if(!addr) addr = result;
        usr_blocks.emplace_back(result, kernel_memory_mgr::page_aligned_region_size(addr, len));
        array_zero<uint8_t>(result, len);
        if(result.plus(len )> mapped_max) mapped_max = result.plus(len).page_aligned().plus((result.plus(len) % PAGESIZE) ? PAGESIZE : 0L);
        return result;
    }
    return addr_t{ uintptr_t(-ENOMEM) };
}
extern "C"
{ 
    uintptr_t translate_vaddr(addr_t addr) { if(paging_table pt = __find_table(addr)) return (pt[addr.page_idx].physical_address << 12) | addr.offset; else return 0; }
    addr_t translate_user_pointer(addr_t ptr) { uframe_tag* ctask_frame = current_active_task()->frame_ptr; if(ctask_frame->magic != UFRAME_MAGIC) return nullptr; kernel_memory_mgr::get().enter_frame(ctask_frame); addr_t result{ kernel_memory_mgr::get().translate_vaddr_in_current_frame(ptr) }; kernel_memory_mgr::get().exit_frame(); return result; }
    addr_t syscall_sbrk(ptrdiff_t incr)
    {
        uframe_tag* ctask_frame = current_active_task()->frame_ptr;
        if(ctask_frame->magic != UFRAME_MAGIC) return addr_t{ uintptr_t(-EINVAL) };
        kernel_memory_mgr::get().enter_frame(ctask_frame);
        addr_t result = ctask_frame->extent;
        bool success = ctask_frame->shift_extent(incr);
        kernel_memory_mgr::get().exit_frame();
        if(success) { return result; }
        else return addr_t{ uintptr_t(-ENOMEM) };
    }
    addr_t syscall_mmap(addr_t addr, size_t len, int prot, int flags, int fd, ptrdiff_t offset)
    {
        uframe_tag* ctask_frame = current_active_task()->frame_ptr;
        if(ctask_frame->magic != UFRAME_MAGIC || !len || size_t(offset) > len || offset % PAGESIZE) return addr_t{ uintptr_t(-EINVAL) };
        if(!prot) return nullptr;
        addr_t min(std::max(mmap_min_addr, ctask_frame->mapped_max.val()));
        if(min != min.page_aligned()) min = min.plus(PAGESIZE).page_aligned();
        if(!(flags & MAP_FIXED)) addr = std::max(min, addr).page_aligned();
        else if(addr && (addr < min || addr != addr.page_aligned())) return addr_t{ uintptr_t(-EINVAL) };
        kernel_memory_mgr::get().enter_frame(ctask_frame);
        addr_t result = ctask_frame->mmap_add(addr, len, prot & PROT_WRITE, prot & PROT_READ);
        kernel_memory_mgr::get().exit_frame();
        if(!(flags & MAP_ANONYMOUS))
        {
            filesystem* fsptr = get_fs_instance();
            if(!fsptr) return addr_t{ uintptr_t(-ENOSYS) };
            else try 
            { 
                file_inode* n = get_by_fd(fsptr,current_active_task()->self, fd);
                if(n)
                {
                    size_t data_len = std::min(size_t(len - offset), n->size());
                    file_inode::pos_type pos = n->tell();
                    n->seek(offset, std::ios_base::beg);
                    n->read(result, data_len);
                    n->seek(pos);
                    return result;
                }
            } 
            catch(std::exception& e) { panic(e.what()); }
            return addr_t{ uintptr_t(-EBADF) };
        }
        return result;
    }
    int syscall_munmap(addr_t addr, size_t len)
    {
        uframe_tag* ctask_frame = current_active_task()->frame_ptr;
        if(ctask_frame->magic != UFRAME_MAGIC) return addr_t{ uintptr_t(-EINVAL) };
        if(addr > ctask_frame->mapped_max) return 0;
        len = std::min(len, size_t(ctask_frame->mapped_max - addr));
        __unmap_pages(addr, truncate(len, PAGESIZE), ctask_frame->pml4);
        return 0;
    }
}