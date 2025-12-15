#include "intel_e1000e.hpp"
constexpr int rxbase_hi		= e1000_rdbah(0);
constexpr int rxbase_lo		= e1000_rdbal(0);
constexpr int rxlen			= e1000_rdlen(0);
constexpr int rxh			= e1000_rdh(0);
constexpr int rxt			= e1000_rdt(0);
constexpr int txbase_hi		= e1000_tdbah(0);
constexpr int txbase_lo		= e1000_tdbal(0);
constexpr int txlen			= e1000_tdlen(0);
constexpr int txh			= e1000_tdh(0);
constexpr int txt			= e1000_tdt(0);
constexpr int txdctl		= e1000_txdctl(0);
static inline uint16_t get_io_bar(pci_config_space* device)
{
	if(device->header_type == st)
		for(int i = 0; i < 6; i++) 
			if(device->header_0x0.bar[i] & 0x01)
				return static_cast<uint16_t>(device->header_0x0.bar[i] & 0xFFFCUS);
	return 0;
}
template<ooos::condition_callable FT>
static inline bool await_completion(time_t max_spin, FT&& ft)
{
	for(time_t spin = 0UZ; spin < max_spin; spin++)
		if(ft())
			return true;
	return ft();
}
static inline addr_t compute_base_addr(uintptr_t bar) { return addr_t((bar & 0x00000001UZ) ? (bar & 0xFFFFFFFCUZ) : (bar & 0xFFFFFFF0UZ)); }
ie1000e::config_type ie1000e::cfg	= e1000e_config();
ooos::generic_config_table& ie1000e::get_config() { return cfg.generic; }
size_t ie1000e::rx_limit() const noexcept { return ooos::get_element<2>(cfg); }
size_t ie1000e::tx_limit() const noexcept { return ooos::get_element<1>(cfg); }
size_t ie1000e::buffer_count() const noexcept { return ooos::get_element<0>(cfg); }
void ie1000e::read_status(dev_status& status) { read_dma(e1000_status, status); }
ie1000e::~ie1000e() = default;
ie1000e::ie1000e(pci_config_space* device) :
	__mmio_region				{},
	__pcie_e1000e_controller    { device },
	__io_addr_port              { get_io_bar(device) },
	__io_data_port              { __io_addr_port + 4US },
	rx_ring						{ this },
	tx_ring						{ this }
								{}
