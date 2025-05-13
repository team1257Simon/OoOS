#include "net/protocol/generic_packet.hpp"
#include "sys/errno.h"
#include "stdexcept"
generic_packet_base::generic_packet_base(void* data, std::type_info const& type, size_t sz) : packet_data(data), packet_type(type), packet_size(sz) {}
int generic_packet_base::read_from(netstack_buffer& buff)
{
    size_t read_size = buff.sgetn(static_cast<char*>(packet_data), packet_size);
    if(read_size < packet_size) return -EPROTO;
    return 0;
}
int generic_packet_base::write_to(netstack_buffer& buff) const
{
    try { buff.sputn(static_cast<const char*>(packet_data), packet_size); }
    catch(std::overflow_error&) { return -EOVERFLOW; }
    catch(std::bad_alloc&) { return -ENOMEM; }
    return 0;
}