#include "xhci.hpp"
namespace ooos
{
	static inline std::array<xhci_device_endpoint, 31UZ> ep_array(xhci_device_slot& s)
	{
		auto create = [&s]<uint8_t ... Is>(std::integer_sequence<uint8_t, Is...>) {
			typedef std::array<xhci_device_endpoint, sizeof...(Is)> result_t;
			return result_t{ xhci_device_endpoint(s, Is) ... };
		};
		return create(std::make_integer_sequence<uint8_t, 31UC>());
	}
	xhci_config_type xhci_host_controller::__cfg(xhci_config());
	void xhci_host_controller::__handle_irq() { /* TODO */ }
	xhci_host_controller::xhci_host_controller(pci_config_space* pci) : __hc_dev(pci), __dev_ctx_array(*this) {}
	generic_config_table& xhci_host_controller::get_config() { return __cfg.generic; }
	size_t xhci_host_controller::size() const { return __slots.size(); }
	size_t xhci_device_slot::interface_count() const { return __builtin_popcount(__active_endpoints); }
	abstract_connectable_device::provider* xhci_device_slot::parent() { return std::addressof(__parent); }
	void xhci_device_endpoint::put_msg(generic_device_message const& msg) { /* TODO */ }
	std::optional<generic_device_message> xhci_device_endpoint::poll_msg() { return std::nullopt; /* TODO */ }
	void xhci_device_endpoint::reset() { /* TODO */ }
	xhci_device_endpoint::xhci_device_endpoint(xhci_device_slot& slot, uint8_t idx) :
		__parent(slot),
		__ctx(slot.__ctx[idx + 1]),
		__transfer_ring(),
		__dequeue_pos(),
		__enqueue_pos(),
		__idx(idx)
	{}
	xhci_device_slot::xhci_device_slot(xhci_host_controller& ctl, uint8_t idx) :
		__parent(ctl),
		__port(ctl.__hc_mem->port_registers[idx]),
		__ctx(ctl.__dev_ctx_array[idx]),
		__doorbell(ctl.__doorbells[idx + 1]),
		__endpoints(ep_array(*this)),
		__active_endpoints(),
		__idx(idx)
	{}
	abstract_connectable_device::interface* xhci_device_slot::operator[](size_t idx)
	{
		if(__unlikely(idx > 30UZ || !(__active_endpoints[0]) || !(__active_endpoints[idx + 1])))
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
	void xhci_host_controller::finalize()
	{
		if(__ptr_array_dma_block)
			release_dma(__ptr_array_dma_block, __ptr_array_dma_size);
		if(__scratchpad_block)
			release_dma(__scratchpad_block, __scratchpad_size);
		__dev_ctx_array.destroy();
		
		__slots.clear();
	}
	bool xhci_host_controller::initialize()
	{
		if(__unlikely(!__hc_dev)) return false;
		bar_desc bar	= compute_bar_info(*__hc_dev, 0);
		addr_t base		= map_dma(bar.base_value, bar.base_size, bar.is_prefetchable);
		if(__unlikely(!base)) log("E: no base address for mmio");
		else
		{
			__hc_caps					= base;
			__hc_mem					= __hc_caps->operational_registers();
			__hc_rt_mem					= __hc_caps->runtime_registers();
			__doorbells					= __hc_caps->doorbell_registers();
			get_element<0>(__cfg)		= std::min(get_element<0>(__cfg), std::min(__hc_caps->max_ports, __hc_caps->max_slots));
			get_element<3>(__cfg)		&= bar.is_prefetchable;
			try
			{
				__dev_ctx_array.create(get_element<0>(__cfg), get_element<3>(__cfg));
				size_t num_scratchpads	= __hc_caps->max_scratchpad_buffers_lo | (__hc_caps->max_scratchpad_buffers_hi << 5);
				size_t total_num_ptrs	= num_scratchpads;
				__ptr_array_dma_size	= total_num_ptrs * sizeof(addr_t);
				__ptr_array_dma_block	= allocate_dma(__ptr_array_dma_size, bar.is_prefetchable);
				size_t dev_page_size	= PAGESIZE << (__builtin_ctzl(__hc_mem->max_pagesize));
				addr_t* ptrs			= __ptr_array_dma_block;
				addr_t* ctx_ptr_pos		= std::addressof(ptrs[1]);	// index 0 is where the scratchpad pointer array pointer goes
				for(xhci_device_context& ctx : __dev_ctx_array)
					*ctx_ptr_pos++		= std::addressof(ctx);
				if(num_scratchpads)
				{
					__scratchpad_size	= dev_page_size * num_scratchpads;
					__scratchpad_block	= allocate_dma(__scratchpad_size, bar.is_prefetchable);
					*ptrs				= ctx_ptr_pos;
					ptrs				= ctx_ptr_pos;
					for(size_t i		= 0UZ; i < num_scratchpads; i++)
						ptrs[i]			= __scratchpad_block.plus(dev_page_size * i);
				}
				uint8_t max_slot		= get_element<0>(__cfg);
				for(uint8_t i			= 0UC; i < max_slot; i++) __slots.emplace_back(*this, i);
				on_irq(__hc_dev->header_0x0.interrupt_line, std::bind_front(&xhci_host_controller::__handle_irq, this));
				//	... WIP
				return true;
			}
			catch(...) { log("E: out of memory"); }
		}
		return false;
	}
	bool xhci_host_controller::add_segment(std::vector<trb_ring_segment>& ring) try
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
}
typedef ooos::xhci_host_controller xhci_host_controller;
EXPORT_MODULE(xhci_host_controller, ooos::api_global->find_pci_device(devclass_sb, subclass_usb, progif_xhci));