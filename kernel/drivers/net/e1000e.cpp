#include "arch/net/e1000e.hpp"
#include "functional"
#include "stdexcept"
e1000e_base::~e1000e_base() 
{
    if(tx_ring.descriptors)
        kmm.deallocate_block(tx_ring.descriptors, tx_ring.buffer_region_size + tx_ring.descriptor_count * sizeof(e1000e_transmit_descriptor));
    if(rx_ring.descriptors)
        kmm.deallocate_block(rx_ring.descriptors, rx_ring.buffer_region_size + rx_ring.descriptor_count * sizeof(e1000e_receive_descriptor));
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
static inline e1000e_ring<e1000e_transmit_descriptor> tx_desc_region_alloc(size_t descriptor_count_factor)
{
    size_t descriptor_count = descriptor_count_factor * e1000_rxtxdesclen_base;
    size_t descriptors_size = descriptor_count * sizeof(e1000e_transmit_descriptor);
    size_t buffer_size = max_single_tx_buffer * descriptor_count + 80;
    size_t total_size = kernel_memory_mgr::dma_size(buffer_size + descriptors_size);
    addr_t allocated = kmm.allocate_dma(buffer_size + descriptors_size, true);
    if(!allocated) throw std::bad_alloc();
    return e1000e_ring<e1000e_transmit_descriptor>
    {
        .descriptors            { allocated },
        .descriptor_count       { descriptor_count },
        .buffer_region_start    { allocated.plus(descriptors_size) },
        .buffer_region_size     { static_cast<size_t>(total_size - descriptors_size) }
    };
}
bool e1000e_base::read_io(int reg_id, uint32_t& r_out)
{
    if(!__io_addr_port) return false;
    __io_addr_port.put(reg_id);
    barrier();
    r_out = __io_data_port.get();
    return true;
}
bool e1000e_base::write_io(int reg_id, uint32_t const& w_in)
{
    if(!__io_addr_port) return false;
    __io_addr_port.put(reg_id);
    barrier();
    __io_data_port.put(w_in);
    barrier();
    return w_in == __io_data_port.get();
}
void e1000e_base::read_dma(int reg_id, uint32_t& r_out)
{
    addr_t where = __mmio_region.plus(reg_id);
    if(where % 4UL) throw std::invalid_argument{ "access must be on dword boundary" };
    r_out = where.ref<uint32_t>();
    fence();
}
void e1000e_base::write_dma(int reg_id, uint32_t const& w_in)
{
    addr_t where = __mmio_region.plus(reg_id);
    if(where % 4UL) throw std::invalid_argument{ "access must be on dword boundary" };
    where.ref<uint32_t>() = w_in;
    fence();
}

static inline e1000e_ring<e1000e_receive_descriptor> rx_desc_region_alloc(size_t descriptor_count_factor)
{
    size_t descriptor_count = descriptor_count_factor * e1000_rxtxdesclen_base;
    size_t descriptors_size = descriptor_count * sizeof(e1000e_receive_descriptor);
    size_t buffer_size = max_single_rx_buffer * descriptor_count;
    size_t total_size = kernel_memory_mgr::dma_size(buffer_size + descriptors_size);
    addr_t allocated = kmm.allocate_dma(buffer_size + descriptors_size, true);
    if(!allocated) throw std::bad_alloc();
    return e1000e_ring<e1000e_receive_descriptor>
    {
        .descriptors            { allocated },
        .descriptor_count       { descriptor_count },
        .buffer_region_start    { allocated.plus(descriptors_size) },
        .buffer_region_size     { static_cast<size_t>(total_size - descriptors_size) }
    };
}
e1000e_base::e1000e_base(pci_config_space* device, size_t descriptor_count_factor) : 
    __has_init                  { false },
    __mmio_region               { map_device_mmio(device) },
    __pcie_e1000e_controller    { device },
    __io_addr_port              { get_io_bar(device) },
    __io_data_port              { __io_addr_port + 4US },
    rx_ring                     { rx_desc_region_alloc(descriptor_count_factor) },
    tx_ring                     { tx_desc_region_alloc(descriptor_count_factor) }
                                {}
bool e1000e_base::initialize()
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
    if(!await_result([&]() -> bool { io_wait(); read_dma(e1000_ctrl, ctrl); return !ctrl->reset; }, 100UL)) return false;
    barrier();
    write_dma(e1000_imc, ~0U);
    barrier();
    read_dma(e1000_status, status);
    barrier();
    if(!configure_model_specific(status)) return false;
    if(!configure_mac_phy(status)) return false;
    uint32_t stat_dummy = 0U;
    for(int i = e1000_stats_min; i < e1000_stats_max; i += 4) { read_dma(i, stat_dummy); read_dma(e1000_status, status); }
    if(!configure_rx(status)) return false;
    if(!configure_tx(status)) return false;
    if(!configure_interrupts(status)) return false;
    return (__has_init = true);
}