#define DHCP_INST
#include <net/protocol/dhcp.hpp>
#include <sys/errno.h>
#include <stdlib.h> // rand()
constexpr static std::allocator<net16> udp_pseudo_alloc;
constexpr const char must_decline_msg[] = "That address is already in use by another client.";
template<> template<> abstract_packet<dhcp_packet>::abstract_packet(udp_header&& that) : abstract_packet(static_cast<size_t>(that.total_length), std::in_place_type<dhcp_packet>, std::move(that)) {}
template<> template<> abstract_packet<dhcp_packet>::abstract_packet(udp_header const& that) : abstract_packet(static_cast<size_t>(that.total_length), std::in_place_type<dhcp_packet>, that) {}
template<> template<> abstract_packet<dhcp_packet>::abstract_packet(dhcp_packet&& that) : abstract_packet(static_cast<size_t>(that.total_length), std::in_place_type<dhcp_packet>, std::move(that)) {}
template<> template<> abstract_packet<dhcp_packet>::abstract_packet(dhcp_packet const& that) : abstract_packet(static_cast<size_t>(that.total_length), std::in_place_type<dhcp_packet>, that) {}
template class abstract_packet<dhcp_packet>;
template abstract_packet<dhcp_packet>::abstract_packet(udp_header&&);
template abstract_packet<dhcp_packet>::abstract_packet(udp_header const&);
template abstract_packet<dhcp_packet>::abstract_packet(dhcp_packet&&);
template abstract_packet<dhcp_packet>::abstract_packet(dhcp_packet const&);
template abstract_packet<dhcp_packet>::abstract_packet(size_t, std::in_place_type_t<dhcp_packet>, dhcp_packet const&);
template abstract_packet<dhcp_packet>::abstract_packet(size_t, std::in_place_type_t<dhcp_packet>, dhcp_packet&&);
template abstract_packet<dhcp_packet>::abstract_packet(size_t, std::in_place_type_t<dhcp_packet>);
template abstract_packet<dhcp_packet>::abstract_packet(size_t, std::in_place_type_t<dhcp_packet>, udp_header&&);
template abstract_packet<dhcp_packet>::abstract_packet(size_t, std::in_place_type_t<dhcp_packet>, udp_header const&);
dhcp_packet::dhcp_packet() noexcept = default;
std::type_info const& protocol_dhcp::packet_type() const { return typeid(dhcp_packet); }
protocol_dhcp::~protocol_dhcp() = default;
protocol_dhcp::protocol_dhcp(protocol_udp* n) : abstract_protocol_handler(n), ipconfig(n->ipconfig), ipresolve(*n->base->ip_resolver), transaction_timers(32UZ) {}
int protocol_dhcp::rebind() { return request(ipconfig.leased_addr, active_renewal_xid); }
dhcp_packet::dhcp_packet(udp_header const& that) noexcept : udp_header(that) {
	source_port			= dhcp_client_port;
	destination_port	= dhcp_server_port;
}
dhcp_packet::dhcp_packet(udp_header&& that) noexcept : udp_header(std::move(that)) {
	source_port			= dhcp_client_port;
	destination_port	= dhcp_server_port;
}
int protocol_dhcp::receive(abstract_packet_base& p)
{
	dhcp_packet* pkt	= p.get_as<dhcp_packet>();
	if(__unlikely(!pkt)) return -EPROTOTYPE;
	addr_t pos			= pkt->parameters;
	while(pos.deref<net8>() != 0xFFUC)
	{
		if(!pos.deref<net8>()) pos		+= 1Z;
		else
		{
			dhcp_parameter* param		= pos;
			if(param->type_code == MESSAGE_TYPE)
			{
				dhcp_message_type type	= param->start().deref<dhcp_message_type>();
				switch(type)
				{
				case OFFER:
					return process_offer_packet(*pkt);
				case NAK:
					return transition_state(ipv4_client_state::INIT);
				case ACK:
					return process_ack_packet(*pkt);
				default:
					return 0;   // nothing to do for any other packet types (for now)
				}
			}
			else pos					+= param->length();
		}
	}
	return -EPROTO;
}
abstract_packet<dhcp_packet> protocol_dhcp::create_packet(mac_t const& dest_mac, ipv4_addr dest_ip, size_t total_size, uint32_t xid)
{
	abstract_packet<dhcp_packet> result(total_size, std::in_place_type<dhcp_packet>, std::forward<ipv4_standard_header>(base->create_packet(dest_mac)));
	if(transaction_timers.contains(xid))
		result->seconds			= static_cast<uint16_t>(sys_time(nullptr) - transaction_timers[xid]);
	result->transaction_id		= xid;
	result->destination_addr	= dest_ip;
	result->operation			= BOOTREQUEST;
	result->source_addr			= ipconfig.leased_addr;
	result->client_ip			= ipconfig.leased_addr;
	result->relay_ip			= empty;
	result->server_ip			= ipconfig.dhcp_server_addr;
	array_copy(result->client_hw, base->mac_addr.data(), sizeof(mac_t));
	return result;
}
void protocol_dhcp::discover(std::vector<net8> const& param_requests)
{
	size_t num_requests							= param_requests.size();
	size_t total_param_size						= (num_requests + 2UZ) + 4UZ; // 2 bytes for the request list type and size, 3 bytes for the message type, 1 byte for the EOT mark
	size_t target_size							= total_dhcp_size(total_param_size);
	size_t actual_size							= up_to_nearest(target_size, 2UZ);
	uint32_t xid								= static_cast<uint32_t>(rand());
	while(transaction_timers.contains(xid)) xid	= static_cast<uint32_t>(rand());
	abstract_packet<dhcp_packet> discover_pkt	= create_packet(broadcast_mac, broadcast, actual_size, xid);
	transaction_timers[xid]						= sys_time(nullptr);
	addr_t pos									= discover_pkt->parameters;
	pos.plus(total_param_size - 1Z).assign(0xFFUC);
	dhcp_parameter* param						= pos;
	param->type_code							= MESSAGE_TYPE;
	param->length()								= 1UC;
	param->start().assign(DISCOVER);
	if(num_requests)
	{
		pos										+= 3Z;
		param									= pos;
		param->type_code						= PARAMETER_REQUEST_LIST;
		param->length()							= num_requests;
		array_copy(param->start(), param_requests.data(), num_requests);
	}
	if(next->transmit(discover_pkt) != 0) throw std::runtime_error("[DHCP] packet transmission failed");
}
int protocol_dhcp::process_offer_packet(dhcp_packet const& p)
{
	if(ipconfig.current_state == ipv4_client_state::INIT && transaction_timers.contains(p.transaction_id))
	{
		ipv4_addr req_addr			= p.your_ip;
		ipconfig.dhcp_server_addr	= p.server_ip;
		addr_t pos					= p.parameters;
		transition_state(ipv4_client_state::SELECTING);
		try { while(pos.deref<dhcp_parameter_type>() != END_OF_TRANSMISSION) pos += process_packet_parameter(pos.deref<dhcp_parameter>()); }
		catch(std::invalid_argument& e) { panic(e.what()); return -EPROTO; }
		catch(std::bad_alloc&)          { return -ENOMEM; }
		if(__unlikely(base->ip_resolver->check_presence(req_addr)))
		{
			if(int err = decline(req_addr, p.transaction_id); __unlikely(err != 0)) return err;
			try                             { discover(std::vector<net8>()); }
			catch(std::runtime_error& e)    { panic(e.what()); return -ENETDOWN; }
			catch(std::bad_alloc&)          { return -ENOMEM; }
		}
		else if(int err = request(req_addr, p.transaction_id); __unlikely(err != 0)) return err;
	}
	return 0;
}
int protocol_dhcp::process_ack_packet(dhcp_packet const& p)
{
	sys_time(std::addressof(ipconfig.lease_acquired_time));
	ipconfig.leased_addr	= p.your_ip;
	addr_t pos				= p.parameters;
	try { while(pos.deref<dhcp_parameter_type>() != END_OF_TRANSMISSION) pos += process_packet_parameter(pos.deref<dhcp_parameter>()); }
	catch(std::invalid_argument& e) { panic(e.what()); return -EPROTO; }
	catch(std::bad_alloc&)          { return -ENOMEM; }
	if(__unlikely(!ipconfig.lease_duration)) return -EPROTO;
	if(ipconfig.lease_duration != 0xFFFFFFFFU)
	{
		if(!ipconfig.lease_renew_time)
			ipconfig.lease_renew_time	= ipconfig.lease_duration / 2;
		if(!ipconfig.lease_rebind_time)
			ipconfig.lease_rebind_time	= (7 * ipconfig.lease_duration) / 8;
	}
	transition_state(ipv4_client_state::BOUND);
	transaction_timers.erase(p.transaction_id);
	return 0;
}
net8 protocol_dhcp::process_packet_parameter(dhcp_parameter const& param)
{
	switch(param.type_code)
	{
	case PAD:
		return 1UC;
	case END_OF_TRANSMISSION:
		return 0UC;
	case SUBNET_MASK:
		ipconfig.subnet_mask				= param.start().deref<net32>();
		break;
	case SERVER_IDENTIFIER:
		if(!ipconfig.dhcp_server_addr)
			ipconfig.dhcp_server_addr		= param.start().deref<net32>();
		break;
	case ROUTER:
		if(param.length() % 4UC) throw std::invalid_argument("[DHCP] malformed packet");
		ipconfig.primary_gateway			= param.start().deref<net32>();
		if(param.length() > 4UC)
			ipconfig.alternate_gateway		= param.start().as<net32>()[1];
		break;
	case DOMAIN_NAME_SERVER:
		if(param.length() % 4UC) throw std::invalid_argument("[DHCP] malformed packet");
		ipconfig.primary_dns_server			= param.start().deref<net32>();
		if(param.length() > 4UC)
			ipconfig.alternate_dns_server	= param.start().as<net32>()[1];
		break;
	case DEFAULT_TTL:
		ipconfig.time_to_live_default		= param.start().deref<net8>();
		break;
	case RENEWAL_TIME_VALUE:
		if(ipconfig.current_state != ipv4_client_state::BOUND)
			ipconfig.lease_renew_time		= param.start().deref<net32>();
		break;
	case REBINDING_TIME_VALUE:
		if(ipconfig.current_state != ipv4_client_state::BOUND)
			ipconfig.lease_rebind_time		= param.start().deref<net32>();
		break;
	case IP_LEASE_TIME:
		if(ipconfig.current_state != ipv4_client_state::BOUND)
			ipconfig.lease_duration			= param.start().deref<net32>();
		break;
	default:
		break;
	// ...
	}
	return param.length() + 2UC;
}
int protocol_dhcp::decline(ipv4_addr addr, uint32_t xid)
{
	constexpr size_t total_param_size			= (sizeof(must_decline_msg) + 2UZ) + (sizeof(ipv4_addr) + 2UZ) * 2UZ + 4UZ;
	constexpr size_t target_size				= total_dhcp_size(total_param_size);
	constexpr size_t actual_size				= up_to_nearest(target_size, 2UZ);
	abstract_packet<dhcp_packet> decline_pkt	= create_packet(broadcast_mac, broadcast, actual_size, xid);
	decline_pkt->seconds						= 0USBE;
	addr_t pos									= decline_pkt->parameters;
	pos.plus(total_param_size - 1Z).assign(0xFFUC);
	dhcp_parameter* param						= pos;
	param->type_code							= MESSAGE_TYPE;
	param->length()								= 1UC;
	param->start().assign(DECLINE);
	pos											+= 3Z;
	param										= pos;
	param->type_code							= SERVER_IDENTIFIER;
	param->length()								= 4UC;
	param->start().assign(ipconfig.dhcp_server_addr);
	pos											+= 6Z;
	param										= pos;
	param->type_code							= REQUESTED_IP;
	param->length()								= 4UC;
	param->start().assign(addr);
	pos											+= 6Z;
	param										= pos;
	param->type_code							= MESSAGE;
	array_copy(pos.as<char>(), must_decline_msg, sizeof(must_decline_msg));
	return next->transmit(decline_pkt);
}
int protocol_dhcp::request(ipv4_addr addr, uint32_t xid)
{
	size_t total_param_size							= (sizeof(ipv4_addr) + 2UZ) * (ipconfig.current_state == ipv4_client_state::SELECTING ? 2UZ : ipconfig.current_state == ipv4_client_state::REBOOT ? 1UZ : 0UZ) + 4UZ;
	size_t actual_size								= total_dhcp_size(total_param_size);    // the above number cannot possibly be odd; it will be either 4, 10, or 16
	abstract_packet<dhcp_packet> request_pkt		= create_packet(broadcast_mac, broadcast, actual_size, xid);
	addr_t pos										= request_pkt->parameters;
	pos.plus(total_param_size - 1Z).assign(0xFFUC);
	dhcp_parameter* param							= pos;
	param->type_code								= MESSAGE_TYPE;
	param->length()									= 1UC;
	param->start().assign(REQUEST);
	if(ipconfig.current_state == ipv4_client_state::SELECTING || ipconfig.current_state == ipv4_client_state::REBOOT)
	{
		if(ipconfig.current_state == ipv4_client_state::SELECTING)
		{
			pos										+= 3Z;
			param									= pos;
			param->type_code						= SERVER_IDENTIFIER;
			param->length()							= 4UC;
			param->start().assign(ipconfig.dhcp_server_addr);
		}
		pos											+= 6Z;
		param										= pos;
		param->type_code							= REQUESTED_IP;
		param->length()								= 4UC;
		param->start().assign(addr);
	}
	return next->transmit(request_pkt);
}
int protocol_dhcp::renew()
{
	if(__unlikely(!ipconfig.leased_addr)) return -EINVAL;
	constexpr size_t total_param_size 		= 4UZ;
	constexpr size_t actual_size			= total_dhcp_size(total_param_size);
	mac_t server_mac						= ipresolve[ipconfig.dhcp_server_addr];
	abstract_packet<dhcp_packet> renew_pkt	= create_packet(server_mac, ipconfig.dhcp_server_addr, actual_size, active_renewal_xid);
	addr_t pos								= renew_pkt->parameters;
	pos.plus(total_param_size - 1Z).assign(0xFFUC);
	dhcp_parameter* param					= pos;
	param->type_code						= MESSAGE_TYPE;
	param->length()							= 1UC;
	param->start().assign(REQUEST);
	return next->transmit(renew_pkt);
}
void protocol_dhcp::reset()
{
	new(std::addressof(ipconfig)) ipv4_config
	{
		.primary_gateway            { empty },
		.primary_dns_server         { empty },
		.alternate_gateway          { empty },
		.alternate_dns_server       { empty },
		.dhcp_server_addr           { empty },
		.leased_addr                { empty },
		.subnet_mask                { empty },
		.lease_acquired_time        { 0UL },
		.lease_duration             { 0U },
		.lease_rebind_time          { 0U },
		.time_to_live_default       { 0x40UC },
		.time_to_live_tcp_default   { 0x40UC },
		.current_state              { ipv4_client_state::INIT }
	};
	discover(std::forward<std::vector<net8>>({ SUBNET_MASK, DOMAIN_NAME_SERVER, DOMAIN_NAME, ROUTER }));
}
int protocol_dhcp::transition_state(ipv4_client_state to_state)
{
	ipconfig.current_state		= to_state;
	try
	{
		switch(to_state)
		{
		case ipv4_client_state::INIT:
			reset();
			return 0;
		case ipv4_client_state::RENEWING:
			active_renewal_xid	= static_cast<uint32_t>(rand());
			return renew();
		case ipv4_client_state::REBINDING:
			return rebind();
		case ipv4_client_state::BOUND:
			active_renewal_xid	= 0U;
		default:
			return 0;
		}
	}
	catch(std::runtime_error& e) { panic(e.what()); }
	return -ENETDOWN;
}