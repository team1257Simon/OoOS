#ifndef __HEAP_ALLOC
#define __HEAP_ALLOC
/**
 * OoOS uses a three-stage heap allocation mechanism for managing memory blocks in kernel- and user-space.
 * The first stage is a bitmap allocator that is statically allocated at startup. It allocates regions of pages.
 * A region allocated from the first stage will be sized to a power of 2 in pages (no fewer than 4 pages), or to a nonzero multiple of 512 pages.
 * The second stage is a linked-list tag allocator that divides regions allocated by the first stage into blocks of arbitrary size and alignment.
 * Allocations in kernel-space, other than for page tables (see allocate_pt()) and MMIO/DMA blocks (see allocate_dma()) use the second stage directly.
 * The third stage is a block pool allocation scheme which itself uses heap-allocated structures. Each process in userspace has its own frame tag.
 * That structure delegates calls to the second stage in order to allocate (potentially physically non-contiguous) blocks of memory for processes.
 * Those are in turn mapped to virtual addresses, either as-requested (i.e. mmap) or in increasing order (i.e. sbrk) to build a process' memory space.
 * The functions are matched by stage: memory allocated in a given stage should be freed by the deallocation function matching that stage.
 * Other than the special DMA hooks, each stage has exactly one deallocation function and two or more allocation functions.
 * Module frames, which maintain a special managed block structure to prevent memory leaks in case of a module error, are considered part of stage 2.
 */
