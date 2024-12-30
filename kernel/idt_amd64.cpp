#include "libk_decls.h"
#include "arch/idt_amd64.h"
#include "kernel/isr_table.hpp"
#include "isr_table.hpp"
std::vector<irq_handler> __handler_tables[16]{};
void irq_table::add_handler(uint8_t idx, irq_handler const &handler) { if(idx >= 16) return;  __handler_tables[idx].push_back(handler); }
void pic_eoi(byte irq ) { if (irq > 7) { outb(command_pic2, sig_pic_eoi); } outb(command_pic1, sig_pic_eoi); }
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
    void isr_dispatch(uint8_t idx)
    {
        if(idx >= 0x20 && idx < 0x30) 
        { 
            byte irq = idx - 0x20;
            for(irq_handler h : __handler_tables[irq]) h();
            pic_eoi(irq);
        }
        // Other stuff as needed
    }
    void idt_init()
    {
        cli();
        pic_remap<0x20, 0x28>();
        for(int i = 0; i < 256; i++) idt_set_descriptor(i, isr_table[i]);
        idt_register();
        sti();
    }
}