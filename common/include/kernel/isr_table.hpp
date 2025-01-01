#ifndef __ISR_TABLE
#define __ISR_TABLE
#include "stdint.h"
#include "functional"
#include "kernel/libk_decls.h"
#include "vector"
#define __isr_registers [[gnu::no_caller_saved_registers]] [[gnu::target("general-regs-only")]]
#define IRQ_LAMBDA() [&]() -> void __isr_registers 
using irq_callback = std::function<void()>;
using interrupt_callback = std::function<void(byte, qword)>;
namespace interrupt_table
{
    bool add_irq_handler(byte idx, irq_callback&& handler);
    void add_interrupt_callback(interrupt_callback&& cb);
}
#endif