#ifndef __ARP
#define __ARP
#include "net/protocol/generic_packet.hpp"
constexpr net16 ethertype_arp = 0x0806SBE;
constexpr net16 arp_req = 0x0001SBE;
constexpr net16 arp_res = 0x0002SBE;
struct attribute(packed) arp_ipv4_packet : ethernet_packet
{
    net16 htype = htype_ethernet;
    net16 ptype = ethertype_ipv4;
    uint8_t hlen = 0x06;
    uint8_t plen = 0x04;
    net16 opcode;
    mac_t src_hw;
    net32 src_pr;
    mac_t dst_hw;
    net32 dst_pr;
    constexpr arp_ipv4_packet() noexcept = default;
    constexpr arp_ipv4_packet(ethernet_packet const& that) noexcept : ethernet_packet(that) { protocol_type = ethertype_arp; array_copy(src_hw, source_mac); array_copy(dst_hw, destination_mac); }
    constexpr arp_ipv4_packet(ethernet_packet&& that) noexcept : ethernet_packet(std::move(that)) { protocol_type = ethertype_arp; array_copy(src_hw, source_mac); array_copy(dst_hw, destination_mac); }
    constexpr arp_ipv4_packet(mac_t&& dst, mac_t&& src, net16 op, net32 dst_ip, net32 src_ip) noexcept : ethernet_packet(std::move(dst), std::move(src), ethertype_arp), opcode(op), src_hw(std::move(src)), src_pr(src_ip), dst_hw(std::move(dst)), dst_pr(dst_ip) {}
};
typedef generic_packet<arp_ipv4_packet> arp_packet;
#endif