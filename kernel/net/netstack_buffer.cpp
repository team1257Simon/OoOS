#include "net/netstack_buffer.hpp"
#include "net/protocol/ipv4.hpp"
#include "stdexcept"
net16 netstack_buffer::rx_packet_type() const { return reinterpret_cast<ethernet_packet const*>(__in_region.__begin)->protocol_type; }
size_t netstack_buffer::ipv4_size() const { return reinterpret_cast<ipv4_standard_packet const*>(__in_region.__begin)->total_length; }
int netstack_buffer::tx_flush()
{
    if(tx_poll) if(int err = tx_poll(*this)) return err;
    fence();
    array_zero(__out_region.__begin, __out_region.__capacity()); 
    __out_region.__setc(0UZ);
    return 0;
}
int netstack_buffer::rx_flush()
{
    if(rx_poll) { if(int err = rx_poll(*this)) return err; }
    fence();
    array_zero(__in_region.__begin, __in_region.__capacity()); 
    __in_region.__setc(0UZ);
    return 0;
}
netstack_buffer::int_type netstack_buffer::overflow(int_type c)
{
    if(tx_limit && (__out_region.__capacity() << 1) > tx_limit)
        throw std::overflow_error{ "cannot expand transmit buffer beyond " + std::to_string(tx_limit) + " bytes" };
    return __base::overflow(c);
}
std::streamsize netstack_buffer::xsputn(const char* s, size_type n)
{
    size_type tx_capacity   = __out_region.__capacity();
    size_type tx_cur        = static_cast<size_type>(__out_region.__end - __out_region.__begin);
    if(tx_cur + n > tx_capacity)
    {
        size_type target    = tx_cur + n;
        if(tx_limit && target > tx_limit)
            throw std::overflow_error{ "cannot expand transmit buffer beyond " + std::to_string(tx_limit) + " bytes" };
        else size(target, std::ios_base::out);
    }
    array_copy(__out_region.__end, s, n);
    pbump(n);
    return n;
}
std::streamsize netstack_buffer::xsgetn(char* s, size_type n)
{
    size_type avail = static_cast<size_type>(__in_region.__max - __in_region.__end);
    n               = std::min(n, avail);
    array_copy(s, __in_region.__end, n);
    gbump(n);
    return n;
}
netstack_buffer::netstack_buffer() :
    rx_poll     {},
    tx_poll     {},
    rx_limit    {},
    tx_limit    {}
                {}
netstack_buffer::netstack_buffer(size_type initial_rx_cap, size_type initial_tx_cap, poll_functor&& rxp, poll_functor&& txp, size_type txl, size_type rxl) :
    rx_poll     { std::move(rxp) },
    tx_poll     { std::move(txp) },
    rx_limit    { rxl },
    tx_limit    { txl }
{ 
    __in_region.__begin     = __allocator.allocate(initial_rx_cap);
    __in_region.__end       = __in_region.__begin;
    __in_region.__max       = __in_region.__begin + initial_rx_cap;
    __out_region.__begin    = __allocator.allocate(initial_tx_cap);
    __out_region.__end      = __out_region.__begin;
    __out_region.__max      = __out_region.__begin + initial_tx_cap;
    array_zero(__in_region.__begin, initial_rx_cap);
    array_zero(__out_region.__begin, initial_tx_cap); 
}