#define DHCP_INST
#include "net/protocol/dhcp.hpp"
#include "sys/errno.h"
#include "stdlib.h" // rand()
constexpr static std::allocator<net16> udp_pseudo_alloc;
template<> template<> abstract_packet<dhcp_packet_base>::abstract_packet(udp_packet_base&& that) : abstract_packet(static_cast<size_t>(that.total_length), std::in_place_type<dhcp_packet_base>, std::move(that)) {}
template<> template<> abstract_packet<dhcp_packet_base>::abstract_packet(udp_packet_base const& that) : abstract_packet(static_cast<size_t>(that.total_length), std::in_place_type<dhcp_packet_base>, that) {}
template<> template<> abstract_packet<dhcp_packet_base>::abstract_packet(dhcp_packet_base&& that) : abstract_packet(static_cast<size_t>(that.total_length), std::in_place_type<dhcp_packet_base>, std::move(that)) {}
template<> template<> abstract_packet<dhcp_packet_base>::abstract_packet(dhcp_packet_base const& that) : abstract_packet(static_cast<size_t>(that.total_length), std::in_place_type<dhcp_packet_base>, that) {}
template class abstract_packet<dhcp_packet_base>;
template abstract_packet<dhcp_packet_base>::abstract_packet(udp_packet_base&&);
template abstract_packet<dhcp_packet_base>::abstract_packet(udp_packet_base const&);
template abstract_packet<dhcp_packet_base>::abstract_packet(dhcp_packet_base&&);
template abstract_packet<dhcp_packet_base>::abstract_packet(dhcp_packet_base const&);
template abstract_packet<dhcp_packet_base>::abstract_packet(size_t, std::in_place_type_t<dhcp_packet_base>, dhcp_packet_base const&);
template abstract_packet<dhcp_packet_base>::abstract_packet(size_t, std::in_place_type_t<dhcp_packet_base>, dhcp_packet_base&&);
template abstract_packet<dhcp_packet_base>::abstract_packet(size_t, std::in_place_type_t<dhcp_packet_base>);
template abstract_packet<dhcp_packet_base>::abstract_packet(size_t, std::in_place_type_t<dhcp_packet_base>, udp_packet_base&&);
template abstract_packet<dhcp_packet_base>::abstract_packet(size_t, std::in_place_type_t<dhcp_packet_base>, udp_packet_base const&);
dhcp_packet_base::dhcp_packet_base() noexcept = default;
dhcp_packet_base::dhcp_packet_base(udp_packet_base const& that) noexcept : udp_packet_base(that) { source_port = dhcp_client_port; destination_port = dhcp_server_port; }
dhcp_packet_base::dhcp_packet_base(udp_packet_base&& that) noexcept : udp_packet_base(std::move(that)) { source_port = dhcp_client_port; destination_port = dhcp_server_port; }
std::type_info const& protocol_dhcp::packet_type() const { return typeid(dhcp_packet_base); }
protocol_dhcp::~protocol_dhcp() = default;
protocol_dhcp::protocol_dhcp(protocol_udp* n) : abstract_protocol_handler(n), ip_addrs(n->ip_addrs) {}
int protocol_dhcp::transmit(abstract_packet_base& p)
{
    dhcp_packet_base* pkt = p.get_as<dhcp_packet_base>();
    if(!pkt) return -EPROTOTYPE;
    return next->transmit(p);
}
int protocol_dhcp::receive(abstract_packet_base& p)
{
    dhcp_packet_base* pkt = p.get_as<dhcp_packet_base>();
    if(!pkt) return -EPROTOTYPE;
    // do the stuff
    return 0;
}
int protocol_dhcp::discover(std::vector<net8> const& param_requests)
{
    size_t num_requests                         = param_requests.size(); 
    size_t target_size                          = total_dhcp_size(num_requests + 6UZ); // 3 bytes for the message type, 2 bytes for the request list type and size, 1 byte for the EOT mark
    size_t actual_size                          = up_to_nearest(target_size, 2UZ);
    abstract_packet<dhcp_packet_base> result(actual_size, std::in_place_type<dhcp_packet_base>, std::forward<ipv4_standard_packet>(base->create_packet(broadcast_mac)));
    result->operation                           = DISCOVER;
    result->transaction_id                      = net32(static_cast<uint32_t>(rand()));
    result->destination_addr                    = broadcast;
    addr_t pos                                  = result->parameters;
    dhcp_parameter* param                       = pos;
    param->type_code                            = MESSAGE_TYPE;
    param->length()                             = 1UC;
    addr_t(param->start()).ref<net8>()          = DISCOVER;
    pos                                         += 3L;
    param                                       = pos;
    param->type_code                            = PARAMETER_REQUEST_LIST;
    param->length()                             = num_requests;
    array_copy(param->start(), param_requests.data(), num_requests);
    pos                                             += num_requests + 2Z;
    if(target_size != actual_size) pos.ref<net8>()  = 0UC, pos += 1L;
    pos.ref<net8>()                                 = 0xFFUC;
    return next->transmit(result);
}