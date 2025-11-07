#define ARP_INST
#include "net/protocol/arp.hpp"
#include "sys/errno.h"
#include "arch/hpet_amd64.hpp"
template class abstract_packet<arpv4_packet>;
template abstract_packet<arpv4_packet>::abstract_packet();
template abstract_packet<arpv4_packet>::abstract_packet(ethernet_header const&);
template abstract_packet<arpv4_packet>::abstract_packet(ethernet_header&&);
template abstract_packet<arpv4_packet>::abstract_packet(arpv4_packet const&);
template abstract_packet<arpv4_packet>::abstract_packet(arpv4_packet&&);
template abstract_packet<arpv4_packet>::abstract_packet(mac_t&&, mac_t&&, net16&&, net32&&, net32&&);
template abstract_packet<arpv4_packet>::abstract_packet(mac_t const&, mac_t const&, net16 const&, net32 const&, net32 const&);
template abstract_packet<arpv4_packet>::abstract_packet(size_t, std::in_place_type_t<arpv4_packet>);
template abstract_packet<arpv4_packet>::abstract_packet(size_t, std::in_place_type_t<arpv4_packet>, ethernet_header const&);
template abstract_packet<arpv4_packet>::abstract_packet(size_t, std::in_place_type_t<arpv4_packet>, ethernet_header&&);
template abstract_packet<arpv4_packet>::abstract_packet(size_t, std::in_place_type_t<arpv4_packet>, arpv4_packet const&);
template abstract_packet<arpv4_packet>::abstract_packet(size_t, std::in_place_type_t<arpv4_packet>, arpv4_packet&&);
template abstract_packet<arpv4_packet>::abstract_packet(size_t, std::in_place_type_t<arpv4_packet>, mac_t&&, mac_t&&, net16&&, net32&&, net32&&);
template abstract_packet<arpv4_packet>::abstract_packet(size_t, std::in_place_type_t<arpv4_packet>, mac_t const&, mac_t const&, net16 const&, net32 const&, net32 const&);
arpv4_packet::arpv4_packet() noexcept = default;
arpv4_packet::arpv4_packet(ethernet_header const& that) noexcept : ethernet_header(that) { protocol_type = ethertype_arp; array_copy(src_hw, source_mac); array_copy(dst_hw, destination_mac); }
arpv4_packet::arpv4_packet(ethernet_header&& that) noexcept : ethernet_header(std::move(that)) { protocol_type = ethertype_arp; array_copy(src_hw, source_mac); array_copy(dst_hw, destination_mac); }
arpv4_packet::arpv4_packet(mac_t&& dst, mac_t&& src, net16 op, net32 dst_ip, net32 src_ip) noexcept : ethernet_header(std::move(dst), std::move(src), ethertype_arp), opcode(op), src_hw(std::move(src)), src_pr(src_ip), dst_hw(std::move(dst)), dst_pr(dst_ip) {}
arpv4_packet::arpv4_packet(mac_t const &dst, mac_t const &src, net16 op, net32 dst_ip, net32 src_ip) noexcept : ethernet_header(dst, src, ethertype_arp), opcode(op), src_hw(src), src_pr(src_ip), dst_hw(dst), dst_pr(dst_ip) {}
protocol_arp::~protocol_arp() = default;
std::type_info const& protocol_arp::packet_type() const { return typeid(arpv4_packet); }
protocol_arp::protocol_arp(protocol_ethernet* eth) : abstract_protocol_handler(eth, eth), abstract_ip_resolver() {}
int protocol_arp::receive(abstract_packet_base& p)
{
	arpv4_packet* pkt = p.get_as<arpv4_packet>();
	if(__unlikely(!pkt)) return -EPROTOTYPE;
	if(pkt->src_pr) previously_resolved.insert_or_assign(pkt->src_pr, std::move(pkt->src_hw));
	if(pkt->opcode == arp_res) return 0;
	if(__unlikely(pkt->opcode != arp_req)) return -EPROTO;
	if(__unlikely(!base->ipv4_client_config->leased_addr) || __builtin_memcmp(base->mac_addr.data(), pkt->dst_hw.data(), 6UZ) != 0) return 0;
	ethernet_header hdr = base->create_packet(pkt->src_hw);
	abstract_packet<arpv4_packet> response(std::move(hdr));
	pkt->src_pr = base->ipv4_client_config->leased_addr;
	pkt->dst_pr = pkt->src_pr;
	pkt->opcode = arp_res;
	return next->transmit(response);
}
mac_t& protocol_arp::resolve(ipv4_addr addr)
{
	if(!await_result([&]() -> bool { return check_presence(addr); }))
		throw std::runtime_error("[ARP] could not resolve IP " + stringify(addr) + " because its owner did not respond");
	return previously_resolved[addr];
}
bool protocol_arp::check_presence(ipv4_addr addr)
{
	ethernet_header hdr = base->create_packet(empty_mac);
	abstract_packet<arpv4_packet> pkt(std::move(hdr));
	pkt->dst_pr         = addr;
	pkt->opcode         = arp_req;
	if(base->ipv4_client_config->current_state == ipv4_client_state::BOUND) { pkt->src_pr = base->ipv4_client_config->leased_addr; }
	else pkt->src_pr    = 0UBE;
	if(base->transmit(pkt) != 0) throw std::runtime_error("[ARP] packet transmission failed");
	hpet.delay_us(100UL);
	return previously_resolved.contains(addr);
}