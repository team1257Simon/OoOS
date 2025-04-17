#include "arch/apic.hpp"
#include "kernel_mm.hpp"
static apic_map* __find_and_enable();
apic::apic(unsigned id) : __apic_mem(__find_and_enable()), __local_id{ id } {}
void apic::eoi() volatile { __apic_mem->eoi.value = 0; } // writing a 0 to this register signals an eoi in apic mode
static apic_map* __find_and_enable()
{
    madt_t* madt = static_cast<madt_t*>(find_system_table("APIC"));
    if(!madt) return nullptr;
    uintptr_t physical_base = madt->local_apic_physical_address;
    size_t table_size = madt->header.length - (8 + sizeof(acpi_header));
    addr_t table_ptr(madt->record_data);
    madt_record_header* h;
    for(size_t i = 0; i < table_size; i += h->length)
    {
        h = table_ptr.plus(i);
        if(h->type == APIC_ADDRESS_OVERRIDE)
        {
            local_apic_addr_override* o = table_ptr.plus(i);
            physical_base = o->local_apic_physical_addr;
            break;
        }
    }
    addr_t the_apic = kmm.map_mmio_region(physical_base, sizeof(apic_map));
    outb(data_pic1, 0xFFui8);
    outb(data_pic2, 0xFFui8);
    write_msr<ia32_apic_base>(qword((physical_base & 0xFFFFF000) | apic_enable, (physical_base >> 32) & 0x0F));
    apic_map* result = the_apic;
    result->logical_dest = 0x0FU << 24;
    barrier();
    result->dest_fmt = 0xFFFFFFFFU;
    barrier();
    result->spurious_iv |= 0x1FFU;
    barrier();
    return result;
}