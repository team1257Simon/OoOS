#ifndef __HEAP_ALLOC
#define __HEAP_ALLOC
#include "kernel/libk_decls.h"
#include "vector"
#ifndef MAX_BLOCK_EXP
#define MAX_BLOCK_EXP 32U
#endif
#ifndef MIN_BLOCK_EXP
#define MIN_BLOCK_EXP 8U // This times 2^(index) is the full size of a block at that index in a frame 
#endif
#ifndef MAX_COMPLETE_REGIONS
#define MAX_COMPLETE_REGIONS 5U
#endif
#define PROT_READ	0x1		/* Page can be read.  */
#define PROT_WRITE	0x2		/* Page can be written.  */
#define PROT_EXEC	0x4		/* Page can be executed.  */
#define PROT_NONE	0x0		/* Page can not be accessed.  */
/* Sharing types (must choose one and only one of these).  */
#define MAP_SHARED	0x01		/* Share changes.  */
#define MAP_PRIVATE	0x02		/* Changes are private.  */
/* Other flags.  */
#define MAP_FIXED	    0x10
#define MAP_FILE	    0x00
#define MAP_ANONYMOUS   0x20
constexpr unsigned region_cap = MAX_COMPLETE_REGIONS;
constexpr unsigned min_exponent = MIN_BLOCK_EXP;
constexpr unsigned max_exponent = MAX_BLOCK_EXP;
// 64 bits means new levels of l33tpuns! Now featuring Pokemon frustrations using literal suffixes.
constexpr uint64_t block_magic = 0xB1600FBA615FULL;
// Of course, we wouldn't want to offend anyone, so... (the 7s are T's...don't judge me >.<)
constexpr uint64_t kframe_magic = 0xD0BE7AC7FUL;
// Oh yea we did...
constexpr uint64_t uframe_magic = 0xACED17C001B012;
constexpr size_t page_size = PAGESIZE;
constexpr size_t page_table_length = PT_LEN;
constexpr size_t region_size = page_size * page_table_length;
constexpr uintptr_t mmap_min_addr = 0x500000UL;
constexpr size_t block_index_range = max_exponent - min_exponent;
constexpr size_t max_block_index = block_index_range - 1;
constexpr addr_t sysres_base{ 0xFFFF800000000000 };
struct block_tag
{
    uint64_t magic{ block_magic };
    size_t block_size;
    size_t held_size;
    int64_t index;
    block_tag* left_split{ nullptr };
    block_tag* right_split{ nullptr };
    block_tag* previous{ nullptr };
    block_tag* next{ nullptr };
    size_t align_bytes{ 0 };
    constexpr block_tag() = default;
    constexpr block_tag(size_t size, size_t held, int64_t idx, block_tag* left, block_tag* right, block_tag* prev = nullptr, block_tag* nxt = nullptr, size_t align = 0) noexcept :
        block_size      { size },
        held_size       { held },
        index           { idx },
        left_split      { left },
        right_split     { right },
        previous        { prev },
        next            { nxt },
        align_bytes     { align }
                        {}
    constexpr block_tag(size_t size, size_t held, int64_t idx = -1, size_t align = 0) noexcept : block_size{ size }, held_size{ held }, index { idx }, align_bytes { align } {}
    constexpr size_t allocated_size() const noexcept { return block_size - sizeof(block_tag); }
    constexpr size_t available_size() const noexcept { return allocated_size() - (held_size + align_bytes); }
    constexpr addr_t actual_start() const noexcept { return addr_t(this).plus(sizeof(block_tag) + align_bytes); }
    block_tag* split();
} __pack;
struct kframe_tag
{
    uint64_t magic{ kframe_magic };
    uint16_t complete_regions[block_index_range]{};
    block_tag* available_blocks[block_index_range]{};
private:
    spinlock_t __my_mutex{};
public:
    constexpr kframe_tag() = default;
    void insert_block(block_tag* blk, int idx);
    void remove_block(block_tag* blk);
    addr_t allocate(size_t size, size_t align = 0);
    void deallocate(addr_t ptr, size_t align = 0);
    addr_t reallocate(addr_t ptr, size_t size, size_t align = 0);
    addr_t array_allocate(size_t num, size_t size);
private:
    block_tag* __create_tag(size_t size, size_t align);
    block_tag* __melt_left(block_tag* tag) noexcept;
    block_tag* __melt_right(block_tag* tag) noexcept;
    void __lock();
    void __unlock();
} __pack;
struct block_descr
{
    addr_t physical_start;
    addr_t virtual_start;
    size_t size;
    bool write{ true };
    bool execute{ true };
};
struct uframe_tag
{
    uint64_t magic{ uframe_magic };
    paging_table pml4;
    addr_t base;
    addr_t extent;
    addr_t mapped_max;
    addr_t sysres_wm;
    addr_t sysres_extent;
    addr_t dynamic_extent;
    std::vector<addr_t> kernel_allocated_blocks{};
    std::vector<block_descr> usr_blocks{};
private:
    spinlock_t __my_mutex{};
    void __lock();
    void __unlock();
public:
    constexpr uframe_tag(paging_table cr3, addr_t st_base, addr_t st_extent) noexcept : 
        pml4            { cr3 },
        base            { st_base },
        extent          { st_extent },
        mapped_max      { st_extent },
        sysres_wm       { sysres_base },
        sysres_extent   { sysres_base },
        dynamic_extent  { nullptr }
                        {}
    ~uframe_tag();
    bool shift_extent(ptrdiff_t amount);
    addr_t mmap_add(addr_t addr, size_t len, bool write, bool exec);
    addr_t sysres_add(size_t n);
    bool mmap_remove(addr_t addr, size_t len);
    void accept_block(block_descr&& desc);
    void transfer_block(uframe_tag& that, block_descr const& which);
    void drop_block(block_descr const& which);
    friend constexpr std::strong_ordering operator<=>(uframe_tag const& __this, uframe_tag const& __that) noexcept { return __this.pml4 <=> __that.pml4; }
};
enum block_idx : uint8_t
{
    I0  = 0x01,
    I1  = 0x02,
    I2  = 0x04,
    I3  = 0x08,
    I4  = 0x10,
    I5  = 0x20,
    I6  = 0x40,
    I7  = 0x80,
    ALL = 0xFF
};
enum block_size : uint32_t
{
    S512 = 512*PAGESIZE,
    S256 = 256*PAGESIZE,
    S128 = 128*PAGESIZE,
    S64  = 64*PAGESIZE,
    S32  = 32*PAGESIZE,
    S16  = 16*PAGESIZE,
    S08  = 8*PAGESIZE,
    S04  = 4*PAGESIZE
};
#define BS2BI(i) i == S04 ? (I6 | I7) : (i == S08 ? I5 : (i ==  S16 ? I4 : (i ==  S32 ? I3 : (i ==  S64 ? I2 : (i == S128 ? I1 : (i == S256 ? I0 : ALL))))))
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
 *  In addition, a region can be allocated in its entirety as a 512-page block.
 *  Blocks are assigned as follows:
 *      1. When memory is requested (i.e. a frame needs additional blocks), the amount of memory requested is used to determine the block size to allocate.
 *          - If the requested block is larger than 256 pages but smaller than 512 pages, a 512-page block is allocated to be divided later.
 *          - If the requested block is larger than 512 pages, a set of physically-contiguous 512-page blocks is allocated as a single block to be divided later.
 *      2. Physical addresses are assigned in order low-to-high, globally, from regions of conventional memory.
 *      3. An amount of identity-mapped space directly after the kernel is reserved at startup for use with these structures. It is never released.
 *          - This amount depends on how much memory is available; 512 bytes track 1 GB of RAM. The kernel's frame-tag will also go here.
 *          - The pagefile entry for the kernel has been allocated by the bootloader and should be below the kernel in memory. 
 *            Entries generated after startup will come from the kernel heap.
 *      4. Virtual addresses are assigned in order low-to-high per page frame. Each page frame tracks its own address-mapping watermark.
 *          - Because of the extreme size of the address space, it should not be necessary to track released virtual addresses.
 *            Instead, blocks that are released from the frame are unmapped, and if they are reallocated later their virtual address will likely change.
 *      5. The kernel heap is initially allocated and mapped by the bootloader and consists of all addresses between the status byte array
 *         and the address corresponding to the end of the identity-mapped region (1GB by default) at startup.
 */
