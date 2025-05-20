#define DHCP_INST
#include "net/protocol/dhcp.hpp"
#include "stdlib.h" // rand()
constexpr static std::allocator<net16> udp_pseudo_alloc;
template<> template<> generic_packet<dhcp_packet_base>::generic_packet(udp_packet_base&& that) : generic_packet(static_cast<size_t>(that.total_length), std::in_place_type<dhcp_packet_base>, std::move(that)) {}
template<> template<> generic_packet<dhcp_packet_base>::generic_packet(udp_packet_base const& that) : generic_packet(static_cast<size_t>(that.total_length), std::in_place_type<dhcp_packet_base>, that) {}
template<> template<> generic_packet<dhcp_packet_base>::generic_packet(dhcp_packet_base&& that) : generic_packet(static_cast<size_t>(that.total_length), std::in_place_type<dhcp_packet_base>, std::move(that)) {}
template<> template<> generic_packet<dhcp_packet_base>::generic_packet(dhcp_packet_base const& that) : generic_packet(static_cast<size_t>(that.total_length), std::in_place_type<dhcp_packet_base>, that) {}
template class generic_packet<dhcp_packet_base>;
template generic_packet<dhcp_packet_base>::generic_packet(udp_packet_base&&);
template generic_packet<dhcp_packet_base>::generic_packet(udp_packet_base const&);
template generic_packet<dhcp_packet_base>::generic_packet(dhcp_packet_base&&);
template generic_packet<dhcp_packet_base>::generic_packet(dhcp_packet_base const&);
template generic_packet<dhcp_packet_base>::generic_packet(size_t, std::in_place_type_t<dhcp_packet_base>, dhcp_packet_base const&);
template generic_packet<dhcp_packet_base>::generic_packet(size_t, std::in_place_type_t<dhcp_packet_base>, dhcp_packet_base&&);
template generic_packet<dhcp_packet_base>::generic_packet(size_t, std::in_place_type_t<dhcp_packet_base>);
template generic_packet<dhcp_packet_base>::generic_packet(size_t, std::in_place_type_t<dhcp_packet_base>, udp_packet_base&&);
template generic_packet<dhcp_packet_base>::generic_packet(size_t, std::in_place_type_t<dhcp_packet_base>, udp_packet_base const&);
generic_packet<dhcp_packet_base> dhcp_protocol_handler::build_dhcp_discover(std::vector<net8> const& param_requests) 
{
    typedef generic_packet<dhcp_packet_base> dhcp_packet;
	size_t num_requests     = param_requests.size();
    size_t parameters_size  = num_requests + 6UZ; // 3 bytes for the message type, 2 bytes for the request list type and size, 1 byte for the EOT mark
    size_t target_size      = total_dhcp_size(parameters_size);
    size_t actual_size      = up_to_nearest(target_size, 2UZ);
    dhcp_packet result(actual_size, std::in_place_type<dhcp_packet_base>, std::forward<ipv4_standard_packet>(ethernet_packet(broadcast_mac, mac_addr)));
    array_copy(result->client_hw, result->source_mac.data(), 6UZ);
    result->total_length                = net16(static_cast<uint16_t>(result.packet_size - sizeof(ethernet_packet)));
    result->udp_length                  = net16(static_cast<uint16_t>(result.packet_size - sizeof(ipv4_standard_packet)));
    result->operation                   = DISCOVER;
    result->transaction_id              = net32(static_cast<uint32_t>(rand()));
    result->destination_addr            = broadcast;
    addr_t pos                          = result->parameters;
    dhcp_parameter* param               = pos;
    param->type_code                    = MESSAGE_TYPE;
    param->length()                     = 1UC;
    addr_t(param->start()).ref<net8>()  = DISCOVER;
    pos                                 += 3L;
    param                               = pos;
    param->type_code                    = PARAMETER_REQUEST_LIST;
    param->length()                     = num_requests;
    array_copy(param->start(), param_requests.data(), num_requests);
    pos                                             += num_requests + 2Z;
    if(target_size != actual_size) pos.ref<net8>()  = 0UC, pos += 1L;
    pos.ref<net8>()                                 = 0xFFUC;
    result->compute_ipv4_csum();
    result->compute_udp_csum();
    return result;
}