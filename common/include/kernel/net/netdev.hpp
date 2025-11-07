#ifndef __NETDEV
#define __NETDEV
#include "net/protocol/arp.hpp"
#include "bits/stl_queue.hpp"
class net_device
{
	friend void net_tests();
protected:
	mac_t mac_addr;
	std::ext::resettable_queue<netstack_buffer> transfer_buffers;
	protocol_arp arp_handler;
	protocol_ethernet base_handler;
	int rx_transfer(netstack_buffer&) noexcept;
	constexpr void* tx_base(std::ext::resettable_queue<netstack_buffer>::iterator i) { return i->pbase(); }
	constexpr void* rx_base(std::ext::resettable_queue<netstack_buffer>::iterator i) { return i->eback(); }
public:
	net_device();
	virtual ~net_device();
	virtual bool initialize()                   = 0;
	virtual void enable_transmit()              = 0;
	virtual void enable_receive()               = 0;
	virtual void disable_transmit()             = 0;
	virtual void disable_receive()              = 0;
	virtual int poll_rx()                       = 0;
	virtual int poll_tx(netstack_buffer& buff)  = 0;
	virtual int transmit(abstract_packet_base& p);
	protocol_handler& add_protocol(net16 id, protocol_handler&& ph);
	constexpr protocol_ethernet* get_ethernet_handler() { return std::addressof(base_handler); }
	constexpr mac_t const& get_mac_addr() const { return mac_addr; }
	template<std::derived_from<abstract_protocol_handler> PT> requires std::constructible_from<PT, protocol_ethernet*> PT& add_protocol_handler(net16 id) { return add_protocol(id, std::move(create_handler<PT>(std::addressof(base_handler)))).template cast<PT>(); }
	// ...
};
#endif