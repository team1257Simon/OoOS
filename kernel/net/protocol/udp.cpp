#define UDP_INST
#include "net/protocol/udp.hpp"
#include "sys/errno.h"
template<> template<> abstract_packet<udp_header>::abstract_packet(ipv4_standard_header&& that) : abstract_packet(that.total_length, std::in_place_type<udp_header>, std::move(that)) {}
template class abstract_packet<udp_header>;
template abstract_packet<udp_header>::abstract_packet(ipv4_standard_header&&);
typedef abstract_packet<udp_header> udp_packet;
generic_udp_handler::generic_udp_handler(protocol_udp* n, std::function<int(abstract_packet_base &)>&& f) : abstract_protocol_handler(n), acceptor(std::move(f)) {}
int generic_udp_handler::receive(abstract_packet_base& p) { return acceptor(p); }
int generic_udp_handler::transmit(abstract_packet_base& p) { return next->transmit(p); }
std::type_info const& generic_udp_handler::packet_type() const { return typeid(udp_header); }
udp_header::udp_header() noexcept { protocol = UDP; }
udp_header::udp_header(ipv4_standard_header const& that) noexcept : ipv4_standard_header(that) { protocol = UDP; }
udp_header::udp_header(ipv4_standard_header&& that) noexcept : ipv4_standard_header(std::move(that)) { protocol = UDP; }
protocol_udp::protocol_udp(protocol_ipv4* n) : abstract_protocol_handler(n), ports(1024UZ), ipconfig(n->client_config) {}
std::type_info const& protocol_udp::packet_type() const { return typeid(udp_header); }
protocol_handler& protocol_udp::add_port_handler(uint16_t port, protocol_handler&& ph) { return ports.emplace(std::piecewise_construct, std::forward_as_tuple(port), std::forward_as_tuple(std::move(ph))).first->second; }
void udp_header::compute_udp_csum()
{
	udp_csum                    = 0USBE;
	uint32_t intermediate_csum  = 0U;
	intermediate_csum           += source_addr.hi;
	intermediate_csum           += source_addr.lo;
	intermediate_csum           += destination_addr.hi;
	intermediate_csum           += destination_addr.lo;
	intermediate_csum           += protocol;
	intermediate_csum           += udp_length;
	net16* words                = std::addressof(source_port);
	size_t num_words            = udp_length / sizeof(net16);
	if((static_cast<size_t>(udp_length) % 2) != 0)
		intermediate_csum += addr_t(this).plus(static_cast<ptrdiff_t>(udp_length) - 1Z).deref<net8>();
	for(size_t i = 0; i < num_words; i++) intermediate_csum += words[i];
	dword dw_csum       		= intermediate_csum;
	intermediate_csum   		= dw_csum.hi + dw_csum.lo;
	dw_csum             		= intermediate_csum;
	intermediate_csum   		= dw_csum.hi + dw_csum.lo;
	udp_csum            		= ~(static_cast<uint16_t>(intermediate_csum));
}
bool udp_header::verify_udp_csum() const
{
	uint32_t intermediate_csum  = 0U;
	intermediate_csum           += source_addr.hi;
	intermediate_csum           += source_addr.lo;
	intermediate_csum           += destination_addr.hi;
	intermediate_csum           += destination_addr.lo;
	intermediate_csum           += protocol;
	intermediate_csum           += udp_length;
	net16 const* words          = std::addressof(source_port);
	size_t num_words            = udp_length / sizeof(net16);
	if((static_cast<size_t>(udp_length) % 2) != 0)
		intermediate_csum += addr_t(this).plus(static_cast<ptrdiff_t>(udp_length) - 1Z).deref<net8>();
	for(size_t i = 0; i < num_words; i++) intermediate_csum += words[i];
	dword dw_csum       		= intermediate_csum;
	intermediate_csum   		= dw_csum.hi + dw_csum.lo;
	dw_csum             		= intermediate_csum;
	intermediate_csum   		= dw_csum.hi + dw_csum.lo;
	dw_csum             		= intermediate_csum;
	return static_cast<uint16_t>(~(dw_csum.lo)) == 0US;
}
int protocol_udp::transmit(abstract_packet_base& p)
{
	udp_header* hdr	= p.get_as<udp_header>();
	if(!hdr) throw std::bad_cast();
	hdr->udp_length	= static_cast<uint16_t>(p.packet_size - sizeof(ipv4_standard_header));
	hdr->compute_udp_csum();
	return next->transmit(p);
}
int protocol_udp::receive(abstract_packet_base& p)
{
	udp_header* hdr				= p.get_as<udp_header>();
	if(__unlikely(!hdr)) return -EPROTOTYPE;
	if(__unlikely(!hdr->verify_udp_csum())) return -EPROTO;
	if(ports.contains(hdr->destination_port))
	{
		protocol_handler& ph    = ports[hdr->destination_port];
		p.packet_type           = ph->packet_type();
		return ph->receive(p);
	}
	return -EPIPE;
}