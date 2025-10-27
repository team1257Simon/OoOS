#include "arch/apic.hpp"
#include "kernel_mm.hpp"
#include "kdebug.hpp"
constexpr uint64_t ioapic_default_physical_base = 0xFEC00000UL;
apic::apic(unsigned id) : __apic_mem{ nullptr }, __local_id{ id } {}
void apic::eoi() volatile { if(__apic_mem) __apic_mem->eoi.value = 0U; } // writing a 0 to this register signals an eoi in apic mode
bool apic::init() volatile
{
    madt_t* madt = static_cast<madt_t*>(find_system_table("APIC"));
    if(__unlikely(!madt)) return false;
    uintptr_t physical_base         = madt->local_apic_physical_address;
    uintptr_t ioapic_physical_base  = 0;
    bool have_sapic                 = false;
    size_t table_size               = madt->header.length - (8 + sizeof(acpi_header));
    addr_t table_ptr(madt->record_data);
    madt_record_header* h;
    for(size_t i = 0; i < table_size; i += h->length)
    {
        h = table_ptr.plus(i);
        switch(h->type)
        {
        case APIC_ADDRESS_OVERRIDE:
            if(table_ptr.plus(i).deref<local_apic_addr_override>().local_apic_physical_addr)
                physical_base = table_ptr.plus(i).deref<local_apic_addr_override>().local_apic_physical_addr;
            break;
        case IO_SAPIC:
            have_sapic = (table_ptr.plus(i).deref<io_sapic_data>().io_sapic_physical_addr != 0);
            if(have_sapic)
                ioapic_physical_base = table_ptr.plus(i).deref<io_sapic_data>().io_sapic_physical_addr;
            break;
        case IO_APIC:
            if(table_ptr.plus(i).deref<io_apic_data>().io_apic_physical_address && !have_sapic) 
                ioapic_physical_base = table_ptr.plus(i).deref<io_apic_data>().io_apic_physical_address;
            break;
        default:
            break;
        }
    }
    if(!ioapic_physical_base) ioapic_physical_base = ioapic_default_physical_base;
    addr_t the_apic             = kmm.map_dma(physical_base, sizeof(apic_map), false);
    addr_t the_ioapic           = kmm.map_dma(ioapic_physical_base, sizeof(ioapic), false);
    uint32_t frequency          = cpuid(0x15U, 0U).ecx;
    if(!frequency) frequency    = cpuid(0x16U, 0U).ecx;
    uint32_t count_target       = magnitude(frequency) * 1000;
    fence();
    outb(data_pic1, 0xFFUC);
    outb(data_pic2, 0xFFUC);
    write_msr<ia32_apic_base>(qword((physical_base & 0xFFFFF000U) | apic_enable, (physical_base >> 32) & 0x0FU));
    __apic_mem                          = the_apic;
    __ioapic_mem                        = the_ioapic;
    __apic_mem->logical_dest.value      = 0x0F000000U;
    barrier();
    __apic_mem->dest_fmt.value          = 0xFFFFFFFFU;
    barrier();
    __apic_mem->spurious_iv.value       = 0x1FFU;
    barrier();
    __apic_mem->timer_divide.value      = 0b1011U;
    barrier();
    uint32_t tvec                       = (__apic_mem->lvt_timer.value & 0xFFFEFF00U) | 0x20U;
    barrier();
    __apic_mem->lvt_timer.value         = tvec;
    barrier();
    __apic_mem->timer_count_init.value = count_target;
    barrier();
    for(size_t i = 0; i < 16; i++, barrier())
    {
        __ioapic_mem->select_reg    = (0x10U + i * 2);
        barrier();
        uint32_t data               = (__ioapic_mem->data_reg & ~0x100FFU) | (0x20U + i);
        barrier();
        __ioapic_mem->data_reg      = data;
    }
    fence();
    return true;
}