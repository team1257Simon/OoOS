#include "arch/net/e1000e.hpp"
#include "isr_table.hpp"
#include "functional"
#include "stdexcept"
#include "errno.h"
#include "kdebug.hpp"
constexpr uint32_t all_ints = 0x1F6DCU;
constexpr int rxbase_hi     = e1000_rdbah(0);
constexpr int rxbase_lo     = e1000_rdbal(0);
constexpr int rxlen         = e1000_rdlen(0);
constexpr int rxh           = e1000_rdh(0);
constexpr int rxt           = e1000_rdt(0);
constexpr int txbase_hi     = e1000_tdbah(0);
constexpr int txbase_lo     = e1000_tdbal(0);
constexpr int txlen         = e1000_tdlen(0);
constexpr int txh           = e1000_tdh(0);
constexpr int txt           = e1000_tdt(0);
constexpr int txdctl        = e1000_txdctl(0);
constexpr std::alignas_allocator<char, int128_t> __buffer_alloc;
void e1000e::read_status(dev_status& status) { read_dma(e1000_status, status); }
e1000e::~e1000e() = default;
void e1000e::on_interrupt() 
{
    irq_state icr{};
    read_dma(e1000_icr, icr);
    write_dma(e1000_icr, icr);
    if(icr->link_status_change)
    {
        dev_ctrl ctl{};
        read_dma(e1000_ctrl, ctl);
        ctl->set_link_up = true;
        write_dma(e1000_ctrl, ctl);
    }
    if(icr->rxdt_min_thresh || icr->rx_timer || icr->rxq0) poll_rx();
    read_dma(tx_ring.head_descriptor);
    // more as needed
}
void e1000e::enable_transmit()
{
    tx_ctrl ctl{};
    read_dma(e1000_tctl, ctl);
    ctl->enable = true;
    write_dma(e1000_tctl, ctl);
}
void e1000e::enable_receive()
{
    rx_ctrl ctl{};
    read_dma(e1000_rctl, ctl);
    ctl->enable = true;
    write_dma(e1000_rctl, ctl);
}
void e1000e::disable_transmit()
{
    tx_ctrl ctl{};
    read_dma(e1000_tctl, ctl);
    ctl->enable = false;
    write_dma(e1000_tctl, ctl);
}
void e1000e::disable_receive()
{
    rx_ctrl ctl{};
    read_dma(e1000_rctl, ctl);
    ctl->enable = false;
    write_dma(e1000_rctl, ctl);
}
static inline uint16_t get_io_bar(pci_config_space* device)
{
    if(device->header_type == st)
    {
        for(int i = 0; i < 6; i++) 
            if(device->header_0x0.bar[i] & 0x01)
                return static_cast<uint16_t>(device->header_0x0.bar[i] & 0xFFFCUS);
    }
    return 0;
}
static inline addr_t map_device_mmio(pci_config_space* device)
{
    uint32_t bar    = device->header_0x0.bar[0];
    addr_t base     = kmm.map_dma(addr_t(compute_base(bar)), 0x20000UZ, (bar & BIT(3)) != 0);
    if(!base) throw std::bad_alloc();
    return base;
}
bool e1000e::dev_reset()
{
    dev_ctrl ctrl;
    barrier();
    read_dma(e1000_icr);        // read the ICR register to clear it
    barrier();
    write_dma(e1000_imc, ~0U);
    barrier();
    read_dma(e1000_ctrl, ctrl);
    barrier();
    ctrl->reset = true;
    write_dma(e1000_ctrl, ctrl);
    if(__unlikely(!await_result([&]() -> bool { io_wait(); read_dma(e1000_ctrl, ctrl); return !ctrl->reset; }))) { panic("[e1000e] device hung on reset"); return false; }
    barrier();
    write_dma(e1000_imc, ~0U);
    barrier();
    return true;
}
bool e1000e::read_io(int reg_id, uint32_t& r_out)
{
    if(!__io_addr_port) return false;
    __io_addr_port.put(reg_id);
    barrier();
    r_out = __io_data_port.get();
    return true;
}
bool e1000e::write_io(int reg_id, uint32_t const& w_in)
{
    if(!__io_addr_port) return false;
    __io_addr_port.put(reg_id);
    barrier();
    __io_data_port.put(w_in);
    barrier();
    return w_in == __io_data_port.get();
}
void e1000e::read_dma(int reg_id, uint32_t& r_out)
{
    addr_t where = __mmio_region.plus(reg_id);
    if(__unlikely(where.full % 4UL)) throw std::invalid_argument("[e1000e] dma access must be on dword boundary");
    r_out = where.deref<uint32_t>();
    fence();
}
uint32_t e1000e::read_dma(int reg_id)
{
    uint32_t result;
    read_dma(reg_id, result);
    return result;
}
void e1000e::write_dma(int reg_id, uint32_t const& w_in)
{
    addr_t where = __mmio_region.plus(reg_id);
    if(__unlikely(where.full % 4UL)) throw std::invalid_argument{ "[e1000e] dma access must be on dword boundary" };
    where.assign(w_in);
    fence();
}
bool e1000e::__mdio_await(mdic& mdic_reg)
{
    return await_result([&]() -> bool 
    {
        io_wait();
        read_dma(e1000_mdic, mdic_reg);
        if(mdic_reg->error) throw std::runtime_error{ "[e1000e] mdic read error" };
        return mdic_reg->ready;
    });
}
void e1000e::read_phy(int phy_reg, uint16_t& out)
{
    mdic mdic_reg
    {
        .data_struct
        {
            .phy_register_address   { static_cast<uint8_t>(phy_reg & 0x1FUC) },
            .phy_address            { 1US },
            .operation              { 1UC },
            .ready                  { false },
            .interrupt_enable       { true },
            .error                  { false }
        }
    };
    write_dma(e1000_mdic, mdic_reg);
    __mdio_await(mdic_reg);
    out = mdic_reg->data;
}
uint16_t e1000e::read_eeprom(uint16_t eep_addr)
{
    ee_read rd
    {
        .data_struct
        {
            .read_start     { true },
            .read_done      { false },
            .read_address   { static_cast<uint16_t>(eep_addr & 0x3FFFUS) },
            .read_data      { 0US }
        }
    };
    write_dma(e1000_eerd, rd);
    await_result([&]() -> bool { io_wait(); read_dma(e1000_eerd, rd); return rd->read_done; });
    return rd->read_data;
}
void e1000e::write_phy(int phy_reg, uint16_t data)
{
    mdic mdic_reg
    {
        .data_struct
        {
            .data                   { data },
            .phy_register_address   { static_cast<uint8_t>(phy_reg & 0x1FUC) },
            .phy_address            { 1US },
            .operation              { 2UC },
            .ready                  { false },
            .interrupt_enable       { true },
            .error                  { false }
        }
    };
    write_dma(e1000_mdic, mdic_reg);
    __mdio_await(mdic_reg);
}
e1000e::e1000e(pci_config_space* device, size_t descriptor_count_factor) : 
    __has_init                  { false },
    __mmio_region               { map_device_mmio(device) },
    __pcie_e1000e_controller    { device },
    __io_addr_port              { get_io_bar(device) },
    __io_data_port              { __io_addr_port + 4US },
    rx_ring                     { descriptor_count_factor },
    tx_ring                     { descriptor_count_factor }
                                {}
