#include "net/protocol/tcp.hpp"
#include "sched/scheduler.hpp"
#include "sys/errno.h"
#include "ow-crypt.h"
tcp_transmission_timer::tcp_transmission_timer() noexcept : retransmission_timeout(tsci.us_to_tsc(1000000UL)) {}
isn_gen::isn_gen() : selector_clock(cpu_timer_stopwatch::started), selector_crypto_salt(create_hash_setting_string()) {}
void isn_gen::regen_salt() { selector_crypto_salt = std::move(create_hash_setting_string()); }
tcp_session_buffer::tcp_session_buffer() = default;
tcp_header::tcp_header() noexcept { protocol = TCP; }
tcp_header::tcp_header(ipv4_standard_header const& that) noexcept : ipv4_standard_header(that) { protocol = TCP; }
tcp_header::tcp_header(ipv4_standard_header&& that) noexcept : ipv4_standard_header(std::move(that)) { protocol = TCP; }
addr_t tcp_header::payload_start() const { return addr_t(std::addressof(source_port)).plus(fields.data_offset * sizeof(net32)); }
addr_t tcp_header::payload_end() const { return addr_t(this).plus(static_cast<ptrdiff_t>(static_cast<uint16_t>(total_length))); }
bool tcp_header::has_data() const noexcept { return payload_end() != payload_start(); }
size_t tcp_header::segment_len() const { return static_cast<size_t>(std::max(1Z, static_cast<ptrdiff_t>(total_length) - static_cast<ptrdiff_t>(fields.data_offset * sizeof(net32) + sizeof(ipv4_standard_header)))); }
bool tcp_header::src_chk(ipv4_addr addr, uint16_t port) const noexcept { return addr == source_addr && net16(port) == source_port; }
bool tcp_header::seq_chk(uint32_t seq) const noexcept { return net32(seq) == sequence_number; }
bool tcp_header::ack_chk(uint32_t ack) const noexcept { return net32(ack) == ack_number; }
bool tcp_header::is_simple_ack() const noexcept { return fields.ack_flag && !(fields.finish_flag || fields.push_flag || fields.syn_flag) && !has_data(); }
bool tcp_header::peer_chk(tcp_connection_info const& connection_info) const noexcept { return src_chk(connection_info.remote_host, connection_info.remote_port) && (ack_chk(connection_info.next_send_sequence) || ack_chk(connection_info.last_send_sequence)) && seq_chk(connection_info.next_receive_sequence); }
protocol_tcp::protocol_tcp(protocol_ipv4 *n) : abstract_protocol_handler(n), ipconfig(n->client_config), generate_isn() {}
std::type_info const& protocol_tcp::packet_type() const { return typeid(tcp_header); }
std::type_info const& tcp_port_handler::packet_type() const { return typeid(tcp_header); }
int tcp_port_handler::receive(abstract_packet_base& p) { if(__unlikely(!p.get_as<tcp_header>())) throw std::bad_cast(); return rx_process(reinterpret_cast<tcp_packet&>(p)); }
sequence_map::iterator tcp_port_handler::rx_add_packet(tcp_packet& p) { return receive_packets.emplace(std::piecewise_construct, std::forward_as_tuple(p->sequence_number), std::forward_as_tuple(std::move(p))).first; }
std::streamsize tcp_session_buffer::xsputn(const char* s, size_type n)
{
	size_type send_capacity   = __out_region.__capacity();
	size_type send_cur        = static_cast<size_type>(__out_region.__end - __out_region.__begin);
	if(send_cur + n > send_capacity)
	{
		size_type target = std::max(send_capacity << 1, send_cur + n);
		try { size(target, std::ios_base::out); }
		catch(...) { return 0UZ; }
	}
	array_copy(__out_region.__end, s, n);
	__out_region.__adv(n);
	return n;
}
std::streamsize tcp_session_buffer::xsgetn(char* s, size_type n)
{
	size_type avail = static_cast<size_type>(__in_region.__max() - __in_region.__end);
	n               = std::min(n, avail);
	array_copy(s, __in_region.__end, n);
	gbump(n);
	return n;
}
std::streamsize tcp_session_buffer::rx_push(const void* payload_start, const void* payload_end)
{
	std::streamsize result = static_cast<std::streamsize>(addr_t(payload_end) - addr_t(payload_start));
	size_type receive_cur = __in_region.__capacity();
	expand(result, std::ios_base::in);
	array_copy(__in_region.__get_ptr(receive_cur), static_cast<char const*>(payload_start), result);
	return result;
}
void tcp_header::compute_tcp_checksum()
{
	uint32_t intermediate_csum	= 0U;
	size_t tcp_length			= static_cast<size_t>(static_cast<uint16_t>(total_length)) - ihl * sizeof(net32);
	intermediate_csum			+= source_addr.hi;
	intermediate_csum			+= source_addr.lo;
	intermediate_csum			+= destination_addr.hi;
	intermediate_csum			+= destination_addr.lo;
	intermediate_csum			+= protocol;
	intermediate_csum			+= tcp_length;
	net16 const* words			= std::addressof(source_port);
	size_t num_words			= tcp_length / sizeof(net16);
	if((tcp_length % 2) != 0)
		intermediate_csum		+= payload_end().minus(1Z).deref<net8>();
	for(size_t i = 0; i < num_words; i++) intermediate_csum += words[i];
	dword dw_csum				= intermediate_csum;
	intermediate_csum			= dw_csum.hi + dw_csum.lo;
	dw_csum						= intermediate_csum;
	intermediate_csum			= dw_csum.hi + dw_csum.lo;
	tcp_checksum				= net16(~(static_cast<uint16_t>(intermediate_csum)));
}
bool tcp_header::verify_tcp_checksum() const
{
	uint32_t intermediate_csum	= 0U;
	size_t tcp_length			= static_cast<size_t>(static_cast<uint16_t>(total_length)) - ihl * sizeof(net32);
	intermediate_csum			+= source_addr.hi;
	intermediate_csum			+= source_addr.lo;
	intermediate_csum			+= destination_addr.hi;
	intermediate_csum			+= destination_addr.lo;
	intermediate_csum			+= protocol;
	intermediate_csum			+= tcp_length;
	net16 const* words			= std::addressof(source_port);
	size_t num_words			= tcp_length / sizeof(net16);
	if((tcp_length % 2) != 0)
		intermediate_csum		+= payload_end().minus(1Z).deref<net8>();
	for(size_t i = 0; i < num_words; i++) intermediate_csum += words[i];
	dword dw_csum				= intermediate_csum;
	intermediate_csum			= dw_csum.hi + dw_csum.lo;
	dw_csum						= intermediate_csum;
	intermediate_csum			= dw_csum.hi + dw_csum.lo;
	dw_csum						= intermediate_csum;
	return static_cast<uint16_t>(~(dw_csum.lo)) == 0US;
}
constexpr static time_t abs_diff(time_t a, time_t b)
{
	if(a > b)
		return a - b;
	return b - a;
}
void tcp_transmission_timer::update()
{
	time_t r = stopwatch.split();
	if(smoothed_round_trip_time)
	{
		round_trip_time_variation	*= 3;
		round_trip_time_variation	/= 4;
		round_trip_time_variation	+= abs_diff(smoothed_round_trip_time, r) / 4;
		smoothed_round_trip_time	*= 7;
		smoothed_round_trip_time	/= 8;
		smoothed_round_trip_time	+= r / 8;
	}
	else { smoothed_round_trip_time = r; round_trip_time_variation = r / 2; }
	retransmission_timeout 			= smoothed_round_trip_time + std::max(1UL, round_trip_time_variation * 4);
}
uint32_t isn_gen::operator()(ipv4_addr localip, uint16_t localport, ipv4_addr remoteip, uint16_t remoteport) const
{
	std::string keystr          = stringify(localip) + ":" + std::to_string(localport) + "::" + stringify(remoteip) + ":" + std::to_string(remoteport);
	std::string hashstr         = create_crypto_string(keystr, selector_crypto_salt);
	suseconds_t four_usec_ticks = (tsci.tsc_to_us(selector_clock.get())) >> 2;
	return static_cast<uint32_t>((four_usec_ticks + std::elf64_gnu_hash()(hashstr.data())) & 0xFFFFFFFFU);
}
tcp_port_handler::tcp_port_handler(protocol_tcp& tcp, tcp_application& app, uint16_t port) :
	abstract_protocol_handler   { std::addressof(tcp) },
	local_host                  { tcp },
	application                 { app },
	local_port                  { port },
	connection_state            { tcp_connection_state::CLOSED }
								{}
