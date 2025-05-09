#include "arch/net/e1000e.hpp"
#include "isr_table.hpp"
#include "functional"
#include "stdexcept"
static std::alignas_allocator<char, int128_t> __buffer_alloc;
__isrcall void e1000e::on_interrupt() {} // TODO
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
    for(e1000e_receive_descriptor* d = rx_ring.head_descriptor; d < rx_ring.max_descriptor; d++)
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
    addr_t base = kmm.map_dma(compute_base(bar), 0x20000, (bar & (1 << 3)) != 0);
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
            .operation              { 0b01 },
            .ready                  { 0 },
            .interrupt_enable       { 1 },
            .error                  { 0 }
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
            .read_start     { 1 },
            .read_done      { 0 },
            .read_address   { static_cast<uint16_t>(eep_addr & 0x3FFFUS) },
            .read_data      {}
        }
    };
    write_dma(e1000_eerd, rd);
    await_result([&]() -> bool { read_dma(e1000_eerd, rd); return rd->read_done; });
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
            .operation              { 0b10 },
            .ready                  { 0 },
            .interrupt_enable       { 1 },
            .error                  { 0 }
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
    __pcie_e1000e_controller->command.interrupt_disable = true;
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
    if(!await_result([&]() -> bool { io_wait(); read_dma(e1000_ctrl, ctrl); return !ctrl->reset; })) return false;
    barrier();
    write_dma(e1000_imc, ~0U);
    barrier();
    read_dma(e1000_status, status);
    barrier();
    if(!configure_mac_phy(status)) return false;
    uint32_t init_zero = 0U;
    for(int i = e1000_stats_min; i < e1000_stats_max; i += 4) { read_dma(i, (init_zero = 0U)); read_dma(e1000_status, status); }
    for(int i = 0; i < 128; i++) { write_dma(e1000_mta + 4 * i, (init_zero = 0U)); read_dma(e1000_status, status); }
    if(!configure_rx(status)) return false;
    if(!configure_tx(status)) return false;
    if(!configure_interrupts(status)) return false;
    return (__has_init = true);
}
bool e1000e::configure_interrupts(dev_status& st)
{
    uint32_t enable_all = 0x1F6DCU;
    write_dma(e1000_ims, enable_all);
    read_dma(e1000_icr, enable_all); // read the register to clear it
    read_dma(e1000_status, st);
    interrupt_table::add_irq_handler(__pcie_e1000e_controller->header_0x0.interrupt_pin, std::bind(&e1000e::on_interrupt, this));
    irq_clear_mask(__pcie_e1000e_controller->header_0x0.interrupt_pin);
    __pcie_e1000e_controller->command.interrupt_disable = false;
    return true;
}
bool e1000e::configure_mac_phy(dev_status& st)
{
    word mac_word = read_eeprom(0US);
    mac_addr[0] = mac_word.lo;
    mac_addr[1] = mac_word.hi;
    mac_word = read_eeprom(1US);
    mac_addr[2] = mac_word.lo;
    mac_addr[3] = mac_word.hi;
    mac_word = read_eeprom(2US);
    mac_addr[4] = mac_word.lo;
    mac_addr[5] = mac_word.hi;
    dev_ctrl ctl{};
    read_dma(e1000_ctrl, ctl);
    ctl->set_link_up = true;
    write_dma(e1000_ctrl, ctl);
    read_dma(e1000_status, st);
    return st->link_up;
}
bool e1000e::configure_tx(dev_status& st)
{
    for(e1000e_transmit_descriptor* d = tx_ring.head_descriptor; d < tx_ring.max_descriptor; d++) std::construct_at(d);
    constexpr int txbase_hi = e1000_tdbah(0);
    constexpr int txbase_lo = e1000_tdbal(0);
    constexpr int txlen = e1000_tdlen(0);
    constexpr int txh = e1000_tdh(0);
    constexpr int txt = e1000_tdt(0);
    constexpr int txdctl = e1000_txdctl(0);
    uint32_t tx_head = 0;
    uint32_t tx_tail = static_cast<uint32_t>(tx_ring.max_descriptor - tx_ring.head_descriptor);
    qword txbase = reinterpret_cast<uintptr_t>(tx_ring.head_descriptor);
    uint32_t tx_total_len = tx_tail * 16U;
    write_dma(txbase_hi, txbase.hi);
    write_dma(txbase_lo, txbase.lo);
    read_dma(e1000_status, st);
    write_dma(txlen, tx_total_len);
    read_dma(e1000_status, st);
    write_dma(txh, tx_head);
    write_dma(txt, tx_tail);
    tx_desc_ctrl tctl;
    read_dma(txdctl, tctl);
    tctl->writeback_thresh = 1;
    tctl->descriptor_granularity = 1;
    write_dma(txdctl, tctl);
    read_dma(e1000_status, st);
    tx_ctrl ctrl_reg
    {
        .data_struct
        {
            .enable                     { 1 },
            .pad_short_packets          { 1 },
            .collision_threshold        { 0x0FUC },
            .collision_distance         { 0x03UC },
            .software_xoff              { 0 },
            .retransmit_late_collision  { 0 },
            .underrun_no_retransmit     { 0 },
            .tdtms                      { 0 },
            .multi_request_support      { 0 }
        }
    };
    write_dma(e1000_tctl, ctrl_reg);
    read_dma(e1000_status, st);
    return true;
}
bool e1000e::configure_rx(dev_status& st)
{
    for(e1000e_receive_descriptor* d = rx_ring.head_descriptor; d < rx_ring.max_descriptor; d++)
    {
        std::construct_at(d);
        char* buffer = __buffer_alloc.allocate(max_single_rx_buffer);
        if(!buffer) return false;
        d->read.buffer_addr = translate_vaddr(buffer);
    }
    constexpr int rxbase_hi = e1000_rdbah(0);
    constexpr int rxbase_lo = e1000_rdbal(0);
    constexpr int rxlen = e1000_rdlen(0);
    constexpr int rxh = e1000_rdh(0);
    constexpr int rxt = e1000_rdt(0);
    uint32_t rx_head = 0;
    uint32_t rx_tail = static_cast<uint32_t>(rx_ring.max_descriptor - rx_ring.head_descriptor);
    qword rxbase = reinterpret_cast<uintptr_t>(rx_ring.head_descriptor);
    uint32_t rx_total_len = rx_tail * 16U;
    write_dma(rxbase_hi, rxbase.hi);
    write_dma(rxbase_lo, rxbase.lo);
    read_dma(e1000_status, st);
    write_dma(rxlen, rx_total_len);
    read_dma(e1000_status, st);
    write_dma(rxh, rx_head);
    write_dma(rxt, rx_tail);
    read_dma(e1000_status, st);
    rx_ctrl ctrl_reg
    {
        .data_struct
        {
            .enable                     { 1 },
            .store_bad_packets          { 1 },
            .unicast_promiscuous        { 1 },
            .multicast_promiscuous      { 1 },
            .long_packet_enable         { 1 },
            .loopback_mode              { 0 },
            .rdtms                      { 0 },
            .desc_type                  { 0 },
            .multicast_offset           { 0 },
            .broadcast_accept           { 1 },
            .buffer_size_shift          { 1 },
            .vlan_filter_enable         { 0 },
            .canonical_form_enable      { 0 },
            .canonical_form_indicator   { 0 },
            .discard_pause_frames       { 0 },
            .pass_mac_control_frames    { 0 },
            .extended_buffer_size       { 1 },
            .strip_ethernet_crc         { 1 },
            .flex_buffer_size           { 0 }
        }
    };
    write_dma(e1000_rctl, ctrl_reg);
    read_dma(e1000_status, st);
    return true;
}