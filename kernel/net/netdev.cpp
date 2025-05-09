#include "net/netdev.hpp"
net_device::net_device() = default;
net_device::~net_device() = default;
void net_device::register_stack(netstack_buffer::poll_functor&& f) 
{ 
    up_stack_functor = std::move(f);
    for(netstack_buffer& b : transfer_buffers) 
        b.rx_poll = netstack_buffer::poll_functor(up_stack_functor);
}