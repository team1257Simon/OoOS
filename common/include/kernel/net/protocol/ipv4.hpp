#ifndef __IPV4
#define __IPV4
#include <net/netdev_module.hpp>
#include <bits/hash_set.hpp>
enum ecn_t : net8
{
	NON_ECT = 0b00UC, // not ECN-capable
	ECN_1   = 0b01UC, // ECN capable, flag 1
	ECN_0   = 0b10UC, // ECN capable, flag 0
	ECN_CE  = 0b11UC  // Congestion Experienced
};
enum dscp_t : net8
{
	DS_STANDARD				= 0b000000UC,
	DS_LOWER_EFFORT			= 0b000001UC,
	DS_HIGH_THROUGHPUT_AF11	= 0b001010UC,
	DS_HIGH_THROUGHPUT_AF12	= 0b001100UC,
	DS_HIGH_THROUGHPUT_AF13	= 0b001110UC,
	DS_OAM					= 0b010000UC,
	DS_LOW_LATENCY_AF21		= 0b010010UC,
	DS_LOW_LATENCY_AF22		= 0b010100UC,
	DS_LOW_LATENCY_AF23		= 0b010110UC,
	DS_BROADCAST_VIDEO		= 0b011000UC,
	DS_STREAMING_AF31		= 0b011010UC,
	DS_STREAMING_AF32		= 0b011100UC,
	DS_STREAMING_AF33		= 0b011110UC,
	DS_REALTIME_INTERACTIVE	= 0b100000UC,
	DS_CONFERENCING_AF41	= 0b100010UC,
	DS_CONFERENCING_AF42	= 0b100100UC,
	DS_CONFERENCING_AF43	= 0b100110UC,
	DS_SIGNALING			= 0b101000UC,
	DS_TELEPHONE			= 0b101110UC,
	DS_NETWORK_CTL			= 0b110000UC
};
enum ipver_t : net8
{
	IPV4 = 0x4,
	// no others are valid here
};
enum ihl_t : net8
{
	IHL20B	= 0x5,
	IHL24B	= 0x6,
	IHL28B	= 0x7,
	IHL32B	= 0x8,
	IHL36B	= 0x9,
	IHL40B	= 0xA,
	IHL44B	= 0xB,
	IHL48B	= 0xC,
	IHL52B	= 0xD,
	IHL56B	= 0xE,
	IHL60B	= 0xF
};
constexpr inline net16 fragment_offset_mask	= 0x1FFFUSBE;
constexpr inline net16 no_fragment_bit		= 0x2000USBE;
constexpr inline net16 more_fragments_bit	= 0x4000USBE;
template<ihl_t L>
struct __pack ipv4_header : ethernet_header
{
	struct attribute(packed, aligned(1))
	{
		ihl_t ihl		: 4 = L;			// value is header length in dwords; almost always 20 bytes or 5 dwords
		ipver_t version	: 4 = IPV4;
		dscp_t dscp		: 6 = DS_STANDARD;	// differentiated services codepoint
		ecn_t ecn		: 2 = NON_ECT;		// explicit congestion notification
	};
	net16 total_length;						// length of the whole packet
	net16 identification	= 0USBE;
	net16 fragment_info		= 0USBE;		// 0b0DMXXXXXXXXXXXXX; D = don't fragment, M = more fragments remain, X... = fragment offset
	net8 time_to_live;
	ipv4_transport_protocol protocol;
	net16 header_checksum;
	net32 source_addr;
	net32 destination_addr;					// intended recipient address; the destination MAC address will be for the gateway if this is not on the local network
	net32 options[static_cast<uint8_t>(L) - 0x5UC];
	constexpr ipv4_header() noexcept = default;
	constexpr ipv4_header(ethernet_header const& ep) noexcept : ethernet_header(ep) { protocol_type = ethertype_ipv4; }
	constexpr ipv4_header(ethernet_header&& ep) noexcept : ethernet_header(std::move(ep)) { protocol_type = ethertype_ipv4; }
	constexpr void compute_ipv4_csum()
	{
		constexpr size_t num_words	= static_cast<size_t>(L) * sizeof(uint32_t) / sizeof(uint16_t);
		header_checksum				= 0USBE;
		net16* words				= addr_t(std::addressof(total_length)).minus(sizeof(net16));
		uint32_t intermediate_csum	= 0U;
		for(size_t i = 0; i < num_words; i++) intermediate_csum += words[i];
		dword dw_csum		= intermediate_csum;
		intermediate_csum	= dw_csum.hi + dw_csum.lo;
		dw_csum				= intermediate_csum;
		intermediate_csum	= dw_csum.hi + dw_csum.lo;
		header_checksum		= net16(~(static_cast<uint16_t>(intermediate_csum)));
	}
	constexpr bool verify_ipv4_csum() const
	{
		constexpr size_t num_words	= static_cast<size_t>(L) * sizeof(uint32_t) / sizeof(uint16_t);
		net16 const* words			= addr_t(std::addressof(total_length)).minus(sizeof(net16));
		uint32_t intermediate_csum	= 0U;
		for(size_t i = 0; i < num_words; i++) intermediate_csum += words[i];
		dword dw_csum		= intermediate_csum;
		intermediate_csum	= dw_csum.hi + dw_csum.lo;
		dw_csum				= intermediate_csum;
		intermediate_csum	= dw_csum.hi + dw_csum.lo;
		dw_csum				= intermediate_csum;
		return static_cast<uint16_t>(~(dw_csum.lo)) == 0US;
	}
};
#ifndef IP_INST
extern template struct ipv4_header<IHL20B>;
extern template class abstract_packet<ipv4_header<IHL20B>>;
typedef abstract_packet<ipv4_header<IHL20B>> ipv4_packet;
typedef ipv4_header<IHL20B> ipv4_standard_header;
extern template abstract_packet<ipv4_header<IHL20B>>::abstract_packet();
extern template abstract_packet<ipv4_header<IHL20B>>::abstract_packet(ethernet_header const&);
extern template abstract_packet<ipv4_header<IHL20B>>::abstract_packet(ethernet_header&&);
extern template abstract_packet<ipv4_header<IHL20B>>::abstract_packet(ipv4_header<IHL20B> const&);
extern template abstract_packet<ipv4_header<IHL20B>>::abstract_packet(ipv4_header<IHL20B>&&);
extern template abstract_packet<ipv4_header<IHL20B>>::abstract_packet(size_t, std::in_place_type_t<ipv4_header<IHL20B>>);
extern template abstract_packet<ipv4_header<IHL20B>>::abstract_packet(size_t, std::in_place_type_t<ipv4_header<IHL20B>>, ethernet_header const&);
extern template abstract_packet<ipv4_header<IHL20B>>::abstract_packet(size_t, std::in_place_type_t<ipv4_header<IHL20B>>, ethernet_header&&);
extern template abstract_packet<ipv4_header<IHL20B>>::abstract_packet(size_t, std::in_place_type_t<ipv4_header<IHL20B>>, ipv4_header<IHL20B> const&);
extern template abstract_packet<ipv4_header<IHL20B>>::abstract_packet(size_t, std::in_place_type_t<ipv4_header<IHL20B>>, ipv4_header<IHL20B>&&);
#endif
typedef protocol_handler_map<ipv4_transport_protocol> transport_map;
typedef std::hash_set<ipv4_addr, uint32_t, cast_t<uint32_t, size_t>, equals_t, std::allocator<ipv4_addr>, cast_t<ipv4_addr, uint32_t>> ipv4_addr_set;
struct protocol_ipv4 final : abstract_protocol_handler
{
	transport_map transports;
	ipv4_config client_config;
	protocol_ipv4(protocol_ethernet* eth);
	virtual ~protocol_ipv4();
	virtual int transmit(abstract_packet_base& p) override;
	virtual int receive(abstract_packet_base& p) override;
	virtual std::type_info const& packet_type() const override;
	protocol_handler& add_transport(ipv4_transport_protocol id, protocol_handler&& ph);
	template<std::derived_from<abstract_protocol_handler> PT> requires std::constructible_from<PT, protocol_ipv4*> PT& add_transport_handler(ipv4_transport_protocol id) { return add_transport(id, std::move(create_handler<PT>(this))).template cast<PT>();  }
};
#endif