void tcp_port_handler::close()
{
	connection_state = tcp_connection_state::CLOSED;
	new(std::addressof(connection_info)) tcp_connection_info{};
	send_packets.clear();
	receive_packets.clear();
	new(std::addressof(timer)) tcp_transmission_timer();
	data.reset();
}
void tcp_port_handler::open(ipv4_addr peer, uint16_t port, tcp_connection_type local_type, tcp_connection_type remote_type)
{
	new(std::addressof(connection_info)) tcp_connection_info
	{
		.remote_host			{ peer },
		.remote_port			{ port },
		.local_connection_type	{ local_type },
		.remote_connection_type	{ remote_type },
		.initial_send_sequence	{ local_host.generate_isn(local_host.ipconfig.leased_addr, local_port, peer, port) }
	};
	connection_info.current_send_sequence	= connection_info.initial_send_sequence;
	connection_info.next_send_sequence		= connection_info.initial_send_sequence;
}
int tcp_port_handler::rx_process(tcp_packet& p)
{
	using enum tcp_connection_state;
	if(connection_state == CLOSED) return -ENOTCONN;
	if(p->peer_chk(connection_info))
	{
		if(timer.stopwatch)
			timer.update();
		connection_info.peer_window_size = p->window_size;
	}
	if(connection_state == LISTEN)
	{
		if(p->fields.syn_flag && !p->fields.ack_flag)
		{
			if(p->fields.finish_flag || p->fields.reset_flag) return -EPROTO;
			open(p->source_addr, p->source_port, tcp_connection_type::PASSIVE, tcp_connection_type::ACTIVE);
			return rx_initial(p);
		}
	}
	else if(connection_state == SYN_SENT)
	{
		if(p->fields.finish_flag || p->fields.reset_flag) return -EPROTO;
		if(p->src_chk(connection_info.remote_host, connection_info.remote_port) && p->fields.syn_flag)
		{
			connection_info.remote_connection_type = tcp_connection_type::PASSIVE;
			if(p->fields.ack_flag) return rx_establish(p);
			else return rx_initial(p);
		}
	}
	else if(connection_state == SYN_RECEIVED)
	{
		if(p->fields.finish_flag) return -EPROTO;
		if(p->seq_chk(connection_info.next_receive_sequence))
		{
			if(p->fields.reset_flag)
				rx_reset();
			else if(p->fields.ack_flag && p->fields.syn_flag)
				return rx_establish(p);
			else return -EPROTO;
		}
	}
	else if(p->peer_chk(connection_info))
	{
		if(connection_state == ESTABLISHED)
		{
			if(p->fields.finish_flag)
				return rx_begin_close(p);
			else return rx_accept_payload(p);
		}
		else if(connection_state == FIN_WAIT_1)
		{
			if(p->fields.finish_flag)
				return tx_simultaneous_close();
			connection_state = FIN_WAIT_2;
			return rx_accept_payload(p);
		}
		else if(connection_state == FIN_WAIT_2)
		{
			if(p->fields.finish_flag)
				return set_await_close();
			else return rx_accept_payload(p);
		}
		else if(connection_state == CLOSING) return set_await_close();
		else if(connection_state == LAST_ACK) close();
	}
	return 0;
}
tcp_packet& tcp_port_handler::create_packet(size_t payload_size, size_t option_size, uint16_t window_size)
{
	size_t padded_option_size					= option_size ? up_to_nearest(option_size + 1UZ, 4UZ) : 0UZ;
	abstract_ip_resolver& res					= *base->ip_resolver;
	mac_t remote_mac							= res[res.check_presence(connection_info.remote_host) ? connection_info.remote_host : local_host.ipconfig.primary_gateway];
	sequence_map::iterator i					= send_packets.emplace(std::piecewise_construct, std::forward_as_tuple(connection_info.next_send_sequence), std::forward_as_tuple(sizeof(tcp_header) + padded_option_size + payload_size, std::in_place_type<tcp_header>, std::forward<ipv4_standard_header>(base->create_packet(remote_mac)))).first;
	if(padded_option_size)
	{
		addr_t option_start						= i->second->options;
		option_start.plus(padded_option_size - 1Z).assign(0UC);
		if(option_size != padded_option_size) array_fill(option_start.plus(option_size), 1UC, static_cast<size_t>(padded_option_size - option_size - 1Z));
	}
	i->second->total_length						= static_cast<uint16_t>(i->second.packet_size);
	i->second->sequence_number					= i->first;
	i->second->ack_number						= connection_info.next_receive_sequence;
	i->second->window_size						= window_size;
	i->second->fields.data_offset				= static_cast<net8>((sizeof(tcp_header) - sizeof(ipv4_standard_header) + padded_option_size) / sizeof(net32));
	i->second->source_addr						= local_host.ipconfig.leased_addr;
	i->second->source_port						= local_port;
	i->second->destination_addr					= connection_info.remote_host;
	i->second->destination_port					= connection_info.remote_port;
	connection_info.current_send_sequence		= i->first;
	connection_info.next_send_sequence			= static_cast<uint32_t>(i->first + i->second->segment_len());
	return i->second;
}
int tcp_port_handler::rx_initial(tcp_packet& p)
{
	using enum tcp_connection_state;
	try
	{
		connection_info.peer_window_size				= p->window_size;
		if(p->has_data())
		{
			sequence_map::iterator i					= rx_add_packet(p);
			connection_info.initial_receive_sequence	= i->first;
			connection_info.current_receive_sequence	= i->first;
			connection_info.receive_commit_sequence		= i->first;
			connection_info.next_receive_sequence		= static_cast<uint32_t>(i->first + i->second->segment_len());
		}
		else
		{
			connection_info.initial_receive_sequence	= p->sequence_number;
			connection_info.current_receive_sequence	= p->sequence_number;
			connection_info.next_receive_sequence		= p->sequence_number + 1U;
		}
		tcp_packet& ack_packet							= create_packet(0UZ);
		ack_packet->fields.ack_flag						= true;
		ack_packet->fields.syn_flag						= true;
		ack_packet->compute_tcp_checksum();
		if(int err = tx_send_next(); __unlikely(err != 0)) return err;
		connection_state								= SYN_RECEIVED;
	}
	catch(std::bad_alloc&) { return -ENOMEM; }
	return 0;
}
int tcp_port_handler::tx_send_next()
{
	if(!send_packets.contains(connection_info.current_send_sequence))
		return -ENOTCONN;
	if(!timer.stopwatch)
		timer.stopwatch.start();
	connection_info.last_send_sequence = connection_info.current_send_sequence;
	return local_host.next->transmit(send_packets[connection_info.current_send_sequence]);
}
int tcp_port_handler::rx_establish(tcp_packet& p)
{
	using enum tcp_connection_state;
	if(p->ack_chk(connection_info.next_send_sequence)) try
	{
		bool is_first = !connection_info.initial_receive_sequence;
		if(is_first)   // first received packet; this occurs when the local host initiated the connection to a passive peer
		{
			connection_info.initial_receive_sequence	= p->sequence_number;
			connection_info.current_receive_sequence	= p->sequence_number;
			connection_info.next_receive_sequence		= p->sequence_number;
			connection_info.peer_window_size			= p->window_size;
		}
		if(is_first || p->seq_chk(p->fields.ack_flag ? connection_info.current_receive_sequence : connection_info.next_receive_sequence))
		{
			connection_info.current_receive_sequence	= connection_info.next_receive_sequence;
			connection_info.next_receive_sequence		+= p->segment_len();
			if(p->has_data()) rx_add_packet(p);
			rx_commit();
			tcp_packet& ack_packet						= create_packet(0UZ);
			ack_packet->fields.ack_flag					= true;
			ack_packet->fields.syn_flag					= true;
			ack_packet->compute_tcp_checksum();
			if(int err = tx_send_next(); __unlikely(err != 0)) return err;
			connection_state							= ESTABLISHED;
		}
	}
	catch(std::bad_alloc&) { return -ENOMEM; }
	else return tx_reset(p->ack_number);
	return 0;
}
void tcp_port_handler::rx_commit()
{
	if(connection_info.receive_commit_sequence)
	{
		for(sequence_map::iterator i = receive_packets.find(connection_info.receive_commit_sequence); i != receive_packets.end(); i = receive_packets.find(connection_info.receive_commit_sequence))
		{
			if(i->second->has_data())
				data.rx_push(i->second->payload_start(), i->second->payload_end());
			connection_info.receive_commit_sequence += i->second->segment_len();
		}
	}
}
void tcp_port_handler::rx_reset()
{
	send_packets.clear();
	receive_packets.clear();
	new(std::addressof(timer)) tcp_transmission_timer();
	data.reset();
	ipv4_addr remote			= connection_info.remote_host;
	uint16_t port				= connection_info.remote_port;
	tcp_connection_type rtype	= connection_info.remote_connection_type, ltype = connection_info.local_connection_type;
	new(std::addressof(connection_info)) tcp_connection_info
	{
		.remote_host			{ remote },
		.remote_port			{ port },
		.local_connection_type	{ ltype },
		.remote_connection_type	{ rtype }
	};
	connection_state			= tcp_connection_state::LISTEN;
}
int tcp_port_handler::rx_accept_payload(tcp_packet& p)
{
	if(!p->is_simple_ack()) try
	{
		if(p->has_data())
		{
			sequence_map::iterator i = rx_add_packet(p);
			if(!connection_info.receive_commit_sequence) connection_info.receive_commit_sequence = i->first;
			if(i->second->fields.push_flag) rx_commit();
		}
		if(int err = tx_ack(); __unlikely(err != 0)) return err;
	}
	catch(std::bad_alloc&) { return -ENOMEM; }
	else connection_info.last_send_sequence = compute_following_sequence(connection_info.last_send_sequence);
	return 0;
}
uint32_t tcp_port_handler::compute_following_sequence(uint32_t from) const
{
	sequence_map::const_iterator i = send_packets.find(from);
	if(__unlikely(i == send_packets.end())) throw std::invalid_argument("[tcp] no packet with that sequence");
	return i->first + static_cast<uint32_t>(i->second->segment_len());
}
int tcp_port_handler::rx_begin_close(tcp_packet& p)
{
	// TODO: notify the user that the other side has requested to close
	connection_state = tcp_connection_state::CLOSE_WAIT;
	return rx_accept_payload(p);
}
int tcp_port_handler::tx_reset(uint32_t use_seq)
{
	abstract_ip_resolver& res		= *base->ip_resolver;
	mac_t remote_mac				= res[res.check_presence(connection_info.remote_host) ? connection_info.remote_host : local_host.ipconfig.primary_gateway];
	sequence_map::iterator i		= send_packets.emplace(std::piecewise_construct, std::forward_as_tuple(connection_info.next_send_sequence), std::forward_as_tuple(sizeof(tcp_header), std::in_place_type<tcp_header>, std::forward<ipv4_standard_header>(base->create_packet(remote_mac)))).first;
	i->second->total_length			= static_cast<uint16_t>(i->second.packet_size);
	i->second->sequence_number		= use_seq;
	i->second->window_size			= 16384USBE;
	i->second->fields.data_offset	= static_cast<net8>((sizeof(tcp_header) - sizeof(ipv4_standard_header)) / sizeof(net32));
	i->second->source_addr			= local_host.ipconfig.leased_addr;
	i->second->source_port			= local_port;
	i->second->destination_addr		= connection_info.remote_host;
	i->second->destination_port		= connection_info.remote_port;
	if(int err = next->transmit(i->second); __unlikely(err != 0)) return err;
	// TODO: we might need to retransmit the correct sequence...add that here if applicable
	return 0;
}
int tcp_port_handler::tx_ack()
{
	tcp_packet& ack_packet		= create_packet(0UZ);
	ack_packet->fields.ack_flag	= true;
	ack_packet->compute_tcp_checksum();
	if(int err = tx_send_next(); __unlikely(err != 0)) return err;
	return 0;
}
int tcp_port_handler::tx_begin_close()
{
	tcp_packet& fin_packet			= create_packet(0UZ);
	fin_packet->fields.ack_flag		= true;
	fin_packet->fields.finish_flag	= true;
	fin_packet->compute_tcp_checksum();
	if(int err = tx_send_next(); __unlikely(err != 0)) return err;
	connection_state = tcp_connection_state::FIN_WAIT_1;
	return 0;
}
int tcp_port_handler::tx_simultaneous_close()
{
	if(int err = tx_ack(); __unlikely(err != 0)) return err;
	connection_state = tcp_connection_state::CLOSING;
	return 0;
}
int tcp_port_handler::set_await_close()
{
	connection_state = tcp_connection_state::TIME_WAIT;
	scheduler::defer_sec(2UL, std::bind(&tcp_port_handler::close, this));
	return 0;
}