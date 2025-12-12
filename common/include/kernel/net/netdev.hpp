#ifndef __NETDEV
#define __NETDEV
#include <net/protocol/arp.hpp>
class net_device : public netdev_helper, public abstract_netdev
{
	friend class netstack_buffer;
	friend void net_tests();
protected:
	std::ext::resettable_queue<netstack_buffer> transfer_buffers;
	virtual int rx_transfer(netstack_buffer&) noexcept final;
	constexpr void* tx_base(std::ext::resettable_queue<netstack_buffer>::iterator i) { return i->pbase(); }
	constexpr void* rx_base(std::ext::resettable_queue<netstack_buffer>::iterator i) { return i->eback(); }
public:
	net_device();
	virtual int transmit(abstract_packet_base& p) final;
	virtual ~net_device();
	bool initialize();
	virtual protocol_handler& add_protocol(net16 id, protocol_handler&& ph) final;
	template<std::derived_from<abstract_protocol_handler> PT> requires(std::constructible_from<PT, protocol_ethernet*>)
	inline PT& add_protocol_handler(net16 id) { return add_protocol(id, std::move(create_handler<PT>(std::addressof(ethernet_handler)))).template cast<PT>(); }
	// ...
};
#endif