bool e1000e::initialize()
{
    if(__has_init) return true;
    dev_status status;
    __pcie_e1000e_controller->command.memory_space  = true;
    __pcie_e1000e_controller->command.io_space      = true;
    __pcie_e1000e_controller->command.bus_master    = true;
    if(__unlikely(!dev_reset())) return false;
    if(__unlikely(!configure_mac_phy(status))) return false; 
    for(int i = e1000_stats_min; i < e1000_stats_max; i += 4) { read_dma(i); read_status(status); }
    for(int i = 0; i < 128; i++) { write_dma(e1000_mta + 4 * i, 0U); read_status(status); }
    tx_bind tx_poll = std::bind(&e1000e::poll_tx, this, std::placeholders::_1);
    rx_bind rx_poll = std::bind(&e1000e::rx_transfer, this, std::placeholders::_1);
    for(size_t i = 0; i < rx_ring.count(); i++) transfer_buffers.emplace(max_single_rx_buffer, max_single_tx_buffer, rx_poll, tx_poll, max_single_rx_buffer, max_single_tx_buffer);
    if(__unlikely(!configure_rx(status))) return false;
    if(__unlikely(!configure_tx(status))) return false;
    if(__unlikely(!configure_interrupts(status))) return false;
    enable_receive();
    return (__has_init = true);
}
bool e1000e::configure_interrupts(dev_status& st)
{
    write_dma(e1000_ims, all_ints);
    read_dma(e1000_icr); // read the register to clear it
    read_status(st);
    interrupt_table::add_irq_handler(__pcie_e1000e_controller->header_0x0.interrupt_line, std::bind(&e1000e::on_interrupt, this));
    __pcie_e1000e_controller->command.interrupt_disable = false;
    return true;
}
bool e1000e::configure_mac_phy(dev_status& st)
{
    word mac_word       = read_eeprom(0US);
    mac_addr[0]         = mac_word.lo;
    mac_addr[1]         = mac_word.hi;
    mac_word            = read_eeprom(1US);
    mac_addr[2]         = mac_word.lo;
    mac_addr[3]         = mac_word.hi;
    mac_word            = read_eeprom(2US);
    mac_addr[4]         = mac_word.lo;
    mac_addr[5]         = mac_word.hi;
    dev_ctrl ctl{};
    read_dma(e1000_ctrl, ctl);
    ctl->set_link_up    = true;
    write_dma(e1000_ctrl, ctl);
    if(await_result([&]() -> bool { io_wait(); read_status(st); return st->link_up; })) return true;
    panic("[NET/e1000e] device hung on link-up signal");
    return false;
}
bool e1000e::configure_tx(dev_status& st)
{
    std::ext::resettable_queue<netstack_buffer>::iterator i = transfer_buffers.begin();
    for(e1000e_transmit_descriptor* d = tx_ring.descriptors; d < tx_ring.max_descriptor; d++, i++)
        new(d) e1000e_transmit_descriptor{ .buffer_addr{ translate_vaddr(tx_base(i)) } };
    uint32_t tx_tail        = static_cast<uint32_t>(tx_ring.max_descriptor - tx_ring.descriptors);
    qword txbase            = reinterpret_cast<uintptr_t>(tx_ring.descriptors);
    uint32_t tx_total_len   = tx_tail * 16U;
    write_dma(txbase_hi, txbase.hi);
    write_dma(txbase_lo, txbase.lo);
    read_status(st);
    write_dma(txlen, tx_total_len);
    read_status(st);
    write_dma(txh, tx_ring.head_descriptor);
    write_dma(txt, tx_tail);
    tx_desc_ctrl tctl;
    read_dma(txdctl, tctl);
    tctl->writeback_thresh          = 1UC;
    tctl->descriptor_granularity    = true;
    write_dma(txdctl, tctl);
    read_status(st);
    tx_ctrl ctrl_reg
    {
        .data_struct
        {
            .enable                     { true },
            .pad_short_packets          { true },
            .collision_threshold        { 0x0FUC },
            .collision_distance         { 0x03UC },
            .software_xoff              { false },
            .retransmit_late_collision  { false },
            .underrun_no_retransmit     { false },
            .tdtms                      { 0UC },
            .multi_request_support      { false }
        }
    };
    write_dma(e1000_tctl, ctrl_reg);
    read_status(st);
    return true;
}
bool e1000e::configure_rx(dev_status& st)
{
    std::ext::resettable_queue<netstack_buffer>::iterator i = transfer_buffers.begin();
    for(e1000e_receive_descriptor* d = rx_ring.descriptors; d < rx_ring.max_descriptor; d++, i++) 
        new(d) e1000e_receive_descriptor{ .read{ .buffer_addr{ translate_vaddr(rx_base(i)) } } };
    rx_ring.tail_descriptor = static_cast<uint32_t>(rx_ring.max_descriptor - rx_ring.descriptors) / e1000_rxtxdesclen_base;
    qword rxbase            = reinterpret_cast<uintptr_t>(rx_ring.descriptors);
    uint32_t rx_total_len   = rx_ring.tail_descriptor * e1000_rxtxdesclen_base * sizeof(e1000e_receive_descriptor);
    write_dma(rxbase_hi, rxbase.hi);
    write_dma(rxbase_lo, rxbase.lo);
    read_status(st);
    write_dma(rxlen, rx_total_len);
    read_status(st);
    write_dma(rxh, rx_ring.head_descriptor);
    write_dma(rxt, rx_ring.tail_descriptor);
    read_status(st);
    rx_ctrl ctrl_reg
    {
        .data_struct
        {
            .enable                     { false }, // enabling rx comes at the very end
            .store_bad_packets          { true },
            .unicast_promiscuous        { true },
            .multicast_promiscuous      { true },
            .long_packet_enable         { true },
            .loopback_mode              { 0UC },
            .rdtms                      { 0UC },
            .desc_type                  { 0UC },
            .multicast_offset           { 0UC },
            .broadcast_accept           { true },
            .buffer_size_shift          { 1UC },
            .vlan_filter_enable         { false },
            .canonical_form_enable      { false },
            .canonical_form_indicator   { false },
            .discard_pause_frames       { false },
            .pass_mac_control_frames    { false },
            .extended_buffer_size       { true },
            .strip_ethernet_crc         { true },
            .flex_buffer_size           { 0UC }
        }
    };
    write_dma(e1000_rctl, ctrl_reg);
    read_status(st);
    return true;
}
int e1000e::poll_tx(netstack_buffer& buff)
{
    e1000e_transmit_descriptor& tail    = tx_ring.tail();
    tail.flags.length                   = static_cast<uint16_t>(buff.count(std::ios_base::out));
    tail.flags.cmd                      = 0b1011UC;
    tail.fields.status                  = 0UC;
    tx_ring.tail_descriptor             = (tx_ring.tail_descriptor + 1U) % tx_ring.count();
    write_dma(txt, tx_ring.tail_descriptor);
    if(!await_result([&]() -> bool { io_wait(); return (tail.fields.status & 0x1UC) != 0; })) return -ENETDOWN;
    return 0;
}
int e1000e::poll_rx()
{
    uint32_t head                   = rx_ring.head_descriptor;
    read_dma(rxh, rx_ring.head_descriptor);
    netstack_buffer* active_buffer  = nullptr;
    std::vector<netstack_buffer*> to_process;
    for(uint32_t i = head; i < rx_ring.head_descriptor; fence(), i++)
    {
        e1000e_receive_descriptor& desc = rx_ring.descriptors[i];
        netstack_buffer& buff           = *(transfer_buffers.begin() + i);
        if(desc.read.status.done && !desc.read.errors)
        {
            buff.pubseekpos(std::streampos(desc.read.length), std::ios_base::in);
            if(desc.read.status.end_of_packet)
            {
                try { to_process.push_back(active_buffer ? active_buffer : std::addressof(buff)); }
                catch(...) { return -ENOMEM; }
                active_buffer = nullptr;
            }
            else
            {
                if(!active_buffer)
                    active_buffer = std::addressof(buff);
                else
                    active_buffer->rx_accumulate(buff);
            }
        }
        addr_t(std::addressof(desc.read.status)).assign(0UC);
        rx_ring.tail_descriptor = (rx_ring.tail_descriptor + 1U) % rx_ring.count();
        addr_t(std::addressof(rx_ring.tail())).plus(sizeof(uintptr_t)).assign(0UL);
    }
    write_dma(rxt, rx_ring.tail_descriptor);
    fence();
    for(netstack_buffer* buff : to_process) buff->rx_flush();
    return 0;
}