#include "arch/ahci.hpp"
#include "string.h"
#include "isr_table.hpp"
#include "arch/arch_amd64.h"
#include "heap_allocator.hpp"
#include "stdexcept"
constexpr size_t cl_offs_base{ 32uL * sizeof(hba_cmd_header) };
constexpr size_t fis_offs_base{ 32uL * cl_offs_base };
constexpr uint64_t table_offs_base{ fis_offs_base + 32uL * sizeof(hba_fis) };
constexpr size_t prdt_size{ prdt_entries_count * sizeof(hba_prdt_entry) };
static inline ahci_device check_type(hba_port const& p)
{
    BARRIER;
    if((dword(p.s_status).lo.lo & 0x0F) != port_present || (dword(p.s_status).lo.hi & 0x0F) != port_active) return none;
    BARRIER;
    switch(p.sig)
    {
    case sig_atapi:
        return atapi;
    case sig_semb:
        return semb;
    case sig_pmul:
        return pmul;
    case sig_sata:
        return sata;
    default:
        return none;
    }
}
static inline bool __port_data_busy(hba_port* port) { BARRIER; int i = port->task_file; BARRIER; return (dword(i)[busy_bit]) || (dword(i)[drq_bit]); }
static inline bool __port_cmd_busy(hba_port* port, int slot) { BARRIER; int i = port->cmd_issue; BARRIER; int j = port->i_state; BARRIER; return (dword(i)[slot]) && !(dword(j)[descriptor_processed_bit]); }
static inline bool __port_nack_stop(hba_port* port) { BARRIER; int i = port->cmd; BARRIER; return dword(i)[cr_bit] || dword(i)[fr_bit]; }
ahci_driver ahci_driver::__instance{};
bool ahci_driver::__has_init = false;
void ahci_driver::__issue_command(uint8_t idx, int slot)
{
    hba_port* port{ &(__my_abar->ports[idx]) };
    unsigned spin{ 0 };
    BARRIER;
    while(__port_data_busy(port) && spin < max_wait) { BARRIER; spin++; }
    BARRIER;
    if(__port_data_busy(port)) { throw std::runtime_error("Port number " + std::to_string(idx) + " is hung"); }
    BARRIER;
    port->cmd_issue |= BIT(slot);
    BARRIER;
    unsigned st;
    spin = 0;
    while(__port_cmd_busy(port, slot) && spin < max_ext_wait) { BARRIER; st = port->i_state; BARRIER; if(dword(st).hi.hi & i_state_hi_byte_error) { throw std::runtime_error("Port number " + std::to_string(idx) + " hardware error"); } spin++; BARRIER;}
    BARRIER;
    st = port->i_state;
    if(dword(st).hi.hi & i_state_hi_byte_error) { throw std::runtime_error("Port number " + std::to_string(idx) + " hardware error"); }
    BARRIER;
    __last_command_on_port[idx] = slot;
}
void ahci_driver::__build_h2d_fis(qword start, dword count, uint16_t *buffer, ata_command command, hba_cmd_table *cmdtbl, uint16_t l)
{
    BARRIER;
    memset(cmdtbl, 0, sizeof(hba_cmd_table));
    BARRIER;
    qword addr{ translate_vaddr(buffer) };
    for(uint16_t i = 0; i <= l; i++, addr += 0x1000)
    {
        BARRIER;
        cmdtbl->prdt_entries[i].data_base = addr.lo;
        cmdtbl->prdt_entries[i].data_upper = addr.hi;
        cmdtbl->prdt_entries[i].byte_count = i == l ? (((count.lo.lo & 0x0F) * 0x200 - 1) | 0x80000001) : 0x80001FFF;
        BARRIER;
    }
    fis_reg_h2d* fis = reinterpret_cast<fis_reg_h2d*>(&(cmdtbl->cmd_fis[0]));
    BARRIER;
    fis->type = reg_h2d;
    fis->ctype = 1;
    fis->command = command;
    fis->lba0 = start.lo.lo.lo;
    fis->lba1 = start.lo.lo.hi;
    fis->lba2 = start.lo.hi.lo;
    fis->device = 0x40;
    fis->lba3 = start.lo.hi.hi;
    fis->lba4 = start.hi.lo.lo;
    fis->lba5 = start.hi.lo.hi;
    fis->count = count.lo;
    BARRIER;
}
bool ahci_driver::__handoff_busy() { return (dword(__my_abar->bios_os_handoff)[bos_bit] || dword(__my_abar->bios_os_handoff)[bb_bit]); }
void ahci_driver::__init_irq()
{
    for(int i = 0; i < 32; i++, __sync_synchronize()) { if(has_port(i)) { __my_abar->i_status |= (1 << i); BARRIER; uint32_t s1 = __my_abar->ports[i].i_state; BARRIER; __my_abar->ports[i].i_state = s1; BARRIER; } }
    interrupt_table::add_irq_handler(__my_ahci_controller->header_0x0.interrupt_line, LAMBDA_ISR() { this->handle_irq(); });
    irq_clear_mask(__my_ahci_controller->header_0x0.interrupt_line);
}
bool ahci_driver::init_instance(pci_device_list *ls) noexcept
{
    if(__has_init) return true;
    if(!ls) return false;
    return (__has_init = __instance.init(ls->find(dev_class_ahci, subclass_ahci_controller)));
}
ahci_driver *ahci_driver::get_instance() { return &__instance; }
bool ahci_driver::is_initialized() { return __has_init; }
bool ahci_driver::init(pci_config_space *ps) noexcept
{
    if(!ps) return false;
    try
    {
        __my_ahci_controller = ps;
        __my_abar = compute_base(ps->header_0x0.bar[5]);
        __my_block = heap_allocator::get().allocate_mmio_block(S128);
        BARRIER;
        __my_ahci_controller->command.bus_master = 1;
        __my_ahci_controller->command.memory_space = 1;
        __my_ahci_controller->command.interrupt_disable = 0;
        BARRIER;
        __my_abar->ghc |= hba_enable_ahci;
        BARRIER;
        if(__my_abar->bios_os_handoff_sup)
        {
            __my_abar->bios_os_handoff |= BIT(oos_bit);
            BARRIER;
            unsigned spin = 0;
            while(__handoff_busy() && spin < max_wait) { BARRIER; spin++; }
            if(__handoff_busy()) { panic("AHCI bios handoff error"); return false; }
        }
        BARRIER;
        for(int i = 0; i < 32; i++)
        {
            if(!dword(__my_abar->p_implemented)[i]) continue;
            __num_ports += 1;
            __my_devices[i] = check_type(__my_abar->ports[i]);
            BARRIER;
            if(__my_devices[i] == none) continue;
            try
            {
                port_soft_reset(i);
                port_rebase(i);
                BARRIER;
                __my_abar->ports[i].cmd |= port_power_on;
                BARRIER;
                uint32_t e = __my_abar->ports[i].s_err;
                BARRIER;
                __my_abar->ports[i].s_err = e;
                BARRIER;
            }
            catch(std::exception& e) { panic(e.what()); xpanic("skipping port " + std::to_string(i)); __my_devices[i] = none; }
        }
        for(int i = 0; i < 32; i++) { if(has_port(i)) { __init_irq(); return true; } }
    }
    catch(std::exception& e) { panic(e.what()); return false; }
    panic("no devices");
    return false;
}
bool ahci_driver::is_done(uint8_t i) { uint32_t st = __my_abar->ports[i].i_state; BARRIER; return !(st == 0 || __port_cmd_busy(&__my_abar->ports[i], __last_command_on_port[i])) && !is_busy(i); }
void ahci_driver::start_port(uint8_t i)
{
    if(i > __num_ports) throw std::out_of_range("port " + std::to_string(i) + " is out of range ");
    uint64_t spin = 0;
    while(__my_abar->ports[i].cmd & hba_command_cr && spin < max_ext_wait) { BARRIER; spin++; }
    BARRIER;
    if(__my_abar->ports[i].cmd & hba_command_cr) throw std::runtime_error("Port number " + std::to_string(i) + " is hung");
    BARRIER;
    __my_abar->ports[i].cmd |= hba_command_fre;
    BARRIER;
    __my_abar->ports[i].cmd |= hba_cmd_start;
}
void ahci_driver::stop_port(uint8_t i)
{
    if(i > __num_ports) throw std::out_of_range("port " + std::to_string(i) + " is out of range");
    BARRIER;
    __my_abar->ports[i].cmd &= ~hba_cmd_start;
    BARRIER;
    __my_abar->ports[i].cmd &= ~hba_command_fre;
    BARRIER;
    uint64_t spin = 0;
    while(__port_nack_stop(&__my_abar->ports[i]) && spin < max_ext_wait) { BARRIER; spin++; }
    BARRIER;
    if(__port_nack_stop(&__my_abar->ports[i])) throw std::runtime_error("Port number " + std::to_string(i) + " is hung");
}
void ahci_driver::port_rebase(uint8_t idx)
{
    stop_port(idx);
    vaddr_t cl_base = __my_block + ptrdiff_t(idx * cl_offs_base);
    vaddr_t f_base = __my_block + ptrdiff_t(fis_offs_base + idx * sizeof(hba_fis));
    __my_abar->ports[idx].command_list = cl_base;
    memset(static_cast<char*>(cl_base), 0, sizeof(hba_cmd_header) * 32);
    BARRIER;
    __my_abar->ports[idx].fis_receive = f_base;
    memset(static_cast<char*>(f_base), 0, sizeof(hba_fis));
    BARRIER;
    for(int jdx = 0; jdx < 32; jdx++)
    {
        vaddr_t t_base = __my_block + ptrdiff_t(table_offs_base + idx * prdt_size + jdx * sizeof(hba_cmd_table));
        __my_abar->ports[idx].command_list[jdx].command_table = t_base;
        BARRIER;
        memset(static_cast<char*>(t_base), 0, prdt_size);
    }
    start_port(idx);
}
static int find_cmdslot(hba_port* port)
{
    BARRIER;
    uint32_t slots = (port->s_active | port->cmd_issue);
    BARRIER;
    for(int i = 0; i < 32; i++) if(!(dword(slots)[i])) return i;
    return -1;
}
void ahci_driver::port_soft_reset(uint8_t idx)
{
    hba_port* port{ &(__my_abar->ports[idx]) };
    unsigned spin{ 0 };
    while(__my_abar->ports[idx].cmd_issue != 0 && spin < max_wait) { BARRIER; spin++; }
    BARRIER;
    if(__my_abar->ports[idx].cmd_issue != 0) throw std::runtime_error("Port number " + std::to_string(idx) + " is hung");   
    BARRIER;
    __my_abar->ports[idx].cmd &= ~hba_cmd_start;
    BARRIER;
    spin = 0;
    while(((__my_abar->ports[idx].cmd & hba_command_cr) != 0) && spin < max_wait) { BARRIER; spin++; }
    BARRIER;
    __my_abar->ports[idx].cmd |= hba_cmd_start;
    int s1 = -1, s2 = -1;
    BARRIER;
    uint32_t slots = __my_abar->ports[idx].s_active | __my_abar->ports[idx].cmd_issue;
    BARRIER;
    for(int i = 0; i < 32 && s1 < 0 && s2 < 0; i++)
    {
        if(!(dword(slots)[i])) 
        {
            if(s1 < 0) s1 = i; 
            else s2 = i; 
        }
    } 
    if(s1 < 0 && s2 < 0) throw std::runtime_error("Port number " + std::to_string(idx) + " has no available slots");
    BARRIER;
    __my_abar->ports[idx].command_list[s1].cl_busy = true;
    __my_abar->ports[idx].command_list[s1].reset = true;
    __my_abar->ports[idx].command_list[s1].cmd_fis_len =  sizeof(fis_reg_h2d) / sizeof(uint32_t);
    BARRIER;
    hba_cmd_table* ctbrst = __my_abar->ports[idx].command_list[s1].command_table;
    BARRIER;
    memset(ctbrst, 0, sizeof(hba_cmd_table));
    BARRIER;
    fis_reg_h2d* frst = vaddr_t{ &(ctbrst->cmd_fis[0]) };
    frst->type = reg_h2d;
    frst->control = soft_reset_bit;
    frst->ctype = 0;   
    spin = 0;
    while(__port_data_busy(port) && spin < max_wait) { BARRIER; spin++; }
    BARRIER;
    if(__port_data_busy(port)) { return hard_reset_fallback(idx);  }
    BARRIER;
    __my_abar->ports[idx].s_active = BIT(s1);
    BARRIER;
    __my_abar->ports[idx].cmd_issue = BIT(s1);
    BARRIER;
    spin = 0;
    while(__port_cmd_busy(port, s1) && spin < max_wait) { if(__my_abar->ports[idx].i_state & hba_error) { return hard_reset_fallback(idx); } spin++; BARRIER; }
    BARRIER;
    if(__port_cmd_busy(port, s1)) { return hard_reset_fallback(idx); }
    if(s2 < 0) s2 = s1;
    __my_abar->ports[idx].command_list[s2].cl_busy = false;
    __my_abar->ports[idx].command_list[s2].reset = false;
    __my_abar->ports[idx].command_list[s2].cmd_fis_len = sizeof(fis_reg_h2d) / sizeof(uint32_t);
    BARRIER;
    hba_cmd_table* ctbdia = __my_abar->ports[idx].command_list[s2].command_table;
    BARRIER;
    memset(ctbdia, 0, sizeof(hba_cmd_table));
    BARRIER;
    fis_reg_h2d* fdia = reinterpret_cast<fis_reg_h2d*>(&(ctbdia->cmd_fis[0]));
    fdia->type = reg_h2d;
    BARRIER;
    __my_abar->ports[idx].s_active |= BIT(s2);
    BARRIER;
    __my_abar->ports[idx].cmd_issue |= BIT(s2);
    BARRIER;
    spin = 0;
    while(__port_cmd_busy(port, s2) && spin < max_wait) { if(__my_abar->ports[idx].i_state & hba_error) { return hard_reset_fallback(idx); } spin++; BARRIER; }
    if(__port_cmd_busy(port, s2)) { return hard_reset_fallback(idx); }
    BARRIER;
    if(port->i_state & hba_error) { return hard_reset_fallback(idx); }
    BARRIER;
}
void ahci_driver::hard_reset_fallback(uint8_t idx)  { xdirect_write("hard reset required for port " + std::to_string(idx) + "\n"); port_hard_reset(idx); }
void ahci_driver::port_hard_reset(uint8_t idx)
{
    if(!has_port(idx)) throw std::out_of_range("port " + std::to_string(idx) + " does not exist");
    BARRIER;
    stop_port(idx);
    unsigned spin{ 0 };
    while(((__my_abar->ports[idx].cmd & hba_command_cr) != 0) && spin < max_wait) { BARRIER; spin++; }
    BARRIER;
    if(!(dword(__my_abar->ports[idx].cmd)[hba_sud_bit]) && __my_abar->sud_supported) { BARRIER; __my_abar->ports[idx].cmd |= hba_command_spin_up_disk; }
    BARRIER;
    __my_abar->ports[idx].s_control |= hba_control_det;
    BARRIER;
    spin = 0;
    while( spin < max_wait) { BARRIER; spin++; }
    BARRIER;
    __my_abar->ports[idx].s_control &= ~hba_control_det;
    BARRIER;
    spin = 0;
    while(((dword(__my_abar->ports[idx].s_status).lo.lo & 0x0F) != port_present) && spin < max_ext_wait) { BARRIER; spin++; }
    if((dword(__my_abar->ports[idx].s_status).lo.lo & 0x0F) != port_present) { throw std::runtime_error("Port number " + std::to_string(idx) + " failed to start"); }
    BARRIER;
    uint32_t bits = __my_abar->ports[idx].s_err;
    BARRIER;
    __my_abar->ports[idx].s_err |= bits;
    BARRIER;
    start_port(idx);
}
bool ahci_driver::has_port(uint8_t i) { return __my_devices[i] != none; }
void ahci_driver::read_sectors(uint8_t idx, qword start, dword count, uint16_t *buffer)
{
    if(!has_port(idx)) { throw std::out_of_range("port " + std::to_string(idx) + " does not exist"); }
    int slot = find_cmdslot(&(__my_abar->ports[idx]));
    BARRIER;
    if(slot < 0) throw std::runtime_error("Port number " + std::to_string(idx) + " has no available slots");
    __my_abar->ports[idx].command_list[slot].cmd_fis_len = 5;
    __my_abar->ports[idx].command_list[slot].w_direction = 0; // d2h write
    __my_abar->ports[idx].command_list[slot].atapi = 0;
    uint16_t l{ static_cast<uint16_t>((count.lo - 1) >> 4) };
    __my_abar->ports[idx].command_list[slot].prdt_length = l + 1; // each entry can transfer at most 8kb, or 16 sectors
    BARRIER;
    __build_h2d_fis(start, count, buffer, ata_command::read_dma_ext, __my_abar->ports[idx].command_list[slot].command_table, l);
    __issue_command(idx, slot);
    unsigned spin = 0;
    while(!is_done(idx) && spin < max_wait) { BARRIER; spin++; }
}
void ahci_driver::write_sectors(uint8_t idx, qword start, dword count, const uint16_t *buffer)
{
    if(!has_port(idx)) throw std::out_of_range("port " + std::to_string(idx) + " does not exist");
    int slot = find_cmdslot(&(__my_abar->ports[idx]));
    BARRIER;
    if(slot < 0) throw std::runtime_error("Port number " + std::to_string(idx) + " has no available slots");
    __my_abar->ports[idx].command_list[slot].cmd_fis_len = 5;
    __my_abar->ports[idx].command_list[slot].w_direction = 1; // h2d write
    __my_abar->ports[idx].command_list[slot].atapi = 0;
    uint16_t l{ static_cast<uint16_t>((count.lo - 1) >> 4) };
    __my_abar->ports[idx].command_list[slot].prdt_length = l + 1; // each entry can transfer at most 8kb, or 16 sectors
    BARRIER;
    __build_h2d_fis(start, count, const_cast<uint16_t*>(buffer), ata_command::write_dma_ext, __my_abar->ports[idx].command_list[slot].command_table, l);
    __issue_command(idx, slot);
    unsigned spin = 0;
    while(is_busy(idx) && spin < max_wait) { BARRIER; spin++; }
}
__isrcall void ahci_driver::handle_irq()
{
    uint32_t s = __my_abar->i_status;
	BARRIER;
    __my_abar->i_status = s;
    if(!(s)) return; // This means the IRQ is for someone else
    for(size_t i = 0; i < 32; i++)
    {
        uint32_t s1 = __my_abar->ports[i].i_state;
        BARRIER;
        __my_abar->ports[i].i_state = s1;
        BARRIER;
        if(has_port(i) && (dword(s)[i]) && (s1 & hba_error)) port_soft_reset(i);
        BARRIER;
    }
}
bool ahci_driver::is_busy(uint8_t i) { BARRIER; uint32_t c = __my_abar->ports[i].cmd; BARRIER; return __port_data_busy(&__my_abar->ports[i]) || (c & hba_command_cr) == 0; }
void ahci_driver::p_identify(uint8_t idx, identify_data *data)
{
    if(!has_port(idx)) throw std::out_of_range("port " + std::to_string(idx) + " is out of range ");
    hba_port* port{ &(__my_abar->ports[idx]) };
    int slot = find_cmdslot(port);
    if(slot < 0) throw std::runtime_error("Port number " + std::to_string(idx) + " has no available slots");
    qword addr = vaddr_t{ data };
    BARRIER;
    __my_abar->ports[idx].command_list[slot].cmd_fis_len = 5;
    __my_abar->ports[idx].command_list[slot].prdt_length = 1;
    __my_abar->ports[idx].command_list[slot].w_direction = 0;
    __my_abar->ports[idx].command_list[slot].command_table->prdt_entries[0].data_base = addr.lo;
    __my_abar->ports[idx].command_list[slot].command_table->prdt_entries[0].data_upper = addr.hi;
    __my_abar->ports[idx].command_list[slot].command_table->prdt_entries[0].byte_count = 511 | 0x80000001;
    BARRIER;
    fis_reg_h2d* fis = vaddr_t{ __my_abar->ports[idx].command_list[slot].command_table->cmd_fis };
    memset(fis, 0, sizeof(fis_reg_h2d));
    BARRIER;
    fis->type = reg_h2d;
    fis->ctype = 1;
    fis->command = identify;
    fis->device = 0;
    BARRIER;
    try { __issue_command(idx, slot); } catch(std::exception& e) { panic(e.what()); panic("error on port identify; attempt soft reset"); port_soft_reset(idx); }
    unsigned spin{ 0 };
    while(!is_done(idx) && spin < max_wait) { BARRIER; spin++; }
}
int8_t ahci_driver::which_port(ahci_device d) { for(int8_t i = 0; i < 32; i++) if(__my_devices[i] == d) return i; return -1; }
ahci_device ahci_driver::get_device_type(uint8_t i) { return __my_devices[i]; }
uint32_t ahci_driver::last_read_count(uint8_t i) { BARRIER; return __my_abar->ports[i].command_list[__last_command_on_port[i]].prd_count; }