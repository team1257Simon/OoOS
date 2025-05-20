#define IP_INST
#include "net/protocol/ipv4.hpp"
template<> template<> generic_packet<ipv4_packet_with_options<IHL20B>>::generic_packet(ipv4_packet_with_options<IHL20B> const& that) : generic_packet(static_cast<size_t>(that.total_length), std::in_place_type<ipv4_packet_with_options<IHL20B>>, that) {}
template<> template<> generic_packet<ipv4_packet_with_options<IHL20B>>::generic_packet(ipv4_packet_with_options<IHL20B>&& that) : generic_packet(static_cast<size_t>(that.total_length), std::in_place_type<ipv4_packet_with_options<IHL20B>>, std::move(that)) {}
template struct ipv4_packet_with_options<IHL20B>;
template class generic_packet<ipv4_packet_with_options<IHL20B>>;
template generic_packet<ipv4_packet_with_options<IHL20B>>::generic_packet();
template generic_packet<ipv4_packet_with_options<IHL20B>>::generic_packet(ethernet_packet const&);
template generic_packet<ipv4_packet_with_options<IHL20B>>::generic_packet(ethernet_packet&&);
template generic_packet<ipv4_packet_with_options<IHL20B>>::generic_packet(ipv4_packet_with_options<IHL20B> const& that);
template generic_packet<ipv4_packet_with_options<IHL20B>>::generic_packet(ipv4_packet_with_options<IHL20B>&& that);
template generic_packet<ipv4_packet_with_options<IHL20B>>::generic_packet(size_t, std::in_place_type_t<ipv4_packet_with_options<IHL20B>>);
template generic_packet<ipv4_packet_with_options<IHL20B>>::generic_packet(size_t, std::in_place_type_t<ipv4_packet_with_options<IHL20B>>, ethernet_packet const&);
template generic_packet<ipv4_packet_with_options<IHL20B>>::generic_packet(size_t, std::in_place_type_t<ipv4_packet_with_options<IHL20B>>, ethernet_packet&&);
template generic_packet<ipv4_packet_with_options<IHL20B>>::generic_packet(size_t, std::in_place_type_t<ipv4_packet_with_options<IHL20B>>, ipv4_packet_with_options<IHL20B> const&);
template generic_packet<ipv4_packet_with_options<IHL20B>>::generic_packet(size_t, std::in_place_type_t<ipv4_packet_with_options<IHL20B>>, ipv4_packet_with_options<IHL20B>&&);