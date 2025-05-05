#ifndef __HIGH_PRECISION_TIMER
#define __HIGH_PRECISION_TIMER
#include "arch/arch_amd64.h"
struct hpet_desc_table
{
    acpi_header header; // "HPET"
    struct
    {
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
struct hpet
{
    uint8_t rev_id;
    uint8_t comparator_count        : 5;
    bool counter_is_64_bits         : 1;
    bool                            : 1;
    bool legacy_replacement_capable : 1;
    uint16_t pci_vendor_id          : 16;
    uint32_t period                 : 32;   //  period in femtoseconds (1 nanosecond = 1 million femtoseconds)
    uint64_t                        : 64;
    uint64_t general_config         : 64;   // bit 0 enables interrupts; bit 1 enables legacy replacement. All other bits are reserved and should be kept as-is
    uint64_t                        : 64;
    qword timer_interrupt_status;           // bit n represents the interrupt for timer n. The high 32 bits are always 0
    uint64_t main_counter;
    struct timer_n_t
    {
        struct
        {
            bool                        : 1;
            bool level_trigger          : 1;
            bool interrupt_enable       : 1;
            bool periodic_enable        : 1;
            bool periodic_capable       : 1;
            bool is_64_bits             : 1;
            bool timer_value_set        : 1;
            bool                        : 1;
            bool force_32_bits          : 1;
            uint8_t ioapic_irq_route    : 5;
            bool use_fsb_enable         : 1;
            bool use_fsb_supported      : 1;
            uint16_t                    : 16;
            uint32_t allowed_irq_routes : 32;
        } __pack;
        uint64_t comparator_value;
        uint32_t fsb_interrupt_value;
        uint32_t fsb_interrupt_addr;
    } __pack timers[32];                    // there may be as many as 32 timers; probably fewer, but the memory section is the same size regardless
} __pack;
#endif