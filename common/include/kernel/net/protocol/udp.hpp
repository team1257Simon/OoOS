#ifndef __UDP
#define __UDP
#include "net/protocol/ipv4.hpp"
struct attribute(packed) udp_packet_base : ipv4_standard_packet
{
    net16 source_port;
    net16 destination_port;
    net16 udp_length;
    net16 udp_csum;
    constexpr udp_packet_base() noexcept = default;
    constexpr udp_packet_base(ipv4_standard_packet const& that) noexcept : ipv4_standard_packet(that) { protocol = UDP; }
    constexpr udp_packet_base(ipv4_standard_packet&& that) noexcept : ipv4_standard_packet(std::move(that)) { protocol = UDP; }
    void compute_udp_csum();
};
#endif