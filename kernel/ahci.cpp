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
    if((dword(p.s_status).lo.lo & 0x0F) != port_present || (dword(p.s_status).lo.hi & 0x0F) != port_active) return none;
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
static inline bool __port_data_busy(hba_port* port) { return (dword(port->task_file)[busy_bit] || dword(port->task_file)[drq_bit]); }
static inline bool __port_cmd_busy(hba_port* port, int slot) { return dword(port->cmd_issue)[slot] || dword(port->i_state)[busy_bit]; }
static inline bool __port_nack_stop(hba_port* port) { return port->cmd & hba_command_cr || port->cmd & hba_command_fr; }
ahci_driver ahci_driver::__instance{};
bool ahci_driver::__has_init = false;
void ahci_driver::__issue_command(uint8_t port_idx, int slot)
{
    hba_port* port = &__my_abar->ports[port_idx];
    unsigned spin;
    for(spin = 0; __port_data_busy(port) && spin < max_wait; spin++, __sync_synchronize());
    if( __port_data_busy(port)) { throw std::runtime_error("Port number " + std::to_string(port_idx) + " is hung"); }
    __sync_synchronize();
    port->cmd_issue = (1 << slot);
    __sync_synchronize();
    for(spin = 0; __port_cmd_busy(port, slot) && spin < max_ext_wait; spin++, __sync_synchronize()) { if(dword(port->i_state).hi.hi & i_state_hi_byte_error) { throw std::runtime_error("Port number " + std::to_string(port_idx) + " hardware error"); } }
    if(dword(port->i_state).hi.hi & i_state_hi_byte_error) { throw std::runtime_error("Port number " + std::to_string(port_idx) + " hardware error");}
    __sync_synchronize();
    __last_command_on_port[port_idx] = slot;
}
void ahci_driver::__build_h2d_fis(qword start, dword count, uint16_t *buffer, ata_command command, hba_cmd_table *cmdtbl, uint16_t l)
{
    __sync_synchronize();
    memset(cmdtbl, 0, sizeof(hba_cmd_table));
    __sync_synchronize();
    qword addr =  translate_vaddr(buffer);
    for(uint16_t i = 0; i <= l; i++, addr += 0x1000, __sync_synchronize())
    {
        cmdtbl->prdt_entries[i].data_base = addr.lo;
        cmdtbl->prdt_entries[i].data_upper = addr.hi;
        cmdtbl->prdt_entries[i].byte_count = i == l ? (((count.lo.lo & 0x0F) * 0x200 - 1) | 0x80000001) : 0x80001FFF;
    }
    fis_reg_h2d* fis = reinterpret_cast<fis_reg_h2d*>(cmdtbl->cmd_fis);
    __sync_synchronize();
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
    __sync_synchronize();
}
hba_cmd_header *ahci_driver::__ctbl_base(uint8_t idx) { return __has_init ? static_cast<hba_cmd_header*>(__my_block + ptrdiff_t(idx * cl_offs_base)) : __my_abar->ports[idx].command_list; }
bool ahci_driver::__handoff_busy() { return (dword(__my_abar->bios_os_handoff)[bos_bit] || dword(__my_abar->bios_os_handoff)[bb_bit]); }
void ahci_driver::__init_irq()
{
    uint8_t irq = __my_ahci_controller->header_0x0.interrupt_line;
    interrupt_table::add_irq_handler(irq, INTERRUPT_LAMBDA() { this->handle_irq(); });
    irq_clear_mask(irq);
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
        __my_block = heap_allocator::get().allocate_mmio_block(S128, 1024);
        __my_ahci_controller->command.bus_master = 1;
        __my_ahci_controller->command.memory_space = 1;
        __my_ahci_controller->command.interrupt_disable = 0;
        __sync_synchronize();
        if(__my_abar->bios_os_handoff_sup)
        {
            __my_abar->bios_os_handoff |= (1 << oos_bit);
            for(unsigned spin = 0; __handoff_busy() && spin < max_wait; spin++, __sync_synchronize());
            if(__handoff_busy()) { panic("AHCI bios handoff error"); return false; }
        }
        __my_abar->ghc |= hba_enable_ahci;
        __sync_synchronize();
        for(int i = 0; i < 32; i++, __sync_synchronize())
        {
            if(!dword(__my_abar->p_implemented)[i]) continue;
            __num_ports += 1;
            __my_devices[i] = check_type(__my_abar->ports[i]);
            if(__my_devices[i] == none) continue;
            try
            {
                port_soft_reset(i);
                port_rebase(i);
                __my_abar->ports[i].cmd |= port_power_on;
            }
            catch(std::exception& e) { panic(e.what()); panic("skipping port"); }
        }
        for(int i = 0; i < 32; i++) { if(has_port(i)) { __init_irq(); return true; } }
    }
    catch(std::exception& e) { panic(e.what()); }
    return false;
}
bool ahci_driver::is_done(uint8_t i) { __sync_synchronize(); return !((__my_abar->ports[i].i_state) == 0) && !is_busy(i); }
void ahci_driver::start_port(uint8_t i)
{
    if(i > __num_ports) throw std::out_of_range("port " + std::to_string(i) + " is out of range ");
    for(uint64_t spin = 0; __my_abar->ports[i].cmd & hba_command_cr && spin < max_ext_wait; spin++, __sync_synchronize());
    __sync_synchronize();
    if(__my_abar->ports[i].cmd & hba_command_cr) throw std::runtime_error("Port number " + std::to_string(i) + " is hung");
    __sync_synchronize();
    __my_abar->ports[i].cmd |= (hba_command_fre | hba_cmd_start);
}
void ahci_driver::stop_port(uint8_t i)
{
    if(i > __num_ports) throw std::out_of_range("port " + std::to_string(i) + " is out of range");
    __sync_synchronize();
    __my_abar->ports[i].cmd &= ~(hba_cmd_start | hba_command_fre);
    __sync_synchronize();
    for(uint64_t spin = 0; __port_nack_stop(&__my_abar->ports[i]) && spin < max_ext_wait; spin++, __sync_synchronize());
    __sync_synchronize();
    if( __port_nack_stop(&__my_abar->ports[i])) throw std::runtime_error("Port number " + std::to_string(i) + " is hung");
}
void ahci_driver::port_rebase(uint8_t idx)
{
    stop_port(idx);
    uintptr_t a_base = translate_vaddr(__my_block);
    vaddr_t vcl_base = __my_block + ptrdiff_t(idx * cl_offs_base);
    vaddr_t vf_base = __my_block + ptrdiff_t(fis_offs_base + idx * sizeof(hba_fis));
    uintptr_t cl_base = a_base + idx * cl_offs_base;
    uintptr_t f_base = a_base + fis_offs_base + idx * sizeof(hba_fis);
    __my_abar->ports[idx].command_list = reinterpret_cast<hba_cmd_header*>(cl_base);
    __my_abar->ports[idx].fis_receive = reinterpret_cast<hba_fis*>(f_base);
    __sync_synchronize();
    memset(static_cast<char*>(vcl_base), 0, sizeof(hba_cmd_header) * 32);
    memset(static_cast<char*>(vf_base), 0, sizeof(hba_fis));
    for(int jdx = 0; jdx < 32; jdx++, __sync_synchronize())
    {
        vaddr_t vt_base = __my_block + ptrdiff_t(table_offs_base + idx * prdt_size + jdx * sizeof(hba_cmd_table));
        uintptr_t t_base = a_base + table_offs_base + idx * prdt_size + jdx * sizeof(hba_cmd_table);
        static_cast<hba_cmd_header*>(vcl_base)[jdx].command_table = reinterpret_cast<hba_cmd_table*>(t_base);
        __sync_synchronize();
        memset(static_cast<char*>(vt_base), 0, prdt_size);
    }
    start_port(idx);
}
static int find_cmdslot(hba_port* port)
{
    __sync_synchronize();
    uint32_t slots = (port->s_active | port->cmd_issue);
    __sync_synchronize();
    for(int i = 0; i < 32; i++) if(!(dword(slots)[i])) return i;
    return -1;
}
void ahci_driver::port_soft_reset(uint8_t idx)
{
    hba_port* port = &__my_abar->ports[idx];
    unsigned spin;
    for(spin = 0; port->cmd_issue != 0 && spin < max_wait; spin++, __sync_synchronize());
    __sync_synchronize();
    if(port->cmd_issue != 0) throw std::runtime_error("Port number " + std::to_string(idx) + " is hung");
    __sync_synchronize();
    port->cmd &= ~hba_cmd_start;
    __sync_synchronize();
    for(spin = 0; ((port->cmd & hba_command_cr) != 0) && spin < max_wait; spin++, __sync_synchronize());
    __sync_synchronize();
    port->cmd |= hba_cmd_start;
    int s1 = -1, s2 = -1;
    __sync_synchronize();
    uint32_t slots = (port->s_active | port->cmd_issue);
    __sync_synchronize();
    for(int i = 0; i < 32 && s1 < 0 && s2 < 0; i++, __sync_synchronize())
    {
        if(!(dword(slots)[i])) 
        {
            if(s1 < 0) s1 = i; 
            else s2 = i; 
        } 
    }
    if(s1 < 0 && s2 < 0) throw std::runtime_error("Port number " + std::to_string(idx) + " has no available slots");
    __sync_synchronize();
    hba_cmd_header* clist = &__ctbl_base(idx)[s1];
    clist->cl_busy = true;
    clist->reset = true;
    clist->cmd_fis_len =  sizeof(fis_reg_h2d) / sizeof(uint32_t);
    __sync_synchronize();
    hba_cmd_table* ctbrst = clist->command_table;
    __sync_synchronize();
    memset(ctbrst, 0, sizeof(hba_cmd_table));
    __sync_synchronize();
    fis_reg_h2d* frst = reinterpret_cast<fis_reg_h2d*>(&(ctbrst->cmd_fis[0]));
    frst->type = reg_h2d;
    frst->control = soft_reset_bit;
    frst->ctype = 0;
    __sync_synchronize();
    for(spin = 0; s2 < 0 && dword(port->s_active)[s1] && spin < max_wait; spin++, __sync_synchronize());
    if(s2 < 0) s2 = s1;
    clist = &__ctbl_base(idx)[s2];
    clist->cl_busy = false;
    clist->reset = false;
    clist->cmd_fis_len = sizeof(fis_reg_h2d) / sizeof(uint32_t);
    __sync_synchronize();
    hba_cmd_table* ctbdia = clist->command_table;
    __sync_synchronize();
    memset(ctbdia, 0, sizeof(hba_cmd_table));
    __sync_synchronize();
    fis_reg_h2d* fdia = reinterpret_cast<fis_reg_h2d*>(&(ctbdia->cmd_fis[0]));
    fdia->type = reg_h2d;
    __sync_synchronize();
    for(spin = 0; __port_data_busy(port) && spin < max_wait; spin++, __sync_synchronize()); 
    __sync_synchronize();
    if(__port_data_busy(port)) return port_hard_reset(idx);
    __sync_synchronize();
    port->s_active = 1 << s1;
    __sync_synchronize();
    port->cmd_issue = 1 << s1;
    __sync_synchronize();
    for(spin = 0; dword(port->cmd_issue)[s1] && spin < max_wait; spin++, __sync_synchronize()) { if(port->i_state & hba_error) return port_hard_reset(idx); }
    __sync_synchronize();
    port->s_active |= 1 << s2;
    __sync_synchronize();
    port->cmd_issue |= 1 << s2;
    __sync_synchronize();
    for(spin = 0; dword(port->cmd_issue)[s2] && spin < max_wait; spin++, __sync_synchronize()) { if(port->i_state & hba_error)  { return port_hard_reset(idx); } }
    if(dword(port->cmd_issue)[s2]) return port_hard_reset(idx);
    __sync_synchronize();
    if(port->i_state & hba_error) return port_hard_reset(idx);
    __sync_synchronize();
}
void ahci_driver::port_hard_reset(uint8_t port_idx)
{
    if(!has_port(port_idx)) throw std::out_of_range("port " + std::to_string(port_idx) + " does not exist");
    hba_port* port = &__my_abar->ports[port_idx];
    stop_port(port_idx);
    unsigned spin;
    for(spin = 0; ((port->cmd & hba_command_cr) != 0) && spin < max_wait; spin++, __sync_synchronize());
    __sync_synchronize();
    if(!(dword(port->cmd)[hba_sud_bit])) { __sync_synchronize(); port->cmd |= hba_command_spin_up_disk; }
    __sync_synchronize();
    port->s_control |= hba_control_det;
    __sync_synchronize();
    for(spin = 0; spin < max_wait; spin++);
    __sync_synchronize();
    port->s_control &= ~hba_control_det;
    __sync_synchronize();
    for(spin = 0; ((dword(port->s_status).lo.lo & 0x0F) != port_present) && spin < max_ext_wait; spin++, __sync_synchronize());
    if((dword(port->s_status).lo.lo & 0x0F) != port_present) { throw std::runtime_error("Port number " + std::to_string(port_idx) + " failed to start"); }
    __sync_synchronize();
    uint32_t bits = port->s_err;
    __sync_synchronize();
    port->s_err |= bits;
    __sync_synchronize();
    start_port(port_idx);
}
bool ahci_driver::has_port(uint8_t i) { return __my_devices[i] != none; }
void ahci_driver::read_sectors(uint8_t port_idx, qword start, dword count, uint16_t *buffer)
{
    if(!has_port(port_idx)) throw std::out_of_range("port " + std::to_string(port_idx) + " does not exist");
    hba_port* port = &__my_abar->ports[port_idx];
    int slot = find_cmdslot(port);
    if(slot < 0) throw std::runtime_error("Port number " + std::to_string(port_idx) + " has no available slots");
    hba_cmd_header* clist = &__ctbl_base(port_idx)[slot];
    clist->cmd_fis_len = 5;
    clist->w_direction = 0; // d2h write
    clist->atapi = 0;
    uint16_t l = static_cast<uint16_t>(((count - 1) >> 4));
    clist->prdt_length = l + 1; // each entry can transfer at most 8kb, or 16 sectors
    __sync_synchronize();
    __build_h2d_fis(start, count, buffer, ata_command::read_dma_ext, clist->command_table, l);
    __issue_command(port_idx, slot);
    for(unsigned spin = 0; is_busy(port_idx) && spin < max_wait; spin++);
}
void ahci_driver::write_sectors(uint8_t port_idx, qword start, dword count, const uint16_t *buffer)
{
    if(!has_port(port_idx)) throw std::out_of_range("port " + std::to_string(port_idx) + " does not exist");
    hba_port* port = &__my_abar->ports[port_idx];
    int slot = find_cmdslot(port);
    if(slot < 0) throw std::runtime_error("Port number " + std::to_string(port_idx) + " has no available slots");
    hba_cmd_header* clist = &__ctbl_base(port_idx)[slot];
    clist->cmd_fis_len = 5;
    clist->w_direction = 1; // h2d write
    clist->atapi = 0;
    uint16_t l = static_cast<uint16_t>(((count - 1) >> 4));
    clist->prdt_length = l + 1; // each entry can transfer at most 8kb, or 16 sectors
    __sync_synchronize();
    __build_h2d_fis(start, count, const_cast<uint16_t*>(buffer), ata_command::write_dma_ext, clist->command_table, l);
    __issue_command(port_idx, slot);
    for(unsigned spin = 0; is_busy(port_idx) && spin < max_wait; spin++);
}
[[gnu::target("general-regs-only")]]
void ahci_driver::handle_irq()
{
    uint32_t s = __my_abar->i_status;
	__sync_synchronize();
    __my_abar->i_status = s;
    if(!(s)) return; // This means the IRQ is for someone else
    for(size_t i = 0; i < 32; i++)
    {
        uint32_t s1 = __my_abar->ports[i].i_state;
        __sync_synchronize();
        __my_abar->ports[i].i_state = s1;
        if(has_port(i) && (s & (1 << i)) && (s1 & hba_error)) port_soft_reset(i);
    }
}
bool ahci_driver::is_busy(uint8_t i) { __sync_synchronize(); return __port_data_busy(&__my_abar->ports[i]) || (__my_abar->ports[i].cmd & hba_command_cr) == 0; }
void ahci_driver::p_identify(uint8_t port_idx, identify_data *data)
{
    if(!has_port(port_idx)) throw std::out_of_range("port " + std::to_string(port_idx) + " is out of range ");
    hba_port* port = &__my_abar->ports[port_idx];
    int slot = find_cmdslot(port);
    if(slot < 0) throw std::runtime_error("Port number " + std::to_string(port_idx) + " has no available slots");
    hba_cmd_header* clist = &__ctbl_base(port_idx)[slot];
    clist->cmd_fis_len = 5;
    clist->prdt_length = 1;
    clist->w_direction = 0;
    hba_cmd_table* ctbl = clist->command_table;
	vaddr_t va_data{ data }; 
    qword addr = va_data;
    ctbl->prdt_entries[0].data_base = addr.lo;
    ctbl->prdt_entries[0].data_upper = addr.hi;
    ctbl->prdt_entries[0].byte_count = 511 | 0x80000001;
    __sync_synchronize();
    fis_reg_h2d* fis = reinterpret_cast<fis_reg_h2d*>(ctbl->cmd_fis);
    memset(fis, 0, sizeof(fis_reg_h2d));
    __sync_synchronize();
    fis->type = reg_h2d;
    fis->ctype = 1;
    fis->command = identify;
    fis->device = 0;
    try { __issue_command(port_idx, slot); }
    catch(std::exception& e) { panic(e.what()); panic("error on port identify; attempt soft reset"); port_soft_reset(port_idx); }
}
int8_t ahci_driver::which_port(ahci_device d) { for(int8_t i = 0; i < 32; i++) if(__my_devices[i] == d) return i; return -1; }
ahci_device ahci_driver::get_device_type(uint8_t i) { return __my_devices[i]; }
uint32_t ahci_driver::last_read_count(uint8_t i) { __sync_synchronize(); return __ctbl_base(i)[__last_command_on_port[i]].prd_count; }