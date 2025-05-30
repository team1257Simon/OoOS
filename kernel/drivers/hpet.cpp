#include "arch/hpet_amd64.hpp"
#include "kernel_mm.hpp"
#include "isr_table.hpp"
#include "kdebug.hpp"
extern "C"
{
    extern void no_waiting_op();
    extern void (*callback_8)();    // sounds mysterious but is just a function pointer for a one-off callback on IRQ 8
    extern volatile bool delay_flag;
}
constexpr static uint8_t get_irq_vector(hpet_t::hpet_timer_t const volatile& t) { return (t.caps_and_config & timer_n_irq_mask) >> timer_n_irq_shift; }
constexpr static void set_irq_vector(hpet_t::hpet_timer_t volatile& t, uint8_t n) { t.caps_and_config |= (n << timer_n_irq_shift); }
hpet_amd64 hpet_amd64::__instance{};
hpet_amd64::hpet_amd64() = default;
hpet_amd64& hpet_amd64::get() { return __instance; }
bool hpet_amd64::init_instance() { return __instance.__init(); }
uint64_t hpet_amd64::read_counter() { return __hpet->main_counter; }
uint64_t hpet_amd64::count() { return __instance.__hpet->main_counter; }
time_t hpet_amd64::count_usec() { return __instance.__hpet->main_counter / __instance.__frequency_megahertz; }
void hpet_amd64::delay_usec(time_t usec) { __instance.delay_us(usec); }
void hpet_amd64::delay_usec(time_t usec, void (*action)()) { __instance.delay_us(usec, action); }
bool hpet_amd64::__init()
{
    void* tbl = find_system_table("HPET");
    if(__unlikely(!tbl)) return false;
    hpet_desc_table* dtbl   = static_cast<hpet_desc_table*>(tbl);
    addr_t mapped           = kmm.map_dma(dtbl->base_addr.address, sizeof(hpet_t), false);
    if(__unlikely(!mapped)) return false;
    __hpet                  = mapped.as<hpet_t volatile>();
    uint32_t period         = __hpet->period;
    __frequency_megahertz   = period_dividend / period;
    set_irq_vector(__hpet->timers[2], 8UC);
    fence();
    uint64_t cfg    = __hpet->timers[2].caps_and_config;
    cfg             |= timer_n_interrupt_enable;
    barrier();
    __hpet->timers[2].caps_and_config = cfg;
    fence();
    cfg = __hpet->general_config;
    cfg |= 0x1UL;
    cfg &= ~0x2UL;
    barrier();
    __hpet->general_config = cfg;
    fence();
    return (__has_init = true);
}
void hpet_amd64::delay_us(time_t usec)
{
    delay_flag                          = true;
    time_t time_val                     = __frequency_megahertz * usec + __hpet->main_counter;
    __hpet->timers[2].comparator_value  = time_val;
    while(delay_flag) { pause(); if(__hpet->main_counter > time_val) delay_flag = false; }
}
void hpet_amd64::delay_us(time_t usec, void (*action)())
{
    delay_flag              = true;
    if(action) callback_8   = action;
    time_t time_val         = __frequency_megahertz * usec + __hpet->main_counter;
    __hpet->timers[2].comparator_value = time_val;
    while(delay_flag) { pause(); if(__hpet->main_counter > time_val) delay_flag = false; }
}