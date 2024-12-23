#include "libk_decls.h"
#include "arch/idt_amd64.h"

extern "C"
{
    extern void* isr_table[];
    extern idt_entry_t* idt_table;
    extern void idt_register();
    static void idt_set_descriptor(uint8_t vector, void* isr)
    {
        idt_entry_t* descriptor    = &idt_table[vector];
        descriptor->isr_low        = reinterpret_cast<uint64_t>(isr) & 0xFFFF;
        descriptor->kernel_cs      = 0x8;
        descriptor->ist            = 0;
        descriptor->attributes     = 0x8E;
        descriptor->isr_mid        = (reinterpret_cast<uint64_t>(isr) >> 16) & 0xFFFF;
        descriptor->isr_high       = (reinterpret_cast<uint64_t>(isr) >> 32) & 0xFFFFFFFF;
        descriptor->reserved       = 0;
    }
    void isr_dispatch(uint8_t vector)
    {

    }
    void idt_init()
    {
        cli();
        for(int i = 0; i < 256; i++) idt_set_descriptor(i, isr_table[i]);
        idt_register();
        sti();
    }
}