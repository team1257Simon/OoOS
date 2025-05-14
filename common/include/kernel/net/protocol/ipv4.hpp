#ifndef __IPV4
#define __IPV4
#include "net/protocol/generic_packet.hpp"
enum ecn_t : net8
{
    NON_ECT = 0b00UC, // not ECN-capable
    ECN_1   = 0b01UC, // ECN capable, flag 1
    ECN_0   = 0b10UC, // ECN capable, flag 0
    ECN_CE  = 0b11UC  // Congestion Experienced
};
enum dscp_t : net8
{
    DS_STANDARD             = 0b00000UC,
    DS_LOWER_EFFORT         = 0b00001UC,
    DS_HIGH_THROUGHPUT_AF11 = 0b01010UC,
    DS_HIGH_THROUGHPUT_AF12 = 0b01100UC,
    DS_HIGH_THROUGHPUT_AF13 = 0b01110UC,
    DS_OAM                  = 0b010000UC,
    DS_LOW_LATENCY_AF21     = 0b010010UC,
    DS_LOW_LATENCY_AF22     = 0b010100UC,
    DS_LOW_LATENCY_AF23     = 0b010110UC,
    DS_BROADCAST_VIDEO      = 0b011000UC,
    DS_STREAMING_AF31       = 0b011010UC,
    DS_STREAMING_AF32       = 0b011100UC,
    DS_STREAMING_AF33       = 0b011110UC,
    DS_REALTIME_INTERACTIVE = 0b100000UC,
    DS_CONFERENCING_AF41    = 0b100010UC,
    DS_CONFERENCING_AF42    = 0b100100UC,
    DS_CONFERENCING_AF43    = 0b100110UC,
    DS_SIGNALING            = 0b101000UC,
    DS_TELEPHONE            = 0b101110UC,
    DS_NETWORK_CTL          = 0b110000UC
};
enum ipver_t : net8
{
    IPV4 = 0x4,
    // no others are valid here
};
enum ihl_t : net8
{
    IHL20B = 0x5,
    IHL24B = 0x6,
    IHL28B = 0x7,
    IHL32B = 0x8,
    IHL36B = 0x9,
    IHL40B = 0xA,
    IHL44B = 0xB,
    IHL48B = 0xC,
    IHL52B = 0xD,
    IHL56B = 0xE,
    IHL60B = 0xF
};
constexpr inline net16 fragment_offset_mask = 0x1FFFSBE;
constexpr inline net16 no_fragment_bit = 0x2000SBE;
constexpr inline net16 more_fragments_bit = 0x4000SBE;
template<ihl_t L>
struct attribute(packed) ipv4_packet_with_options : ethernet_packet
{
    struct attribute(packed, aligned(1))
    {
        ihl_t ihl       : 4 = L;                // value is header length in dwords; almost always 20 bytes or 5 dwords
        ipver_t version : 4 = IPV4;
        dscp_t dscp     : 6 = DS_STANDARD;      // differentiated services codepoint
        ecn_t ecn       : 2 = NON_ECT;          // explicit congestion notification
    };
    net16 total_length;                         // length of the whole packet
    net16 identification;
    net16 fragment_info;                        // 0b0DMXXXXXXXXXXXXX; D = don't fragment, M = more fragments remain, X... = fragment offset
    net8 time_to_live;
    ipv4_transport_protocol protocol;
    net16 header_checksum;
    net32 source_addr;
    net32 destination_addr;                     // intended recipient address; the destination MAC address will be for the gateway if this is not on the local network
    net32 options[static_cast<uint8_t>(L) - 0x5UC];
    constexpr ipv4_packet_with_options() noexcept = default;
    constexpr ipv4_packet_with_options(ethernet_packet const& ep) noexcept : ethernet_packet(ep) { protocol_type = ethertype_ipv4; }
    constexpr ipv4_packet_with_options(ethernet_packet&& ep) noexcept : ethernet_packet(std::move(ep)) { protocol_type = ethertype_ipv4; }
};
typedef ipv4_packet_with_options<IHL20B> ipv4_standard_packet;
#endif