#include "xhci.hpp"
typedef std::pair<std::optional<ooos::sysfstream>, int> sysfs_result;
namespace ooos
{
	using std::ranges::distance;
	using std::views::filter;
	using std::bind_front;
	xhci_config_type xhci_host_controller::__cfg(xhci_config());
	xhci_device_endpoint xhci_device_slot::__create_ep(xhci_device_slot& slot, uint8_t i) { return xhci_device_endpoint(slot, i); }
	void xhci_host_controller::__disable_slot_cb(xhci_completion_event_trb&, xhci_device_slot& s) { s.disable(); }
	std::array<xhci_device_endpoint, 31UZ> xhci_device_slot::__create_array(xhci_device_slot& slot) { return sequence_transform(bind_front(__create_ep, slot), __ep_seq()); }
	generic_config_table& xhci_host_controller::get_config() { return __cfg.generic; }
	size_t xhci_host_controller::size() const { return __slots.size(); }
	size_t xhci_device_slot::interface_count() const { return static_cast<size_t>(distance(__endpoints | filter(bind_front(&xhci_device_endpoint::active)))); }
	abstract_connectable_device::provider* xhci_device_slot::parent() { return std::addressof(__parent); }
	void xhci_device_endpoint::ring_doorbell(uint16_t stream_id) { parent.ring_doorbell(idx, stream_id); }
	void xhci_device_endpoint::put_msg(generic_device_message const& msg) { /* TODO */ }
	std::optional<generic_device_message> xhci_device_endpoint::poll_msg() { return std::nullopt; /* TODO */ }
	abstract_connectable_device::interface::type xhci_device_endpoint::interface_type() const { return active() ? static_cast<type>(ctx.type) : type::NONE; }
	xhci_host_controller::xhci_host_controller(pci_config_space* pci) :
		__hc_dev(pci),
		__dev_ctx_array(*this),
		__input_ctx_array(*this),
		__slots(this),
		__handlers(this),
		__ctx_and_sp_ptrs(*this),
		__scratchpads(*this),
		__event_ring_segment_tables(*this),
		__port_protocols(this),
		cmd_callbacks(32UZ, this),
		transfer_callbacks(32UZ, this),
		cmd_ring(this)
	{}
	xhci_device_endpoint::xhci_device_endpoint(xhci_device_slot& slot, uint8_t idx) :
		parent(slot),
		ctx(slot.get_ep(idx)),
		streams(std::nullopt),
		transfer_trb_ring(std::nullopt),
		idx(idx)
	{}
	xhci_device_slot::xhci_device_slot(xhci_host_controller& ctl, uint8_t idx) :
		__parent(ctl),
		__ctx(ctl.get_slot(idx)),
		__input_ctx(ctl.get_input_slot(idx)),
		__doorbell(ctl.get_db(idx)),
		__port(std::ref(ctl.get_port(0UC))),	// initially just use the port at index 0; when the device is connected we will reassign
		__endpoints(__create_array(*this)),
		__idx(idx + 1UC)
	{}
	abstract_connectable_device::interface* xhci_device_slot::operator[](size_t idx)
	{
		if(__unlikely(idx >= 31UZ) || !__endpoints[idx])
			return nullptr;
		return std::addressof(__endpoints[idx]);
	}
	abstract_connectable_device* xhci_host_controller::operator[](size_t id)
	{
		if(__unlikely(id >= __slots.size()))
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
	void xhci_trb_ring::advance_enq() noexcept
	{
		++enqueue_ptr;
		if(enqueue_ptr->type == trb_type::LINK)
			cycle_state ^= reinterpret_cast<xhci_link_trb&>(*enqueue_ptr++).tc_or_ent;
		if(enqueue_ptr == ring_end())
			reset_ptr(enqueue_ptr);
	}
	void xhci_trb_ring::advance_deq() noexcept
	{
		++dequeue_ptr;
		if(dequeue_ptr->type == trb_type::LINK)
			cycle_state ^= reinterpret_cast<xhci_link_trb&>(*dequeue_ptr++).tc_or_ent;
		if(dequeue_ptr == ring_end())
			reset_ptr(dequeue_ptr);
	}
	void xhci_host_controller::finalize()
	{
		for(xhci_event_handler& h : __handlers) h.event_ring.segments.clear();
		__handlers.clear();
		__event_ring_segment_tables.destroy();
		__dev_ctx_array.destroy();
		__ctx_and_sp_ptrs.destroy();
		if(__hc_caps->scratchpad_restore)
		{
			sysfs_result p			= sysfs_open("xhci_host_controller#scratchpads");
			if(p.first.has_value()) [[likely]]
			{
				size_t size			= __scratchpads.size_bytes();
				const char* from	= __scratchpads.data();
				p.first->write(from, size);
			}
			else log("W: failed to write XHCI state for restore");
		}
		__scratchpads.destroy();
		__slots.clear();
		cmd_ring.segments.clear();
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
			__doorbells						= __hc_caps->doorbell_registers();
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
				__input_ctx_array.create(get_element<0>(__cfg), get_element<3>(__cfg));
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
					if(__hc_caps->scratchpad_restore)
					{
						sysfs_result p		= sysfs_open("xhci_host_controller#scratchpads");
						if(p.first.has_value())
							p.first->read(__scratchpads.data(), __scratchpads.size_bytes());
					}
				}
				uint8_t max_slot			= get_element<0>(__cfg);
				for(uint8_t i				= 0UC; i < max_slot; i++) __slots.emplace_back(*this, i);
				if(__unlikely(!add_segment(cmd_ring.segments))) return false;
				for(xhci_extended_capability_base* ext_cap = __hc_caps->extended_capabilities(); ext_cap; ext_cap = ext_cap->get_next())
					if(ext_cap->code == xhci_extended_capability_code::SUPPORTED_PROTOCOL)
						__port_protocols.emplace_back(addr_t(ext_cap).deref<xhci_supported_protocol>(), this);
				cmd_ring.init_ptrs();
				hc.ctx_addr_array_ptr		= addr_t(__ctx_and_sp_ptrs.data());
				hc.cmd_ring					= std::to_address(cmd_ring.dequeue_ptr);
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
		nx_link.type			= trb_type::LINK;
		if(prev_size)
		{
			xhci_link_trb& prev_link	= reinterpret_cast<xhci_link_trb&>(ring[prev_size - 1Z].back());
			prev_link.type				= trb_type::LINK;
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
		//	Event ring 0 is the primary interrupter. Certain event TRBs always target this ring, so we have to handle all those in this function.
		auto handler0	= [this](xhci_generic_trb& e) -> void
		{
			trb_type t	= e.type;
			switch(t)
			{
			case trb_type::PORT_STATUS_CHANGE_EVENT:
				__handle_port_status_change(reinterpret_cast<xhci_port_status_change_event_trb&>(e));
				break;
			case trb_type::HOST_CONTROLLER_EVENT:
				__handle_hc_event(reinterpret_cast<xhci_host_controller_event_trb&>(e));
				break;
			case trb_type::DOORBELL_EVENT:
				__handle_db_event(reinterpret_cast<xhci_doorbell_event_trb&>(e));
				break;
			case trb_type::MFINDEX_WRAP_EVENT:
				__handle_mfindex_event(reinterpret_cast<xhci_mfindex_wrap_event_trb&>(e));
				break;
			case trb_type::COMMAND_COMPLETION_EVENT:
				__handle_completion_event(reinterpret_cast<xhci_completion_event_trb&>(e));
				break;
			default:
				logf("W: event ring 0 got TRB with unrecognized code %hhi", t);
				break;
			}
		};
		//	Event ring 1 simply corresponds to an interrupter target of 1 in the TRB for a transfer or command.
		//	I've chosen to use it for all transfer events, which means any transfer TRB will use the index 1 for that field.
		auto handler1	= [this](xhci_generic_trb& e) -> void
		{
			trb_type t							= e.type;
			if(t != trb_type::TRANSFER_EVENT) logf("W: event ring 1 got TRB with unrecognized code %hhi", t);
			else
			{
				xhci_transfer_event_trb& event	= reinterpret_cast<xhci_transfer_event_trb&>(e);
				transfer_event_origin origin(event.slot_id, event.endpoint_id);
				if(transfer_callbacks.contains(origin))
					transfer_callbacks[origin](event);
				else if(event.code != completion_code::CC_SUCCESS)
					logf("E: xHC returned error code %i", event.code);	// TODO: probably more than this for error code handling
				transfer_callbacks.erase(origin);
			}
		};
		auto handler2	= [this](xhci_generic_trb& e) -> void { /* TODO: secondary interrupter 2 */ };
		auto handler3	= [this](xhci_generic_trb& e) -> void { /* TODO: secondary interrupter 3 */ };
		//	I chose to allocate 4 MSI(x) vectors for this driver.
		//	Because a significant number of event types target the same ring, giving the remaining types one interrupter each results in 4 total.
		//	The lookup for "what generated this event" will use a hashtable structure and a "std::function at home" callback scheme.
		//	Anything that expects an event to signal completion can use this to handle anything that needs to wait for the completion to occur.
		//	These callbacks are fire-and-forget, meaning they can store heap variables which will automatically be freed after the call occurs.
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
		interrupter.moderation_interval						= xhci_host_controller::imod_interval();
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
		typedef decltype(interrupter.event_ring_dequeue_base) dequeue_ptr;
		while(event_ring.dequeue_ptr->type.cycle == event_ring.cycle_state)
		{
			handler(*event_ring.dequeue_ptr);
			//	Advancing the pointer requires more work than just ++
			event_ring.advance_deq();
		}
		dequeue_ptr dp						= interrupter.event_ring_dequeue_base;
		barrier();
		dp									= std::to_address(event_ring.dequeue_ptr);
		dp.event_handler_busy				= false;
		barrier();
		interrupter.event_ring_dequeue_base	= dp;
	}
	void xhci_host_controller::__handle_port_status_change(xhci_port_status_change_event_trb& e)
	{
		uint8_t which		= e.port_id;
		if(__hc_mem->port_registers[which].status_ctl.current_connect_status)
		{
			typedef mod_mm_vec<xhci_port_protocol>::iterator proto_it;
			proto_it proto	= protocol_for(which);
			if(__unlikely(proto == __port_protocols.end())) logf("W: ignoring status change event for out of range port ID %hhi", which);
			else
			{
				xhci_enable_slot_cmd_trb* trb	= new(std::to_address(cmd_ring.enqueue_ptr)) xhci_enable_slot_cmd_trb();
				//	Most of this TRB is zeroed, so an aggregate initializer would be pretty dumb-looking.
				//	Manually setting these fields basically does the same thing anyway.
				trb->cycle						= cmd_ring.cycle_state;
				trb->type						= trb_type::ENABLE_SLOT_CMD;
				trb->slot_type					= proto->protocol_slot_type;
				cmd_ring.advance_enq();
				//	This callback actually sets off a rather lengthy chain of async commands and callbacks.
				//	Every step from here forward that might return before completing (and thus use an async callback) has its own function.
				cmd_callbacks.insert(std::make_pair(trb, std::bind(&xhci_host_controller::__enable_slot_cb, this, std::placeholders::_1, which)));
				__doorbells[0]					= xhci_doorbell();
			}
		}
		else
		{
			typedef mod_mm_vec<xhci_device_slot>::iterator slot_it;
			xhci_hc_port const& port	= __hc_mem->port_registers[which];
			slot_it slot				= std::ranges::find_if(__slots, [&port](xhci_device_slot const& s) -> bool { return s.is_on_port(port); });
			if(__unlikely(slot == __slots.end())) logf("W: got detach event for port ID %hhi which is not tied to a device", which);
			else
			{
				xhci_disable_slot_cmd_trb* trb	= new(std::to_address(cmd_ring.enqueue_ptr)) xhci_disable_slot_cmd_trb();
				trb->cycle				= cmd_ring.cycle_state;
				trb->type				= trb_type::DISABLE_SLOT_CMD;
				trb->slot_id			= slot->id();
				cmd_ring.advance_enq();
				cmd_callbacks.insert(std::make_pair(trb, std::bind(&xhci_host_controller::__disable_slot_cb, this, std::placeholders::_1, *slot)));
				__doorbells[0]			= xhci_doorbell();
			}
		}
	}
	void xhci_host_controller::__handle_hc_event(xhci_host_controller_event_trb& e) {}
	void xhci_host_controller::__handle_db_event(xhci_doorbell_event_trb& e) {}
	void xhci_host_controller::__handle_mfindex_event(xhci_mfindex_wrap_event_trb& e) {}
	void xhci_host_controller::__handle_completion_event(xhci_completion_event_trb& e)
	{
		addr_t trb_ptr	= e.data_ptr;
		if(cmd_callbacks.contains(trb_ptr))
			cmd_callbacks[trb_ptr](e);
		else if(e.code != completion_code::CC_SUCCESS)
			logf("E: xHC returned error code %hhi", e.code);	// TODO: probably more than this for error code handling
		cmd_callbacks.erase(trb_ptr);
	}
	void xhci_host_controller::__enable_slot_cb(xhci_completion_event_trb& e, uint8_t port_id)
	{
		if(e.code == completion_code::CC_SUCCESS && e.slot_id)
		{
			uint8_t slot_id					= static_cast<uint8_t>(e.slot_id - 1UC);
			if(__unlikely(slot_id >= __slots.size()))
				logf("E: slot id %hhi is out of range for count of %hhi", slot_id, static_cast<uint8_t>(__slots.size()));
			else
			{
				xhci_device_slot& slot		= __slots[slot_id];
				__ctx_and_sp_ptrs[slot_id]	= std::construct_at(std::addressof(__dev_ctx_array[slot_id]));
				std::construct_at(std::addressof(__input_ctx_array[slot_id]));
				slot.enable(port_id);
			}
		}
		else logf("E: xHC returned error code %hhi", e.code);	// TODO: probably more than this for error code handling
	}
	void xhci_device_slot::__discover_endpoints() { /** TODO */ }
	void xhci_device_slot::__pre_discover_endpoints(uint16_t ep0_default_max_pkt)
	{
		//	If the value is already set, we don't need to change anything, so go right to the next step
		if(__ctx[0].max_packet_size == ep0_default_max_pkt) __discover_endpoints();
		else
		{
			__input_ctx[0].max_packet_size		= ep0_default_max_pkt;
			__input_ctx.add_flags[1]			= true;
			xhci_evaluate_context_cmd_trb* trb	= new(std::to_address(__parent.cmd_ring.enqueue_ptr))
			xhci_evaluate_context_cmd_trb
			{
				{ .data_ptr						= trb_data_ptr::of(std::addressof(__input_ctx)) },
				{},
				{
					.cycle						= __parent.cmd_ring.cycle_state,
					.interrupt_on_completion	= true,
					.type						= trb_type::EVALUATE_CONTEXT_CMD
				},
				{ .slot_id						= __idx }
			};
			__parent.cmd_ring.advance_enq();
			__parent.cmd_callbacks.insert(std::make_pair(trb, [this](xhci_completion_event_trb&) -> void { __discover_endpoints(); }));
			__parent.hc_db()					= xhci_doorbell();
		}
	}
	void xhci_device_slot::__pre_discover_endpoints_cb(usb_device_descriptor const& desc, xhci_transfer_event_trb& e)
	{
		if(e.code == completion_code::CC_SUCCESS)
			return __pre_discover_endpoints(desc.ep0_max_packet_size);
		__parent.logf("E: xHC returned error code %hhi", e.code);	// TODO: probably more than this for error code handling
		__endpoints[0].transfer_trb_ring.reset();
	}
	void xhci_device_slot::__address_device_cb(xhci_completion_event_trb& e, uint64_t dev_bitrate)
	{
		constexpr uint64_t br_highspeed	= 480000000UL;
		constexpr uint64_t br_lowspeed	= 1500000UL;
		if(dev_bitrate < br_highspeed && dev_bitrate > br_lowspeed)
		{
			transfer_event_origin origin(__idx, 0UC);
			xhci_trb_ring& transfer = *__endpoints[0].transfer_trb_ring;
			bool cycle_bit			= transfer.cycle_state;
			auto fn					= bind_for_message(std::in_place_type<usb_device_descriptor>, &xhci_device_slot::__pre_discover_endpoints_cb, this);
			new(std::to_address(transfer.enqueue_ptr)) xhci_setup_trb
			{
				{
					.direction					= usb_transfer_direction::D2H,
					.request_type				= usb_request_type::STANDARD,
					.recipient					= usb_request_target::DEVICE,
					.request_code				= usb_setup_request_code::GET_DESCRIPTOR,
					.value_field				= { .descriptor_request { .desc_type { usb_descriptor_type::DDT_DEVICE } } },
					.length						= 8UC,
				},
				{
					.trb_transfer_length		= 8UC,
					.tds_or_tbc					= 0UC,	// this is the only TRB in this TD
					.interrupter_target			= 1US,	// transfer events go to ring 1
				},
				{
					.cycle						= cycle_bit,
					.is_immediate				= true,
					.type						= trb_type::SETUP
				},
				{ .transfer_type				= control_transfer_type::IN_DATA_STAGE }
			};
			transfer.advance_enq();
			cycle_bit							= transfer.cycle_state;
			new(std::to_address(transfer.enqueue_ptr)) xhci_data_trb
			{
				{ .data_ptr						= trb_data_ptr::of(fn.base()) },
				{
					.trb_transfer_length		= 8UC,
					.tds_or_tbc					= 0UC,
					.interrupter_target			= 1US,
				},
				{
					.cycle						= cycle_bit,
					.type						= trb_type::DATA
				},
				{ .data_direction				= true }
			};
			transfer.advance_enq();
			cycle_bit							= transfer.cycle_state;
			new(std::to_address(transfer.enqueue_ptr)) xhci_status_trb
			{
				{},
				{ .interrupter_target			= 1US },
				{
					.cycle						= cycle_bit,
					.interrupt_on_completion	= true,
					.type						= trb_type::STATUS,
				},
				{ .data_direction				= false }
			};
			transfer.advance_enq();
			__parent.transfer_callbacks.emplace(std::piecewise_construct, std::make_tuple(origin), std::forward_as_tuple(std::move(fn)));
			__doorbell							= xhci_doorbell(1UC, 0UC, 0US);
		}
		//	All roads lead to Rome, but for full-speed devices there's extra work we can skip if the device is low-, high-, or super-speed
		else __pre_discover_endpoints(dev_bitrate > br_highspeed ? 512US : dev_bitrate > br_lowspeed ? 64US : 8US);
	}
	void xhci_device_slot::enable(uint8_t port_idx)
	{
		typedef mod_mm_vec<xhci_port_protocol>::iterator proto_it;
		typedef mod_mm_vec<xhci_protocol_speed_id>::iterator sid_it;
		using enum trb_type;
		proto_it proto	= __parent.protocol_for(port_idx);
		if(__unlikely(!__parent.check(proto)))
			__parent.logf("E: port id %hhi has no associated protocol", port_idx);
		else
		{
			uint8_t psid		= __parent.get_port(port_idx).get().status_ctl.speed_value_id;
			sid_it sid			= std::ranges::find_if(proto->speed_ids, [=](xhci_protocol_speed_id const& i) -> bool { return i.id_value == psid; });
			if(__unlikely(sid == proto->speed_ids.end()))
				__parent.logf("E: port id %hhi has unrecognized speed id %hhi", port_idx, psid);
			else
			{
				//	TODO: figure out route string and root hub port number values
				__endpoints[0].transfer_trb_ring.emplace(__parent.create_ring()).set_cycle_bit(true);
				assign_port(port_idx);
				uint64_t rate	= sid->calc_bitrate();
				bool high_speed	= rate >= 480000000UL;
				xhci_endpoint_context* ep0	= new(std::addressof(__input_ctx[0])) xhci_endpoint_context
				{
					.error_limit			= 3UC,
					.type					= endpoint_type::EPT_CONTROL,
					.max_packet_size		= high_speed ? 64US : 8US,
					.dequeue_ptr			= __endpoints[0].transfer_trb_ring->dequeue_ptr
				};
				ep0->dequeue_ptr.dequeue_cycle_state	= true;
				bool ccycle								= __parent.cmd_ring.cycle_state;
				void* trb								= new(std::to_address(__parent.cmd_ring.enqueue_ptr++)) xhci_address_device_cmd_trb
				{
					{ trb_data_ptr::of(std::addressof(__input_ctx)) },
					{},
					{ .cycle		= ccycle, .type	= ADDRESS_DEVICE_CMD, },
					{ .slot_id		= __idx }
				};
				if(__parent.cmd_ring.enqueue_ptr == __parent.cmd_ring.ring_end()) __parent.cmd_ring.on_loop();
				//	The bitrate information is still important in the next step, which is an async call.
				//	Fortunately, we can use std::bind to "remember" that info and use it as a parameter to said async call without any stack problems.
				//	Note that the function we're in will return before the device is fully usable, but it will itself also be called asynchronously.
				__parent.cmd_callbacks.insert(std::make_pair(trb, std::bind(&xhci_device_slot::__address_device_cb, this, std::placeholders::_1, rate)));
				__parent.hc_db()	= xhci_doorbell();
			}
		}
	}
	void xhci_device_slot::disable()
	{
		for(xhci_device_endpoint& e : std::views::filter(__endpoints, std::bind_front(&xhci_device_endpoint::active)))
		{
			// WIP; might need to do more cleanup here
			e.streams.reset();
			e.transfer_trb_ring.reset();
		}
	}
}
typedef ooos::xhci_host_controller xhci_host_controller;
EXPORT_MODULE(xhci_host_controller, ooos::api_global->find_pci_device(devclass_sb, subclass_usb, progif_xhci));