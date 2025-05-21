#ifndef __UDP
#define __UDP
#include "net/protocol/ipv4.hpp"
struct __pack udp_packet_base : ipv4_standard_packet
{
    net16 source_port;
    net16 destination_port;
    net16 udp_length;
    net16 udp_csum;
    udp_packet_base() noexcept;
    udp_packet_base(ipv4_standard_packet const& that) noexcept;
    udp_packet_base(ipv4_standard_packet&& that) noexcept;
    void compute_udp_csum();
    bool verify_udp_csum() const;
};
struct protocol_udp;
struct generic_udp_handler : abstract_protocol_handler
{
    std::function<int(abstract_packet_base&)> acceptor;
    generic_udp_handler(protocol_udp* n, std::function<int(abstract_packet_base&)>&& f);
    virtual std::type_info const& packet_type() const override;
    virtual int transmit(abstract_packet_base& p) override;
    virtual int receive(abstract_packet_base& p) override;
};
typedef protocol_handler_map<uint16_t> udp_port_map;
#ifndef UDP_INST
extern template class abstract_packet<udp_packet_base>;
extern template abstract_packet<udp_packet_base>::abstract_packet(ipv4_standard_packet&&);
#endif
struct protocol_udp : abstract_protocol_handler
{
    udp_port_map ports;
    ipv4_addr_set* const ip_addrs;
    protocol_udp(protocol_ipv4* n);
    protocol_handler& add_port_handler(uint16_t port, protocol_handler&& ph);
    virtual int transmit(abstract_packet_base& p) override;
    virtual std::type_info const& packet_type() const override;
    virtual int receive(abstract_packet_base& p) override;
    template<std::derived_from<abstract_protocol_handler> PT> requires std::constructible_from<PT, protocol_udp*> PT& add_port(uint16_t id) { return add_port_handler(id, std::move(create_handler<PT>(this))).template cast<PT>(); }
};
#endif