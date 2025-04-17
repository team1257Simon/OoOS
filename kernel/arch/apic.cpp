#include "arch/apic.hpp"
#include "kernel_mm.hpp"
apic_map* apic::__enable(uintptr_t physical_base)
{
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
apic::apic(uintptr_t base, unsigned int id) :
    __apic_mem  { __enable(base) },
    __local_id  { id } 
                {}