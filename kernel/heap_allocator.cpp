#include "kernel/heap_allocator.hpp"
#include "heap_allocator.hpp"
extern "C"
{
    unsigned char __end;
    frame_tag* __kernel_frame_tag = reinterpret_cast<frame_tag*>(&__end);
}
constexpr inline unsigned char* __end_of_tag = &__end + sizeof(frame_tag);
constexpr inline gb_status* __the_status_bytes = reinterpret_cast<gb_status*>(__end_of_tag);
static uint8_t __heap_allocator_data_loc[sizeof(heap_allocator)];
heap_allocator* const heap_allocator::__instance = reinterpret_cast<heap_allocator*>(__heap_allocator_data_loc);
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
                rem -= S512;
                regions++;
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

void heap_allocator::init_instance(pagefile *pagefile, mmap_t *mmap)
{
    size_t n = how_many_status_arrays(mmap->total_memory);
    uintptr_t heap = reinterpret_cast<uintptr_t>(__end_of_tag) + n * sizeof(gb_status);
    new (__the_status_bytes) gb_status[n];
    new (__kernel_frame_tag) frame_tag{ pagefile->boot_entry, heap };
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
    // TODO
    return 0;
}

vaddr_t heap_allocator::allocate_block(vaddr_t const &base, size_t sz, uint64_t align)
{
    vaddr_t aligned { align != 0 ? up_to_nearest(base, align) : static_cast<uintptr_t>(base) };
    uintptr_t phys = __find_claim_avail_region(sz);
    if(phys) return sys_mmap(aligned, phys, div_roundup(sz, PAGESIZE));
    return 0ul;
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

block_tag *block_tag::split()
{
    uint32_t rem = block_size - sizeof(block_tag) - held_size;
    block_tag* that = reinterpret_cast<block_tag*>(std::bit_cast<uintptr_t>(this) + sizeof(block_tag) + held_size);
    new (that) block_tag{rem, 0, -1, this, this->right_split };
    if(that->right_split) that->right_split->left_split = that;
    this->right_split = that;
    this->block_size -= that->block_size;
    return that;
}
