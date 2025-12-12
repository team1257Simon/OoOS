#ifndef __ARP
#define __ARP
#include <net/protocol/net_abstract.hpp>
constexpr net16 ethertype_arp	= 0x0806USBE;
constexpr net16 arp_req			= 0x0001USBE;
constexpr net16 arp_res			= 0x0002USBE;
struct __pack arpv4_packet : ethernet_header
{
	net16 htype	= htype_ethernet;
	net16 ptype	= ethertype_ipv4;
	net8 hlen	= 0x06UC;
	net8 plen	= 0x04UC;
	net16 opcode;
	mac_t src_hw;
	net32 src_pr;
	mac_t dst_hw;
	net32 dst_pr;
	arpv4_packet() noexcept;
	arpv4_packet(ethernet_header const& that) noexcept;
	arpv4_packet(ethernet_header&& that) noexcept;
	arpv4_packet(mac_t&& dst, mac_t&& src, net16 op, net32 dst_ip, net32 src_ip) noexcept;
	arpv4_packet(mac_t const& dst, mac_t const& src, net16 op, net32 dst_ip, net32 src_ip) noexcept;
};
#ifndef ARP_INST
typedef abstract_packet<arpv4_packet> arp_packet;
extern template class abstract_packet<arpv4_packet>;
extern template abstract_packet<arpv4_packet>::abstract_packet();
extern template abstract_packet<arpv4_packet>::abstract_packet(ethernet_header const&);
extern template abstract_packet<arpv4_packet>::abstract_packet(ethernet_header&&);
extern template abstract_packet<arpv4_packet>::abstract_packet(arpv4_packet const&);
extern template abstract_packet<arpv4_packet>::abstract_packet(arpv4_packet&&);
extern template abstract_packet<arpv4_packet>::abstract_packet(mac_t&&, mac_t&&, net16&&, net32&&, net32&&);
extern template abstract_packet<arpv4_packet>::abstract_packet(mac_t const&, mac_t const&, net16 const&, net32 const&, net32 const&);
extern template abstract_packet<arpv4_packet>::abstract_packet(size_t, std::in_place_type_t<arpv4_packet>);
extern template abstract_packet<arpv4_packet>::abstract_packet(size_t, std::in_place_type_t<arpv4_packet>, ethernet_header const&);
extern template abstract_packet<arpv4_packet>::abstract_packet(size_t, std::in_place_type_t<arpv4_packet>, ethernet_header&&);
extern template abstract_packet<arpv4_packet>::abstract_packet(size_t, std::in_place_type_t<arpv4_packet>, arpv4_packet const&);
extern template abstract_packet<arpv4_packet>::abstract_packet(size_t, std::in_place_type_t<arpv4_packet>, arpv4_packet&&);
extern template abstract_packet<arpv4_packet>::abstract_packet(size_t, std::in_place_type_t<arpv4_packet>, mac_t&&, mac_t&&, net16&&, net32&&, net32&&);
extern template abstract_packet<arpv4_packet>::abstract_packet(size_t, std::in_place_type_t<arpv4_packet>, mac_t const&, mac_t const&, net16 const&, net32 const&, net32 const&);
#endif
struct protocol_arp : abstract_protocol_handler, abstract_ip_resolver
{
	protocol_arp(protocol_ethernet* eth);
	virtual ~protocol_arp();
	virtual int receive(abstract_packet_base& p) override;
	virtual std::type_info const& packet_type() const override;
	virtual mac_t& resolve(ipv4_addr addr) override;
	virtual bool check_presence(ipv4_addr addr);
};
struct netdev_helper
{
	protocol_arp arp_handler;
	protocol_ethernet ethernet_handler;
	netdev_helper(mac_t const&);
	virtual int rx_transfer(netstack_buffer&) noexcept = 0;
	virtual int transmit(abstract_packet_base&) = 0;
	virtual protocol_handler& add_protocol(net16 id, protocol_handler&& ph) = 0;
	constexpr protocol_ethernet* get_ethernet_handler() { return std::addressof(ethernet_handler); }
	constexpr abstract_ip_resolver* get_ip_resolver() { return std::addressof(arp_handler); }
};
#endif