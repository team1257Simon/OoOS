#ifndef __HIGH_PRECISION_TIMER
#define __HIGH_PRECISION_TIMER
#include "arch/arch_amd64.h"
#include "atomic"
struct hpet_desc_table
{
    acpi_header header; // "HPET"
    struct
    {
        uint8_t rev_id                      : 8;
        uint8_t comparator_count            : 5;
        bool counter_size_x64               : 1;
        bool                                : 1;
        bool legacy_replacement_capable     : 1;
        uint16_t pci_vendor_id              : 16;
    } __pack;
    generic_address_structure base_addr;
    uint8_t hpet_seq_num;
    uint16_t min_ticks;
    uint8_t page_prot;
} __pack;
constexpr uint64_t timer_n_irq_mask = 0x3E00UL;
constexpr int timer_n_irq_shift = 9;
constexpr uint64_t timer_n_allowed_interrupt_mask = 0xFFFFFFFF00000000;
constexpr int timer_n_allowed_interrupt_shift = 32;
constexpr uint64_t timer_n_interrupt_enable = 0x0004UL;
constexpr uint64_t timer_n_periodic_enable = 0x0008UL;
constexpr uint64_t timer_n_periodic_capable = 0x0010UL;
constexpr uint8_t comparator_count_mask = 0b11111;
struct hpet_t
{
    uint8_t rev_id;
    uint8_t comparator_info;
    uint16_t pci_vendor_id;
    uint32_t period;                        // period in femtoseconds (1 nanosecond = 1 million femtoseconds)
    uint64_t rsv0;
    uint64_t general_config;                // bit 0 enables interrupts; bit 1 enables legacy replacement. All other bits are reserved and should be kept as-is
    uint64_t rsv[25];
    uint64_t timer_interrupt_status;        // bit n represents the interrupt for timer n. The high 32 bits are always 0
    uint64_t rsv2;
    uint64_t main_counter;
    struct hpet_timer_t
    {
        uint64_t rsv;
        uint64_t caps_and_config;
        uint64_t comparator_value;
        uint32_t fsb_interrupt_value;
        uint32_t fsb_interrupt_addr;
    } timers[32];                           // there may be as many as 32 timers; probably fewer, but the memory section is the same size regardless
};
constexpr time_t period_dividend = 1000000000UL;    // period is the divisor; quotient is frequency in megahertz
typedef void (*awaiting_action)();
class hpet_amd64
{
    bool __has_init;
    hpet_t volatile* __hpet;
    time_t __frequency_megahertz;
    static hpet_amd64 __instance;
    hpet_amd64();
    bool __init();
public:
    static hpet_amd64& get();
    static bool init_instance();
    void delay_us(time_t usec);
    void delay_us(time_t usec, awaiting_action action);
    uint64_t read_counter();
    static void delay_usec(time_t usec);
    static void delay_usec(time_t usec, awaiting_action action);
    static uint64_t count();
    static time_t count_usec();
};
constexpr auto test = offsetof(hpet_t, main_counter);
#define hpet hpet_amd64::get()
#endif