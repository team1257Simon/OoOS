#include <net/protocol/net_abstract.hpp>
#include <sys/errno.h>
#include <stdexcept>
constexpr static inline std::string digits_out(uint8_t b) { return { "0123456789ABCDEF"[(b >> 4) & 0x0FUC], "0123456789ABCDEF"[b & 0x0FUC] }; }
constexpr static inline std::string digits_in(uint8_t b) { return { "0123456789ABCDEF"[(b >> 4) & 0x0FUC], "0123456789ABCDEF"[b & 0x0FUC], ':' };  }
std::string stringify(ipv4_addr ip) { return std::to_string(ip.hi.hi) + "." + std::to_string(ip.hi.lo) + "." + std::to_string(ip.lo.hi) + "." + std::to_string(ip.lo.lo); }
std::string stringify(mac_t const& mac) { std::string result(digits_in(mac[0])); for(int i = 1; i < 5; i++) result += digits_in(mac[i]); return result + digits_out(mac[5]); }
abstract_packet_base::abstract_packet_base(void* data, std::type_info const& type, size_t sz, void (*dealloc)(void*, size_t)) : packet_data(data), packet_type(type), packet_size(sz), release_fn(dealloc) {}
abstract_packet_base::~abstract_packet_base() { if(packet_data) (*release_fn)(packet_data, packet_size); }
abstract_ip_resolver::abstract_ip_resolver() : previously_resolved(1024UZ) {}
protocol_ethernet::protocol_ethernet(abstract_ip_resolver* ip_res, std::function<int(abstract_packet_base&)>&& tx_fn, mac_t const& mac) : abstract_protocol_handler(nullptr, this), ip_resolver(ip_res), transmit_fn(std::move(tx_fn)), handlers(64UZ), mac_addr(mac) {}
protocol_ethernet::~protocol_ethernet() = default;
std::type_info const& protocol_ethernet::packet_type() const { return typeid(ethernet_header); }
int protocol_ethernet::transmit(abstract_packet_base& p) { return transmit_fn(p); }
ethernet_header protocol_ethernet::create_packet(mac_t const& dest) { return ethernet_header(dest, mac_addr); }
abstract_ip_resolver::~abstract_ip_resolver() = default;
abstract_protocol_handler::abstract_protocol_handler(abstract_protocol_handler* n) : next(n), base(next->base) {}
abstract_protocol_handler::abstract_protocol_handler(abstract_protocol_handler* n, protocol_ethernet* b) : next(n), base(b) {}
int abstract_protocol_handler::receive(abstract_packet_base& p) { return 0; }
int abstract_protocol_handler::transmit(abstract_packet_base& p) { if(!next) throw std::runtime_error("cannot send packet with empty protocol"); return next->transmit(p); }
abstract_packet_base::abstract_packet_base(abstract_packet_base const& that) :
	packet_data { ::operator new(that.packet_size) },
	packet_type { that.packet_type },
	packet_size { that.packet_size },
	release_fn  { that.release_fn }
				{ array_copy(packet_data, static_cast<char const*>(that.packet_data), packet_size); }
abstract_packet_base::abstract_packet_base(abstract_packet_base&& that) :
	packet_data { that.packet_data },
	packet_type { std::move(that.packet_type) },
	packet_size { that.packet_size },
	release_fn  { that.release_fn }
				{ that.packet_data = nullptr; }
abstract_packet_base::abstract_packet_base(netstack_buffer& buffer, std::type_info const& type) :
	packet_data { ::operator new(buffer.count(std::ios_base::in)) },
	packet_type { type },
	packet_size { buffer.count(std::ios_base::in) },
	release_fn  { ::operator delete }
				{ array_copy(packet_data, buffer.eback(), packet_size); }
abstract_packet_base& abstract_packet_base::operator=(abstract_packet_base const& that)
{
	if(packet_data) (*release_fn)(packet_data, packet_size);
	packet_data = ::operator new(that.packet_size);
	packet_type = that.packet_type;
	packet_size = that.packet_size;
	release_fn  = that.release_fn;
	array_copy(packet_data, static_cast<char const*>(that.packet_data), packet_size);
	return *this;
}
abstract_packet_base& abstract_packet_base::operator=(abstract_packet_base&& that)
{
	if(packet_data) (*release_fn)(packet_data, packet_size);
	packet_data			= that.packet_data;
	packet_type			= std::move(that.packet_type);
	packet_size			= that.packet_size;
	release_fn			= that.release_fn;
	that.packet_data	= nullptr;
	return *this;
}
int abstract_packet_base::read_from(netstack_buffer& buff)
{
	size_t read_size	= buff.count(std::ios_base::in);
	if(read_size < packet_size) return -EPROTO;
	array_copy(packet_data, buff.eback(), packet_size);
	return 0;
}
int abstract_packet_base::write_to(netstack_buffer& buff) const
{
	try { buff.sputn(static_cast<const char*>(packet_data), packet_size); }
	catch(std::overflow_error&)	{ return -EOVERFLOW; }
	catch(std::bad_alloc&)		{ return -ENOMEM; }
	return 0;
}
mac_t const& abstract_ip_resolver::operator[](ipv4_addr addr)
{
	if(previously_resolved.contains(addr))
		return previously_resolved[addr];
	return resolve(addr);
}
int protocol_ethernet::receive(abstract_packet_base& p)
{
	ethernet_header* hdr	= static_cast<ethernet_header*>(p.packet_data);
	if(handlers.contains(hdr->protocol_type))
	{
		protocol_handler& h	= handlers[hdr->protocol_type];
		p.packet_type		= h->packet_type();
		return h->receive(p);
	}
	return -EPROTONOSUPPORT;
}