typedef struct mem_status_byte
{
    uint8_t the_byte : 8;
private:
    constexpr bool __has(uint8_t i) const noexcept { return (the_byte & i) == 0; }
 public:
    constexpr bool all_free() const noexcept { return the_byte == 0; }
    constexpr bool has_free(block_idx i) const noexcept { return __has(i); }
    constexpr bool all_used() const noexcept { return the_byte == 0xFF; }
    constexpr void set_used(block_idx i) noexcept { the_byte |= i; }
    constexpr void set_free(block_idx i) noexcept { the_byte &= ~i; }
    constexpr bool operator[](block_idx i) const noexcept { return has_free(i); }
    constexpr bool operator[](block_size i) const noexcept { if(i == S04) return __has(I7) || __has(I6); return __has(i == S04 ? (I6 | I7) : (i == S08 ? I5 : (i == S16 ? I4 : (i == S32 ? I3 : (i == S64 ? I2 : (i == S128 ? I1 : (i == S256 ? I0 : ALL))))))); }
    constexpr operator bool() const noexcept { return !all_used(); }
    constexpr bool operator!() const noexcept { return all_used(); }
    constexpr static unsigned int gb_of(uintptr_t addr) { return addr / gigabyte; }
    constexpr static unsigned int sb_of(uintptr_t addr) { return (addr / region_size) % 512; }
} __align(1) __pack status_byte, gb_status[512];
class kernel_memory_mgr
{
    spinlock_t __heap_mutex{};                  // Calls to kernel allocations lock this mutex to prevent comodification
    spinlock_t __user_mutex{};                  // Separate mutex for userspace calls because userspace memory will be delegated from kernel blocks
    gb_status* const __status_bytes;            // Array of 512-byte arrays
    size_t const __num_status_bytes;            // Length of said array
    uintptr_t const __kernel_heap_begin;        // Convenience pointer to the end of above array
    addr_t __kernel_cr3;                        // The location of the kernel's top-level paging structure
    uintptr_t __watermark{ 0 };   // Updated when a block is allocated or released; provides a guess as to where to start searching for blocks
    addr_t __suspended_cr3{ nullptr };          // Saved cr3 value for a frame suspended in order to access kernel paging structures
    uframe_tag* __active_frame{ nullptr };
    static kernel_memory_mgr* __instance;
    constexpr kernel_memory_mgr(gb_status* status_bytes, size_t num_status_bytes, uintptr_t kernel_heap_addr, addr_t get_kernel_cr3) noexcept :
        __status_bytes              { status_bytes },
        __num_status_bytes          { num_status_bytes },
        __kernel_heap_begin         { kernel_heap_addr },
        __kernel_cr3                { get_kernel_cr3 }
                                    {}
    constexpr status_byte* __get_sb(uintptr_t addr) { return std::addressof(__status_bytes[status_byte::gb_of(addr)][status_byte::sb_of(addr)]); }
    constexpr status_byte& __status(uintptr_t addr) { return *__get_sb(addr); }
    void __mark_used(uintptr_t addr_start, size_t num_regions);
    uintptr_t __claim_region(uintptr_t addr, block_idx idx);
    uintptr_t __find_and_claim(size_t sz);
    void __release_region(size_t sz, uintptr_t start);
    void __lock();
    void __unlock();
    void __userlock();
    void __userunlock();
    void __suspend_frame() noexcept;
    void __resume_frame() noexcept;
public:
    static void init_instance(mmap_t* mmap);
    static kernel_memory_mgr& get();
    static size_t aligned_size(addr_t start, size_t requested);
    static void suspend_user_frame();
    static void resume_user_frame();
    constexpr uintptr_t open_wm() const { return __watermark; }
    kernel_memory_mgr(kernel_memory_mgr const&) = delete;
    kernel_memory_mgr(kernel_memory_mgr&&) = delete;
    kernel_memory_mgr& operator=(kernel_memory_mgr const&) = delete;
    kernel_memory_mgr& operator=(kernel_memory_mgr&&) = delete;
    void enter_frame(uframe_tag* ft) noexcept;
    void exit_frame() noexcept;
    void map_to_current_frame(std::vector<block_descr> const& blocks);
    paging_table allocate_pt();
    uintptr_t frame_translate(addr_t addr);
    addr_t allocate_kernel_block(size_t sz);
    addr_t allocate_mmio_block(size_t sz);
    addr_t map_mmio_region(uintptr_t addr, size_t sz);
    addr_t allocate_user_block(size_t sz, addr_t start, size_t align = 0UL, bool write = true, bool execute = true);
    addr_t duplicate_user_block(size_t sz, addr_t start, bool write, bool execute);
    addr_t identity_map_to_user(addr_t what, size_t sz, bool write = true, bool execute = true);
    void deallocate_block(addr_t const& base, size_t sz, bool should_unmap = false);
    addr_t copy_kernel_mappings(paging_table target);
};
#define use_kmm(name)
#define kmm kernel_memory_mgr::get()
extern "C" void* aligned_malloc(size_t size, size_t align);
extern "C" addr_t syscall_sbrk(ptrdiff_t incr);                                                             // void* sbrk(ptrdiff_t incr);
extern "C" addr_t syscall_mmap(addr_t addr, size_t len, int prot, int flags, int fd, ptrdiff_t offset);     // void* mmap(void* addr, size_t len, int prot, int flags, ptrdiff_t offset);
extern "C" int syscall_munmap(addr_t addr, size_t len);                                                     // int munmap(void* addr, size_t len);
#endif