#include <libk_decls.h>
#include <vector>
#ifndef MAX_BLOCK_EXP
#define MAX_BLOCK_EXP 32U
#endif
#ifndef MIN_BLOCK_EXP
#define MIN_BLOCK_EXP 8U		// This times 2^(index) is the full size of a block at that index in a frame
#endif
#ifndef MAX_COMPLETE_REGIONS
#define MAX_COMPLETE_REGIONS 5U
#endif
#define PROT_READ	0x1			/* Page can be read.			*/
#define PROT_WRITE	0x2			/* Page can be written.			*/
#define PROT_EXEC	0x4			/* Page can be executed.		*/
#define PROT_NONE	0x0			/* Page can not be accessed.	*/
/* Sharing types (must choose one and only one of these).		*/
#define MAP_SHARED	0x01		/* Share changes.				*/
#define MAP_PRIVATE	0x02		/* Changes are private.			*/
/* Other flags.	*/
#define MAP_FIXED		0x10
#define MAP_FILE		0x00
#define MAP_ANONYMOUS	0x20
constexpr unsigned region_cap		= MAX_COMPLETE_REGIONS;
constexpr unsigned min_exponent		= MIN_BLOCK_EXP;
constexpr unsigned max_exponent		= MAX_BLOCK_EXP;
// 64 bits means new levels of l33tpuns! Now featuring Pokemon frustrations using literal suffixes.
constexpr uint64_t block_magic		= 0xB1600FBA615FULL;
// Of course, we wouldn't want to offend anyone, so... (the 7s are T's...don't judge me >.<)
constexpr uint64_t kframe_magic		= 0xD0BE7AC7FUL;
// Oh yea we did...
constexpr uint64_t uframe_magic		= 0xACED17C001B012;
constexpr size_t page_size			= PAGESIZE;
constexpr size_t page_table_length	= PT_LEN;
constexpr size_t region_size		= page_size * page_table_length;
constexpr addr_t mmap_min_addr		= 0x500000LA;
constexpr size_t block_index_range	= max_exponent - min_exponent;
constexpr size_t max_block_index	= block_index_range - 1Z;
constexpr addr_t sysres_base		= 0xFFFF800000000000LA;
// Describes a memory block in stage 2.
struct block_tag
{
	uint64_t magic			= block_magic;
	size_t block_size;
	size_t held_size;
	block_tag* left_split;
	block_tag* right_split;
	block_tag* previous;
	block_tag* next;
	int index;
	uint32_t align_bytes;
	constexpr block_tag()	= default;
	constexpr block_tag(size_t size, size_t held, int idx, block_tag* left, block_tag* right, block_tag* prev = nullptr, block_tag* nxt = nullptr, uint32_t align = 0U) noexcept :
		block_size	{ size },
		held_size	{ held },
		left_split	{ left },
		right_split	{ right },
		previous	{ prev },
		next		{ nxt },
		index		{ idx },
		align_bytes	{ align }
					{}
	constexpr block_tag(size_t size, size_t held, int idx = -1, uint32_t align = 0U) noexcept :
		block_size	{ size },
		held_size	{ held },
		left_split	{ nullptr },
		right_split	{ nullptr },
		previous	{ nullptr },
		next		{ nullptr },
		index		{ idx },
		align_bytes	{ align }
					{}
	constexpr size_t allocated_size() const noexcept { return block_size - sizeof(block_tag); }
	constexpr size_t aligned_size() const noexcept { return block_size - (align_bytes + sizeof(block_tag)); }
	constexpr size_t available_size() const noexcept { return allocated_size() - (held_size + align_bytes); }
	constexpr addr_t actual_start() const noexcept { return addr_t(this).plus(sizeof(block_tag) + align_bytes); }
	constexpr bool is_free() const noexcept { return this->index >= 0 && !this->held_size; }
	constexpr bool is_split() const noexcept { return left_split || right_split; }
	block_tag* split();
} __pack;
struct kframe_tag
{
	uint64_t magic			= kframe_magic;
	uint16_t complete_regions[block_index_range];
	block_tag* available_blocks[block_index_range];
private:
	spinlock_t __my_mutex;
public:
	constexpr kframe_tag()	= default;
	//	Second-stage allocation. Invoked by kernel-space malloc() and new/new[] calls.
	addr_t allocate(size_t size, size_t align = 0UZ) noexcept;
	//	Second-stage deallocation. Invoked by kernel-space free() and delete/delete[] calls.
	void deallocate(addr_t ptr, size_t align = 0UZ) noexcept;
	//	Second-stage allocation for POD/trivally-copyable data types. Invoked by kernel-space realloc() calls and internally by some allocators.
	addr_t reallocate(addr_t ptr, size_t size, size_t align = 0UZ) noexcept;
	//	Second-stage allocation for arrays. Invoked by kernel-space calloc() calls; as specified by the C++ standard, new[] does not use this.
	addr_t array_allocate(size_t num, size_t size) noexcept;
	//	The functions below are used by the above code, as well as by the allocator tag used by modules (which is somewhat more complex).
	/*------------------------------------------------------*/
	block_tag* create_tag(size_t size, size_t align) noexcept;
	block_tag* melt_left(block_tag* tag) noexcept;
	block_tag* melt_right(block_tag* tag) noexcept;
	block_tag* find_tag(addr_t ptr, size_t align) noexcept;
	block_tag* get_for_allocation(size_t size, size_t align) noexcept;
	void release_block(block_tag* tag) noexcept;
	void insert_block(block_tag* blk, int idx) noexcept;
	void remove_block(block_tag* blk) noexcept;
private:
	void __lock();
	void __unlock();
};
struct kframe_exports
{
	typedef addr_t (kframe_tag::*alloc_fn)(size_t, size_t);
	typedef void (kframe_tag::*dealloc_fn)(addr_t, size_t);
	typedef addr_t (kframe_tag::*realloc_fn)(addr_t, size_t, size_t);
	alloc_fn allocate;
	alloc_fn array_allocate;
	dealloc_fn deallocate;
	realloc_fn reallocate;
};
// Describes a memory block in stage 3.
struct block_descriptor
{
	addr_t physical_start;
	addr_t virtual_start;
	size_t size;
	size_t align	{ 0UL };
	bool write		{ true };
	bool execute	{ true };
};
constexpr bool operator==(block_descriptor const& __this, block_descriptor const& __that) noexcept
{
	return	__this.physical_start	== __that.physical_start
			&& __this.virtual_start	== __that.virtual_start
			&& __this.size			== __that.size;
}
struct uframe_tag
{
	uint64_t magic;
	paging_table pml4;
	addr_t base;
	addr_t extent;
	addr_t mapped_max;
	addr_t sysres_wm;
	addr_t sysres_extent;
	addr_t dynamic_extent{};
	std::vector<block_tag*> page_table_blocks{};
	std::vector<block_descriptor> usr_blocks{};
	std::vector<block_descriptor*> shared_blocks{};
private:
	spinlock_t __my_mutex;
	void __lock();
	void __unlock();
public:
	constexpr uframe_tag(paging_table cr3, addr_t st_base, addr_t st_extent) noexcept :
		magic					{ uframe_magic },
		pml4					{ cr3 },
		base					{ st_base },
		extent					{ st_extent },
		mapped_max				{ st_extent },
		sysres_wm				{ sysres_base },
		sysres_extent			{ sysres_base }
								{}
	friend constexpr std::strong_ordering operator<=>(uframe_tag const& __this, uframe_tag const& __that) noexcept { return __this.pml4 <=> __that.pml4; }
	bool shift_extent(ptrdiff_t amount);
	addr_t mmap_add(addr_t addr, size_t len, bool write, bool exec);
	addr_t sysres_add(size_t n);
	bool mmap_remove(addr_t addr, size_t len);
	void accept_block(block_descriptor&& desc);
	void transfer_block(uframe_tag& that, block_descriptor const& which);
	block_descriptor* add_block(size_t sz, addr_t start, size_t align = 0UZ, bool write = true, bool execute = true, bool allow_global_shared = false);
	addr_t translate(addr_t addr);
};
enum block_idx : uint8_t
{
	I0	= 0x01,
	I1	= 0x02,
	I2	= 0x04,
	I3	= 0x08,
	I4	= 0x10,
	I5	= 0x20,
	I6	= 0x40,
	I7	= 0x80,
	ALL = 0xFF
};
enum block_size : uint32_t
{
	S512	=  512 * page_size,
	S256	=  256 * page_size,
	S128	=  128 * page_size,
	S64		=	64 * page_size,
	S32		=	32 * page_size,
	S16		=	16 * page_size,
	S08		=	 8 * page_size,
	S04		=	 4 * page_size
};
#define BS2BI(i) i == S04 ? (I6 | I7) : (i == S08 ? I5 : (i ==	S16 ? I4 : (i ==	S32 ? I3 : (i ==	S64 ? I2 : (i == S128 ? I1 : (i == S256 ? I0 : ALL))))))
// Describes a 512-page region in stage 1.
typedef struct status_byte
{
	uint8_t the_byte;
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
} gb_status[512];
/*
 *	Each 512-page region of physical memory is divided into the following blocks:
 *	[256P] B0: |< 000 - 255 >|
 *	[128P] B1: |< 256 - 383 >|
 *	[064P] B2: |< 383 - 447 >|
 *	[032P] B3: |< 448 - 479 >|
 *	[016P] B4: |< 480 - 495 >|
 *	[008P] B5: |< 496 - 503 >|
 *	[004P] B6: |< 504 - 507 >|
 *	[004P] B7: |< 508 - 511 >|
 *	In addition, a region can be allocated in its entirety as a 512-page block.
 *	Blocks are assigned as follows:
 *		1. When memory is requested (i.e. a frame needs additional blocks), the amount of memory requested is used to determine the block size to allocate.
 *			- If the requested block is larger than 256 pages but smaller than 512 pages, a full 512-page region is allocated.
 *			- If the requested block is larger than 512 pages, a set of physically-contiguous 512-page regions is allocated as a single block.
 *			- In all other cases, the requested amount (in pages) will be rounded up to the next power of 2, to a minimum of 4, and used as the size of the block.
 *			-- Any successful block allocation will therefore allocate at least 16 kilobytes of memory as a single contiguous block.
 *			-- Blocks are then divided by the kernel frame (either the main kernel's or that of the relevant module) to satisfy individual allocation requests.
 *		2. Physical addresses are assigned in order low-to-high, globally, from regions of conventional memory.
 *			- When a block is allocated, a watermark pointer serving as a hint for searches is set to equal the end of that block.
 *			- When a page block is released, the watermark pointer is updated to reflect the newly-available address if it is lower.
 *			- If the watermark is below an in-use block, that block will prevent allocating contiguous regions containing it.
 *			- If a search reaches the end of available memory, the watermark will be reset to the start of the kernel heap.
 *			- Searches for available blocks track where the watermark was when they started, so the search will not reset the watermark more than once.
 *		3. An amount of identity-mapped space directly after the kernel is reserved at startup for use with these structures. It is never released.
 *			- This amount depends on how much memory is available; 512 bytes track 1 GB of RAM. The kernel's frame-tag will also go here.
 *			- The page-table data for the kernel has been allocated by the bootloader and should be in a region marked as loader data (and thus not available).
 *			- Page table entries generated after startup (such as for process page frames) will come from the kernel heap.
 *		4. Virtual addresses are assigned in order low-to-high per userspace page frame. Each page frame tracks its own address-mapping watermark.
 *			- Because of the extreme size of the address space, it should not be necessary to track released virtual addresses.
 *			Instead, blocks that are released from the frame are unmapped, and if they are reallocated later their virtual address will likely change.
 *		5. The bootloader will have identity-mapped all available memory to the kernel's page tables. The kernel therefore operates in a fully identity-mapped space.
 *			Kernel workers and module code will likewise see only identity-mapped memory. In order to facilitate access to userspace pointers,
 *			userspace frames will store pointers to their page tables that can be used to translate those pointers when in syscalls.
 */
