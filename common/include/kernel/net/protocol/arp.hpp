#ifndef __ARP
#define __ARP
#include "net/protocol/generic_packet.hpp"
constexpr net16 ethertype_arp   = 0x0806USBE;
constexpr net16 arp_req         = 0x0001USBE;
constexpr net16 arp_res         = 0x0002USBE;
struct attribute(packed) arpv4_packet : ethernet_packet
{
    net16 htype = htype_ethernet;
    net16 ptype = ethertype_ipv4;
    net8 hlen   = 0x06UC;
    net8 plen   = 0x04UC;
    net16 opcode;
    mac_t src_hw;
    net32 src_pr;
    mac_t dst_hw;
    net32 dst_pr;
    constexpr arpv4_packet() noexcept = default;
    constexpr arpv4_packet(ethernet_packet const& that) noexcept : ethernet_packet(that) { protocol_type = ethertype_arp; array_copy(src_hw, source_mac); array_copy(dst_hw, destination_mac); }
    constexpr arpv4_packet(ethernet_packet&& that) noexcept : ethernet_packet(std::move(that)) { protocol_type = ethertype_arp; array_copy(src_hw, source_mac); array_copy(dst_hw, destination_mac); }
    constexpr arpv4_packet(mac_t&& dst, mac_t&& src, net16 op, net32 dst_ip, net32 src_ip) noexcept : ethernet_packet(std::move(dst), std::move(src), ethertype_arp), opcode(op), src_hw(std::move(src)), src_pr(src_ip), dst_hw(std::move(dst)), dst_pr(dst_ip) {}
    constexpr arpv4_packet(mac_t const& dst, mac_t const& src, net16 op, net32 dst_ip, net32 src_ip) noexcept : ethernet_packet(dst, src, ethertype_arp), opcode(op), src_hw(src), src_pr(src_ip), dst_hw(dst), dst_pr(dst_ip) {}
};
#ifndef ARP_INST
typedef generic_packet<arpv4_packet> arp_packet;
extern template class generic_packet<arpv4_packet>;
extern template generic_packet<arpv4_packet>::generic_packet();
extern template generic_packet<arpv4_packet>::generic_packet(ethernet_packet const&);
extern template generic_packet<arpv4_packet>::generic_packet(ethernet_packet&&);
extern template generic_packet<arpv4_packet>::generic_packet(arpv4_packet const&);
extern template generic_packet<arpv4_packet>::generic_packet(arpv4_packet&&);
extern template generic_packet<arpv4_packet>::generic_packet(mac_t&&, mac_t&&, net16&&, net32&&, net32&&);
extern template generic_packet<arpv4_packet>::generic_packet(mac_t const&, mac_t const&, net16 const&, net32 const&, net32 const&);
extern template generic_packet<arpv4_packet>::generic_packet(size_t, std::in_place_type_t<arpv4_packet>);
extern template generic_packet<arpv4_packet>::generic_packet(size_t, std::in_place_type_t<arpv4_packet>, ethernet_packet const&);
extern template generic_packet<arpv4_packet>::generic_packet(size_t, std::in_place_type_t<arpv4_packet>, ethernet_packet&&);
extern template generic_packet<arpv4_packet>::generic_packet(size_t, std::in_place_type_t<arpv4_packet>, arpv4_packet const&);
extern template generic_packet<arpv4_packet>::generic_packet(size_t, std::in_place_type_t<arpv4_packet>, arpv4_packet&&);
extern template generic_packet<arpv4_packet>::generic_packet(size_t, std::in_place_type_t<arpv4_packet>, mac_t&&, mac_t&&, net16&&, net32&&, net32&&);
extern template generic_packet<arpv4_packet>::generic_packet(size_t, std::in_place_type_t<arpv4_packet>, mac_t const&, mac_t const&, net16 const&, net32 const&, net32 const&);
#endif
#endif