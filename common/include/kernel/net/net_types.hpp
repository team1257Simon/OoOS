#ifndef __NET_TYPES
#define __NET_TYPES
#include "libk_decls.h"
#include "array"
typedef uint8_t net8;
typedef __be16 net16;
typedef __be32 net32;
typedef __be64 net64;
typedef net32 ipv4_addr;
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
constexpr net16 htype_ethernet = 0x0001USBE;
constexpr net16 ethertype_ipv4 = 0x0800USBE;
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
    constexpr ethernet_packet(mac_t const& dest, mac_t const& src) noexcept : destination_mac(dest), source_mac(src) {}
    constexpr ethernet_packet(mac_t&& dest, mac_t&& src) noexcept : destination_mac(std::move(dest)), source_mac(std::move(src)) {}
    constexpr ethernet_packet(mac_t const& dest, mac_t const& src, net16 proto) noexcept : destination_mac(dest), source_mac(src), protocol_type(proto) {}
    constexpr ethernet_packet(mac_t&& dest, mac_t&& src, net16 proto) noexcept : destination_mac(std::move(dest)), source_mac(std::move(src)), protocol_type(proto) {}
};
#pragma GCC diagnostic push
// See the note in kernel_defs.h — this was probably more work than it was worth to write/do correctly, but I was feeling petty at the time and hadn't eaten lunch yet.
#pragma GCC diagnostic ignored "-Wliteral-suffix"
constexpr ipv4_addr operator""IPV4(const char* str, std::size_t)
{
    uint8_t values[4]{};
    for(size_t i = 0, j = 0, k = 0; j < 4; j++, i += k + 1) { for(k = 0; k < 4; k++) { if(str[i + k] == '.' || !str[i + k]) { for(size_t l = k, x = 1; l > 0; l--, x *= 10) { values[j] += static_cast<uint8_t>((str[i + l - 1] - '0') * x); } break; } } }
    return net32(values);
}
#pragma GCC diagnostic pop
constexpr ipv4_addr loopback = "127.0.0.1"IPV4;
constexpr ipv4_addr broadcast = "255.255.255.255"IPV4;
constexpr mac_t broadcast_mac = { 0xFFUC, 0xFFUC, 0xFFUC, 0xFFUC, 0xFFUC, 0xFFUC };
constexpr mac_t empty_mac = {};
#endif