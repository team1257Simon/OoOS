#include "kernel_mm.hpp"
#include "frame_manager.hpp"
#include "direct_text_render.hpp"
#include "errno.h"
#include "kdebug.hpp"
#include "sched/task_ctx.hpp"
using std::addressof;
extern "C"
{
	extern unsigned char	__start[];
	extern unsigned char	__end[];
	extern unsigned char	__code;
	extern kframe_tag*		__kernel_frame_tag;
	extern unsigned char	sigtramp_code[4096];
	extern const size_t		kernel_pages;
	paging_table			kernel_cr3;
}
constexpr ptrdiff_t	bt_offset		= sizeof(block_tag);
constexpr size_t	min_block_size	= 1UL << min_exponent;
constexpr size_t	max_block_size	= 1UL << max_block_index;
constexpr size_t	st_bits			= CHAR_BIT * sizeof(size_t);
constexpr size_t	ltob_bit_diff	= CHAR_BIT * (sizeof(uint64_t) - sizeof(uint8_t));
static uint8_t		__kmm_data[sizeof(kernel_memory_mgr)];
static size_t		total_memory;
static size_t		remaining_memory;
kernel_memory_mgr*	kernel_memory_mgr::__instance;
constexpr int		clzb(uint8_t b) { return __builtin_clzl(b) - ltob_bit_diff; }
constexpr uintptr_t	ramp_shift(uintptr_t base, int i) { return i ? base * (1 << (i - 1)) : 0UL; }
constexpr uintptr_t	block_offset(uintptr_t addr, block_idx idx) { return addr + ramp_shift(S04, clzb(idx)); }
static paging_table	__build_new_pt(paging_table in, uint16_t idx, bool write_thru);
static paging_table	__build_new_pt(paging_table in, uint16_t idx, bool write_thru);
static paging_table	__get_table(addr_t of_page, bool write_thru, paging_table pml4);
static void			__unmap_pages(addr_t start, size_t pages, addr_t pml4);
static bool			__is_code_page(addr_t addr) { return addr_t(addressof(__code)) <= addr && addr < addr_t(addressof(__end)); }
constexpr uint32_t	calculate_block_index(size_t size) { return size < min_block_size ? 0 : size > max_block_size ? max_block_index : (st_bits - __builtin_clzl(size)) - min_exponent; }
constexpr block_size nearest(size_t sz) { return sz <= S04 ? S04 : sz <= S08 ? S08 : sz <= S16 ? S16 : sz <= S32 ? S32 : sz <= S64 ? S64 : sz <= S128 ? S128 : sz <= S256 ? S256 : S512; }
constexpr size_t	region_size_for(size_t sz) { return sz > S512 ? (up_to_nearest(sz, region_size)) : nearest(sz); }
static paging_table	__get_table(addr_t of_page, bool write_thru) { return __get_table(of_page, write_thru, get_cr3()); }
constexpr uint32_t	add_align_size(addr_t tag, size_t align) { return static_cast<uint32_t>(align > 1 ? (up_to_nearest<size_t>(tag + bt_offset, align) - static_cast<ptrdiff_t>(tag.full + bt_offset)) : 0); }
kernel_memory_mgr&	kernel_memory_mgr::get() { return *__instance; }
uintptr_t			kernel_memory_mgr::__claim_region(uintptr_t addr, block_idx idx) { __status(addr).set_used(idx); return block_offset(addr, idx); }
void				kernel_memory_mgr::__lock() { lock(addressof(__heap_mutex)); __suspend_frame(); }
void				kernel_memory_mgr::__unlock() { release(addressof(__heap_mutex)); __resume_frame(); }
void				kernel_memory_mgr::__userlock() { lock(addressof(__user_mutex)); }
void				kernel_memory_mgr::__userunlock() { release(addressof(__user_mutex)); }
void				kernel_memory_mgr::__mark_used(uintptr_t start, size_t num_regions) { for(size_t i = 0; i < num_regions; i++, start += region_size) __get_sb(start)->set_used(ALL); }
size_t				kernel_memory_mgr::dma_size(size_t requested) { return region_size_for(requested); }
size_t				kernel_memory_mgr::aligned_size(addr_t start, size_t requested) { return static_cast<size_t>(start.plus(requested).next_page_aligned() - start.page_aligned()); }
void				kernel_memory_mgr::suspend_user_frame() { __instance->__suspend_frame(); }
void				kernel_memory_mgr::resume_user_frame() { __instance->__resume_frame(); }
size_t				kernel_memory_mgr::currently_used_memory() { return static_cast<size_t>(total_memory - remaining_memory); }
size_t				kernel_memory_mgr::total_available_memory() { return total_memory; }
size_t				kernel_memory_mgr::remaining_unused_memory() { return remaining_memory; }
void				kframe_tag::__lock() { lock(addressof(__my_mutex)); }
void				kframe_tag::__unlock() { release(addressof(__my_mutex)); }
void				uframe_tag::__lock() { lock(addressof(__my_mutex)); }
void				uframe_tag::__unlock() { release(addressof(__my_mutex)); }
addr_t				kernel_memory_mgr::copy_kernel_mappings(paging_table target)
{
	addr_t start		= __start;
	addr_t curr			= start;
	paging_table pt		= __get_table(curr, false);
	paging_table upt	= __get_table(curr, false, target);
	if(__unlikely(!upt || !pt)) return nullptr;
	for(size_t i = 0; i < kernel_pages; i++, curr += page_size)
	{
		uint16_t p_idx = curr.page_idx;
		if(i != 0 && p_idx == 0)
		{
			pt	= __get_table(curr, true);
			upt	= __get_table(curr, false, target);
			if(__unlikely(!upt || !pt)) return nullptr;
		}
		pt_entry& u_entry	= upt[p_idx];
		pt_entry& k_entry	= pt[p_idx];
		array_copy<uint64_t>(addressof(u_entry), addr_t(addressof(k_entry)), sizeof(pt_entry) / sizeof(uint64_t));
		if(curr.as<uint8_t>() >= sigtramp_code && curr.as<uint8_t>() < addressof(sigtramp_code[4096])) u_entry.user_access =true;
	}
	return start;
}
void kernel_memory_mgr::enter_frame(uframe_tag* ft) noexcept
{
	lock(addressof(__heap_mutex));
	this->__active_frame	= ft;
	release(addressof(__heap_mutex));
}
void kernel_memory_mgr::exit_frame() noexcept
{
	lock(addressof(__heap_mutex));
	this->__active_frame	= nullptr;
	release(addressof(__heap_mutex));
}
static paging_table __find_table(addr_t of_page, paging_table pml4 = get_cr3())
{
	if(pml4[of_page.pml4_idx].present)
		if(paging_table pdp = addr_t(pml4[of_page.pml4_idx].physical_address << 12); pdp[of_page.pdp_idx].present)
			if(paging_table pd = addr_t(pdp[of_page.pdp_idx].physical_address << 12); pd[of_page.pd_idx].present)
				return addr_t(pd[of_page.pd_idx].physical_address << 12);
	return nullptr;
}
static void __set_kernel_page_flags(uintptr_t max)
{
	paging_table pt	= nullptr;
	for(addr_t addr	= __start; addr < max; addr += page_size)
	{
		if(!pt || !addr.page_idx) pt		= __find_table(addr);
		if(pt)
		{
			pt[addr.page_idx].global		= true;
			pt[addr.page_idx].write			= !__is_code_page(addr);
			pt[addr.page_idx].user_access	= false;
		}
	}
}
static paging_table __build_new_pt(paging_table in, uint16_t idx, bool write_thru)
{
	paging_table result			= kmm.allocate_pt();
	if(result)
	{
		new(addressof(in[idx])) pt_entry
		{
			.present			= true,
			.write				= true,
			.user_access		= true,
			.write_thru			= write_thru,
			.physical_address	= std::bit_cast<uintptr_t>(result) >> 12,
		};
	}
	return result;
}
static paging_table __get_table(addr_t of_page, bool write_thru, paging_table pml4)
{
	uint16_t pml4_idx				= of_page.pml4_idx;
	uint16_t pdp_idx				= of_page.pdp_idx;
	uint16_t pd_idx					= of_page.pd_idx;
	if(pt_entry& entry_pml4			= pml4[pml4_idx]; entry_pml4.present)
	{
		entry_pml4.write_thru		= write_thru;
		entry_pml4.user_access		= true;
		paging_table pdp			= addr_t(entry_pml4.physical_address << 12);
		if(pt_entry& entry_pdp		= pdp[pdp_idx]; entry_pdp.present)
		{
			entry_pdp.write_thru	= write_thru;
			entry_pdp.user_access	= true;
			paging_table pd			= addr_t(entry_pdp.physical_address << 12);
			if(pt_entry& entry_pd	= pd[pd_idx]; entry_pd.present)
			{
				entry_pd.write_thru	= write_thru;
				entry_pd.user_access = true;
				return addr_t(entry_pd.physical_address << 12);
			}
			else return __build_new_pt(pd, pd_idx, write_thru);
		}
		else if(paging_table pd = __build_new_pt(pdp, pdp_idx, write_thru)) return __build_new_pt(pd, pd_idx, write_thru);
		else return nullptr;
	}
	if(paging_table pdp		= __build_new_pt(pml4, pml4_idx, write_thru))
		if(paging_table pd	= __build_new_pt(pdp, pdp_idx, write_thru))
			return __build_new_pt(pd, pd_idx, write_thru);
	return nullptr;
}
static addr_t __map_kernel_pages(addr_t start, size_t pages, bool global)
{
	if(!start) return nullptr;
	addr_t curr		= start;
	uintptr_t phys	= curr;
	paging_table pt	= __get_table(curr, false);
	if(__unlikely(!pt)) return nullptr;
	bool modified	= false;
	for(size_t i	= 0; i < pages; i++, curr += page_size, phys += page_size)
	{
		uint16_t px	= curr.page_idx;
		if(i != 0 && px == 0) {
			pt		= __get_table(curr, false);
			if(__unlikely(!pt)) return nullptr;
		}
		pt_entry& entry	= pt[px];
		if(entry.present && (entry.global || entry.physical_address == phys >> 12)) continue;
		new(addressof(entry)) pt_entry
		{
			.present			= true,
			.write				= true,
			.user_access		= false,
			.write_thru			= false,
			.cache_disable		= false,
			.global				= global,
			.physical_address	= phys >> 12,
		};
		modified = true;
	}
	if(modified) tlb_flush();
	return start;
}
static addr_t __map_mmio_pages(addr_t start, size_t pages)
{
	if(!start) return nullptr;
	addr_t curr		= start;
	paging_table pt	= __get_table(curr, true);
	if(__unlikely(!pt)) return nullptr;
	for(size_t i = 0; i < pages; i++, curr += page_size)
	{
		if(i != 0 && curr.page_idx == 0) pt = __get_table(curr, true);
		new(addressof(pt[curr.page_idx])) pt_entry
		{
			.present			= true,
			.write				= true,
			.user_access		= false,
			.write_thru			= true,
			.global				= true,
			.physical_address	= curr.full >> 12,
		};
	}
	tlb_flush();
	return start;
}
static addr_t __map_uncached_mmio_pages(addr_t start, size_t pages)
{
	if(!start) return nullptr;
	addr_t curr		= start;
	paging_table pt	= __get_table(curr, true);
	if(__unlikely(!pt)) return nullptr;
	for(size_t i = 0; i < pages; i++, curr += page_size)
	{
		if(i != 0 && curr.page_idx == 0) { pt = __get_table(curr, true); if(__unlikely(!pt)) return nullptr; }
		new(addressof(pt[curr.page_idx])) pt_entry
		{
			.present			= true,
			.write				= true,
			.user_access		= false,
			.cache_disable		= true,
			.global				= true,
			.physical_address	= curr.full >> 12,
		};
	}
	tlb_flush();
	return start;
}
static addr_t __map_user_pages(addr_t start_vaddr, uintptr_t start_paddr, size_t pages, paging_table pml4, bool write, bool execute)
{
	if(start_vaddr && start_paddr && pages && pml4)
	{
		addr_t curr		= start_vaddr;
		uintptr_t phys	= start_paddr;
		paging_table pt	= __get_table(curr, false, pml4);
		if(__unlikely(!pt)) return nullptr;
		for(size_t i = 0; i < pages; i++, curr += page_size, phys += page_size)
		{
			if(i != 0 && curr.page_idx == 0) { pt = __get_table(curr, false, pml4); if(__unlikely(!pt)) return nullptr; }
			new(addressof(pt[curr.page_idx])) pt_entry
			{
				.present			= true,
				.write				= write,
				.user_access		= true,
				.physical_address	= phys >> 12,
				.execute_disable	= !execute
			};
		}
		tlb_flush();
		return start_vaddr;
	}
	else return nullptr;
}
static void __unmap_pages(addr_t start, size_t pages, addr_t pml4)
{
	if(start && pages)
	{
		addr_t curr		= start;
		paging_table pt	= nullptr;
		for(size_t i = 0; i < pages; i++, curr += PAGESIZE)
		{
			if(!pt || curr.page_idx == 0)
				pt		= __find_table(curr, pml4);
			if(pt)
			{
				pt_entry& entry	= pt[curr.page_idx];
				if(entry.global) continue;
				addr_t(addressof(entry)).deref<uint64_t>() = 0UL;
				asm volatile("invlpg (%0)" ::"r"(curr.full) : "memory");
			}
		}
	}
}
void kernel_memory_mgr::__suspend_frame() noexcept
{
	addr_t cur_cr3	= get_cr3();
	if(cur_cr3 == addr_t(kernel_cr3)) return;
	__suspended_cr3	= cur_cr3;
	set_cr3(kernel_cr3);
}
void kernel_memory_mgr::__resume_frame() noexcept
{
	if(!__suspended_cr3) return;
	set_cr3(__suspended_cr3);
	__suspended_cr3	= nullptr;
}
uintptr_t kernel_memory_mgr::__find_and_claim(size_t sz)
{
	uintptr_t addr			= up_to_nearest(__watermark, region_size);
	if(sz > S256)
	{
		size_t num_regions	= div_round_up(sz, region_size);
		uintptr_t result	= addr;
		for(size_t n = num_regions; status_byte::gb_of(addr) < __num_status_bytes && n > 0; addr += region_size)
		{
			if(__status(addr).all_free())
			{
				n--;
				if(!n)
				{
					for(uintptr_t i = result; i <= addr; i += region_size)
						__status(i).set_used(ALL);
					return result;
				}
				continue;
			}
			n		= num_regions;
			result	= addr + region_size;
		}
	}
	else
	{
		for(block_size bs = nearest(sz); status_byte::gb_of(addr) < __num_status_bytes; addr += region_size)
		{
			if(__status(addr)[bs])
			{
				switch(bs)
				{
					case S04: return __claim_region(addr, __status(addr).has_free(I7) ? I7 : I6);
					case S08: return __claim_region(addr, I5);
					case S16: return __claim_region(addr, I4);
					case S32: return __claim_region(addr, I3);
					case S64: return __claim_region(addr, I2);
					case S128: return __claim_region(addr, I1);
					default: return __claim_region(addr, I0);
				}
			}
		}
	}
	// If we didn't find any, try again with the watermark reset in case it's inaccurate
	if(__watermark	> __kernel_heap_begin + __num_status_bytes)
		__watermark	= __kernel_heap_begin + __num_status_bytes;
	else return 0;
	return __find_and_claim(sz);
}
void kernel_memory_mgr::__release_region(size_t sz, uintptr_t start)
{
	block_size bs		= nearest(sz);
	if(sz > S256)
	{
		uintptr_t addr	= start;
		size_t n		= div_round_up(sz, region_size);
		for(size_t i	= 0; i < n; i++, addr += region_size) __status(addr).set_free(ALL);
	}
	else
	{
		switch(bs)
		{
			case S256: __status(start).set_free(I0); break;
			case S128: __status(start).set_free(I1); break;
			case S64: __status(start).set_free(I2); break;
			case S32: __status(start).set_free(I3); break;
			case S16: __status(start).set_free(I4); break;
			case S08: __status(start).set_free(I5); break;
			default: __status(start).set_free((start % region_size == 0) ? I7 : I6); break;
		}
	}
}
void kernel_memory_mgr::init_instance(mmap_t* mmap)
{
	kernel_cr3						= get_cr3();
	__kernel_frame_tag				= new(__end) kframe_tag();
	addr_t		sb_addr				= addr_t(__kernel_frame_tag).plus(sizeof(kframe_tag));
	size_t		num_status_bytes	= div_round_up(mmap->total_memory, gigabyte);
	uintptr_t	heap				= sb_addr.plus(num_status_bytes * sizeof(gb_status));
	gb_status*	status_bytes		= new(sb_addr) gb_status[num_status_bytes];
	(__instance						= new(__kmm_data) kernel_memory_mgr(status_bytes, num_status_bytes, heap))->__mark_used(0UL, div_round_up(heap, region_size));
	total_memory					= mmap->total_memory;
	for(size_t i = 0; i < mmap->num_entries; i++)
	{
		if(mmap->entries[i].type != AVAILABLE)
		{
			size_t taken_pages		= div_round_up(mmap->entries[i].len, PT_LEN);
			__instance->__mark_used(mmap->entries[i].addr, taken_pages);
			total_memory			-= taken_pages * page_size;
			if(mmap->entries[i].type == MMIO)
				__map_mmio_pages(addr_t(mmap->entries[i].addr), mmap->entries[i].len);
		}
	}
	__instance->__watermark			= heap;
	__set_kernel_page_flags(sb_addr);
	remaining_memory				= total_memory;
}
addr_t kernel_memory_mgr::allocate_dma(size_t sz, bool prefetchable) noexcept
{
	__lock();
	addr_t result				= nullptr;
	uintptr_t phys				= __find_and_claim(sz);
	if(__builtin_expect(phys != 0, true))
	{
		size_t total_sz			= div_round_up(region_size_for(sz), page_size);
		if(prefetchable) result	= __map_mmio_pages(addr_t(phys), total_sz);
		else result				= __map_uncached_mmio_pages(addr_t(phys), total_sz);
		__watermark				= std::max(phys, __watermark);
		remaining_memory		-= total_sz * page_size;
	}
	__unlock();
	return result;
}
addr_t kernel_memory_mgr::map_dma(uintptr_t addr, size_t sz, bool prefetchable)
{
	size_t npage	= div_round_up(sz, page_size);
	__lock();
	addr_t result	= prefetchable ? __map_mmio_pages(addr_t(addr), npage) : __map_uncached_mmio_pages(addr_t(addr), npage);
	if(__builtin_expect(static_cast<bool>(result), true)) { __mark_used(addr, npage); }
	__unlock();
	return result;
}
void kernel_memory_mgr::deallocate_dma(addr_t addr, size_t sz) noexcept
{
	__lock();
	if(addr)
	{
		size_t total_sz		= div_round_up(region_size_for(sz), page_size);
		uintptr_t phys		= __map_kernel_pages(addr, total_sz, true);
		__release_region(sz, phys);
		__watermark			= std::min(phys, __watermark);
		remaining_memory	+= total_sz * page_size;;
	}
	__unlock();
}
addr_t kernel_memory_mgr::allocate_user_block(size_t sz, addr_t start, size_t align, bool write, bool execute) noexcept
{
	addr_t pml4			= __active_frame ? __active_frame->pml4 : get_cr3();
	__userlock();
	size_t rsz			= aligned_size(start, sz); // allocate to the end of page so the userspace doesn't see kernel data structures
	addr_t result		= __kernel_frame_tag->allocate(rsz, align);
	if(!start) start	= result;
	if(result && !__map_user_pages(start.trunc(std::max(align, page_size)), result, div_round_up(rsz, page_size), pml4, write, execute)) {
		__kernel_frame_tag->deallocate(result, align);
		result			= nullptr;
	}
	__userunlock();
	return result;
}
paging_table kernel_memory_mgr::allocate_pt() noexcept
{
	constexpr size_t pt_size	= sizeof(pt_entry) * page_table_length;
	constexpr size_t total_sz	= up_to_nearest(pt_size + bt_offset, page_size);
	constexpr size_t rsz		= region_size_for(total_sz * 8);
	constexpr uint32_t exp		= calculate_block_index(pt_size + bt_offset);
	block_tag* tag				= nullptr;
	for(tag = __kernel_frame_tag->available_blocks[exp]; bool(tag); tag = tag->next)
	{
		tag->align_bytes		= add_align_size(tag, page_size);
		if(tag->available_size() >= pt_size)
		{
			__kernel_frame_tag->remove_block(tag);
			if(!tag->left_split && !tag->right_split) __kernel_frame_tag->complete_regions[exp]--;
			break;
		}
		tag->align_bytes		= 0UL;
	}
	if(!tag)
	{
		addr_t allocated(__find_and_claim(rsz));
		if(__unlikely(!allocated)) return nullptr;
		if(__unlikely(!translate_vaddr(allocated))) __map_kernel_pages(allocated, rsz / page_size, true);
		tag				= new(allocated) block_tag(rsz, 0);
		__watermark		= std::max(allocated.full, __watermark);
	}
	tag->held_size		= pt_size;
	tag->align_bytes	= add_align_size(tag, page_size);
	addr_t result		= tag->actual_start();
	__builtin_memset(result, 0, page_size);
	if(tag->available_size() >= min_block_size + bt_offset) __kernel_frame_tag->insert_block(tag->split(), -1);
	if(__unlikely(!result)) return nullptr;
	if(__active_frame)
	{
		bool lk			= test_lock(addressof(__heap_mutex));
		if(lk) __unlock();
		__active_frame->kernel_allocated_blocks.push_back(result);
		if(lk) __lock();
	}
	remaining_memory	-= tag->block_size;
	return result;
}
addr_t kernel_memory_mgr::allocate_kernel_block(size_t sz) noexcept
{
	__lock();
	addr_t phys(__find_and_claim(sz)),	result(nullptr);
	__unlock();
	if(__unlikely(!phys)) return nullptr;
	__lock();
	result		= __map_kernel_pages(phys, div_round_up(region_size_for(sz), page_size), true);
	__watermark	= std::max(uintptr_t(phys), __watermark);
	__unlock();
	return result;
}
void kernel_memory_mgr::deallocate_block(addr_t base, size_t sz, bool should_unmap) noexcept
{
	uintptr_t phys	= frame_translate(base);
	addr_t pml4		= __active_frame ? __active_frame->pml4 : nullptr;
	if(__unlikely(!phys)) return;
	__lock();
	__release_region(sz, phys);
	if(should_unmap && __active_frame) __unmap_pages(base, div_round_up(sz, page_size), pml4);
	__watermark		= std::min(phys, __watermark);
	__unlock();
}
uintptr_t kernel_memory_mgr::frame_translate(addr_t addr)
{
	paging_table pt	= __find_table(addr, __active_frame ? __active_frame->pml4 : kernel_cr3);
	if(pt && pt[addr.page_idx].present && pt[addr.page_idx].physical_address) return (pt[addr.page_idx].physical_address << 12) | addr.offset;
	return 0;
}
void kernel_memory_mgr::map_to_current_frame(block_descriptor const& blk)
{
	addr_t pml4 = __active_frame ? __active_frame->pml4 : get_cr3();
	__lock();
	__map_user_pages(blk.virtual_start, blk.physical_start, div_round_up(blk.size, page_size), pml4, blk.write, blk.execute);
	__unlock();
}
void kernel_memory_mgr::map_to_current_frame(std::vector<block_descriptor> const& blocks)
{
	addr_t pml4	= __active_frame ? __active_frame->pml4 : get_cr3();
	__lock();
	for(block_descriptor const& blk : blocks) { __map_user_pages(blk.virtual_start, blk.physical_start, div_round_up(blk.size, page_size), pml4, blk.write, blk.execute); }
	__unlock();
}
block_tag* kframe_tag::create_tag(size_t size, size_t align) noexcept
{
	size_t actual_size	= std::max(size + bt_offset, align) + align;
	addr_t allocated	= kmm.allocate_kernel_block(actual_size);
	if(__unlikely(!allocated)) return nullptr;
	return new(allocated) block_tag(region_size_for(actual_size), size, -1, add_align_size(allocated, align));
}
block_tag* kframe_tag::melt_left(block_tag* tag) noexcept
{
	block_tag* left		= tag->left_split;
	left->block_size	+= tag->block_size;
	left->right_split	= tag->right_split;
	if(tag->right_split) tag->right_split->left_split = left;
	remove_block(tag);
	return left;
}
block_tag* kframe_tag::melt_right(block_tag* tag) noexcept
{
	block_tag* right	= tag->right_split;
	remove_block(right);
	tag->block_size		+= right->block_size;
	tag->right_split	= right->right_split;
	if(right->right_split) right->right_split->left_split = tag;
	return tag;
}
block_tag* kframe_tag::find_tag(addr_t ptr, size_t align) noexcept
{
	block_tag* tag	= ptr - bt_offset;
	for(size_t i 	= 0; tag && (tag->magic != block_magic) && (!align || i < align); i++)
		tag			= addr_t(tag).minus(1L);
	if(tag && tag->magic != block_magic)
		tag			= ptr.page_aligned();
	return tag && tag->magic == block_magic ? tag : nullptr;
}
void kframe_tag::insert_block(block_tag* blk, int idx) noexcept
{
	blk->index									= idx < 0 ? (calculate_block_index(blk->block_size)) : idx;
	if(available_blocks[blk->index] && blk != available_blocks[blk->index]) {
		available_blocks[blk->index]->previous	= blk;
		blk->next								= available_blocks[blk->index];
	}
	available_blocks[blk->index]				= blk;
}
void kframe_tag::remove_block(block_tag* blk) noexcept
{
	if(available_blocks[blk->index]		== blk)
		available_blocks[blk->index]	= blk->next;
	else if(blk->previous)
		blk->previous->next				= blk->next;
	if(blk->next)
		blk->next->previous				= blk->previous;
	blk->next							= nullptr;
	blk->previous						= nullptr;
	blk->index							= -1;
}
block_tag* kframe_tag::get_for_allocation(size_t size, size_t align) noexcept
{
	__lock();
	int64_t	idx					= calculate_block_index(size);
	block_tag* tag				= nullptr;
	for(tag 					= available_blocks[idx]; tag != nullptr; tag = tag->next)
	{
		if(tag == tag->next)
		{
			tag->next			= nullptr;
			direct_write("[MM] W: tag at ");
			debug_print_addr(tag);
			direct_writeln("was its own next tag");
		}
		if(tag == tag->previous)
		{
			tag->previous		= nullptr;
			direct_write("[MM] W: tag at ");
			debug_print_addr(tag);
			direct_writeln("was its own previous tag");
		}
		size_t align_add		= add_align_size(tag, align);
		if(tag->available_size() >= size + align_add)
		{
			remove_block(tag);
			tag->held_size		= size;
			tag->align_bytes	= align_add;
			if(!tag->left_split && !tag->right_split) complete_regions[idx]--;
			break;
		}
	}
	if(!tag)
	{
		if(__unlikely(!(tag		= create_tag(size, align))))
		{
			__unlock();
			panic("[MM] allocation failed");
			debug_print_num(size);
			direct_writeln("bytes were requested");
			return nullptr;
		}
		idx						= calculate_block_index(tag->allocated_size()) - MIN_BLOCK_EXP;
	}
	if(tag->available_size() >= min_block_size + bt_offset) insert_block(tag->split(), -1);
	remaining_memory			-= tag->block_size;
	__unlock();
	return tag;
}
void kframe_tag::release_block(block_tag* tag) noexcept
{
	tag->held_size		= 0;
	tag->align_bytes	= 0;
	remaining_memory	+= tag->block_size;
	while(tag->left_split && (tag->left_split->index >= 0))		tag	= melt_left(tag);
	while(tag->right_split && (tag->right_split->index >= 0))	tag	= melt_right(tag);
	int64_t idx			= calculate_block_index(tag->allocated_size());
	if(!tag->left_split && !tag->right_split && complete_regions[idx] >= region_cap) kmm.deallocate_block(tag, tag->block_size, false);
	else
	{
		if(!tag->left_split && !tag->right_split)
			complete_regions[idx]++;
		insert_block(tag, idx);
	}
}
addr_t kframe_tag::allocate(size_t size, size_t align) noexcept
{
	if(__unlikely(!size)) { direct_writeln("[MM] W: size zero alloc"); return nullptr; }
	block_tag* tag	= get_for_allocation(size, align);
	if(__unlikely(!tag)) return nullptr;
	return tag->actual_start();
}
void kframe_tag::deallocate(addr_t ptr, size_t align) noexcept
{
	if(ptr)
	{
		__lock();
		block_tag* tag	= find_tag(ptr, align);
		if(__unlikely(!tag))
		{
			direct_write("[MM] W: attempted to deallocate an invalid tag; check for memory corruption at or near address ");
			debug_print_num(ptr);
			direct_write("\n");
		}
		else release_block(tag);
		__unlock();
	}
}
addr_t kframe_tag::reallocate(addr_t ptr, size_t size, size_t align) noexcept
{
	if(!ptr) return allocate(size, align);
	if(!size)
	{
		direct_writeln("[MM] W: size zero alloc");
		deallocate(ptr, align);
		return nullptr;
	}
	block_tag* tag	= find_tag(ptr, align);
	if(tag && tag->allocated_size() >= size + add_align_size(tag, align))
	{
		tag->align_bytes	= add_align_size(tag, align);
		tag->held_size		= size;
		return tag->actual_start();
	}
	addr_t result			= allocate(size, align);
	if(result) { atomic_copy<uint8_t>(result, ptr, tag->held_size > size ? size : tag->held_size); }
	deallocate(ptr, align);
	return result;
}
addr_t kframe_tag::array_allocate(size_t num, size_t size) noexcept
{
	addr_t result	= allocate(num * size, size);
	if(__builtin_expect(static_cast<bool>(result), true)) __builtin_memset(result, 0, num * size);
	return result;
}
block_tag* block_tag::split()
{
	block_tag* that		= new(actual_start().plus(held_size)) block_tag(available_size(), 0, -1, this, right_split);
	if(that->right_split) that->right_split->left_split = that;
	this->right_split	= that;
	this->block_size	-= that->block_size;
	return that;
}
bool uframe_tag::shift_extent(ptrdiff_t amount)
{
	if(amount == 0) return true; // nothing to do, vacuous success; sbrk(0) is useful to get the initial value of the break/extent
	if(amount < 0)
	{
		__lock();
		uintptr_t amt_freed	= -amount;
		if(__builtin_expect(static_cast<size_t>(extent - base) > amt_freed, true))
		{
			addr_t target	= extent + amount;
			std::vector<block_descriptor>::reverse_iterator i;
			for(i = usr_blocks.rend(); i->physical_start >= target && i->write; i++) {
				__unmap_pages(i->physical_start, i->size / page_size, pml4);
				__kernel_frame_tag->deallocate(i->physical_start, i->align);
			}
			usr_blocks.erase((--i).base(), usr_blocks.end());
			if(usr_blocks.empty()) { extent = base = nullptr; }
			else extent		= usr_blocks.back().physical_start.plus(usr_blocks.back().size);
			__unlock();
			return !usr_blocks.empty();
		}
		return false;
	}
	size_t added				= kernel_memory_mgr::aligned_size(extent, static_cast<size_t>(amount));
	block_descriptor* allocated	= add_block(added, extent, page_size, true, false);
	if(__unlikely(!allocated)) return false;
	extent						= allocated->virtual_start.plus(allocated->size);
	array_zero(allocated->physical_start.as<uint64_t>(), allocated->size / sizeof(uint64_t));
	return true;
}
addr_t uframe_tag::mmap_add(addr_t addr, size_t len, bool write, bool exec)
{
	addr						= addr.page_aligned();
	bool use_extent				= !addr;
	if(use_extent) addr			= extent;
	block_descriptor* result	= add_block(len, addr, page_size, write, exec);
	if(__builtin_expect(result != nullptr, true))
	{
		size_t actual			= kernel_memory_mgr::aligned_size(addr, len);
		array_zero(result->physical_start.as<uint64_t>(), actual / sizeof(uint64_t));
		addr_t top				= addr.plus(actual);
		mapped_max				= std::max(mapped_max, top);
		extent					= std::max(extent, top);
		return addr;
	}
	return addr_t(static_cast<uintptr_t>(-ENOMEM));
}
void uframe_tag::accept_block(block_descriptor&& desc)
{
	__lock();
	block_descriptor& blk	= usr_blocks.emplace_back(std::move(desc));
	kmm.enter_frame(this);
	kmm.map_to_current_frame(blk);
	kmm.exit_frame();
	__unlock();
}
void uframe_tag::transfer_block(uframe_tag& that, block_descriptor const& which)
{
	__lock();
	for(std::vector<block_descriptor>::iterator i = usr_blocks.begin(); i != usr_blocks.end(); i++)
	{
		if(which.physical_start == i->physical_start)
		{
			that.accept_block(std::move(*i));
			usr_blocks.erase(i);
			break;
		}
	}
	__unlock();
}
void uframe_tag::drop_block(block_descriptor const& which)
{
	__lock();
	for(std::vector<block_descriptor>::iterator i = usr_blocks.begin(); i != usr_blocks.end(); i++)
	{
		if(which.physical_start == i->physical_start)
		{
			__unmap_pages(i->physical_start, i->size / page_size, pml4);
			__kernel_frame_tag->deallocate(i->physical_start, i->align);
			usr_blocks.erase(i);
			break;
		}
	}
	__unlock();
}
addr_t uframe_tag::translate(addr_t addr)
{
	kmm.enter_frame(this);
	addr_t result(kmm.frame_translate(addr));
	kmm.exit_frame();
	return result;
}
block_descriptor* uframe_tag::add_block(size_t sz, addr_t start, size_t align, bool write, bool execute, bool allow_global_shared)
{
	if(allow_global_shared && !write) { return fm.get_global_shared(this, sz, start, align, execute); }
	block_descriptor* result	= nullptr;
	__lock();
	kmm.enter_frame(this);
	addr_t allocated			= kmm.allocate_user_block(sz, start, align, write, execute);
	kmm.exit_frame();
	if(allocated) { result		= std::addressof(usr_blocks.emplace_back(allocated, start, sz, align, write, execute)); }
	__unlock();
	return result;
}
bool uframe_tag::mmap_remove(addr_t addr, size_t len)
{
	if(addr > mapped_max) return false;
	len = std::min(len, static_cast<size_t>(mapped_max - addr));
	for(size_t i = 0; i < usr_blocks.size(); i++)
	{
		if(usr_blocks[i].virtual_start >= addr)
			continue;
		if(usr_blocks[i].virtual_start.plus(usr_blocks[i].size) > addr) {
			drop_block(usr_blocks[i]);
			break;
		}
	}
	return true;
}
addr_t uframe_tag::sysres_add(size_t n)
{
	if(sysres_wm.plus(n).alignup(alignof(void*)) > sysres_extent)
	{
		addr_t mapping_target	= sysres_extent;
		kmm.enter_frame(this);
		addr_t allocated		= kmm.allocate_user_block(up_to_nearest(n, alignof(void*)), mapping_target, 0UL, false, false);
		kmm.exit_frame();
		if(__unlikely(!allocated)) return nullptr;
		kernel_allocated_blocks.push_back(allocated);
		sysres_extent			+= kernel_memory_mgr::aligned_size(mapping_target, n);
	}
	addr_t result				= sysres_wm;
	sysres_wm					= sysres_wm.plus(n).alignup(alignof(void*));
	return result;
}
extern "C"
{
	void* aligned_malloc(size_t size, size_t align) { return __kernel_frame_tag->allocate(size, align); }
	void aligned_free(void* ptr, size_t align) { __kernel_frame_tag->deallocate(ptr, align); }
	block_tag* block_malloc(size_t size, size_t align) { return __kernel_frame_tag->get_for_allocation(size, align); }
	block_tag* locate_block(void* object, size_t align) { return object ? __kernel_frame_tag->find_tag(object, align) : nullptr; }
	void block_free(block_tag* tag) { __kernel_frame_tag->release_block(tag); }
	uintptr_t translate_vaddr(addr_t addr)
	{
		paging_table pt	= __find_table(addr);
		if(pt) return (pt[addr.page_idx].physical_address << 12) | addr.offset;
		else return 0;
	}
	addr_t translate_user_pointer(addr_t ptr)
	{
		uframe_tag* ctask_frame	= current_active_task()->frame_ptr;
		if(__unlikely(ctask_frame->magic != uframe_magic)) return nullptr;
		kmm.enter_frame(ctask_frame);
		addr_t result(kmm.frame_translate(ptr));
		kmm.exit_frame();
		return result;
	}
}