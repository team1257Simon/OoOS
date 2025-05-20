#include "net/protocol/generic_packet.hpp"
#include "sys/errno.h"
#include "stdexcept"
generic_packet_base::generic_packet_base(void* data, std::type_info const& type, size_t sz, void (*dealloc)(void*, size_t)) : packet_data(data), packet_type(type), packet_size(sz), release_fn(dealloc) {}
generic_packet_base::~generic_packet_base() { if(packet_data) (*release_fn)(packet_data, packet_size); }
generic_packet_base::generic_packet_base(generic_packet_base const& that) :
    packet_data { ::operator new(that.packet_size) },
    packet_type { that.packet_type },
    packet_size { that.packet_size },
    release_fn  { that.release_fn }
                { array_copy(packet_data, static_cast<char const*>(that.packet_data), packet_size); }
generic_packet_base::generic_packet_base(generic_packet_base&& that) :
    packet_data { that.packet_data },
    packet_type { std::move(that.packet_type) },
    packet_size { that.packet_size },
    release_fn  { that.release_fn }
                { that.packet_data = nullptr; }
generic_packet_base::generic_packet_base(netstack_buffer& buffer, std::type_info const& type) :
    packet_data { ::operator new(buffer.count(std::ios_base::in)) },
    packet_type { type },
    packet_size { buffer.count(std::ios_base::in) },
    release_fn  { ::operator delete }
                { array_copy(packet_data, buffer.eback(), packet_size); }
generic_packet_base& generic_packet_base::operator=(generic_packet_base const& that)
{
    if(packet_data) (*release_fn)(packet_data, packet_size);
    packet_data = ::operator new(that.packet_size);
    packet_type = that.packet_type;
    packet_size = that.packet_size;
    release_fn  = that.release_fn;
    array_copy(packet_data, static_cast<char const*>(that.packet_data), packet_size);
    return *this;
}
generic_packet_base& generic_packet_base::operator=(generic_packet_base&& that)
{
    if(packet_data) (*release_fn)(packet_data, packet_size);
    packet_data = that.packet_data;
    packet_type = std::move(that.packet_type);
    packet_size = that.packet_size;
    release_fn  = that.release_fn;
    that.packet_data = nullptr;
    return *this;
}
int generic_packet_base::read_from(netstack_buffer& buff)
{
    size_t read_size = buff.count(std::ios_base::in);
    if(read_size < packet_size) return -EPROTO;
    array_copy(packet_data, buff.eback(), packet_size);
    return 0;
}
int generic_packet_base::write_to(netstack_buffer& buff) const
{
    try { buff.sputn(static_cast<const char*>(packet_data), packet_size); }
    catch(std::overflow_error&) { return -EOVERFLOW; }
    catch(std::bad_alloc&) { return -ENOMEM; }
    return 0;
}