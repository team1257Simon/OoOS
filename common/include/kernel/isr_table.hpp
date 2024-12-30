#ifndef __ISR_TABLE
#define __ISR_TABLE
#include "stdint.h"
#include "functional"
#include "kernel/libk_decls.h"
#include "vector"
using runnable = std::function<void()>;
using interrupt_callback = std::function<void(byte)>;
namespace interrupt_table
{
    bool add_irq_handler(byte idx, runnable&& handler);
    void add_interrupt_callback(interrupt_callback&& cb);
}
#endif