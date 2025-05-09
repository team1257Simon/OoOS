#ifndef __NETDEV
#define __NETDEV
#include "libk_decls.h"
class net_device
{
protected:
    uint8_t mac_addr[6];
public:
    net_device();
    virtual ~net_device();
    virtual bool initialize()       = 0;
    virtual void enable_transmit()  = 0;
    virtual void enable_receive()   = 0;
    virtual void disable_transmit() = 0;
    virtual void disable_receive()  = 0;
    // ...
};
#endif