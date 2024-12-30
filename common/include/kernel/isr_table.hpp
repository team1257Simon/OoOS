#ifndef __ISR_TABLE
#define __ISR_TABLE
#include "stdint.h"
#include "functional"
#include "kernel/libk_decls.h"
#include "vector"
using irq_handler = void(*)();
namespace irq_table
{
    void add_handler(uint8_t idx, irq_handler const& handler);
}
#endif