class kernel_memory_mgr
{
	spinlock_t __heap_mutex{};					// Calls to kernel allocations lock this mutex to prevent comodification
	spinlock_t __user_mutex{};					// Separate mutex for userspace calls because userspace memory will be split from kernel blocks
	gb_status* const __status_bytes;			// Array of 512-byte arrays
	size_t const __num_status_bytes;			// Length of said array
	uintptr_t const __kernel_heap_begin;		// Convenience pointer to the end of above array
	uintptr_t __watermark;						// Updated when a block is allocated or released; provides a guess as to where to start searching for blocks
	addr_t __suspended_cr3{};					// Saved cr3 value for a frame suspended in order to access kernel paging structures
	uframe_tag* __active_frame{};				// Tag for the frame currently being modified, if any
	static kernel_memory_mgr* __instance;
	constexpr kernel_memory_mgr(gb_status* status_bytes, size_t num_status_bytes, uintptr_t kernel_heap_addr) noexcept :
		__status_bytes				{ status_bytes },
		__num_status_bytes			{ num_status_bytes },
		__kernel_heap_begin			{ kernel_heap_addr },
		__watermark					{ kernel_heap_addr }
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
	constexpr uintptr_t open_wm() const { return __watermark; }
	static void init_instance(mmap_t* mmap);
	static kernel_memory_mgr& get();
	// Computes the actual size that will be allocated for a page-aligned block if its address is start.
	static size_t aligned_size(addr_t start, size_t requested);
	// Computes the actual size that will be allocated for a DMA block, accounting for alignment and mapping constraints.
	static size_t dma_size(size_t requested);
	static void suspend_user_frame();
	static void resume_user_frame();
	static size_t currently_used_memory();
	static size_t total_available_memory();
	static size_t remaining_unused_memory();
	kernel_memory_mgr(kernel_memory_mgr const&) = delete;
	kernel_memory_mgr(kernel_memory_mgr&&) = delete;
	kernel_memory_mgr& operator=(kernel_memory_mgr const&) = delete;
	kernel_memory_mgr& operator=(kernel_memory_mgr&&) = delete;
	// Helpers for mapping memory and handling which frame is being modified at a given time.
	/*----------------------------------------------------------------------*/
	// Sets ft as the active frame, i.e. the frame to be modified by mapping and third-stage allocation and deallocation functions.
	void enter_frame(uframe_tag* ft) noexcept;
	// Translates addr as from a virtual address in the current active frame to a physical address.
	uintptr_t frame_translate(addr_t addr);
	// Sets the active frame to a null pointer; frame-dependent functions will fail until another frame is entered.
	void exit_frame() noexcept;
	// Behaves as for(block_descriptor const& bd : blocks) map_to_current_frame(bd);
	void map_to_current_frame(std::vector<block_descriptor> const& blocks);
	// Maps the block as specified by its physical and virtual address fields and permission flags.
	void map_to_current_frame(block_descriptor const& block);
	// Ensures the kernel is mapped (as ring 0 accessible only) to a given frame (specified in the form of that frame's PML4 table pointer).
	addr_t copy_kernel_mappings(paging_table target);
	// Identity-maps the given address (usually one specified by a device, e.g. in a PCI base address register) with flags for DMA use.
	addr_t map_dma(uintptr_t addr, size_t sz, bool prefetchable);
	// First-stage allocation; allocates a region sized to a power of 2 (at least 4), or a multiple of 512, in pages.
	__nointerrupts __noinline addr_t allocate_kernel_block(size_t sz) noexcept;
	// Third-stage allocation; allocates a block sized in pages and maps it to the active frame at the given starting address and with the given permissions.
	__nointerrupts __noinline addr_t allocate_user_block(size_t sz, addr_t start, size_t align = 0UZ, bool write = true, bool execute = true) noexcept;
	// First-stage deallocation.
	__nointerrupts __noinline void deallocate_block(addr_t base, size_t sz, bool should_unmap = false) noexcept;
	// Third-stage deallocation.
	__nointerrupts __noinline void deallocate_user_block(addr_t base, size_t sz, size_t align, bool should_unmap) noexcept;
	// First-stage allocation; allocates a region for use as a page table.
	__nointerrupts paging_table allocate_pt() noexcept;
	// Special allocation; allocates a region with page flags for DMA.
	__nointerrupts addr_t allocate_dma(size_t sz, bool prefetchable) noexcept;
	// Special deallocation that matches allocate_dma.
	__nointerrupts void deallocate_dma(addr_t addr, size_t sz) noexcept;
};
#define kmm kernel_memory_mgr::get()
extern "C" void* aligned_malloc(size_t size, size_t align);
extern "C" void aligned_free(void* ptr, size_t align);
extern "C" block_tag* block_malloc(size_t size, size_t align);
extern "C" void block_free(block_tag* tag);
extern "C" block_tag* locate_block(void* object, size_t align);
extern "C" addr_t syscall_sbrk(ptrdiff_t incr);																// void* sbrk(ptrdiff_t incr);
extern "C" addr_t syscall_mmap(addr_t addr, size_t len, int prot, int flags, int fd, ptrdiff_t offset);		// void* mmap(void* addr, size_t len, int prot, int flags, ptrdiff_t offset);
extern "C" int syscall_munmap(addr_t addr, size_t len);														// int munmap(void* addr, size_t len);
#endif