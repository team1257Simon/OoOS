#ifndef __APIC
#define __APIC
#include "arch/arch_amd64.h"
struct attribute(packed, aligned(16)) apic_reg
{
	uint32_t value;
	uint32_t align_bytes[3];
};
struct attribute(packed, aligned(16)) apic_map
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
};
struct __pack ioapic
{
	uint32_t select_reg;
	uint32_t unused[3];
	uint32_t data_reg;
};
class apic
{
	apic_map volatile* __apic_mem;
	ioapic volatile* __ioapic_mem;
	unsigned int __local_id;
public:
	inline bool valid() const volatile { return __apic_mem != nullptr && __ioapic_mem != nullptr; }
	inline apic_map volatile* get_map() volatile { return __apic_mem; }
	inline apic_map const volatile* get_map() const volatile { return __apic_mem; }
	inline int get_id() const volatile noexcept { return __local_id; }
	apic(unsigned id = 0);
	void eoi() volatile;
	bool init() volatile;
};
#endif