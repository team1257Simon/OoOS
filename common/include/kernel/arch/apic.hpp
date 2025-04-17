#ifndef __APIC
#define __APIC
#include "arch/arch_amd64.h"
typedef uint32_t& dword_reference;
typedef uint32_t&& dword_rvalue_reference;
typedef uint32_t const& dword_const_reference;
struct apic_reg 
{
    uint32_t value;
    constexpr operator dword_reference() & noexcept { return value; }
    constexpr operator dword_const_reference() const& noexcept { return value; }
    constexpr operator dword_rvalue_reference() && noexcept { return std::move(value); }
    constexpr apic_reg(uint32_t val = 0U) : value(val) {}
}  __align(16);
struct apic_map
{
    int128_t rsv0[2]; 
    apic_reg lapic_id;
    apic_reg vers;
    int128_t rsv1[4];
    apic_reg tpr;
    apic_reg apr;
    apic_reg ppr;
    apic_reg eoi;
    apic_reg rrd;
    apic_reg logical_dest;
    apic_reg dest_fmt;
    apic_reg spurious_iv;
    apic_reg in_service[8];
    apic_reg trigger_mode[8];
    apic_reg irr[8];
    apic_reg error;
    int128_t rsv2[6];
    apic_reg lvc_cmci;
    apic_reg icr0;
    apic_reg icr1;
    apic_reg lvt_timer;
    apic_reg lvt_thermal;
    apic_reg lvt_performance;
    apic_reg lvt_lint0;
    apic_reg lvt_lint1;
    apic_reg lvt_err;
    apic_reg timer_count_init;
    apic_reg timer_count_curr;
    int128_t rsv3[4];
    apic_reg timer_divide;
    int128_t rsv4;
} __align(16);
class apic
{
    apic_map* __apic_mem;
    unsigned int __local_id;
    static apic_map* __enable(uintptr_t physical_base);
public:
    apic(uintptr_t base, unsigned int id = 0U);
    constexpr apic_map& get_map() { return *__apic_mem; }
    constexpr apic_map const& get_map() const { return *__apic_mem; }
};
#endif