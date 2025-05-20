#define ARP_INST
#include "net/protocol/arp.hpp"
template class generic_packet<arpv4_packet>;
template generic_packet<arpv4_packet>::generic_packet();
template generic_packet<arpv4_packet>::generic_packet(ethernet_packet const&);
template generic_packet<arpv4_packet>::generic_packet(ethernet_packet&&);
template generic_packet<arpv4_packet>::generic_packet(arpv4_packet const&);
template generic_packet<arpv4_packet>::generic_packet(arpv4_packet&&);
template generic_packet<arpv4_packet>::generic_packet(mac_t&&, mac_t&&, net16&&, net32&&, net32&&);
template generic_packet<arpv4_packet>::generic_packet(mac_t const&, mac_t const&, net16 const&, net32 const&, net32 const&);
template generic_packet<arpv4_packet>::generic_packet(size_t, std::in_place_type_t<arpv4_packet>);
template generic_packet<arpv4_packet>::generic_packet(size_t, std::in_place_type_t<arpv4_packet>, ethernet_packet const&);
template generic_packet<arpv4_packet>::generic_packet(size_t, std::in_place_type_t<arpv4_packet>, ethernet_packet&&);
template generic_packet<arpv4_packet>::generic_packet(size_t, std::in_place_type_t<arpv4_packet>, arpv4_packet const&);
template generic_packet<arpv4_packet>::generic_packet(size_t, std::in_place_type_t<arpv4_packet>, arpv4_packet&&);
template generic_packet<arpv4_packet>::generic_packet(size_t, std::in_place_type_t<arpv4_packet>, mac_t&&, mac_t&&, net16&&, net32&&, net32&&);
template generic_packet<arpv4_packet>::generic_packet(size_t, std::in_place_type_t<arpv4_packet>, mac_t const&, mac_t const&, net16 const&, net32 const&, net32 const&);