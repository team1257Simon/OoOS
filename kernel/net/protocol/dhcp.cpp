#define DHCP_INST
#include "net/protocol/dhcp.hpp"
#include "sys/errno.h"
#include "stdlib.h" // rand()
#include "kdebug.hpp"
constexpr static std::allocator<net16> udp_pseudo_alloc;
constexpr const char must_decline_msg[] = "That address is already in use by another client.";
static std::vector<net32> consume_variadic_dword_parameter(dhcp_parameter const& p) { return std::vector<net32>(p.start().as<net32>(), p.start().plus(p.length()).as<net32>()); }
template<> template<> abstract_packet<dhcp_packet_base>::abstract_packet(udp_packet_base&& that) : abstract_packet(static_cast<size_t>(that.total_length), std::in_place_type<dhcp_packet_base>, std::move(that)) {}
template<> template<> abstract_packet<dhcp_packet_base>::abstract_packet(udp_packet_base const& that) : abstract_packet(static_cast<size_t>(that.total_length), std::in_place_type<dhcp_packet_base>, that) {}
template<> template<> abstract_packet<dhcp_packet_base>::abstract_packet(dhcp_packet_base&& that) : abstract_packet(static_cast<size_t>(that.total_length), std::in_place_type<dhcp_packet_base>, std::move(that)) {}
template<> template<> abstract_packet<dhcp_packet_base>::abstract_packet(dhcp_packet_base const& that) : abstract_packet(static_cast<size_t>(that.total_length), std::in_place_type<dhcp_packet_base>, that) {}
template class abstract_packet<dhcp_packet_base>;
template abstract_packet<dhcp_packet_base>::abstract_packet(udp_packet_base&&);
template abstract_packet<dhcp_packet_base>::abstract_packet(udp_packet_base const&);
template abstract_packet<dhcp_packet_base>::abstract_packet(dhcp_packet_base&&);
template abstract_packet<dhcp_packet_base>::abstract_packet(dhcp_packet_base const&);
template abstract_packet<dhcp_packet_base>::abstract_packet(size_t, std::in_place_type_t<dhcp_packet_base>, dhcp_packet_base const&);
template abstract_packet<dhcp_packet_base>::abstract_packet(size_t, std::in_place_type_t<dhcp_packet_base>, dhcp_packet_base&&);
template abstract_packet<dhcp_packet_base>::abstract_packet(size_t, std::in_place_type_t<dhcp_packet_base>);
template abstract_packet<dhcp_packet_base>::abstract_packet(size_t, std::in_place_type_t<dhcp_packet_base>, udp_packet_base&&);
template abstract_packet<dhcp_packet_base>::abstract_packet(size_t, std::in_place_type_t<dhcp_packet_base>, udp_packet_base const&);
dhcp_packet_base::dhcp_packet_base() noexcept = default;
dhcp_packet_base::dhcp_packet_base(udp_packet_base const& that) noexcept : udp_packet_base(that) { source_port = dhcp_client_port; destination_port = dhcp_server_port; }
dhcp_packet_base::dhcp_packet_base(udp_packet_base&& that) noexcept : udp_packet_base(std::move(that)) { source_port = dhcp_client_port; destination_port = dhcp_server_port; }
std::type_info const& protocol_dhcp::packet_type() const { return typeid(dhcp_packet_base); }
protocol_dhcp::~protocol_dhcp() = default;
protocol_dhcp::protocol_dhcp(protocol_udp* n) : abstract_protocol_handler(n), ipconfig(n->ipconfig), transaction_timers(32UZ) {}
int protocol_dhcp::receive(abstract_packet_base& p)
{
    dhcp_packet_base* pkt = p.get_as<dhcp_packet_base>();
    if(__builtin_expect(!pkt, false)) return -EPROTOTYPE;
    addr_t pos = pkt->parameters;
    while(pos.ref<net8>() != 0xFFUC)
    {
        if(!pos.ref<net8>()) pos += 1Z;
        else
        {
            dhcp_parameter* param = pos;
            if(param->type_code == MESSAGE_TYPE)
            {
                dhcp_message_type type = param->start().ref<dhcp_message_type>();
                switch(type)
                {
                case OFFER:
                    return process_offer_packet(*pkt);
                case NAK:
                    ipconfig.lease_duration     = 0U;
                    ipconfig.lease_renew_time   = 0U;
                    ipconfig.lease_rebind_time  = 0U;
                    ipconfig.leased_addr        = 0UBE;
                    ipconfig.current_state      = ipv4_client_state::INIT;
                    return 0;
                case ACK:
                    return process_ack_packet(*pkt);
                default:
                    return 0;   // nothing to do for any other packet types (for now)
                }
            }
            else pos += param->length();
        }
    }
    return -EPROTO;
}
abstract_packet<dhcp_packet_base> protocol_dhcp::create_packet(mac_t const& dest_mac, ipv4_addr dest_ip, size_t total_size, uint32_t xid)
{
    abstract_packet<dhcp_packet_base> result(total_size, std::in_place_type<dhcp_packet_base>, std::forward<ipv4_standard_packet>(base->create_packet(dest_mac)));
    result->transaction_id      = net32(xid);
    if(transaction_timers.contains(xid)) result->seconds = net16(static_cast<uint16_t>(sys_time(nullptr) - transaction_timers[xid]));
    result->destination_addr    = dest_ip;
    result->operation           = BOOTREQUEST;
    result->source_addr         = ipconfig.leased_addr;
    result->server_ip           = ipconfig.dhcp_server_addr;
    array_copy(result->client_hw, base->mac_addr.data(), sizeof(mac_t));
    return result;
}
void protocol_dhcp::discover(std::vector<net8> const& param_requests)
{
    size_t num_requests                             = param_requests.size();
    size_t total_param_size                         = (num_requests + 2UZ) + 4UZ; // 2 bytes for the request list type and size, 3 bytes for the message type, 1 byte for the EOT mark
    size_t target_size                              = total_dhcp_size(total_param_size); 
    size_t actual_size                              = up_to_nearest(target_size, 2UZ);
    uint32_t xid                                    = static_cast<uint32_t>(rand());
    while(transaction_timers.contains(xid)) xid     = static_cast<uint32_t>(rand());
    abstract_packet<dhcp_packet_base> discover_pkt  = create_packet(broadcast_mac, broadcast, actual_size, xid);
    transaction_timers[xid]                         = sys_time(nullptr);
    addr_t pos                                      = discover_pkt->parameters;
    array_zero<uint8_t>(pos, total_param_size);
    pos.plus(total_param_size - 1Z).assign(0xFFUC);
    dhcp_parameter* param                           = pos;
    param->type_code                                = MESSAGE_TYPE;
    param->length()                                 = 1UC;
    param->start().assign(DISCOVER);
    if(num_requests)
    {
        pos                                         += 3Z;
        param                                       = pos;
        param->type_code                            = PARAMETER_REQUEST_LIST;
        param->length()                             = num_requests;
        array_copy(param->start(), param_requests.data(), num_requests);
    }
    ipconfig.current_state = ipv4_client_state::INIT;
    if(next->transmit(discover_pkt) != 0) throw std::runtime_error{ "[DHCP] packet transmission failed" };
}
int protocol_dhcp::process_offer_packet(dhcp_packet_base const& p)
{
    if(ipconfig.current_state == ipv4_client_state::INIT && transaction_timers.contains(p.transaction_id))
    {
        ipv4_addr req_addr          = p.your_ip;
        ipconfig.dhcp_server_addr   = p.server_ip;
        addr_t pos                  = p.parameters;
        ipconfig.current_state = ipv4_client_state::SELECTING;
        try { while(pos.ref<dhcp_parameter_type>() != END_OF_TRANSMISSION) pos += process_packet_parameter(pos.ref<dhcp_parameter>()); }
        catch(std::invalid_argument& e) { panic(e.what()); return -EPROTO; }
        catch(std::bad_alloc&)          { return -ENOMEM; }
        if(__builtin_expect(base->ip_resolver->check_presence(req_addr), false))
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
int protocol_dhcp::process_ack_packet(dhcp_packet_base const& p)
{
    sys_time(std::addressof(ipconfig.lease_acquired_time));
    ipconfig.leased_addr = p.your_ip;
    addr_t pos           = p.parameters;
    try { while(pos.ref<dhcp_parameter_type>() != END_OF_TRANSMISSION) pos += process_packet_parameter(pos.ref<dhcp_parameter>()); }
    catch(std::invalid_argument& e) { panic(e.what()); return -EPROTO; }
    catch(std::bad_alloc&)          { return -ENOMEM; }
    if(__unlikely(!ipconfig.lease_duration)) return -EPROTO;
    if(ipconfig.lease_duration != 0xFFFFFFFFU)
    {
        if(!ipconfig.lease_renew_time)
            ipconfig.lease_renew_time = ipconfig.lease_duration / 2;
        if(!ipconfig.lease_rebind_time)
            ipconfig.lease_rebind_time = (7 * ipconfig.lease_duration) / 8;
    }
    ipconfig.current_state = ipv4_client_state::BOUND;
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
        ipconfig.subnet_mask            = param.start().ref<net32>();
        break;
    case SERVER_IDENTIFIER:
        if(!ipconfig.dhcp_server_addr) ipconfig.dhcp_server_addr = param.start().ref<net32>();
        break;
    case ROUTER:
        if(param.length() % 4UC) throw std::invalid_argument("[DHCP] malformed packet");
        ipconfig.gateway_addrs          = std::move(consume_variadic_dword_parameter(param));
        break;
    case DOMAIN_NAME_SERVER:
        if(param.length() % 4UC) throw std::invalid_argument("[DHCP] malformed packet");
        ipconfig.dns_server_addrs       = std::move(consume_variadic_dword_parameter(param));
        break;
    case DEFAULT_TTL:
        ipconfig.time_to_live_default   = param.start().ref<net8>();
        break;
    case RENEWAL_TIME_VALUE:
        if(ipconfig.current_state != ipv4_client_state::BOUND) ipconfig.lease_renew_time    = param.start().ref<net32>();
        break;
    case REBINDING_TIME_VALUE:
        if(ipconfig.current_state != ipv4_client_state::BOUND) ipconfig.lease_rebind_time   = param.start().ref<net32>();
        break;
    case IP_LEASE_TIME:
        if(ipconfig.current_state != ipv4_client_state::BOUND) ipconfig.lease_duration      = param.start().ref<net32>();
        break;
    default:
        break;
    // ...
    }
    return param.length() + 2UC;
}
int protocol_dhcp::decline(ipv4_addr addr, uint32_t xid)
{
    constexpr size_t total_param_size               = (sizeof(must_decline_msg) + 2UZ) + (sizeof(ipv4_addr) + 2UZ) * 2UZ + 4UZ;
    constexpr size_t target_size                    = total_dhcp_size(total_param_size);
    constexpr size_t actual_size                    = up_to_nearest(target_size, 2UZ);
    abstract_packet<dhcp_packet_base> decline_pkt   = create_packet(broadcast_mac, broadcast, actual_size, xid);
    decline_pkt->seconds                            = 0USBE;
    addr_t pos                                      = decline_pkt->parameters;
    array_zero<uint8_t>(pos, total_param_size);
    pos.plus(total_param_size - 1Z).assign(0xFFUC);
    dhcp_parameter* param                           = pos;
    param->type_code                                = MESSAGE_TYPE;
    param->length()                                 = 1UC;
    param->start().assign(DECLINE);
    pos                                             += 3Z;
    param                                           = pos;
    param->type_code                                = SERVER_IDENTIFIER;
    param->length()                                 = 4UC;
    param->start().assign(ipconfig.dhcp_server_addr);
    pos                                             += 6Z;
    param                                           = pos;
    param->type_code                                = REQUESTED_IP;
    param->length()                                 = 4UC;
    param->start().assign(addr);
    pos                                             += 6Z;
    param                                           = pos;
    param->type_code                                = MESSAGE;
    array_copy(pos.as<char>(), must_decline_msg, sizeof(must_decline_msg));
    return next->transmit(decline_pkt);
}
int protocol_dhcp::request(ipv4_addr addr, uint32_t xid)
{
    size_t total_param_size                         = (sizeof(ipv4_addr) + 2UZ) * (ipconfig.current_state == ipv4_client_state::SELECTING ? 2UZ : ipconfig.current_state == ipv4_client_state::REBOOT ? 1UZ : 0UZ) + 4UZ;
    size_t actual_size                              = total_dhcp_size(total_param_size);    // the above number cannot possibly be odd; it will be either 4, 10, or 16
    abstract_packet<dhcp_packet_base> request_pkt   = create_packet(broadcast_mac, broadcast, actual_size, xid);
    addr_t pos                                      = request_pkt->parameters;
    array_zero<uint8_t>(pos, total_param_size);
    pos.plus(total_param_size - 1Z).assign(0xFFUC);
    dhcp_parameter* param                           = pos;
    param->type_code                                = MESSAGE_TYPE;
    param->length()                                 = 1UC;
    param->start().assign(REQUEST);
    if(ipconfig.current_state == ipv4_client_state::SELECTING || ipconfig.current_state == ipv4_client_state::REBOOT)
    {
        if(ipconfig.current_state == ipv4_client_state::SELECTING)
        {
            pos                                             += 3Z;
            param                                           = pos;
            param->type_code                                = SERVER_IDENTIFIER;
            param->length()                                 = 4UC;
            param->start().assign(ipconfig.dhcp_server_addr);
        }
        pos                                             += 6Z;
        param                                           = pos;
        param->type_code                                = REQUESTED_IP;
        param->length()                                 = 4UC;
        param->start().assign(addr);
    }
    return next->transmit(request_pkt);
}