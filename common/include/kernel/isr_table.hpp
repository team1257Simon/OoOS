#ifndef __ISR_TABLE
#define __ISR_TABLE
#include "stdint.h"
#include "kernel/libk_decls.h"
#include "kernel_api.hpp"
#include "functional"
#include "vector"
typedef std::function<void()> irq_callback;
typedef std::function<void(byte, qword)> interrupt_callback;
namespace interrupt_table
{
    bool add_irq_handler(byte idx, irq_callback&& handler);
    bool add_irq_handler(void* owner, byte idx, ooos::isr_actor&& handler);
    void deregister_owner(void* owner);
    void add_interrupt_callback(interrupt_callback&& cb);
}
#endif