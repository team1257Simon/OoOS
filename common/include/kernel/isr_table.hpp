#ifndef __ISR_TABLE
#define __ISR_TABLE
#include "stdint.h"
#include "functional"
#include "kernel/libk_decls.h"
#include "vector"
using irq_handler = std::function<void()>;
namespace irq_table
{
    void add_handler(uint8_t idx, irq_handler handler);
}
#endif