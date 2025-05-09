#include "net/netstack_buffer.hpp"
#include "stdexcept"
void netstack_buffer::flushp() { array_zero(__out_region.__begin, __out_region.__capacity()); __out_region.__setc(0UZ); }
int netstack_buffer::sync()
{
    int result = 0;
    if(rx_poll && __in_region.__begin && __in_region.__end > __in_region.__begin)
        result = rx_poll(*this);
    if(result) return result;
    if(tx_poll && __out_region.__begin && __out_region.__end > __out_region.__begin)
        result = tx_poll(*this);
    return result;
}
netstack_buffer::int_type netstack_buffer::overflow(int_type c)
{
    if(tx_limit && (__out_region.__capacity() << 1) > tx_limit)
        throw std::overflow_error{ "cannot expand transmit buffer beyond " + std::to_string(tx_limit) + " bytes" };
    return __base::overflow(c);
}
std::streamsize netstack_buffer::putg(const void* data, size_type n)
{
    if(rx_limit && n > rx_limit) throw std::overflow_error{ "rx buffer target size cannot be larger than " + std::to_string(rx_limit) + " bytes" };
    __in_region.__set_ptrs(std::resize(__in_region.__begin, __in_region.__capacity(), n, __allocator), n);
    array_zero(__in_region.__begin, __in_region.__capacity());
    const char* cdata = static_cast<const char*>(data);
    array_copy(__in_region.__begin, cdata, n);
    return n;
}
std::streamsize netstack_buffer::getp(void* out, size_type n)
{
    size_type len = std::min(n, static_cast<size_type>(__out_region.__end - __out_region.__begin));
    array_copy(out, __out_region.__begin, len);
    return len;
}
netstack_buffer::netstack_buffer() :
    rx_poll     {},
    tx_poll     {},
    rx_limit    {},
    tx_limit    {}
                { size(32UZ); }
netstack_buffer::netstack_buffer(size_type initial_rx_cap, size_type initial_tx_cap, poll_functor&& rxp, poll_functor&& txp, size_type txl, size_type rxl) :
    rx_poll     { std::move(rxp) },
    tx_poll     { std::move(txp) },
    rx_limit    { rxl },
    tx_limit    { txl }
                { size(initial_rx_cap, std::ios_base::in); size(initial_tx_cap, std::ios_base::out); }