void ie1000e::fini_dev() {
	rx_ring.destroy();
	tx_ring.destroy();
}
bool ie1000e::__mdio_await(mdic& mdic_reg)
{
	return await_completion(ooos::get_element<6>(cfg), [&mdic_reg, this]() -> bool 
	{
		io_wait();
		read_dma(e1000_mdic, mdic_reg);
		if(mdic_reg->error) throw std::runtime_error("MDIC read error");
		return mdic_reg->ready;
	});
}
bool ie1000e::dev_reset()
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
	if(__unlikely(!await_completion(ooos::get_element<6>(cfg), [&ctrl, this]() -> bool { io_wait(); read_dma(e1000_ctrl, ctrl); return !ctrl->reset; }))) {
		log("E: device hung on reset");
		return false;
	}
	barrier();
	write_dma(e1000_imc, ~0U);
	barrier();
	return true;
}
bool ie1000e::read_io(int reg_id, uint32_t& r_out)
{
	if(!__io_addr_port) return false;
	__io_addr_port.put(reg_id);
	barrier();
	r_out = __io_data_port.get();
	return true;
}
bool ie1000e::write_io(int reg_id, uint32_t const& w_in)
{
	if(!__io_addr_port) return false;
	__io_addr_port.put(reg_id);
	barrier();
	__io_data_port.put(w_in);
	barrier();
	return w_in == __io_data_port.get();
}
void ie1000e::read_dma(int reg_id, uint32_t& r_out)
{
	addr_t where	= __mmio_region.plus(reg_id);
	if(where.full % 4UL) throw std::invalid_argument("[e1000e] dma access must be on dword boundary");
	r_out			= where.deref<uint32_t>();
	fence();
}
uint32_t ie1000e::read_dma(int reg_id)
{
	uint32_t result;
	read_dma(reg_id, result);
	return result;
}
void ie1000e::write_dma(int reg_id, uint32_t const& w_in)
{
	addr_t where	= __mmio_region.plus(reg_id);
	if(where.full % 4UL) throw std::invalid_argument("[e1000e] dma access must be on dword boundary");
	where.assign(w_in);
	fence();
}
void ie1000e::read_phy(int phy_reg, uint16_t& out)
{
	mdic mdic_reg
	{
		.data_struct
		{
			.phy_register_address	{ static_cast<uint8_t>(phy_reg & 0x1FUC) },
			.phy_address			{ 1US },
			.operation				{ 1UC },
			.ready					{ false },
			.interrupt_enable		{ true },
			.error					{ false }
		}
	};
	write_dma(e1000_mdic, mdic_reg);
	__mdio_await(mdic_reg);
	out = mdic_reg->data;
}
uint16_t ie1000e::read_eeprom(uint16_t eep_addr)
{
	ee_read rd
	{
		.data_struct
		{
			.read_start		{ true },
			.read_done		{ false },
			.read_address	{ static_cast<uint16_t>(eep_addr & 0x3FFFUS) },
			.read_data		{ 0US }
		}
	};
	write_dma(e1000_eerd, rd);
	await_completion(ooos::get_element<6>(cfg), [&rd, this]() -> bool { io_wait(); read_dma(e1000_eerd, rd); return rd->read_done; });
	return rd->read_data;
}
void ie1000e::write_phy(int phy_reg, uint16_t data)
{
	mdic mdic_reg
	{
		.data_struct
		{
			.data					{ data },
			.phy_register_address	{ static_cast<uint8_t>(phy_reg & 0x1FUC) },
			.phy_address			{ 1US },
			.operation				{ 2UC },
			.ready					{ false },
			.interrupt_enable		{ true },
			.error					{ false }
		}
	};
	write_dma(e1000_mdic, mdic_reg);
	__mdio_await(mdic_reg);
}
bool ie1000e::init_dev()
{
	if(__unlikely(!__pcie_e1000e_controller)) {
		log("E: no device found");
		return false;
	}
	uint32_t bar	= __pcie_e1000e_controller->header_0x0.bar[0];
	addr_t base		= map_dma(compute_base_addr(bar), 0x20000UZ, (bar & BIT(3)) != 0);
	if(__unlikely(!base)) log("E: no base address for mmio");
	else try
	{
		rx_ring.create(ooos::get_element<0>(cfg));
		tx_ring.create(ooos::get_element<0>(cfg));
		__mmio_region	= base;
		dev_status status;
		__pcie_e1000e_controller->command.memory_space	= true;
		__pcie_e1000e_controller->command.io_space		= true;
		__pcie_e1000e_controller->command.bus_master	= true;
		if(__unlikely(!dev_reset())) return false;
		if(__unlikely(!configure_mac_phy(status))) return false; 
		for(int i = e1000_stats_min; i < e1000_stats_max; i += 4) { read_dma(i); read_status(status); }
		for(int i = 0; i < 128; i++) { write_dma(e1000_mta + 4 * i, 0U); read_status(status); }
		if(__unlikely(!configure_rx(status))) return false;
		if(__unlikely(!configure_tx(status))) return false;
		if(__unlikely(!configure_interrupts(status))) return false;
		enable_receive();
		return true;
	}
	catch(...) { log("E: out of memory"); }
	return false;
}
bool ie1000e::configure_rx(dev_status& st) try
{
	std::ext::resettable_queue<netstack_buffer>::iterator i = transfer_head();
	for(e1000e_receive_descriptor* d = rx_ring.descriptors; d < rx_ring.max_descriptor; d++, i++) 
		new(d) e1000e_receive_descriptor{ .read{ .buffer_addr{ vtranslate(i->rx_base()) } } };
		rx_ring.tail_descriptor	= static_cast<uint32_t>(rx_ring.max_descriptor - rx_ring.descriptors) / e1000_rxtxdesclen_base;
	qword rxbase			= reinterpret_cast<uintptr_t>(rx_ring.descriptors);
	uint32_t rx_total_len	= rx_ring.tail_descriptor * e1000_rxtxdesclen_base * sizeof(e1000e_receive_descriptor);
	write_dma(rxbase_hi, rxbase.hi);
	write_dma(rxbase_lo, rxbase.lo);
	read_status(st);
	write_dma(rxlen, rx_total_len);
	read_status(st);
	write_dma(rxh, rx_ring.head_descriptor);
	write_dma(rxt, rx_ring.tail_descriptor);
	read_status(st);
	rx_ctrl ctrl_reg{ .data_struct	= ooos::get_element<3>(cfg) };
	write_dma(e1000_rctl, ctrl_reg);
	read_status(st);
	return true;
}
catch(std::exception& e) {
	log(e.what());
	return false;
}
bool ie1000e::configure_tx(dev_status& st) try
{
	std::ext::resettable_queue<netstack_buffer>::iterator i = transfer_head();
	for(e1000e_transmit_descriptor* d = tx_ring.descriptors; d < tx_ring.max_descriptor; d++, i++)
		new(d) e1000e_transmit_descriptor{ .buffer_addr{ vtranslate(i->tx_base()) } };
	uint32_t tx_tail		= static_cast<uint32_t>(tx_ring.max_descriptor - tx_ring.descriptors);
	qword txbase			= reinterpret_cast<uintptr_t>(tx_ring.descriptors);
	uint32_t tx_total_len	= tx_tail * 16U;
	write_dma(txbase_hi, txbase.hi);
	write_dma(txbase_lo, txbase.lo);
	read_status(st);
	write_dma(txlen, tx_total_len);
	read_status(st);
	write_dma(txh, tx_ring.head_descriptor);
	write_dma(txt, tx_tail);
	tx_desc_ctrl tctl;
	read_dma(txdctl, tctl);
	tctl->writeback_thresh			= 1UC;
	tctl->descriptor_granularity	= true;
	write_dma(txdctl, tctl);
	read_status(st);
	tx_ctrl ctrl_reg{ .data_struct	= ooos::get_element<4>(cfg) };
	ctrl_reg->enable				= true;
	write_dma(e1000_tctl, ctrl_reg);
	read_status(st);
	return true;
}
catch(std::exception& e) {
	logf("E: %s", e.what());
	return false;
}
bool ie1000e::configure_mac_phy(dev_status& st) try
{
	word mac_word		= read_eeprom(0US);
	mac_addr[0]			= mac_word.lo;
	mac_addr[1]			= mac_word.hi;
	mac_word			= read_eeprom(1US);
	mac_addr[2]			= mac_word.lo;
	mac_addr[3]			= mac_word.hi;
	mac_word			= read_eeprom(2US);
	mac_addr[4]			= mac_word.lo;
	mac_addr[5]			= mac_word.hi;
	dev_ctrl ctl{};
	read_dma(e1000_ctrl, ctl);
	ctl->set_link_up    = true;
	write_dma(e1000_ctrl, ctl);
	if(await_completion(ooos::get_element<6>(cfg), [&st, this]() -> bool { io_wait(); read_status(st); return st->link_up; })) return true;
	log("E: device hung on link-up signal");
	return false;
}
catch(std::exception& e) {
	logf("E: %s", e.what());
	return false;
}
bool ie1000e::configure_interrupts(dev_status& st) try
{
	irq_config enabled_ints{ .data_struct = ooos::get_element<5>(cfg) };
	write_dma(e1000_ims, enabled_ints);
	read_dma(e1000_icr); // read the register to clear it
	read_status(st);
	on_irq(__pcie_e1000e_controller->header_0x0.interrupt_line, std::bind(&ie1000e::on_interrupt, this));
	__pcie_e1000e_controller->command.interrupt_disable = false;
	return true;
}
catch(std::exception& e) {
	logf("E: %s", e.what());
	return false;
}
void ie1000e::on_interrupt() try
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
}
catch(std::exception& e) { raise_error(e.what()); }
void ie1000e::enable_transmit()
{
	tx_ctrl ctl{};
	read_dma(e1000_tctl, ctl);
	ctl->enable = true;
	write_dma(e1000_tctl, ctl);
}
void ie1000e::enable_receive()
{
	rx_ctrl ctl{};
	read_dma(e1000_rctl, ctl);
	ctl->enable = true;
	write_dma(e1000_rctl, ctl);
}
void ie1000e::disable_transmit()
{
	tx_ctrl ctl{};
	read_dma(e1000_tctl, ctl);
	ctl->enable = false;
	write_dma(e1000_tctl, ctl);
}
void ie1000e::disable_receive()
{
	rx_ctrl ctl{};
	read_dma(e1000_rctl, ctl);
	ctl->enable = false;
	write_dma(e1000_rctl, ctl);
}
int ie1000e::poll_tx(netstack_buffer& buff)
{
	e1000e_transmit_descriptor& tail	= tx_ring.tail();
	tail.flags.length					= static_cast<uint16_t>(buff.count(std::ios_base::out));
	tail.flags.cmd						= 0b1011UC;
	tail.fields.status					= 0UC;
	tx_ring.tail_descriptor				= (tx_ring.tail_descriptor + 1U) % tx_ring.count();
	write_dma(txt, tx_ring.tail_descriptor);
	if(!await_completion(ooos::get_element<6>(cfg), [&tail, this]() -> bool { io_wait(); return (tail.fields.status & 0x1UC) != 0; })) return -ENETDOWN;
	return 0;
}
int ie1000e::poll_rx()
{
	uint32_t head					= rx_ring.head_descriptor;
	read_dma(rxh, rx_ring.head_descriptor);
	netstack_buffer* active_buffer	= nullptr;
	std::vector<netstack_buffer*> to_process;
	for(uint32_t i = head; i < rx_ring.head_descriptor; fence(), i++)
	{
		e1000e_receive_descriptor& desc	= rx_ring.descriptors[i];
		netstack_buffer& buff			= *(transfer_head() + i);
		if(desc.read.status.done && !desc.read.errors)
		{
			buff.pubseekpos(std::streampos(desc.read.length), std::ios_base::in);
			if(desc.read.status.end_of_packet)
			{
				try { to_process.push_back(active_buffer ? active_buffer : std::addressof(buff)); }
				catch(...) { raise_error("out of memory", -ENOMEM); }
				active_buffer			= nullptr;
			}
			else
			{
				if(!active_buffer)
					active_buffer		= std::addressof(buff);
				else
					active_buffer->rx_accumulate(buff);
			}
		}
		addr_t(std::addressof(desc.read.status)).assign(0UC);
		rx_ring.tail_descriptor			= (rx_ring.tail_descriptor + 1U) % rx_ring.count();
		addr_t(std::addressof(rx_ring.tail())).plus(sizeof(uintptr_t)).assign(0UL);
	}
	write_dma(rxt, rx_ring.tail_descriptor);
	fence();
	for(netstack_buffer* buff : to_process) buff->rx_flush();
	return 0;
}
EXPORT_MODULE(ie1000e, ooos::api_global->find_pci_device(device_class_e1000e, device_subclass_e1000e));