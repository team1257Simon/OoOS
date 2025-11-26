#ifndef __TRANSMISSION_CONTROL_PROTOCOL
#define __TRANSMISSION_CONTROL_PROTOCOL
#include <net/protocol/ipv4.hpp>
#include <arch/cpu_time.hpp>
#include <map>
#include <unordered_set>
class tcp_session_buffer : public std::ext::dynamic_duplex_streambuf<char>
{
	typedef std::ext::dynamic_duplex_streambuf<char> __base;
protected:
	virtual std::streamsize xsputn(const char* s, size_type n) override;
	virtual std::streamsize xsgetn(char* s, size_type n) override;
public:
	tcp_session_buffer();
	std::streamsize rx_push(const void* payload_start, const void* payload_end);
};
union __pack tcp_fields_word
{
	net16 full_word{};
	struct __pack
	{
		net8 data_offset	: 4;	// offset of the payload in 32-bit dwords
		bool				: 4;
		bool cwr_flag		: 1;	// congestion window reduced
		bool ece_flag		: 1;	// explicit congestion notification enable
		bool urgent_flag	: 1;
		bool ack_flag		: 1;
		bool push_flag		: 1;
		bool reset_flag		: 1;
		bool syn_flag		: 1;
		bool finish_flag	: 1;
	};
};
struct tcp_connection_info;
struct __pack tcp_header : ipv4_standard_header
{
	net16 source_port;
	net16 destination_port;
	net32 sequence_number;
	net32 ack_number;
	tcp_fields_word fields;
	net16 window_size;
	net16 tcp_checksum;
	net16 urgent_pointer;
	net32 options[];
	tcp_header() noexcept;
	tcp_header(ipv4_standard_header const& that) noexcept;
	tcp_header(ipv4_standard_header&& that) noexcept;
	addr_t payload_start() const;
	addr_t payload_end() const;
	size_t segment_len() const;
	bool has_data() const noexcept;
	bool is_simple_ack() const noexcept;
	void compute_tcp_checksum();
	bool verify_tcp_checksum() const;
	bool src_chk(ipv4_addr addr, uint16_t port) const noexcept;
	bool seq_chk(uint32_t seq) const noexcept;
	bool ack_chk(uint32_t ack) const noexcept;
	bool peer_chk(tcp_connection_info const& connection_info) const noexcept;
};
typedef abstract_packet<tcp_header> tcp_packet;
struct tcp_transmission_timer
{
	time_t retransmission_timeout;
	time_t smoothed_round_trip_time;
	time_t round_trip_time_variation;
	cpu_timer_stopwatch stopwatch;
	tcp_transmission_timer() noexcept;
	void update();
};
enum class tcp_connection_state : char
{
	LISTEN,
	SYN_SENT,
	SYN_RECEIVED,
	ESTABLISHED,
	FIN_WAIT_1,
	FIN_WAIT_2,
	CLOSE_WAIT,
	CLOSING,
	LAST_ACK,
	TIME_WAIT,
	CLOSED,
	MAX			= CLOSED
};
enum class tcp_connection_type : bool { PASSIVE, ACTIVE };
struct protocol_tcp;
typedef std::map<uint32_t, tcp_packet> sequence_map;
struct tcp_application
{
	// TODO
};
struct tcp_connection_info
{
	ipv4_addr remote_host;
	uint16_t remote_port;
	tcp_connection_type local_connection_type;
	tcp_connection_type remote_connection_type;
	uint32_t initial_send_sequence;
	uint32_t last_send_sequence;
	uint32_t current_send_sequence;
	uint32_t next_send_sequence;
	uint32_t initial_receive_sequence;
	uint32_t current_receive_sequence;
	uint32_t next_receive_sequence;
	uint32_t receive_commit_sequence;
	size_t peer_window_size;
};
struct tcp_port_handler : abstract_protocol_handler
{
	protocol_tcp& local_host;
	tcp_application& application;
	uint16_t local_port;
	tcp_connection_state connection_state;
	tcp_connection_info connection_info;
	sequence_map send_packets;
	sequence_map receive_packets;
	tcp_transmission_timer timer;
	tcp_session_buffer data;
	tcp_port_handler(protocol_tcp& tcp, tcp_application& app, uint16_t port);
	virtual std::type_info const& packet_type() const override;
	virtual int receive(abstract_packet_base& p) override;
	tcp_packet& create_packet(size_t payload_size, size_t option_size = 0UZ, uint16_t window_size = 16384US);
	uint32_t compute_following_sequence(uint32_t from) const;
	void open(ipv4_addr peer, uint16_t port, tcp_connection_type local_type, tcp_connection_type remote_type);
	void close();
	int set_await_close();
	int rx_process(tcp_packet& p);
	int rx_initial(tcp_packet& p);
	int rx_establish(tcp_packet& p);
	int rx_accept_payload(tcp_packet& p);
	int rx_begin_close(tcp_packet& p);
	sequence_map::iterator rx_add_packet(tcp_packet& p);
	void rx_commit();
	void rx_reset();
	int tx_ack();
	int tx_reset(uint32_t use_seq);
	int tx_send_next();
	int tx_begin_close();
	int tx_simultaneous_close();
};
struct isn_gen
{
	cpu_timer_stopwatch selector_clock;
	std::string selector_crypto_salt;
	isn_gen();
	void regen_salt();
	uint32_t operator()(ipv4_addr localip, uint16_t localport, ipv4_addr remoteip, uint16_t remoteport) const;
};
struct protocol_tcp : abstract_protocol_handler
{
	ipv4_config& ipconfig;
	isn_gen generate_isn;
	protocol_tcp(protocol_ipv4* n);
	virtual std::type_info const& packet_type() const override;
};
#endif