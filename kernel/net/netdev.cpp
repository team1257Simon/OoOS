#include "net/netdev.hpp"
net_device::net_device() = default;
net_device::~net_device() = default;
int net_device::poll_nop(netstack_buffer&) noexcept { return 0; }
void net_device::register_stack(netstack_buffer::poll_functor&& f) 
{ 
    up_stack_functor    = std::move(f);
    for(netstack_buffer& b : transfer_buffers) 
        b.rx_poll       = static_cast<netstack_buffer::poll_functor const&>(up_stack_functor);
}
int net_device::transmit(generic_packet_base& p)
{
    if(transfer_buffers.at_end()) transfer_buffers.restart();
    netstack_buffer& buffer = transfer_buffers.pop();
    int err                 = p.write_to(buffer);
    if(err) return err;
    return buffer.tx_flush();
}