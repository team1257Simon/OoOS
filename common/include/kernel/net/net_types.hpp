#ifndef __NET_TYPES
#define __NET_TYPES
#include "libk_decls.h"
#include "array"
typedef uint8_t net8;
typedef __be16 net16;
typedef __be32 net32;
typedef __be64 net64;
typedef std::array<net8, 6> mac_t;
enum ipv4_transport_protocol : net8
{
    ICMP    = 1,
    IGMP    = 2,
    TCP     = 6,
    UDP     = 17,
    ENCAP   = 41,
    OSPF    = 89,
    SCTP    = 132,
};
constexpr net16 htype_ethernet = 0x0001SBE;
constexpr net16 ethertype_ipv4 = 0x0800SBE;
struct attribute(packed) ethernet_packet
{
    mac_t destination_mac;
    mac_t source_mac;
    net16 protocol_type;
    constexpr ethernet_packet() noexcept = default;
    constexpr ethernet_packet(ethernet_packet const&) noexcept = default;
    constexpr ethernet_packet(ethernet_packet&&) noexcept = default;
    constexpr ethernet_packet& operator=(ethernet_packet const&) noexcept = default;
    constexpr ethernet_packet& operator=(ethernet_packet&&) noexcept = default;
    constexpr ethernet_packet(mac_t&& dest, mac_t&& src, net16 proto) noexcept : destination_mac(std::move(dest)), source_mac(std::move(src)), protocol_type(proto) {}
};
#endif