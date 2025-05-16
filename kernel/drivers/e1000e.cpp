#include "arch/net/e1000e.hpp"
#include "isr_table.hpp"
#include "functional"
#include "stdexcept"
#include "errno.h"
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
__isrcall void e1000e::on_interrupt() 
{
    irq_state icr{};
    read_dma(e1000_icr, icr);
    if(icr->link_status_change)
    {
        dev_ctrl ctl{};
        read_dma(e1000_ctrl, ctl);
        ctl->set_link_up = true;
        write_dma(e1000_ctrl, ctl);
    }
    if(icr->rxdt_min_thresh || icr->rx_timer) poll_rx();
    // read again to make sure the status clears
    read_dma(e1000_icr, icr);
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
e1000e::~e1000e()
{
    for(e1000e_receive_descriptor* d = rx_ring.descriptors; d < rx_ring.max_descriptor; d++)
        if(d->read.buffer_addr)
            __buffer_alloc.deallocate(reinterpret_cast<char*>(d->read.buffer_addr), max_single_rx_buffer);
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
    uint32_t bar = device->header_0x0.bar[0];
    addr_t base = kmm.map_dma(compute_base(bar), 0x20000UZ, (bar & BIT(3)) != 0);
    if(!base) throw std::bad_alloc();
    return base;
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
    if(where % 4UL) throw std::invalid_argument{ "e1000e dma access must be on dword boundary" };
    r_out = where.ref<uint32_t>();
    fence();
}
void e1000e::write_dma(int reg_id, uint32_t const& w_in)
{
    addr_t where = __mmio_region.plus(reg_id);
    if(where % 4UL) throw std::invalid_argument{ "e1000e dma access must be on dword boundary" };
    where.ref<uint32_t>() = w_in;
    fence();
}
bool e1000e::__mdio_await(mdic& mdic_reg)
{
    return await_result([&]() -> bool 
    {
        io_wait();
        read_dma(e1000_mdic, mdic_reg);
        if(mdic_reg->error) throw std::runtime_error{ "e1000e mdic read error" };
        return mdic_reg->ready;
    });
}
uint16_t e1000e::read_phy(int phy_reg)
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
    return mdic_reg->data;
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
    dev_ctrl ctrl;
    __pcie_e1000e_controller->command.memory_space = true;
    __pcie_e1000e_controller->command.io_space = true;
    __pcie_e1000e_controller->command.bus_master = true;
    barrier();
    write_dma(e1000_imc, ~0U);
    barrier();
    read_dma(e1000_ctrl, ctrl);
    barrier();
    ctrl->reset = true;
    write_dma(e1000_ctrl, ctrl);
    if(!await_result([&]() -> bool { io_wait(); read_dma(e1000_ctrl, ctrl); return !ctrl->reset; })) { panic("e1000e hung on reset"); return false; }
    barrier();
    write_dma(e1000_imc, ~0U);
    barrier();
    read_dma(e1000_status, status);
    barrier();
    if(!configure_mac_phy(status)) { panic("e1000e hung on link-up signal"); return false; }
    uint32_t init_zero = 0U;
    for(int i = e1000_stats_min; i < e1000_stats_max; i += 4) { read_dma(i, (init_zero = 0U)); read_dma(e1000_status, status); }
    for(int i = 0; i < 128; i++) { write_dma(e1000_mta + 4 * i, (init_zero = 0U)); read_dma(e1000_status, status); }netstack_buffer::poll_functor tx_poll = std::bind(&e1000e::poll_tx, this, std::placeholders::_1);
    for(size_t i = 0; i < rx_ring.count(); i++) transfer_buffers.emplace(64UZ, 64UZ, netstack_buffer::poll_functor(up_stack_functor), std::move(tx_poll), max_single_rx_buffer, max_single_tx_buffer);
    if(!configure_interrupts(status)) return false;
    if(!configure_rx(status)) return false;
    if(!configure_tx(status)) return false;
    return (__has_init = true);
}
bool e1000e::configure_interrupts(dev_status& st)
{
    uint32_t enable_all = 0x1F6DCU;
    write_dma(e1000_ims, enable_all);
    read_dma(e1000_icr, enable_all); // read the register to clear it
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
    return await_result([&]() -> bool { io_wait(); read_status(st); return st->link_up; });
}
bool e1000e::configure_tx(dev_status& st)
{
    for(e1000e_transmit_descriptor* d = tx_ring.descriptors; d < tx_ring.max_descriptor; d++) std::construct_at(d);
    uint32_t tx_head        = 0;
    uint32_t tx_tail        = static_cast<uint32_t>(tx_ring.max_descriptor - tx_ring.descriptors);
    qword txbase            = reinterpret_cast<uintptr_t>(tx_ring.descriptors);
    uint32_t tx_total_len   = tx_tail * 16U;
    write_dma(txbase_hi, txbase.hi);
    write_dma(txbase_lo, txbase.lo);
    read_status(st);
    write_dma(txlen, tx_total_len);
    read_status(st);
    write_dma(txh, tx_head);
    write_dma(txt, tx_tail);
    tx_desc_ctrl tctl;
    read_dma(txdctl, tctl);
    tctl->writeback_thresh          = 1UC;
    tctl->descriptor_granularity    = 1UC;
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
    for(e1000e_receive_descriptor* d = rx_ring.descriptors; d < rx_ring.max_descriptor; d++)
    {
        std::construct_at(d);
        char* buffer        = __buffer_alloc.allocate(max_single_rx_buffer);
        if(!buffer) return false;
        d->read.buffer_addr = translate_vaddr(buffer);
    }

    uint32_t rx_head    = 0U;
    uint32_t rx_tail    = static_cast<uint32_t>(rx_ring.max_descriptor - rx_ring.descriptors);
    qword rxbase        = reinterpret_cast<uintptr_t>(rx_ring.descriptors);
    uint32_t rx_total_len = rx_tail * 16U;
    write_dma(rxbase_hi, rxbase.hi);
    write_dma(rxbase_lo, rxbase.lo);
    read_status(st);
    write_dma(rxlen, rx_total_len);
    read_status(st);
    write_dma(rxh, rx_head);
    write_dma(rxt, rx_tail);
    read_status(st);
    rx_ctrl ctrl_reg
    {
        .data_struct
        {
            .enable                     { true },
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
    e1000e_transmit_descriptor& tail = tx_ring.tail();
    addr_t txbuf        = tail.buffer_addr ? addr_t(tail.buffer_addr) : addr_t(__buffer_alloc.allocate(max_single_tx_buffer));
    array_zero<uint64_t>(txbuf, max_single_tx_buffer / sizeof(uint64_t));
    tail.flags.length   = static_cast<uint16_t>(buff.getp(txbuf, max_single_tx_buffer));
    tail.flags.cmd      = 0b1011UC;
    tail.buffer_addr    = txbuf;
    tail.fields.status  = 0UC;
    write_dma(txt, (tx_ring.tail_descriptor = (tx_ring.tail_descriptor + 1) % tx_ring.count()));
    buff.flushp();
    if(await_result([&]() -> bool { io_wait(); return (tail.fields.status & 0x1) != 0; })) return 0;
    return -ENETDOWN;
}
int e1000e::poll_rx()
{
    while(rx_ring.tail().read.status & 1)
    {
        e1000e_receive_descriptor& tail = rx_ring.tail();
        addr_t packet(tail.read.buffer_addr);
        if((tail.read.status & 2) && !tail.read.errors)
        {
            if(transfer_buffers.at_end()) transfer_buffers.restart();
            netstack_buffer& buff   = transfer_buffers.pop();
            size_t len              = tail.read.length;
            try { buff.putg(packet, len); } catch(std::overflow_error& e) { panic(e.what()); return -EOVERFLOW; }
            int err                 = up_stack_functor(buff);
            if(err) return err;
        }
        tail.read.status = 0UC;
        write_dma(rxt, (rx_ring.tail_descriptor = (rx_ring.tail_descriptor + 1) % rx_ring.count()));
        fence();
    }
    return 0;
}