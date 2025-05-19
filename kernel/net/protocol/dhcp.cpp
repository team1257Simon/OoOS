#include "net/protocol/dhcp.hpp"
#include "stdlib.h" // rand()
constexpr static std::allocator<net16> udp_pseudo_alloc;
dhcp_packet dhcp_protocol_handler::build_dhcp_discover(std::vector<net8> const& param_requests) 
{
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
    result->client_ip                   = 0UBE;
    result->server_ip                   = 0UBE;
    result->your_ip                     = 0UBE;
    result->relay_ip                    = 0UBE;
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
    pos                                             = addr_t(result.packet_data).plus(sizeof(ethernet_packet));
    result->header_checksum                         = ip_checksum(static_cast<net16*>(pos), result->ihl * 2UZ);
    pos                                             += result->ihl * 4Z;
    size_t num_words                                = static_cast<size_t>((result->udp_length + 12UZ) / sizeof(net16));
    net16* pseudo_header                            = udp_pseudo_alloc.allocate(num_words);
    array_zero(pseudo_header, num_words);
    pseudo_header[0] = result->source_addr.hi;
    pseudo_header[1] = result->source_addr.lo;
    pseudo_header[2] = result->destination_addr.hi;
    pseudo_header[3] = result->destination_addr.lo;
    pseudo_header[4] = net16(0UC, result->protocol);
    pseudo_header[5] = result->total_length;
    array_copy(std::addressof(pseudo_header[6]), static_cast<net16*>(pos), static_cast<size_t>(num_words - 6));
    result->udp_checksum = ip_checksum(pseudo_header, num_words);
    udp_pseudo_alloc.deallocate(pseudo_header, num_words);
    return result;
}