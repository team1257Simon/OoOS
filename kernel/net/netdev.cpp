#include "net/netdev.hpp"
#include "sys/errno.h"
net_device::~net_device() = default;
net_device::net_device() : arp_handler(std::addressof(base_handler)), base_handler(std::addressof(arp_handler), std::move(std::bind(&net_device::transmit, this, std::placeholders::_1)), mac_addr) {}
protocol_handler& net_device::add_protocol(net16 id, protocol_handler&& ph) { return base_handler.handlers.emplace(std::piecewise_construct, std::forward_as_tuple(id), std::forward_as_tuple(std::move(ph))).first->second; }
int net_device::transmit(abstract_packet_base& p)
{
    if(transfer_buffers.at_end()) transfer_buffers.restart();
    netstack_buffer& buffer = transfer_buffers.pop();
    int err                 = p.write_to(buffer);
    if(err) return err;
    return buffer.tx_flush();
}
int net_device::rx_transfer(netstack_buffer& b) noexcept
{
    try
    {
        abstract_packet<ethernet_packet> p(b);
        if(p->protocol_type == ethertype_arp) 
        {
            p.packet_type = arp_handler.packet_type();
            if(int err = __builtin_expect(arp_handler.receive(p), 0)) return err;
            return 0;
        }
        int result = base_handler.receive(p);
        if(!result) return 0;
        if(result == -EPROTONOSUPPORT) { klog("[e1000e] W: unrecognized protocol"); return 0; }
        return result;
    }
    catch(std::bad_alloc&)      { return -ENOMEM; }
    catch(std::bad_cast&)       { return -EPROTO; }
    catch(std::exception& e)    { panic(e.what()); return -EINVAL; }
}