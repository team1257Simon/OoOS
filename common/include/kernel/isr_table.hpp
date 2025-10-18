#ifndef __ISR_TABLE
#define __ISR_TABLE
#include "stdint.h"
#include "kernel/libk_decls.h"
#include "kernel_api.hpp"
#include "functional"
#include "vector"
// The current highlight of this OS (if you can call it that) is that I, an insane person, decided to make it possible to use lambdas for ISRs.
#define LAMBDA_ISR(...) [&] __isrcall (__VA_ARGS__) -> void
typedef std::function<void()> irq_callback;
typedef std::function<void(byte, qword)> interrupt_callback;
namespace interrupt_table
{
    using ooos_kernel_module::isr_actor;
    bool add_irq_handler(byte idx, irq_callback&& handler);
    bool add_irq_handler(void* owner, byte idx, isr_actor&& handler);
    void deregister_owner(void* owner);
    void add_interrupt_callback(interrupt_callback&& cb);
}
#endif