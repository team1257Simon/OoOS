#ifndef __NETSKB
#define __NETSKB
#include "ext/dynamic_duplex_streambuf.hpp"
#include "functional"
class netstack_buffer : public std::ext::dynamic_duplex_streambuf<char>
{
    typedef std::ext::dynamic_duplex_streambuf<char> __base;
public:
    typedef std::function<int(netstack_buffer&)> poll_functor;
    poll_functor rx_poll;
    poll_functor tx_poll;
    size_type rx_limit;
    size_type tx_limit;
protected:
    int sync() override;
    virtual int_type overflow(int_type c) override;
    virtual std::streamsize xsputn(const char* s, size_type n) override;
    virtual std::streamsize xsgetn(char* s, size_type n) override;
public:
    virtual std::streamsize putg(const void* data, size_type n);
    virtual std::streamsize getp(void* out, size_type n);
    virtual void flushp();
    netstack_buffer(size_type initial_rx_cap, size_type initial_tx_cap, poll_functor&& rxp, poll_functor&& txp, size_type tx_limit, size_type rx_limit);
    netstack_buffer();
};
#endif