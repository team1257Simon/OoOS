#include "xhci.hpp"
namespace ooos
{
	using std::ranges::distance;
	using std::views::filter;
	using std::bind_front;
	xhci_config_type xhci_host_controller::__cfg(xhci_config());
	xhci_device_endpoint xhci_device_slot::__create_ep(xhci_device_slot& slot, uint8_t i) { return xhci_device_endpoint(slot, i); }
	std::array<xhci_device_endpoint, 31UZ> xhci_device_slot::__create_array(xhci_device_slot& slot) { return sequence_transform(bind_front(__create_ep, slot), __ep_seq()); }
	generic_config_table& xhci_host_controller::get_config() { return __cfg.generic; }
	size_t xhci_host_controller::size() const { return __slots.size(); }
	size_t xhci_device_slot::interface_count() const { return static_cast<size_t>(distance(__endpoints | filter(bind_front(&xhci_device_endpoint::active)))); }
	abstract_connectable_device::provider* xhci_device_slot::parent() { return std::addressof(__parent); }
	void xhci_device_endpoint::__ring_doorbell(uint16_t stream_id) { __parent.__doorbell = xhci_doorbell(__idx, 0UC, stream_id); }
	void xhci_device_endpoint::put_msg(generic_device_message const& msg) { /* TODO */ }
	std::optional<generic_device_message> xhci_device_endpoint::poll_msg() { return std::nullopt; /* TODO */ }
	abstract_connectable_device::interface::type xhci_device_endpoint::interface_type() const { return static_cast<type>(static_cast<uint8_t>(__ctx.type)); }
	xhci_host_controller::xhci_host_controller(pci_config_space* pci) :
		__hc_dev(pci),
		__dev_ctx_array(*this),
		__slots(this),
		__handlers(this),
		__ctx_and_sp_ptrs(*this),
		__scratchpads(*this),
		__event_ring_segment_tables(*this),
		__cmd_ring(this)
	{}
	xhci_device_endpoint::xhci_device_endpoint(xhci_device_slot& slot, uint8_t idx) :
		__parent(slot),
		__ctx(slot.__ctx[idx]),
		__streams(std::nullopt),
		__transfer_trb(std::nullopt),
		__idx(idx)
	{}
	xhci_device_slot::xhci_device_slot(xhci_host_controller& ctl, uint8_t idx) :
		__parent(ctl),
		__port(ctl.__hc_mem->port_registers[idx]),
		__ctx(ctl.__dev_ctx_array[idx]),
		__doorbell(ctl.doorbells[idx + 1]),
		__endpoints(__create_array(*this)),
		__active_endpoints(0UZ),
		__idx(idx)
	{}
	abstract_connectable_device::interface* xhci_device_slot::operator[](size_t idx)
	{
		if(__unlikely(idx >= 31UZ) || !__endpoints[idx])
			return nullptr;
		return std::addressof(__endpoints[idx]);
	}
	abstract_connectable_device* xhci_host_controller::operator[](size_t id)
	{
		if(__unlikely(id > __slots.size()))
			return nullptr;
		return std::to_address(__slots.begin() + id);
	}
	std::optional<size_t> xhci_host_controller::index_of(abstract_connectable_device* dev) const
	{
		xhci_device_slot* typed	= dynamic_cast<xhci_device_slot*>(dev);
		if(typed && typed >= std::to_address(__slots.begin()))
			return std::optional<size_t>(std::in_place, typed - std::to_address(__slots.begin()));
		return std::nullopt;
	}
	std::span<xhci_event_ring_segment_table_entry> xhci_host_controller::erst_sub(uint8_t idx)
	{
		size_t entries_per	= get_element<5>(__cfg);
		size_t start		= entries_per * idx;
		return __event_ring_segment_tables.subspan(start, start + entries_per);
	}
	void xhci_host_controller::finalize()
	{
		for(xhci_event_handler& h : __handlers) h.event_ring.segments.clear();
		__handlers.clear();
		__event_ring_segment_tables.destroy();
		__dev_ctx_array.destroy();
		__ctx_and_sp_ptrs.destroy();
		//	TODO: page scratchpads to disk if needed
		__scratchpads.destroy();
		__slots.clear();
		__cmd_ring.segments.clear();
	}
	bool xhci_host_controller::initialize()
	{
		typedef decltype(__hc_mem->status) hc_status;
		typedef std::span<addr_t>::iterator ptr_it;
		if(__unlikely(!__hc_dev)) return false;
		if(__unlikely(load_config() != 0)) return false;
		bar_desc bar	= compute_bar_info(*__hc_dev, 0);
		addr_t base		= map_dma(bar.base_value, bar.base_size, bar.is_prefetchable);
		if(__unlikely(!base)) log("E: no base address for mmio");
		else
		{
			__hc_caps						= base;
			__hc_mem						= __hc_caps->operational_registers();
			__hc_rt_mem						= __hc_caps->runtime_registers();
			doorbells						= __hc_caps->doorbell_registers();
			uint16_t erst_supported_max		= std::min(255US, static_cast<uint16_t>(1US << __hc_caps->event_ring_segment_table_max_shift2));
			//	Config values have hard limits given by the host controller's capability registers and/or the XHCI specification.
			//	There are also ranges imposed for sanity reasons, e.g. the maximum number of event ring segment table entries per interrupter.
			get_element<0>(__cfg)			= std::min(get_element<0>(__cfg), std::min(__hc_caps->max_ports, __hc_caps->max_slots));
			get_element<1>(__cfg)			= std::min(get_element<1>(__cfg), 4096US);
			get_element<3>(__cfg)			&= bar.is_prefetchable;
			get_element<5>(__cfg)			= std::min(get_element<5>(__cfg), static_cast<uint16_t>(std::min(256UZ, 1UZ << __hc_caps->event_ring_segment_table_max_shift2)));
			//	Write-back configuration values in case they've changed.
			//	The API checks equality and skips the write if everything is the same.
			if(__unlikely(save_config() != 0)) return false;
			volatile xhci_hc_mem& hc		= *__hc_mem;
			volatile hc_status& status		= hc.status;
			if(__unlikely(!await_completion(get_element<4>(__cfg), std::bind_front(&hc_status::ready, status))))
				return log("E: controller hung"), false;
			hc.config.max_slots_enabled		= get_element<0>(__cfg);
			try
			{
				__dev_ctx_array.create(get_element<0>(__cfg), get_element<3>(__cfg));
				size_t num_scratchpads		= __hc_caps->max_scratchpad_buffers_lo | (__hc_caps->max_scratchpad_buffers_hi << 5);
				size_t total_num_ptrs		= num_scratchpads + __dev_ctx_array.size() + 1UZ;
				__ctx_and_sp_ptrs.create(total_num_ptrs, get_element<3>(__cfg));
				if(num_scratchpads)
				{
					size_t dev_page_size	= PAGESIZE << (__builtin_ctzl(hc.max_pagesize));
					__scratchpads.create(dev_page_size * num_scratchpads, get_element<3>(__cfg));
					size_t first_sp_ptr		= __dev_ctx_array.size() + 1UZ;
					addr_t sp_addr			= __scratchpads.data();
					ptr_it p				= __ctx_and_sp_ptrs.begin() + first_sp_ptr;
					__ctx_and_sp_ptrs[0]	= std::to_address(p);
					for(size_t i			= 0UZ; i < num_scratchpads; i++, ++p, sp_addr += dev_page_size)
						*p					= sp_addr;
					//	TODO: load scratchpads from disk if needed and applicable
				}
				uint8_t max_slot			= get_element<0>(__cfg);
				for(uint8_t i				= 0UC; i < max_slot; i++) __slots.emplace_back(*this, i);
				if(__unlikely(!add_segment(__cmd_ring.segments))) return false;
				__cmd_ring.init_ptrs();
				hc.ctx_addr_array_ptr		= addr_t(__ctx_and_sp_ptrs.data());
				hc.cmd_ring					= std::to_address(__cmd_ring.dequeue);
				if(__unlikely(!__configure_interrupters())) return false;
				hc.command.interrupt_enable	= true;
				hc.command.run				= true;
				return true;
			}
			catch(std::exception& e) { logf("E: %s", e.what()); }
		}
		return false;
	}
	bool xhci_host_controller::add_segment(mod_mm_vec<trb_ring_segment>& ring) try
	{
		size_t prev_size		= ring.size();
		trb_ring_segment& seg	= ring.emplace_back(*this, get_element<1>(__cfg), get_element<3>(__cfg));
		array_zero(seg.data(), seg.size());
		xhci_link_trb& nx_link	= *new(std::addressof(seg.back())) xhci_link_trb();
		nx_link.type			= trb_type::TRB_LINK;
		if(prev_size)
		{
			xhci_link_trb& prev_link	= reinterpret_cast<xhci_link_trb&>(ring[prev_size - 1Z].back());
			prev_link.type				= trb_type::TRB_LINK;
			prev_link.data_ptr			= seg.data();
		}
		return true;
	}
	catch(std::exception& e) { return logf("E: %s", e.what()), false; }
	mod_mm_vec<trb_ring_segment> xhci_host_controller::create_ring(size_t n_segments)
	{
		mod_mm_vec<trb_ring_segment> result(this);
		for(size_t i = 0UZ; i < n_segments; i++)
			if(!add_segment(result))
				raise_error("bad_alloc", -ENOMEM);
		return result;
	}
	bool xhci_host_controller::__configure_interrupters()
	{
		auto handler0	= [this](xhci_generic_trb& e) -> void { /* TODO: primary interrupter (0) */ };
		auto handler1	= [this](xhci_generic_trb& e) -> void { /* TODO: secondary interrupter 1 */ };
		auto handler2	= [this](xhci_generic_trb& e) -> void { /* TODO: secondary interrupter 2 */ };
		auto handler3	= [this](xhci_generic_trb& e) -> void { /* TODO: secondary interrupter 3 */ };
		__handlers.emplace_back(*this, 0UC, handler0).init_state().set_enabled(true);
		__handlers.emplace_back(*this, 1UC, handler1).init_state().set_enabled(true);
		__handlers.emplace_back(*this, 2UC, handler2).init_state().set_enabled(true);
		__handlers.emplace_back(*this, 3UC, handler3).init_state().set_enabled(true);
		pci_capabilities_register* msix = find_capability_register(*__hc_dev, pci_capability_id::MSIX);
		if(msix)
		{
			bar_desc msix_bar			= compute_bar_info(*__hc_dev, static_cast<int>(msix->msix.table_offset.bar_idx));
			size_t needed_dma			= msix->msix.message_control.table_size * sizeof(msix_t);
			msix_t volatile* regs		= addr_t(map_dma(msix_bar.base_value + msix->msix.table_offset.table_offset, needed_dma, msix_bar.is_prefetchable));
			for(int i = 0; i < 4; i++)
				if(__unlikely(!register_msi(regs[i], std::ref(__handlers[i]))))
					return false;
			return true;
		}
		else
		{
			addr_t msi						= find_capability_register(*__hc_dev, pci_capability_id::MSI);
			if(__unlikely(!msi)) return log("E: XHCI controller must have either MSIX or MSI support"), false;
			bool is_x64						= msi.deref<pci_capabilities_register>().msi_32.message_control.x64_capable;
			pci_capabilities_register* msir	= msi;
			if(is_x64) return register_msi(msir->msi_64, std::ref(__handlers[0]), std::ref(__handlers[1]), std::ref(__handlers[2]), std::ref(__handlers[3]));
			else return register_msi(msir->msi_32, std::ref(__handlers[0]), std::ref(__handlers[1]), std::ref(__handlers[2]), std::ref(__handlers[3]));
		}
	}
	xhci_event_handler& xhci_event_handler::init_state()
	{
		trb_ring_segment& first_ring						= event_ring.segments.front();
		event_ring_segment_table[0].ring_segment_base_addr	= first_ring.data();
		event_ring_segment_table[0].ring_segment_size		= static_cast<uint16_t>(first_ring.size());
		interrupter.event_ring_segment_table_base			= event_ring_segment_table.data();
		interrupter.segment_table_size						= 1US;
		interrupter.moderation_interval						= get_element<2>(xhci_host_controller::__cfg);
		return *this;
	}
	void xhci_event_handler::add_new_segment()
	{
		size_t n											= event_ring.segments.size();
		parent.add_segment(event_ring.segments);
		trb_ring_segment& added								= event_ring.segments.back();
		event_ring_segment_table[n].ring_segment_base_addr	= added.data();
		event_ring_segment_table[n].ring_segment_size		= static_cast<uint16_t>(added.size());
		barrier();
		uint16_t volatile& size_ref							= interrupter.segment_table_size;
		uint16_t size_val									= size_ref;
		barrier();
		size_ref											= size_val + 1US;
	}
	void xhci_event_handler::operator()()
	{
		typedef std::span<xhci_generic_trb>::iterator ring_it;
		ring_it ring_end	= event_ring.ring_end();
		typedef decltype(interrupter.event_ring_dequeue_base) dequeue_ptr;
		while(event_ring.dequeue->type.cycle == cycle_state)
		{
			handler(*event_ring.dequeue++);
			if(event_ring.dequeue == ring_end) {
				event_ring.reset_ptr(event_ring.dequeue);
				cycle_state	^= true;	// toggle the state bit
			}
		}
		dequeue_ptr dp						= interrupter.event_ring_dequeue_base;
		barrier();
		dp									= std::to_address(event_ring.dequeue);
		dp.event_handler_busy				= false;
		barrier();
		interrupter.event_ring_dequeue_base	= dp;
	}
}
typedef ooos::xhci_host_controller xhci_host_controller;
EXPORT_MODULE(xhci_host_controller, ooos::api_global->find_pci_device(devclass_sb, subclass_usb, progif_xhci));