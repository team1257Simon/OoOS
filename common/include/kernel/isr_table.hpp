#ifndef __ISR_TABLE
#define __ISR_TABLE
#include "stdint.h"
#include "kernel/libk_decls.h"
#include "functional"
#include "vector"
// The current highlight of this OS (if you can call it that) is that I, an insane person, decided to make it possible to use lambdas for ISRs.
#define LAMBDA_ISR(...) [&] __isrcall (__VA_ARGS__) -> void
typedef std::function<void()> irq_callback;
typedef std::function<void(byte, qword)> interrupt_callback;
namespace interrupt_table
{
    bool add_irq_handler(byte idx, irq_callback&& handler);
    void add_interrupt_callback(interrupt_callback&& cb);
}
#endif