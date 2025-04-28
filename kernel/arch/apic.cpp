#include "arch/apic.hpp"
#include "kernel_mm.hpp"
apic::apic(unsigned id) : __apic_mem{ nullptr }, __local_id{ id } {}
void apic::eoi() volatile { if(__apic_mem) __apic_mem->eoi.value = 0; } // writing a 0 to this register signals an eoi in apic mode
bool apic::init() volatile
{
    madt_t* madt = static_cast<madt_t*>(find_system_table("APIC"));
    if(!madt) return false;
    uintptr_t physical_base = madt->local_apic_physical_address;
    uintptr_t ioapic_physical_base = 0xFEC00000UL;
    bool have_sapic = false;
    size_t table_size = madt->header.length - (8 + sizeof(acpi_header));
    addr_t table_ptr(madt->record_data);
    madt_record_header* h;
    for(size_t i = 0; i < table_size; i += h->length)
    {
        h = table_ptr.plus(i);
        switch(h->type)
        {
        case APIC_ADDRESS_OVERRIDE:
            physical_base = table_ptr.plus(i).ref<local_apic_addr_override>().local_apic_physical_addr;
            break;
        case IO_SAPIC:
            have_sapic = true;
            ioapic_physical_base = table_ptr.plus(i).ref<io_sapic_data>().io_sapic_physical_addr;
            break;
        case IO_APIC:
            if(!have_sapic) ioapic_physical_base = table_ptr.plus(i).ref<io_apic_data>().io_apic_physical_address;
            break;
        default:
            break;
        }
    }
    if(!ioapic_physical_base) return false;
    addr_t the_apic = kmm.map_mmio_region(physical_base, sizeof(apic_map));
    addr_t the_ioapic = kmm.map_mmio_region(ioapic_physical_base, sizeof(ioapic));
    outb(data_pic1, 0xFFUC);
    outb(data_pic2, 0xFFUC);
    write_msr<ia32_apic_base>(qword((physical_base & 0xFFFFF000) | apic_enable, (physical_base >> 32) & 0x0F));
    __apic_mem = the_apic;
    __ioapic_mem = the_ioapic;
    __apic_mem->logical_dest.value = (0x0F << 24);
    barrier();
    __apic_mem->dest_fmt.value = 0xFFFFFFFFU;
    barrier();
    __apic_mem->spurious_iv.value = 0x1FF;
    barrier();
    for(size_t i = 0; i < 16; i++)
    {
        __ioapic_mem->select_reg = (0x10 + i * 2);
        barrier();
        uint32_t data = __ioapic_mem->data_reg;
        barrier();
        data &= ~0x100FF;
        data |= (0x20 + i);
        __ioapic_mem->data_reg = data;
        barrier();
    }
    return true;
}