#include "arch/ahci.hpp"
#include "arch/arch_amd64.h"
#include "isr_table.hpp"
#include "kernel_mm.hpp"
#include "stdexcept"
#include "functional"
#include "string.h"
#include "kdebug.hpp"
constexpr size_t cl_offs_base = 32UL * sizeof(hba_cmd_header);
constexpr size_t fis_offs_base = 32UL * cl_offs_base;
constexpr uint64_t table_offs_base = fis_offs_base + 32UL * sizeof(hba_fis);
constexpr size_t prdt_size = prdt_entries_count * sizeof(hba_prdt_entry);
ahci ahci::__instance{};
bool ahci::__has_init = false;
static inline bool __port_data_busy(hba_port* port) { barrier(); dword i = port->task_file; barrier(); return i.lo.lo.b7 /* busy */ || i.lo.lo.b3; /*drq*/ }
static inline bool __port_cmd_busy(hba_port* port, int slot) { barrier(); dword i = port->cmd_issue; barrier(); dword j = port->i_state; barrier(); return !j.lo.lo.b5 /* processed */ && i[slot]; }
static inline bool __port_nack_stop(hba_port* port) { barrier(); dword i = port->cmd; barrier(); return i.lo.hi.b7 /* cr */ || i.lo.hi.b6; /* fr */ }
ahci *ahci::get_instance() { return std::addressof(__instance); }
bool ahci::is_initialized() { return __has_init; }
bool ahci::has_port(uint8_t i) { return __devices[i] != none; }
void ahci::hard_reset_fallback(uint8_t idx) { xklog("[AHCI] hard reset required for port " + std::to_string(idx) + "\n"); port_hard_reset(idx); }
bool ahci::__handoff_busy() { return (dword(__abar->bios_os_handoff)[bos_bit] || dword(__abar->bios_os_handoff)[bb_bit]); }
int8_t ahci::which_port(ahci_device d) { for(int8_t i = 0; i < 32SC; i++) if(__devices[i] == d) return i; return -1SC; }
ahci_device ahci::get_device_type(uint8_t i) { return __devices[i]; }
uint32_t ahci::last_read_count(uint8_t i) { barrier(); return __abar->ports[i].command_list[__last_command_on_port[i]].prd_count; }
bool ahci::is_busy(uint8_t i) { barrier(); uint32_t c = __abar->ports[i].cmd; barrier(); return __port_data_busy(std::addressof(__abar->ports[i])) || (c & hba_command_cr) == 0; }
bool ahci::is_done(uint8_t i) { uint32_t st = __abar->ports[i].i_state; barrier(); return !(st == 0 || __port_cmd_busy(std::addressof(__abar->ports[i]), __last_command_on_port[i])) && !is_busy(i); }
void ahci::__init_irq() 
{ 
    for(int i = 0; i < 32; i++, __sync_synchronize())
    {
        if(has_port(i))
        {
            __abar->i_status |= BIT(i); 
            barrier(); 
            uint32_t s1 = __abar->ports[i].i_state; 
            barrier(); 
            __abar->ports[i].i_state = s1;
            barrier();
        }
    }
    interrupt_table::add_irq_handler(__pci_ahci_controller->header_0x0.interrupt_line, std::bind(&ahci::handle_irq, this)); 
}
static inline ahci_device check_type(hba_port const& p)
{
    barrier();
    if((dword(p.s_status).lo.lo & 0x0FUC) != port_present || (dword(p.s_status).lo.hi & 0x0FUC) != port_active) return none;
    barrier();
    switch(p.sig)
    {
    case sig_atapi: return atapi;
    case sig_semb:  return semb;
    case sig_pmul:  return pmul;
    case sig_sata:  return sata;
    default:        return none;
    }
}
void ahci::__issue_command(uint8_t idx, int slot)
{
    hba_port* port = std::addressof(__abar->ports[idx]);
    if(!await_result([&]() -> bool { return !__port_data_busy(port); })) { throw std::runtime_error("[AHCI] port number " + std::to_string(idx) + " is hung"); }
    barrier();
    port->cmd_issue |= BIT(slot);
    barrier();
    dword st;
    if(!await_result([&]() -> bool { barrier(); st = port->i_state; barrier(); if(st.hi.hi & i_state_hi_byte_error) { throw std::runtime_error("[AHCI] port number " + std::to_string(idx) + " i/o error"); } return !__port_data_busy(port); }, max_ext_wait)) { throw std::runtime_error("Port number " + std::to_string(idx) + " hardware error"); }
    barrier();
    __last_command_on_port[idx] = slot;
}
void ahci::__build_h2d_fis(qword start, dword count, addr_t buffer, ata_command command, hba_cmd_table* cmdtbl, uint16_t l)
{
    constexpr size_t main_op_bcnt = physical_block_size << 4;
    barrier();
    memset(cmdtbl, 0, sizeof(hba_cmd_table));
    barrier();
    qword addr = buffer;
    size_t final_bcnt = (count % 16) * physical_block_size;
    for(uint16_t i = 0; i < l; i++, addr += main_op_bcnt)
    {
        barrier();
        new(std::addressof(cmdtbl->prdt_entries[i])) hba_prdt_entry 
        {
            .data_base = addr.lo,
            .data_base_hi = addr.hi,
            .byte_count = static_cast<uint32_t>((i == l - 1 ? final_bcnt : main_op_bcnt) - 1),
            .interrupt_on_completion = true,
        };
        barrier();
    }
    barrier();
    new(static_cast<void*>(cmdtbl->cmd_fis)) fis_reg_h2d
	{
		.type = reg_h2d,
		.ctype = true,
		.command = command,
		.lba0 = start.lo.lo.lo,
		.lba1 = start.lo.lo.hi,
		.lba2 = start.lo.hi.lo,
		.device = 0x40UC,
		.lba3 = start.lo.hi.hi,
		.lba4 = start.hi.lo.lo,
		.lba5 = start.hi.lo.hi,
		.count = count.lo,
	};
    barrier();
}
bool ahci::init_instance(pci_device_list* ls) noexcept
{
    if(__has_init) return true;
    if(!ls) return false;
    return (__has_init = __instance.init(ls->find(dev_class_storage_controller, subclass_sata_controller)));
}
bool ahci::init(pci_config_space* ps) noexcept
{
    if(!ps) return false;
    try
    {
        __pci_ahci_controller = ps;
        uint32_t bar = ps->header_0x0.bar[5];
        __abar = compute_base(bar);
        __ahci_block = kmm.allocate_dma(S128, (bar & BIT(3)) != 0);
        barrier();
        __pci_ahci_controller->command.bus_master = true;
        __pci_ahci_controller->command.memory_space = true;
        __pci_ahci_controller->command.interrupt_disable = false;
        barrier();
        __abar->ghc |= hba_enable_ahci;
        barrier();
        if(__abar->bios_os_handoff_sup)
        {
            __abar->bios_os_handoff |= BIT(oos_bit);
            barrier();
            unsigned spin = 0;
            while(__handoff_busy() && spin < max_wait) { barrier(); spin++; }
            if(__handoff_busy()) { panic("[AHCI] bios handoff error"); return false; }
        }
        barrier();
        for(int i = 0; i < 32; i++)
        {
            if(!dword(__abar->p_implemented)[i]) continue;
            __num_ports += 1;
            __devices[i] = check_type(__abar->ports[i]);
            barrier();
            if(__devices[i] == none) continue;
            try
            {
                port_soft_reset(i);
                port_rebase(i);
                barrier();
                __abar->ports[i].cmd |= port_power_on;
                barrier();
                uint32_t e = __abar->ports[i].s_err;
                barrier();
                __abar->ports[i].s_err = e;
                barrier();
            }
            catch(std::exception& e) { panic(e.what()); xpanic("[AHCI] skipping port " + std::to_string(i)); __devices[i] = none; }
        }
        for(int i = 0; i < 32; i++) { if(has_port(i)) { __init_irq(); return true; } }
    }
    catch(std::exception& e) { panic(e.what()); return false; }
    panic("[AHCI] no devices");
    return false;
}
void ahci::start_port(uint8_t i)
{
    if(i > __num_ports) throw std::out_of_range("[AHCI] port " + std::to_string(i) + " is out of range ");
    if(!await_result([&]() -> bool { return !dword(__abar->ports[i].cmd).lo.hi.b7; /* CR */ }, max_ext_wait)) throw std::runtime_error("Port number " + std::to_string(i) + " is hung");
    barrier();
    __abar->ports[i].cmd |= hba_command_fre;
    barrier();
    __abar->ports[i].cmd |= hba_cmd_start;
}
void ahci::stop_port(uint8_t i)
{
    if(i > __num_ports) throw std::out_of_range("[AHCI] port " + std::to_string(i) + " is out of range");
    barrier();
    __abar->ports[i].cmd &= ~hba_cmd_start;
    barrier();
    __abar->ports[i].cmd &= ~hba_command_fre;
    barrier();
    if(!await_result([&]() -> bool { return !__port_nack_stop(std::addressof(__abar->ports[i])); }, max_ext_wait)) throw std::runtime_error("Port number " + std::to_string(i) + " is hung");
}
void ahci::port_rebase(uint8_t idx)
{
    stop_port(idx);
    addr_t cl_base = __ahci_block.plus(idx * cl_offs_base);
    addr_t f_base = __ahci_block.plus(fis_offs_base + idx * sizeof(hba_fis));
    __abar->ports[idx].command_list = cl_base;
    memset(cl_base, 0, sizeof(hba_cmd_header) * 32);
    barrier();
    __abar->ports[idx].fis_receive = f_base;
    memset(f_base, 0, sizeof(hba_fis));
    barrier();
    for(int jdx = 0; jdx < 32; jdx++)
    {
        addr_t t_base = __ahci_block.plus(table_offs_base + idx * prdt_size + jdx * sizeof(hba_cmd_table));
        __abar->ports[idx].command_list[jdx].command_table = t_base;
        barrier();
        memset(t_base, 0, prdt_size);
    }
    start_port(idx);
}
static int find_cmdslot(hba_port* port)
{
    barrier();
    uint32_t slots = (port->s_active | port->cmd_issue);
    barrier();
    for(int i = 0; i < 32; i++) if(!(dword(slots)[i])) return i;
    return -1;
}
void ahci::port_soft_reset(uint8_t idx)
{
    hba_port* port = std::addressof(__abar->ports[idx]);
    if(!await_result([&]() -> bool { return port->cmd_issue == 0; })) throw std::runtime_error("[AHCI] port number " + std::to_string(idx) + " is hung");   
    barrier();
    port->cmd &= ~hba_cmd_start;
    barrier();
    await_result([&]() -> bool { return (port->cmd & hba_command_cr) == 0; });
    barrier();
    port->cmd |= hba_cmd_start;
    int s1 = -1, s2 = -1;
    barrier();
    dword slots = port->s_active | port->cmd_issue;
    barrier();
    for(int i = 0; i < 32 && s1 < 0 && s2 < 0; i++) { if(!slots[i]) { if(s1 < 0) s1 = i; else s2 = i; } } 
    if(s1 < 0 && s2 < 0) throw std::runtime_error("[AHCI] port number " + std::to_string(idx) + " has no available slots");
    barrier();
    hba_cmd_header* c1 = new(std::addressof(port->command_list[s1])) hba_cmd_header
    {
        .cmd_fis_len    = static_cast<uint8_t>(sizeof(fis_reg_h2d) / sizeof(uint32_t)),
        .reset          = true,
        .cl_busy        = true,
        .command_table  = port->command_list[s1].command_table
    };
    barrier();
    memset(c1->command_table, 0, sizeof(hba_cmd_table));
    barrier();
    new(static_cast<void*>(c1->command_table->cmd_fis)) fis_reg_h2d
	{
		.type       = reg_h2d,
		.ctype      = false,
		.control    = soft_reset_bit,
	};
    if(!await_result([&]() -> bool { return !__port_data_busy(port); })) { return hard_reset_fallback(idx); }
    barrier();
    port->s_active = BIT(s1);
    barrier();
    port->cmd_issue = BIT(s1);
    barrier();
    if(!await_result([&]() -> bool { return !__port_cmd_busy(port, s1) && !(port->i_state & hba_error); })) { return hard_reset_fallback(idx); }
    if(s2 < 0) s2 = s1;
    hba_cmd_header* c2 = new(std::addressof(port->command_list[s2])) hba_cmd_header
    {
        .cmd_fis_len    = static_cast<uint8_t>(sizeof(fis_reg_h2d) / sizeof(uint32_t)),
        .reset          = false,
        .cl_busy        = false,
        .command_table  = port->command_list[s2].command_table
    };
    barrier();
    memset(c2->command_table, 0, sizeof(hba_cmd_table));
    barrier();
    reinterpret_cast<fis_reg_h2d*>(c2->command_table->cmd_fis)->type = reg_h2d;
    barrier();
    port->s_active |= BIT(s2);
    barrier();
    port->cmd_issue |= BIT(s2);
    barrier();
    if(!await_result([&]() -> bool { return !__port_cmd_busy(port, s2) && !(port->i_state & hba_error); })) { return hard_reset_fallback(idx); }
    barrier();
    if(port->i_state & hba_error) { return hard_reset_fallback(idx); }
    barrier();
}
void ahci::port_hard_reset(uint8_t idx)
{
    if(__unlikely(!has_port(idx))) throw std::out_of_range("[AHCI] port " + std::to_string(idx) + " does not exist");
    hba_port* port = std::addressof(__abar->ports[idx]);
    barrier();
    stop_port(idx);
    await_result([&]() -> bool { return !dword(port->cmd).lo.hi.b7; /* CR */ });
    barrier();
    if(!(dword(port->cmd).lo.lo.b0 /* SUD */) && __abar->sud_supported) { barrier(); port->cmd |= hba_command_spin_up_disk; }
    barrier();
    port->s_control |= hba_control_det;
    barrier();
    for(unsigned spin = 0; spin < max_wait; barrier(), spin++);
    barrier();
    port->s_control &= ~hba_control_det;
    barrier();
    if(!await_result([&]() -> bool { return (dword(port->s_status).lo.lo & 0x0F) == port_present; })) { throw std::runtime_error("[AHCI] port number " + std::to_string(idx) + " failed to start"); }
    barrier();
    uint32_t bits = port->s_err;
    barrier();
    port->s_err |= bits;
    barrier();
    start_port(idx);
}
void ahci::read_sectors(uint8_t idx, qword start, dword count, uint16_t* buffer)
{
    if(__unlikely(!has_port(idx))) { throw std::out_of_range("[AHCI] port " + std::to_string(idx) + " does not exist"); }
    hba_port* port  = std::addressof(__abar->ports[idx]);
    int slot        = find_cmdslot(port);
    barrier();
    if(__unlikely(slot < 0)) throw std::runtime_error("[AHCI] port number " + std::to_string(idx) + " has no available slots");
    hba_cmd_header* cmd = new(std::addressof(port->command_list[slot])) hba_cmd_header
	{
		.cmd_fis_len    = static_cast<uint8_t>(sizeof(fis_reg_h2d) / sizeof(uint32_t)),
		.atapi          = false,
		.w_direction    = false, // d2h write
		.prdt_length    = static_cast<uint16_t>(div_round_up(count, 16UL)), // each entry can transfer at most 8kb, or 16 sectors
        .command_table  = port->command_list[slot].command_table
	};
    barrier();
    __build_h2d_fis(start, count, buffer, ata_command::read_dma_ext, cmd->command_table, cmd->prdt_length);
    __issue_command(idx, slot);
    await_result([&]() -> bool { return !is_busy(idx); });
}
void ahci::write_sectors(uint8_t idx, qword start, dword count, const uint16_t* buffer)
{
    if(__unlikely(!has_port(idx))) throw std::out_of_range("[AHCI] port " + std::to_string(idx) + " does not exist");
    hba_port* port  = std::addressof(__abar->ports[idx]);
    int slot        = find_cmdslot(port);
    barrier();
    if(__unlikely(slot < 0)) throw std::runtime_error("[AHCI] port number " + std::to_string(idx) + " has no available slots");
    hba_cmd_header* cmd = new(std::addressof(port->command_list[slot])) hba_cmd_header
	{
		.cmd_fis_len    = static_cast<uint8_t>(sizeof(fis_reg_h2d) / sizeof(uint32_t)),
		.atapi          = false,
		.w_direction    = true, // h2d write
		.prdt_length    = static_cast<uint16_t>(div_round_up(count, 16UL)), // each entry can transfer at most 8kb, or 16 sectors
        .command_table  = port->command_list[slot].command_table
	};
    barrier();
    __build_h2d_fis(start, count, buffer, ata_command::write_dma_ext, cmd->command_table, cmd->prdt_length);
    __issue_command(idx, slot);
    await_result([&]() -> bool { return !is_busy(idx); });
}
__isrcall void ahci::handle_irq()
{
    uint32_t s = __abar->i_status;
	barrier();
    __abar->i_status = s;
    if(__unlikely(!s)) return; // This means the IRQ is for someone else
    for(size_t i = 0; i < 32; i++)
    {
        uint32_t s1 = __abar->ports[i].i_state;
        barrier();
        __abar->ports[i].i_state = s1;
        barrier();
        if(has_port(i) && (dword(s)[i]) && (s1 & hba_error)) port_soft_reset(i);
        barrier();
    }
}
void ahci::p_identify(uint8_t idx, identify_data* data)
{
    if(__unlikely(!has_port(idx))) throw std::out_of_range("port " + std::to_string(idx) + " is out of range ");
    hba_port* port  = std::addressof(__abar->ports[idx]);
    int slot        = find_cmdslot(port);
    if(__unlikely(slot < 0)) throw std::runtime_error("Port number " + std::to_string(idx) + " has no available slots");
    qword addr = reinterpret_cast<uintptr_t>(data);
    barrier();
    hba_cmd_header* cmd = new(std::addressof(port->command_list[slot])) hba_cmd_header
    {
        .cmd_fis_len    = 5UC,
        .w_direction    = false,
        .prdt_length    = true,
        .command_table  = new(port->command_list[slot].command_table) hba_cmd_table { .prdt_entries { { .data_base = addr.lo, .data_base_hi = addr.hi, .byte_count = 511U, .interrupt_on_completion = true } } }
    };
    barrier();
    new(static_cast<void*>(cmd->command_table->cmd_fis)) fis_reg_h2d
	{
		.type       = reg_h2d,
		.ctype      = true,
		.command    = identify, 
		.device     = 0UC
	};
    try { __issue_command(idx, slot); } catch(std::exception& e) { panic(e.what()); panic("error on port identify; attempt soft reset"); port_soft_reset(idx); }
    await_result([&]() -> bool { return is_done(idx); });
}