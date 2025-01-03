#include "kernel/heap_allocator.hpp"
#include "direct_text_render.hpp"
#include "heap_allocator.hpp"
extern "C"
{
    extern unsigned char __end;
    frame_tag* __kernel_frame_tag = reinterpret_cast<frame_tag*>(&__end);
}
static uint8_t __heap_allocator_data_loc[sizeof(heap_allocator)];
void debug_print_num(uintptr_t num, int lenmax = 16);
extern "C" void direct_write(const char* str);
heap_allocator* heap_allocator::__instance;
constexpr uint32_t get_block_exp(uint64_t size) { if(size < (1ull << MIN_BLOCK_EXP)) return  MIN_BLOCK_EXP; for(size_t j =  MIN_BLOCK_EXP; j < MAX_BLOCK_EXP; j++) if(static_cast<uint64_t>(1ull << j) > size) return j; return MAX_BLOCK_EXP - 1; }
constexpr uint64_t div_roundup(size_t num, size_t denom) { return (num % denom == 0) ? (num / denom) : (1 + (num / denom)); }
constexpr block_size nearest(size_t sz)  { return sz <= S04 ? S04 : sz <= S08 ? S08 : sz <= S16 ? S16 : sz <= S32 ? S32 : sz <= S64 ? S64 : sz <= S128 ? S128 : sz <= S256 ? S256 : S512; }
constexpr size_t how_many_status_arrays(size_t mem_size) { return div_roundup(mem_size, GIGABYTE); }
constexpr uint64_t truncate(uint64_t n, uint64_t unit) { return (n % unit == 0) ? n : n - (n % unit); }
constexpr uint64_t up_to_nearest(uint64_t n, uint64_t unit) { return (n % unit == 0) ? n : truncate(n + unit, unit); }
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
constexpr size_t region_size_for(size_t sz) { return sz > S512 ? (truncate(sz, S512) + nearest(sz % S512)) : nearest(sz); }
constexpr size_t add_align_size(block_tag* tag, size_t align) { return align > 1 ? (up_to_nearest(std::bit_cast<uintptr_t>(tag) + sizeof(block_tag), align) - (std::bit_cast<uintptr_t>(tag) + sizeof(block_tag))) : 0; }
void heap_allocator::__lock() { lock(&__heap_mutex); }
void heap_allocator::__unlock() { release(&__heap_mutex); }
void heap_allocator::__mark_used(uintptr_t addr_start, size_t num_regions) { for(size_t i = 0; i < num_regions; i++, addr_start += REGION_SIZE) __get_sb(addr_start)->set_used(ALL); }
uintptr_t heap_allocator::__new_page_table_block() { for(uintptr_t addr = __kernel_frame_tag->next_vaddr; addr < MMAP_MAX_PG*PAGESIZE; addr += REGION_SIZE) if(__status(addr).all_free()) { __get_sb(addr)->set_used(ALL); return addr; } return 0; }
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
                if(sb[I7]) { sb.set_used(I7); if(!multi) { return block_offset(addr, I7); } }
                else { sb.set_used(I6); if(!multi) { return block_offset(addr, I6); } }
                __mark_used(result, regions);
                return result;
            }
            else if (bs == S512)
            {
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
                        sb.set_used(I7);
                        sb.set_used(I6);
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
        uint64_t offs = start % REGION_SIZE;
        if(bs == S04) { if(offs > I6 * PAGESIZE) { __status(start).set_free(I7); } else { __status(start).set_free(I6); } }
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
                    sb.set_free(I7);
                    sb.set_free(I6);
                default:
                    break;
            }
        }
        else __status(start).set_free((bs == S08 ? I5 : (bs == S16 ? I4 : (bs == S32 ? I3 : (bs == S64 ? I2 : (bs == S128 ? I1 : I0))))));
    }
}
void heap_allocator::init_instance(pagefile *pagefile, mmap_t *mmap)
{
    gb_status* __the_status_bytes = std::bit_cast<gb_status*>(reinterpret_cast<uintptr_t>(&__end) + sizeof(frame_tag));
    __instance = reinterpret_cast<heap_allocator*>(__heap_allocator_data_loc);
    size_t n = how_many_status_arrays(mmap->total_memory);
    uintptr_t heap = reinterpret_cast<uintptr_t>(&__end) + sizeof(frame_tag)+ n * sizeof(gb_status);
    new (__the_status_bytes) gb_status[n];
    new (__kernel_frame_tag) frame_tag{ pagefile->boot_entry, up_to_nearest(heap, PAGESIZE) };
    new (heap_allocator::__instance) heap_allocator{ pagefile, __the_status_bytes, n, heap };
    heap_allocator::__instance->__mark_used(0, div_roundup(heap, REGION_SIZE));
    for(size_t i = 0; i < mmap->num_entries; i++) if(mmap->entries[i].type != AVAILABLE) heap_allocator::__instance->__mark_used(mmap->entries[i].addr, div_roundup(mmap->entries[i].len, PT_LEN));
}
paging_table heap_allocator::allocate_pt()
{
    const size_t pt_size = sizeof(pt_entry) * PT_LEN;
    uint32_t exp = get_block_exp(REGION_SIZE) - MIN_BLOCK_EXP;
    block_tag* tag;
    for(tag = __kernel_frame_tag->available_blocks[exp]; tag != NULL; tag = tag->next) 
    {
        tag->align_bytes = add_align_size(tag, PAGESIZE);
        if (tag->available_size() >= pt_size)
        {
            __kernel_frame_tag->remove_block(tag);
            if(tag->left_split == NULL && tag->right_split == NULL) __kernel_frame_tag->complete_pages[exp]--;
            break;
        }
        tag->align_bytes = 0;
    }
    if(tag == NULL)
    {
        vaddr_t allocated = __new_page_table_block();
        if(!allocated) return NULL;
        tag = new (allocated) block_tag{ REGION_SIZE, 0 };
    }
    tag->held_size = pt_size;
    vaddr_t result = tag->actual_start();
    if(tag->available_size() - sizeof(block_tag) >= (1 << MIN_BLOCK_EXP)) __kernel_frame_tag->insert_block(tag->split(), -1);
    return result;
}
vaddr_t heap_allocator::allocate_block(vaddr_t const &base, size_t sz, uint64_t align)
{
    __lock();
    vaddr_t aligned { static_cast<uintptr_t>(up_to_nearest(base + (ptrdiff_t)sizeof(block_tag), align > PAGESIZE ? align : PAGESIZE) - sizeof(block_tag)) };
    uintptr_t phys = __find_claim_avail_region(sz);
    vaddr_t result = 0uL;
    if(phys) result = sys_mmap(aligned, phys, div_roundup(region_size_for(sz), PAGESIZE));
    __unlock();
    return result;
}
void heap_allocator::deallocate_block(vaddr_t const& base, size_t sz) {  __lock();  if(uintptr_t phys = translate_vaddr(base)){ __release_claimed_region(sz, phys); } __unlock(); }
void frame_tag::__lock() { lock(&__my_mutex); }
void frame_tag::__unlock() { release(&__my_mutex); }
void frame_tag::insert_block(block_tag *blk, int idx) { blk->index = idx < 0 ?(get_block_exp(blk->block_size) - MIN_BLOCK_EXP) : idx; if (available_blocks[blk->index] != NULL) { blk->next = available_blocks[blk->index]; available_blocks[blk->index]->previous = blk; } available_blocks[blk->index] = blk; }
void frame_tag::remove_block(block_tag *blk)
{
    if(available_blocks[blk->index] == blk) available_blocks[blk->index] = blk->next;
    if(blk->previous) blk->previous->next = blk->next;
    if(blk->next) blk->next->previous = blk->previous;
    blk->next = NULL;
    blk->previous = NULL;
    blk->index = -1;
}
vaddr_t frame_tag::allocate(size_t size, size_t align)
{
    if(!size) return nullptr;
    __lock();
    uint32_t idx = get_block_exp(size) - MIN_BLOCK_EXP;
    block_tag* tag = NULL;
    for(tag = available_blocks[idx]; tag != NULL; tag = tag->next)
    {
        if(tag->available_size() >= size + add_align_size(tag, align))
        {
            remove_block(tag);
            tag->held_size = size;
            tag->align_bytes = add_align_size(tag, align);
            if(tag->left_split == NULL && tag->right_split == NULL) complete_pages[idx]--;
            break;
        }
    }
    if(!tag)
    {
        if(!(tag = __create_tag(size, align))) { __unlock(); return nullptr; }
        idx = get_block_exp(tag->allocated_size()) - MIN_BLOCK_EXP;
    }
    tag->index = idx;
    vaddr_t result = tag->actual_start();
    if(tag->available_size() - sizeof(block_tag) >= (1 << MIN_BLOCK_EXP)) insert_block(tag->split(), -1);
    __unlock();
    return result;
}
void frame_tag::deallocate(vaddr_t ptr, size_t align)
{
    if(ptr)
    {
        __lock();
        block_tag* tag = static_cast<block_tag*>(ptr - ptrdiff_t(sizeof(block_tag)));
        for(size_t i = 0; tag->magic != BLOCK_MAGIC; i++) tag = static_cast<block_tag*>(ptr - ptrdiff_t(sizeof(block_tag) + i));
        if(tag->magic == BLOCK_MAGIC)
        {
            tag->held_size = 0;
            tag->align_bytes = 0;
            while(tag->left_split && (tag->left_split->index >= 0)) tag = __melt_left(tag);
            while(tag->right_split && (tag->right_split->index >= 0)) tag = __melt_right(tag);
            unsigned int idx = get_block_exp(tag->allocated_size()) - MIN_BLOCK_EXP;
            if(!tag->left_split && !tag->right_split) { if(complete_pages[idx] >= MAX_COMPLETE_PAGES) { heap_allocator::get().deallocate_block(tag, tag->block_size); } else { complete_pages[idx]++; insert_block(tag, idx); } }
            else { insert_block(tag, idx); }
        }
        __unlock();
    }
}
vaddr_t frame_tag::reallocate(vaddr_t ptr, size_t size, size_t align)
{
    if(!ptr) return allocate(size, align);
    if(!size) return {};
    block_tag* tag = static_cast<block_tag*>(ptr - ptrdiff_t(sizeof(block_tag)));
    for(size_t i = 0; tag->magic != BLOCK_MAGIC; i++) tag = static_cast<block_tag*>(ptr - ptrdiff_t(sizeof(block_tag) + i));
    if(tag->magic == BLOCK_MAGIC && tag->allocated_size() >= size + add_align_size(tag, align))
    {
        tag->align_bytes = add_align_size(tag, align);
        size_t delta = size - (tag->held_size + tag->align_bytes);
        tag->held_size += delta;
        return tag->actual_start();
    }
    vaddr_t result = allocate(size, align);
    if(result) { __builtin_memcpy(result, ptr, tag->held_size > size ? size : tag->held_size); }
    deallocate(ptr);
    return result;
}
vaddr_t frame_tag::array_allocate(size_t num, size_t size)
{
    vaddr_t result = allocate(num * size, size);
    if(result) __builtin_memset(result, 0, num * size);
    return result;
}
block_tag *frame_tag::__create_tag(size_t size, size_t align)
{
    vaddr_t allocated = heap_allocator::get().allocate_block(next_vaddr, size, align);
    if(!allocated) return nullptr;
    size_t full = region_size_for(size);
    next_vaddr += full;
    return new (allocated) block_tag { full, size, -1 };
}
block_tag *frame_tag::__melt_left(block_tag *tag) noexcept
{
    if(!tag->left_split) return tag;
    block_tag* left = tag->left_split;
    left->block_size += tag->block_size;
    left->right_split = tag->right_split;
    if(tag->right_split) tag->right_split->left_split = left;
    remove_block(tag);
    return left;
}
block_tag *frame_tag::__melt_right(block_tag *tag) noexcept
{
    block_tag* right = tag->right_split;
    tag->block_size += right->block_size;
    tag->right_split = right->right_split;
    if(right->right_split) right->right_split->left_split = tag;
    remove_block(right);
    return tag;
}
block_tag *block_tag::split()
{
    using bt_ptr = block_tag*;
    block_tag* that = bt_ptr(this->actual_start() + ptrdiff_t(this->held_size));
    new (that) block_tag { available_size(), 0, -1, this, this->right_split };
    if(that->right_split) that->right_split->left_split = that;
    this->right_split = that;
    this->block_size -= that->block_size;
    return that;
}