#define IP_INST
#include "sys/errno.h"
#include "net/protocol/ipv4.hpp"
template<> template<> abstract_packet<ipv4_header<IHL20B>>::abstract_packet(ipv4_header<IHL20B> const& that) : abstract_packet(static_cast<size_t>(that.total_length), std::in_place_type<ipv4_header<IHL20B>>, that) {}
template<> template<> abstract_packet<ipv4_header<IHL20B>>::abstract_packet(ipv4_header<IHL20B>&& that) : abstract_packet(static_cast<size_t>(that.total_length), std::in_place_type<ipv4_header<IHL20B>>, std::move(that)) {}
template struct ipv4_header<IHL20B>;
template class abstract_packet<ipv4_header<IHL20B>>;
template abstract_packet<ipv4_header<IHL20B>>::abstract_packet();
template abstract_packet<ipv4_header<IHL20B>>::abstract_packet(ethernet_header const&);
template abstract_packet<ipv4_header<IHL20B>>::abstract_packet(ethernet_header&&);
template abstract_packet<ipv4_header<IHL20B>>::abstract_packet(ipv4_header<IHL20B> const& that);
template abstract_packet<ipv4_header<IHL20B>>::abstract_packet(ipv4_header<IHL20B>&& that);
template abstract_packet<ipv4_header<IHL20B>>::abstract_packet(size_t, std::in_place_type_t<ipv4_header<IHL20B>>);
template abstract_packet<ipv4_header<IHL20B>>::abstract_packet(size_t, std::in_place_type_t<ipv4_header<IHL20B>>, ethernet_header const&);
template abstract_packet<ipv4_header<IHL20B>>::abstract_packet(size_t, std::in_place_type_t<ipv4_header<IHL20B>>, ethernet_header&&);
template abstract_packet<ipv4_header<IHL20B>>::abstract_packet(size_t, std::in_place_type_t<ipv4_header<IHL20B>>, ipv4_header<IHL20B> const&);
template abstract_packet<ipv4_header<IHL20B>>::abstract_packet(size_t, std::in_place_type_t<ipv4_header<IHL20B>>, ipv4_header<IHL20B>&&);
typedef ipv4_header<IHL20B> ipv4_standard_header;
protocol_ipv4::protocol_ipv4(protocol_ethernet* eth) : abstract_protocol_handler(eth, eth), transports(64UZ) { base->ipv4_client_config = std::addressof(client_config); }
protocol_ipv4::~protocol_ipv4() = default;
std::type_info const& protocol_ipv4::packet_type() const { return typeid(ipv4_header<IHL20B>); }
protocol_handler& protocol_ipv4::add_transport(ipv4_transport_protocol id, protocol_handler&& ph) { return transports.emplace(std::piecewise_construct, std::forward_as_tuple(id), std::forward_as_tuple(std::move(ph))).first->second; }
int protocol_ipv4::transmit(abstract_packet_base& p)
{
    ipv4_standard_header* pkt = p.get_as<ipv4_standard_header>();
    if(__unlikely(!pkt)) throw std::bad_cast();
    pkt->time_to_live = pkt->protocol_type == TCP ? client_config.time_to_live_tcp_default : client_config.time_to_live_default;
    pkt->total_length = net16(static_cast<uint16_t>(p.packet_size - sizeof(ethernet_header)));
    pkt->compute_ipv4_csum();
    return next->transmit(p);
}
int protocol_ipv4::receive(abstract_packet_base& p)
{
    ipv4_standard_header* pkt = p.get_as<ipv4_standard_header>();
    if(__unlikely(!pkt)) return -EPROTOTYPE;
    if(__unlikely(pkt->ihl != IHL20B)) return -EPROTOTYPE;
    if(__unlikely(!pkt->verify_ipv4_csum())) return -EPROTO;
    if(transports.contains(pkt->protocol))
    {
        protocol_handler& ph    = transports[pkt->protocol];
        p.packet_type           = ph->packet_type();
        return ph->receive(p);
    }
    return -EPROTONOSUPPORT;
}