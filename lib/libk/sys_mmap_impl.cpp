#include "kernel/libk_decls.h"
#include "kernel/heap_allocator.hpp"
static paging_table build_new_pt(paging_table in, uint16_t idx)
{
    paging_table result = heap_allocator::get().allocate_pt();
    if(result)
    {
        in[idx].present = true;
        in[idx].write = true;
        in[idx].physical_address = std::bit_cast<uintptr_t>(result) >> 12;
    }
    return result;
}
static paging_table __get_table(vaddr_t const& of_page)
{
    paging_table pml4 = get_cr3();
    if (pml4[of_page.pml4_idx].present)
    {
        paging_table pdp = vaddr_t{ pml4[of_page.pml4_idx].physical_address << 12 };
        if(pdp[of_page.pdp_idx].present) 
        {
            paging_table pd = vaddr_t { pdp[of_page.pdp_idx].physical_address << 12 };
            if(static_cast<paging_table>(pd)[of_page.pd_idx].present) return vaddr_t { pd[of_page.pd_idx].physical_address << 12 };
            else return build_new_pt(pd, of_page.pd_idx);
            
        }
        else 
        {
            paging_table pd = build_new_pt(pdp, of_page.pdp_idx);
            if(pd) return build_new_pt(pd, of_page.pd_idx);
            
        }
    }
    else
    {
        paging_table pdp = build_new_pt(pml4, of_page.pml4_idx);
        if(pdp)
        {
            paging_table pd = build_new_pt(pdp, of_page.pdp_idx);
            if(pd) return build_new_pt(pd, of_page.pd_idx);
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
            paging_table pd = vaddr_t { pdp[of_page.pdp_idx].physical_address << 12 };
            if(static_cast<paging_table>(pd)[of_page.pd_idx].present) return vaddr_t { pd[of_page.pd_idx].physical_address << 12 };
        }
    }
    return NULL;
}
uintptr_t translate_vaddr(vaddr_t addr)
{
    if(paging_table pt = __find_table(addr)) return (pt[addr.page_idx].physical_address << 12) | addr.offset;
    return 0;
}
extern "C" 
{
    vaddr_t sys_mmap(vaddr_t start, uintptr_t phys, size_t pages)
    {
        if(!start) return {};
        vaddr_t curr { start };
        for(size_t i = 0; i < pages; i++, curr += PAGESIZE, phys += PAGESIZE)
        {
            paging_table pt = __get_table(curr);
            if(!pt) 
            {
                tlb_flush();
                return {};
            }
            pt[curr.page_idx].present = true;
            pt[curr.page_idx].write = true;
            pt[curr.page_idx].physical_address = phys >> 12;
        }
        tlb_flush();
        return start;
    }

    uintptr_t sys_unmap(vaddr_t start, size_t pages)
    {
        if(!start) return 0;
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
}