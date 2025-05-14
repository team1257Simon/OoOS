#ifndef __ARP
#define __ARP
#include "net/protocol/generic_packet.hpp"
constexpr __be16 ethertype_arp = 0x0806SBE;
struct arp_ipv4_packet : ethernet_packet
{
    __be16 htype = 0x0001SBE;
    __be16 ptype = 0x0800SBE;
    uint8_t hlen = 0x06;
    uint8_t plen = 0x04;
    __be16 opcode;
    uint8_t src_hw[6];
    __be32 src_pr;
    uint8_t dst_hw[6];
    __be32 dst_pr;
};
typedef generic_packet<arp_ipv4_packet> arp_packet;
#endif