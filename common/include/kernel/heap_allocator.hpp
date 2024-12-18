#ifndef __HEAP_ALLOC
#define __HEAP_ALLOC
#include "kernel/libk_decls.h"
#ifndef MAX_BLOCK_EXP
#define MAX_BLOCK_EXP 32
#endif
#ifndef MIN_BLOCK_SIZE
#define MIN_BLOCK_SIZE 4 // This times 2^(index) is the number of pages in a block at that index in a frame 
#endif
// 64 bits means new levels of l33tpuns! Now featuring Pokemon frustrations using literal suffixes.
constexpr uint64_t BLOCK_MAGIC = 0xB1600FBA615FULL;
// Of course, we wouldn't want to offend anyone, so... (the 7s are T's...don't judge me >.<)
constexpr uint64_t FRAME_MAGIC = 0xD0BE7AC7FUL;
struct block_tag
{
    uint64_t magic { BLOCK_MAGIC };
    size_t block_size;
    size_t held_size;
    uint32_t index;
    block_tag* left_split { nullptr };
    block_tag* right_split { nullptr };
    block_tag* previous { nullptr };
    block_tag* next { nullptr };
    constexpr block_tag() = default;
    constexpr block_tag(size_t size, size_t held, uint32_t idx, block_tag* left, block_tag* right, block_tag* prev = nullptr, block_tag* nxt = nullptr) noexcept :
        block_size      { size },
        held_size       { held },
        index           { idx },
        left_split      { left },
        right_split     { right },
        previous        { prev },
        next            { nxt }
                        {}
    constexpr block_tag(size_t size, size_t held, uint32_t idx) noexcept : block_size{ size }, held_size{ held }, index { idx } {}
} __pack;
struct frame_tag
{
    uint64_t magic { FRAME_MAGIC };
    page_frame* the_frame;
    vaddr_t next_vaddr {};
    uint16_t complete_pages[MAX_BLOCK_EXP] {};
    block_tag* available_blocks[MAX_BLOCK_EXP] {};
    frame_tag* previous { nullptr };
    frame_tag* next { nullptr };
    constexpr frame_tag() = default;
    constexpr frame_tag(page_frame* frame, uintptr_t vaddr_next = 0, frame_tag* prev = nullptr, frame_tag* nxt = nullptr) noexcept :
        the_frame{ frame }, 
        next_vaddr{ vaddr_next },
        previous{ prev }, 
        next{ nxt } {}
} __pack;
enum block_idx : uint8_t
{
    I0 = 0,
    I1 = 1,
    I2 = 2,
    I3 = 3,
    I4 = 4,
    I5 = 5,
    I6 = 6,
    I7 = 7,
    ALL = 8
};
enum block_size : uint32_t
{
    S512 = 512*PAGESIZE,
    S256 = 256*PAGESIZE,
    S128 = 128*PAGESIZE,
    S64  = 64*PAGESIZE,
    S32  = 32*PAGESIZE,
    S16  = 16*PAGESIZE,
    S8   = 8*PAGESIZE,
    S04   = 4*PAGESIZE
};
/*
 *  Each 512-page region of physical memory is divided into the following blocks:
 *  [256P] B0: |< 000 - 255 >|
 *  [128P] B1: |< 256 - 383 >|
 *  [064P] B2: |< 383 - 447 >|
 *  [032P] B3: |< 448 - 479 >|
 *  [016P] B4: |< 480 - 495 >|
 *  [008P] B5: |< 496 - 503 >|
 *  [004P] B6: |< 504 - 507 >|
 *  [004P] B7: |< 508 - 511 >|
 */
typedef struct mem_status_byte
{
    uint8_t byte : 8;
    constexpr bool all_free() const noexcept
    {
        return byte == 0;
    }
    constexpr bool has_free(block_idx i) const noexcept
    {
        return i == ALL ? all_free() : ((byte & (1 << i)) == 0);
    }
    constexpr bool all_used() const noexcept
    {
        return byte == 0xFF;
    }
    constexpr void set_used(block_idx i) noexcept 
    {
        byte |= (i == ALL ? 0xFF : (1 << i));
    }
    constexpr void set_free(block_idx i) noexcept
    {
        byte &= (i == ALL ? 0 : ~(1 << i));
    }
    constexpr bool operator[](block_idx i) const noexcept
    {
        return has_free(i);
    }
    constexpr bool operator[](block_size i) const noexcept
    {
        return i <= S04 ? has_free(I6) || has_free(I7) : has_free(i <=   S8 ? I5 
                                                                : i <=  S16 ? I4 
                                                                : i <=  S32 ? I3
                                                                : i <=  S64 ? I2 
                                                                : i <= S128 ? I1
                                                                : i <= S256 ? I0 
                                                                : ALL);
    }
    constexpr operator bool() const noexcept
    {
        return !all_used();
    }
    constexpr bool operator!() const noexcept
    {
        return all_used();
    }
} __align(1) __pack status_byte, gb_status[512];
class heap_allocator
{
    frame_tag __kernel_frame;
    pagefile* __my_pagefile;
    uint16_t __active_frame_index;
    // stuff
};
#endif