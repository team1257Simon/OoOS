#ifndef __SIMPLE_IO_PORT
#define __SIMPLE_IO_PORT
#include "arch/arch_amd64.h"
#include "bits/iterator_concepts.hpp"
template<integral_structure IT>
class simple_io_port
{
    uint16_t __port_num;
public:
    constexpr simple_io_port(uint16_t n) : __port_num{ n } {}
    constexpr ~simple_io_port() = default;
    constexpr simple_io_port(simple_io_port const&) = default;
    constexpr simple_io_port(simple_io_port&&) = default;
    constexpr void put(IT i) { out(__port_num, i); }
    constexpr IT get() { return in<IT>(__port_num); }
    constexpr simple_io_port& operator=(simple_io_port const&) = default;
    constexpr simple_io_port& operator=(simple_io_port&&) = default;
    constexpr simple_io_port& operator<<(IT i) { put(i); return *this; }
    constexpr simple_io_port& operator>>(IT& i) { i = get(); return *this; }
    template<std::const_iterable<IT> CT> constexpr simple_io_port& operator<<(CT const& c) { for(IT i = c.begin(); i != c.end(); i++) { put(*i); } return *this; }
    template<std::iterable<IT> CT> constexpr simple_io_port& operator>>(CT& c) { for(IT i = c.begin(); i != c.end(); i++) { *i = get(); } return *this; }
};
#endif