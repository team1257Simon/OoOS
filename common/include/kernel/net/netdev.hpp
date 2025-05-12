#ifndef __NETDEV
#define __NETDEV
#include "net/netstack_buffer.hpp"
#include "bits/stl_queue.hpp"
class net_device
{
protected:
    uint8_t mac_addr[6];
    std::ext::resettable_queue<netstack_buffer> transfer_buffers;
    netstack_buffer::poll_functor up_stack_functor;
public:
    net_device();
    virtual ~net_device();
    virtual bool initialize()       = 0;
    virtual void enable_transmit()  = 0;
    virtual void enable_receive()   = 0;
    virtual void disable_transmit() = 0;
    virtual void disable_receive()  = 0;
    virtual int poll_rx()           = 0;
    void register_stack(netstack_buffer::poll_functor&& f);
    constexpr uint8_t const* get_mac_addr() const { return mac_addr; }
    // ...
};
#endif