#include "libk_decls.h"
#include "arch/idt_amd64.h"
#include "unordered_map"
#include "isr_table.hpp"
#include "kernel_mm.hpp"
#include "array"
#include "string"
#include "kdebug.hpp"
#include "arch/apic.hpp"
std::array<std::unordered_map<void*, ooos::isr_actor>, 16UZ> __managed_handlers{};
std::array<std::vector<irq_callback>, 16UZ> __handler_tables{};
std::vector<interrupt_callback> __registered_callbacks{};
uintptr_t saved_stack_ptr{};
extern volatile apic bsp_lapic;
namespace interrupt_table
{
    spinlock_t __itable_mutex;
    void __lock() { lock(std::addressof(__itable_mutex)); }
    void __unlock() { release(std::addressof(__itable_mutex)); }
    void deregister_owner(void* owner) { if(owner) { __lock(); for(std::unordered_map<void*, ooos::isr_actor>& m : __managed_handlers) m.erase(owner); __unlock(); }  }
    bool add_irq_handler(void* owner, byte idx, ooos::isr_actor&& handler) { if(idx < 16UC && owner) { __lock(); bool result = __managed_handlers[idx].emplace(owner, std::forward<ooos::isr_actor>(handler)).second; __unlock(); return result; } return false; }
    bool add_irq_handler(byte idx, irq_callback&& handler) { if(idx < 16UC) { __lock(); __handler_tables[idx].push_back(std::move(handler)); __unlock(); return __handler_tables[idx].size() == 1; } return false; }
    void add_interrupt_callback(interrupt_callback&& cb) { __registered_callbacks.push_back(std::move(cb)); }
}
inline void pic_eoi(byte irq) 
{
    if(bsp_lapic.valid()) { bsp_lapic.eoi(); return; }
    if(irq > 7) outb(command_pic2, sig_pic_eoi); 
    outb(command_pic1, sig_pic_eoi); 
}
extern "C"
{
    extern uint64_t ecode;
    extern void* isr_table[];
    extern idt_entry_t idt_table[256];
    extern void no_waiting_op();
    void (*callback_8)() = no_waiting_op;
    extern volatile bool delay_flag;
    struct 
    {
        uint16_t size;
        void* idt_ptr;
    } __pack idt_descriptor{};
    extern void idt_register();
    constexpr static void idt_set_descriptor(uint8_t vector, addr_t isr);
    void exception_handler(int exception_number, void* exception_address) { idt_set_descriptor(static_cast<uint8_t>(exception_number), exception_address); }
    constexpr static void idt_set_descriptor(uint8_t vector, addr_t isr)
    {
        new(static_cast<void*>(std::addressof(idt_table[vector]))) idt_entry_t
        {
            .isr_low    { static_cast<uint16_t>(isr.full & 0xFFFFUS) },
            .kernel_cs  { 0x8US },
            .ist        { static_cast<uint8_t>((vector < 0x30UC) ? 1 : 0) },
            .attributes { 0xEEUC },
            .isr_mid    { static_cast<uint16_t>((isr.full >> 16) & 0xFFFFUS) },
            .isr_high   { static_cast<uint32_t>((isr.full >> 32) & 0xFFFFFFFFU) }
        };
    }
    void isr_dispatch(uint8_t idx)
    {
        if(idx == 0x28UC) { (*callback_8)(); delay_flag = false; callback_8 = no_waiting_op; }
        bool is_err = (idx == 0x08UC || (idx > 0x09UC && idx < 0x0FUC) || idx == 0x11UC || idx == 0x15UC || idx == 0x1DUC || idx == 0x1EUC);
        asm volatile("movq %%rsp, %0" : "=m"(saved_stack_ptr) :: "memory");
        if(idx > 0x19UC && idx < 0x30UC) 
        { 
            byte irq{ uint8_t(idx - 0x20UC) };
            kernel_memory_mgr::suspend_user_frame();
            kfx_save();
            for(std::pair<void* const, ooos::isr_actor>& p : __managed_handlers[irq]) p.second();
            for(irq_callback const& h : __handler_tables[irq]) h();
            pic_eoi(irq);    
            kfx_load();
            kernel_memory_mgr::resume_user_frame();
        }
        else 
        { 
            kernel_memory_mgr::suspend_user_frame();
            kfx_save();
            for(interrupt_callback const& c : __registered_callbacks) { if(c) c(idx, is_err ? ecode : 0); }    
            kfx_load();
            kernel_memory_mgr::resume_user_frame();
        }
        // Other stuff as needed
    }
    void idt_init()
    {
        pic_remap<0x20UC, 0x28UC>();
        for(int i = 0; i < 256; i++) idt_set_descriptor(i, isr_table[i]);
        idt_descriptor.size     = 4095US;
        idt_descriptor.idt_ptr  = idt_table;
        idt_register();
    }
}