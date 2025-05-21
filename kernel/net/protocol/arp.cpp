#define ARP_INST
#include "net/protocol/arp.hpp"
#include "sys/errno.h"
template class abstract_packet<arpv4_packet>;
template abstract_packet<arpv4_packet>::abstract_packet();
template abstract_packet<arpv4_packet>::abstract_packet(ethernet_packet const&);
template abstract_packet<arpv4_packet>::abstract_packet(ethernet_packet&&);
template abstract_packet<arpv4_packet>::abstract_packet(arpv4_packet const&);
template abstract_packet<arpv4_packet>::abstract_packet(arpv4_packet&&);
template abstract_packet<arpv4_packet>::abstract_packet(mac_t&&, mac_t&&, net16&&, net32&&, net32&&);
template abstract_packet<arpv4_packet>::abstract_packet(mac_t const&, mac_t const&, net16 const&, net32 const&, net32 const&);
template abstract_packet<arpv4_packet>::abstract_packet(size_t, std::in_place_type_t<arpv4_packet>);
template abstract_packet<arpv4_packet>::abstract_packet(size_t, std::in_place_type_t<arpv4_packet>, ethernet_packet const&);
template abstract_packet<arpv4_packet>::abstract_packet(size_t, std::in_place_type_t<arpv4_packet>, ethernet_packet&&);
template abstract_packet<arpv4_packet>::abstract_packet(size_t, std::in_place_type_t<arpv4_packet>, arpv4_packet const&);
template abstract_packet<arpv4_packet>::abstract_packet(size_t, std::in_place_type_t<arpv4_packet>, arpv4_packet&&);
template abstract_packet<arpv4_packet>::abstract_packet(size_t, std::in_place_type_t<arpv4_packet>, mac_t&&, mac_t&&, net16&&, net32&&, net32&&);
template abstract_packet<arpv4_packet>::abstract_packet(size_t, std::in_place_type_t<arpv4_packet>, mac_t const&, mac_t const&, net16 const&, net32 const&, net32 const&);
arpv4_packet::arpv4_packet() noexcept = default;
arpv4_packet::arpv4_packet(ethernet_packet const& that) noexcept : ethernet_packet(that) { protocol_type = ethertype_arp; array_copy(src_hw, source_mac); array_copy(dst_hw, destination_mac); }
arpv4_packet::arpv4_packet(ethernet_packet&& that) noexcept : ethernet_packet(std::move(that)) { protocol_type = ethertype_arp; array_copy(src_hw, source_mac); array_copy(dst_hw, destination_mac); }
arpv4_packet::arpv4_packet(mac_t&& dst, mac_t&& src, net16 op, net32 dst_ip, net32 src_ip) noexcept : ethernet_packet(std::move(dst), std::move(src), ethertype_arp), opcode(op), src_hw(std::move(src)), src_pr(src_ip), dst_hw(std::move(dst)), dst_pr(dst_ip) {}
arpv4_packet::arpv4_packet(mac_t const &dst, mac_t const &src, net16 op, net32 dst_ip, net32 src_ip) noexcept : ethernet_packet(dst, src, ethertype_arp), opcode(op), src_hw(src), src_pr(src_ip), dst_hw(dst), dst_pr(dst_ip) {}
protocol_arp::~protocol_arp() = default;
std::type_info const& protocol_arp::packet_type() const { return typeid(arpv4_packet); }
protocol_arp::protocol_arp(protocol_ethernet* eth) : abstract_protocol_handler(eth, eth), abstract_ip_resolver() {}
int protocol_arp::receive(abstract_packet_base& p)
{
    arpv4_packet* pkt = p.get_as<arpv4_packet>();
    if(!pkt) return -EPROTOTYPE;
    previously_resolved.insert_or_assign(pkt->src_pr, std::move(pkt->src_hw));
    return 0;
}
mac_t protocol_arp::resolve(ipv4_addr addr)
{
    ethernet_packet base_result = base->create_packet(empty_mac);
    abstract_packet<arpv4_packet> pkt(std::move(base_result));
    pkt->src_hw = base->mac_addr;
    pkt->dst_pr = addr;
    pkt->opcode = arp_req;
    if(base->primary_ip) pkt->src_pr = base->primary_ip;
    if(base->transmit(pkt) != 0) throw std::runtime_error{ "arp: packet transmission failed" };
    if(!await_result([&]() -> bool { return previously_resolved.contains(addr); })) throw std::runtime_error{ "arp: no reply" };
    return previously_resolved[addr];
}