#include "kernel/heap_allocator.hpp"
#include "heap_allocator.hpp"
extern "C"
{
    extern unsigned char __end;
    frame_tag* __kernel_frame_tag = reinterpret_cast<frame_tag*>(&__end);
}
static uint8_t __heap_allocator_data_loc[sizeof(heap_allocator)];
heap_allocator* heap_allocator::__instance;
static constexpr inline unsigned int log2trunc(uint64_t i)
{
    if(i < 4) return 2u;
    for(size_t j = 2; j < MAX_BLOCK_EXP; j++) if(static_cast<uint64_t>(1ull << j) > i) return j;
    return MAX_BLOCK_EXP;
}
static constexpr inline uint64_t div_roundup(size_t num, size_t denom)
{
    return (num % denom == 0) ? (num / denom) : (1 + (num / denom));
}
static constexpr inline block_size nearest(size_t sz) 
{
    return sz <= S04 ? S04 
                : sz <= S08 ? S08
                    : sz <= S16 ? S16
                        : sz <= S32 ? S32
                            : sz <= S64 ? S64
                                : sz <= S128 ? S128
                                    : sz <= S256 ? S256 
                                        : S512;  
}
static constexpr inline size_t how_many_status_arrays(size_t mem_size) 
{
    return div_roundup(mem_size, GIGABYTE);
}
static constexpr inline uint64_t truncate(uint64_t n, uint64_t unit)
{
    return (n % unit == 0) ? n : n - (n % unit);
}
static constexpr inline uint64_t up_to_nearest(uint64_t n, uint64_t unit)
{
    return (n % unit == 0) ? n : truncate(n + unit, unit);
}
static inline uintptr_t block_offset(uintptr_t addr, block_idx idx)
{
    switch(idx)
    {
        case I7:
            addr += S04;
        case I6:
            addr += S08;
        case I5:
            addr += S16;
        case I4:
            addr += S32;
        case I3:
            addr += S64;
        case I2:
            addr += S128;
        case I1:
            addr += S256;
        default:
            return addr;
    }
}
void heap_allocator::__mark_used(uintptr_t addr_start, size_t num_regions)
{
    for(size_t i = 0; i < num_regions; i++, addr_start += REGION_SIZE) __get_sb(addr_start)->set_used(ALL);
}
uintptr_t heap_allocator::__find_claim_avail_region(size_t sz)
{
    uintptr_t addr = up_to_nearest(__physical_open_watermark, REGION_SIZE);
    uintptr_t result = 0;
    size_t regions = 0;
    for(size_t rem = sz; status_byte::gb_of(addr) < __num_status_bytes && rem > 0; addr += REGION_SIZE)
    {
        block_size bs = nearest(rem);
        status_byte& sb = *__get_sb(addr);
        if(!sb[bs])
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
                if(sb[I7]) 
                {
                    sb.set_used(I7);
                    if(result == 0) return block_offset(addr, I7);
                }
                else 
                {
                    sb.set_used(I6);
                    if(result == 0) return block_offset(addr, I6);
                }
                __mark_used(result, regions);
                return result;
            }
            else if (bs == S512)
            {
                if(result == 0) result = addr;
                regions++;
                if(rem <= S512)
                {
                    __mark_used(result, regions);
                    return result;
                }
                rem -= S512;
            }
            else 
            {
                block_idx idx = (bs == S08 ? I5 : (bs == S16 ? I4 : (bs == S32 ? I3 : (bs == S64 ? I2 : (bs == S128 ? I1 : I0)))));
                sb.set_used(idx);
                if(result == 0) return block_offset(addr, idx);
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
    for(size_t rem = sz; rem > 0; rem -= (bs > rem ? rem : bs))
    {
        bs = nearest(rem);
        uint64_t offs = start % REGION_SIZE;
        if(bs == S04)
        {
            if(offs > I6 * PAGESIZE)
            {
                __status(start).set_free(I7);
            }
            else
            {
                __status(start).set_free(I6);
            }
        }
        else if(bs == S512)
        {
            __status(start).set_free(ALL);
        }
        else
        {
            block_idx idx = (bs == S08 ? I5 : (bs == S16 ? I4 : (bs == S32 ? I3 : (bs == S64 ? I2 : (bs == S128 ? I1 : I0)))));
            __status(start).set_free(idx);
        }
    }
}
uintptr_t heap_allocator::__new_page_table_block()
{
    for(uintptr_t addr = __kernel_frame_tag->next_vaddr; addr < MMAP_MAX_PG*PAGESIZE; addr += REGION_SIZE) 
    {
        if(__status(addr).all_free())
        {
            __get_sb(addr)->set_used(ALL);
            return addr;
        }
    }
    return 0;
}
void heap_allocator::__lock()
{
    while(acquire(&__heap_mutex)) PAUSE;
}
void heap_allocator::__unlock()
{
    release(&__heap_mutex);
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
    for(size_t i = 0; i < mmap->num_entries; i++)
    {
        if(mmap->entries[i].type != AVAILABLE) 
        {
            heap_allocator::__instance->__mark_used(mmap->entries[i].addr, div_roundup(mmap->entries[i].len, PT_LEN));
        }
    }
}
paging_table heap_allocator::allocate_pt()
{
    const size_t pt_size = sizeof(pt_entry) * PT_LEN;
    uint32_t exp = log2trunc(REGION_SIZE);
    block_tag* tag;
    for(tag = __kernel_frame_tag->available_blocks[exp]; tag != NULL; tag = tag->next) 
    {
        if (tag->available_size() >= pt_size)
        {
            __kernel_frame_tag->remove_block(tag);
            if(tag->left_split == NULL && tag->right_split == NULL) __kernel_frame_tag->complete_pages[exp]--;
            break;
        }
    }
    if(tag == NULL)
    {
        vaddr_t allocated = __new_page_table_block();
        if(!allocated)
        {
            return NULL;
        }
        tag = new (allocated) block_tag{ REGION_SIZE, 0 };
    }
    vaddr_t base { tag };
    uintptr_t adjusted = up_to_nearest(base + (ptrdiff_t)pt_size, PAGESIZE);
    tag->held_size = static_cast<size_t>(adjusted - static_cast<uintptr_t>(base));
    vaddr_t result { adjusted };
    uint32_t rem = tag->available_size();
    if(rem > (PAGESIZE*MIN_BLOCK_SIZE)) 
    {
        __kernel_frame_tag->insert_block(tag->split(), -1);
    }
    return result;
}
vaddr_t heap_allocator::allocate_block(vaddr_t const &base, size_t sz, uint64_t align)
{
    __lock();
    vaddr_t aligned { static_cast<uintptr_t>(up_to_nearest(base + (ptrdiff_t)sizeof(block_tag), align > PAGESIZE ? align : PAGESIZE) - sizeof(block_tag)) };
    uintptr_t phys = __find_claim_avail_region(sz);
    __unlock();
    if(phys) return sys_mmap(aligned, phys, div_roundup(sz, PAGESIZE));
    return 0ul;
}
void heap_allocator::deallocate_block(vaddr_t const &base, size_t sz)
{
    __lock();
    uintptr_t phys = sys_unmap(base, div_roundup(sz, PAGESIZE));
    __release_claimed_region(sz, phys);
    __unlock();
}
void frame_tag::insert_block(block_tag *blk, int idx)
{
    blk->index = idx < 0 ? log2trunc(blk->block_size) - 1 : idx;
    if (available_blocks[blk->index] != NULL)
    {
        blk->next = available_blocks[blk->index];
        available_blocks[blk->index]->previous = blk;
    }
    available_blocks[blk->index] = blk;
}
void frame_tag::remove_block(block_tag *blk)
{
    if(available_blocks[blk->index] == blk) available_blocks[blk->index] = blk->next;
    if(blk->previous) blk->previous->next = blk->next;
    if(blk->next) blk->next->previous = blk->previous;
    blk->next = NULL;
    blk->previous = NULL;
    blk->index = -1;
}
constexpr static inline size_t add_align_size(block_tag* tag, size_t align)
{
    return align ? (up_to_nearest(std::bit_cast<uintptr_t>(tag) + sizeof(block_tag), align) - std::bit_cast<uintptr_t>(tag)) : 0;
}
vaddr_t frame_tag::allocate(size_t size, size_t align)
{
    if(!size) return {};
    __lock();
    uint32_t idx = log2trunc(size);
    block_tag* tag;
    vaddr_t result;
    for(tag = available_blocks[idx]; tag != NULL; tag = tag->next)
    {
        if(tag->available_size() - add_align_size(tag, align) >= size)
        {
            remove_block(tag);
            result = tag;
            if(align)
            {
                tag->held_size = size + add_align_size(tag, align) - sizeof(block_tag);
                result += add_align_size(tag, align);
            }
            else 
            {
                tag->held_size = size;
                result += sizeof(block_tag);
            }
            if(tag->left_split == NULL && tag->right_split == NULL) complete_pages[idx]--;
            break;
        }
    }
    if(!tag)
    {
        if(!(tag = __create_tag(size, align))) return nullptr;
        idx = log2trunc(tag->block_size - sizeof(block_tag));
        result = vaddr_t { tag } + (ptrdiff_t)sizeof(block_tag);
    }
    tag->index = idx;
    if((tag->available_size() - sizeof(block_tag)) >= MIN_BLOCK_SIZE*PAGESIZE) insert_block(tag->split(), -1);
    __unlock();
    return result;
}
void frame_tag::deallocate(vaddr_t ptr, size_t align)
{
    if(ptr)
    {
        __lock();
        block_tag* tag = (ptr - (ptrdiff_t)sizeof(block_tag));
        for(size_t i = 0; i < align && tag->magic != BLOCK_MAGIC; i++)
        {
            tag = (ptr - (ptrdiff_t)(sizeof(block_tag) + i));
        }
        if(tag->magic == BLOCK_MAGIC)
        {
            while(tag->left_split && (tag->left_split->index >= 0)) tag = __melt_left(tag);
            while(tag->right_split && (tag->right_split->index >= 0)) tag = __melt_right(tag);
            unsigned int idx = log2trunc(tag->allocated_size());
            if(!tag->left_split && !tag->right_split)
            {
                if(complete_pages[idx] >= MAX_COMPLETE_PAGES)
                {
                    heap_allocator::get().deallocate_block(tag, tag->block_size);
                }
                else
                {
                    complete_pages[idx]++;
                    insert_block(tag, idx);
                }
            }
            else
            {
                insert_block(tag, idx);
            }
        }
        __unlock();
    }
}
vaddr_t frame_tag::reallocate(vaddr_t ptr, size_t size, size_t align)
{
    if(!ptr) return allocate(size, align);
    if(!size) return {};
    block_tag* tag = (ptr - (ptrdiff_t)sizeof(block_tag));
    if(tag->magic == BLOCK_MAGIC && tag->block_size >= size)
    {
        size_t delta = size - tag->held_size;
        tag->held_size += delta;
        if((tag->available_size() - sizeof(block_tag)) >= MIN_BLOCK_SIZE*PAGESIZE) insert_block(tag->split(), -1);
        return ptr;
    }
    vaddr_t result = allocate(size, align);
    if(result)
    {
        if(align == 2 && size % 2 == 0)
        {
            swcopy(result, ptr, tag->held_size > size ? size : tag->held_size);
        }
        else if(align == 4 && size % 4 == 0)
        {
            slcopy(result, ptr, tag->held_size > size ? size : tag->held_size);
        }
        else if(align % 8 == 0 && size % 8 == 0)
        {
            sqcopy(result, ptr, tag->held_size > size ? size : tag->held_size);
        }
        else
        {
            sbcopy(result, ptr, tag->held_size > size ? size : tag->held_size);
        }
    }
    deallocate(ptr);
    return result;
}
vaddr_t frame_tag::array_allocate(size_t num, size_t size)
{
    vaddr_t result = allocate(num * size, size);
    if(result)
    {
        switch(size)
        {
            case 2:
                awset(result, 0, num);
                break;
            case 4:
                alset(result, 0, num);
                break;
            case 8:
                aqset(result, 0, num);
                break;
            default:
                abset(result, 0, num * size);
                break;
        }
    }
    return result;
}
constexpr inline size_t region_size_for(size_t sz)
{
    return sz > S512 ? (truncate(sz, S512) + nearest(sz % S512)) : nearest(sz);
}
block_tag *frame_tag::__create_tag(size_t size, size_t align)
{
    while(!heap_allocator::get().avail()) PAUSE;
    vaddr_t allocated = heap_allocator::get().allocate_block(next_vaddr, size, align);
    if(!allocated) return nullptr;
    size_t full = region_size_for(size);
    next_vaddr += full;
    return new (allocated) block_tag { full, size };
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
void frame_tag::__lock()
{
    while(acquire(&__my_mutex)) PAUSE;
}
void frame_tag::__unlock()
{
    release(&__my_mutex);
}
block_tag *block_tag::split()
{
    uint32_t rem = available_size();
    block_tag* that = reinterpret_cast<block_tag*>(std::bit_cast<uintptr_t>(this) + sizeof(block_tag) + held_size);
    new (that) block_tag{rem, 0, -1, this, this->right_split };
    if(that->right_split) that->right_split->left_split = that;
    this->right_split = that;
    this->block_size -= that->block_size;
    return that;
}