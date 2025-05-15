#ifndef __DHCP
#define __DHCP
#include "net/protocol/udp.hpp"
constexpr net32 dhcp_magic = 0x63825363UBE;
struct attribute(packed) dhcp_parameter
{
    uint8_t type_code; // the value 0xFF means end-of-transmission; the following fields do not exist in such a case (i.e. the open-coded array terminates early)
    uint8_t length;
    uint8_t data[];
};
struct attribute(packed) dhcp_packet : udp_packet_base
{
    net8 operation;
    net8 hw_type    = 1UC;
    net8 hw_len     = 0x6UC;
    net8 hops       = 0;
    net32 transaction_id;
    net16 seconds;
    net16 flags     = 0SBE;
    net32 client_ip;
    net32 your_ip;
    net32 server_ip;
    net32 relay_ip;
    net32 client_hw[4];
    char server_name_optional[64];
    char boot_file_name[128];
    net32 magic     = dhcp_magic;
    net32 magic     = dhcp_magic;
    dhcp_parameter parameters[];
    constexpr dhcp_packet() noexcept = default;
    constexpr dhcp_packet(udp_packet_base const& that) noexcept : udp_packet_base(that) {}
    constexpr dhcp_packet(udp_packet_base&& that) noexcept : udp_packet_base(std::move(that)) {}
};
constexpr size_t total_dhcp_size(size_t parameters_size) noexcept { return parameters_size + sizeof(dhcp_packet); }
#endif