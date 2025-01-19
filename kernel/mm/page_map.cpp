#include "libk_decls.h"
#include "heap_allocator.hpp"
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
static paging_table __get_table(vaddr_t const& of_page, bool write_thru)
{
    paging_table pml4 = get_cr3();
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
    return NULL;
}
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
    return NULL;
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
extern "C" 
{
    uintptr_t translate_vaddr(vaddr_t addr) { if(paging_table pt = __find_table(addr)) return (pt[addr.page_idx].physical_address << 12) | addr.offset; else return 0; }
    vaddr_t map_pages(vaddr_t start, uintptr_t phys, size_t pages)
    {
        if(!start) return nullptr;
        vaddr_t curr{ __skip_mmio(start, pages) };
        if(!curr) return nullptr;
        start = curr;
        paging_table pt = __get_table(curr, false);
        if(!pt) { return nullptr; }
        for(size_t i = 0; i < pages; i++, curr += PAGESIZE, phys += PAGESIZE)
        {
            if(i != 0 && curr.page_idx == 0) pt = __get_table(curr, false);
            pt[curr.page_idx].present = true;
            pt[curr.page_idx].write = true;
            pt[curr.page_idx].physical_address = phys >> 12;
        }
        tlb_flush();
        return start;
    }
    uintptr_t unmap_pages(vaddr_t start, size_t pages)
    {
        if(!start) return 0ul;
        uintptr_t result = 0;
        for(size_t i = 0; i < pages; start += PAGESIZE)
        {
            if(paging_table pt = __find_table(start))
            {
                if(result == 0) result = pt[start.page_idx].physical_address;
                pt[start.page_idx].present = false;
                pt[start.page_idx].write = false;
                pt[start.page_idx].physical_address = 0;
            }
        }
        if(result) tlb_flush();
        return result;
    }
    vaddr_t map_mmio_pages(vaddr_t start, size_t pages)
    {
        if(!start) return nullptr;
        vaddr_t curr{ start };
        paging_table pt = __get_table(curr, true);
        if(!pt) return nullptr;
        for(size_t i = 0; i < pages; i++, curr += PAGESIZE)
        {
            if(i != 0 && curr.page_idx == 0) pt = __get_table(curr, true);
            pt[curr.page_idx].present = true;
            pt[curr.page_idx].write = true;
            pt[curr.page_idx].write_thru = true;
            pt[curr.page_idx].physical_address = uint64_t(curr) >> 12;
        }
        tlb_flush();
        return start;
    }
}