#include "libk_decls.h"
#include "arch/idt_amd64.h"
#include "kernel/isr_table.hpp"
#include "isr_table.hpp"
std::vector<irq_callback> __handler_tables[16]{};
std::vector<interrupt_callback> __registered_callbacks{};
namespace interrupt_table
{
    spinlock_t __itable_mutex;
    void __lock() { lock (&__itable_mutex); }
    void __unlock() { release(&__itable_mutex); }
    bool add_irq_handler(byte idx, irq_callback&& handler) { if(idx < 16) { __lock(); __handler_tables[idx].push_back(handler); __unlock(); return __handler_tables[idx].size() == 1; } return false; }
    void add_interrupt_callback(interrupt_callback &&cb) { __registered_callbacks.push_back(cb); }
}
inline void pic_eoi(byte irq) { if (irq > 7) outb(command_pic2, sig_pic_eoi); outb(command_pic1, sig_pic_eoi); }
extern "C"
{
    extern uint64_t ecode;
    extern void* isr_table[];
    extern idt_entry_t* idt_table;
    struct 
    {
        uint16_t size;
        void* idt_ptr;
    } __pack idt_descriptor{};
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
    [[gnu::target("general-regs-only")]]
    [[gnu::no_caller_saved_registers]]
    bool isr_dispatch(uint8_t idx)
    {
        bool is_err = (idx == 0x08 || (idx > 0x09 && idx < 0x0F) || idx == 0x11 || idx == 0x15 || idx == 0x1D || idx == 0x1E);
        if(idx > 0x19 && idx < 0x30) 
        { 
            byte irq{ uint8_t(idx - 0x20ui8) };
            for(irq_callback h : __handler_tables[irq]) h();
            pic_eoi(irq);
        }
        else for(interrupt_callback c : __registered_callbacks) { c(idx, is_err ? ecode : 0); }
        // Other stuff as needed
        return is_err;
    }
    void idt_init()
    {
        cli();
        pic_remap<0x20, 0x28>();
        for(int i = 0; i < 256; i++) idt_set_descriptor(i, isr_table[i]);
        idt_descriptor.size = 4095;
        idt_descriptor.idt_ptr = &idt_table[0];
        idt_register();
        sti();
    }
}