#ifndef __DHCP
#define __DHCP
#include "net/protocol/udp.hpp"
#include "vector"
constexpr net32 dhcp_magic = 0x63825363UBE;
constexpr net16 dhcp_client_port = 68USBE;
constexpr net16 dhcp_server_port = 67USBE;
enum dhcp_operation_type : net8
{
    DISCOVER           = 0x01UC,
    OFFER              = 0x02UC,
    REQUEST            = 0x03UC,
    DECLINE            = 0x04UC,
    ACK                = 0x05UC,
    NAK                = 0x06UC,
    RELEASE            = 0x07UC,
    INFORM             = 0x08UC,
    FORCE_RENEW        = 0x09UC,
    LEASE_QUERY        = 0x0AUC,
    LEASE_UNASSIGNED   = 0x0BUC,
    LEASE_UNKNOWN      = 0x0CUC,
    LEASE_ACTIVE       = 0x0DUC,
    BULK_LEASE_QUERY   = 0x0EUC,
    LEASE_QUERY_DONE   = 0x0FUC,
    ACTIVE_LEASE_QUERY = 0x10UC,
    LEASE_QUERY_STATUS = 0x11UC,
    DHCP_TLS           = 0x12UC
};
enum dhcp_parameter_type : net8
{
    PAD                                = 0UC,
    SUBNET_MASK                        = 1UC,
    TIME_OFFSET                        = 2UC,
    ROUTER                             = 3UC,
    TIME_SERVER                        = 4UC,
    NAME_SERVER                        = 5UC,
    DOMAIN_NAME_SERVER                 = 6UC,
    LOG_SERVER                         = 7UC,
    COOKIE_SERVER                      = 8UC,
    LPR_SERVER                         = 9UC,
    IMPRESS_SERVER                     = 10UC,
    RESOUCE_LOCATION_SERVER            = 11UC,
    HOST_NAME                          = 12UC,
    BOOT_FILE_SIZE                     = 13UC,
    MERIT_DUMP_FILE                    = 14UC,
    DOMAIN_NAME                        = 15UC,
    SWAP_SERVER                        = 16UC,
    ROOT_PATH                          = 17UC,
    EXTENSIONS_PATH                    = 18UC,
    IP_FORWARDING                      = 19UC,
    NON_LOCAL_SOURCE_ROUTING           = 20UC,
    POLICY_FILTER                      = 21UC,
    MAX_REASSEMBLY_SIZE                = 22UC,
    DEFAULT_TTL                        = 23UC,
    PATH_MTU_AGING_TIMEOUT             = 24UC,
    PATH_MTU_PLATEAU_TABLE             = 25UC,
    INTERFAE_MTU                       = 26UC,
    ALL_SUBNETS_LOCAL                  = 27UC,
    BROADCAST_ADDRESS                  = 28UC,
    PERFORM_MASK_DISCOVERY             = 29UC,
    MASK_SUPPLIER                      = 30UC,
    PERFORM_ROUTER_DISCOVERY           = 31UC,
    ROUTER_SOLICITATION_ADDRESS        = 32UC,
    STATIC_ROUTE                       = 33UC,
    TRAILER_ENCAPSULATION              = 34UC,
    ARP_CACHE_TIMEOUT                  = 35UC,
    ETHERNET_ENCAPSULATION             = 36UC,
    DEFAULT_TCP_TTL                    = 37UC,
    TCP_KEEPALIVE_INTERVAL             = 38UC,
    TCP_KEEPALIVE_GARBAGE              = 39UC,
    NETWORK_INFORMATION_SERVICE_DOMAIN = 40UC,
    NETWORK_INFORMATION_SERVERS        = 41UC,
    NTP_SERVERS                        = 42UC,
    VENDOR                             = 43UC,
    NETBIOS_OVER_TCP_NAME_SERVER       = 44UC,
    NETBIOS_OVER_TCP_DATAGRAM_SERVER   = 45UC,
    NETBIOS_OVER_TCP_NODE_TYPE         = 46UC,
    NETBIOS_OVER_TCP_SCOPE             = 47UC,
    XWS_FONT_SERVER                    = 48UC,
    XWS_DISPLAY_MANAGER                = 49UC,
    REQUESTED_IP                       = 50UC,
    IP_LEASE_TIME                      = 51UC,
    OPTION_OVERLOAD                    = 52UC,
    MESSAGE_TYPE                       = 53UC,
    SERVER_IDENTIFIER                  = 54UC,
    PARAMETER_REQUEST_LIST             = 55UC,
    MESSAGE                            = 56UC,
    MAXIMUM_MESSAGE_SIZE               = 57UC,
    RENEWAL_TIME_vALUE                 = 58UC,
    REBINDING_TIME_vALUE               = 59UC,
    VENDOR_CLASS_IDENTIFIER            = 60UC,
    CLIENT_IDENTIFIER                  = 61UC,
    NIS_PLUS_DOMAIN                    = 64UC,
    NIS_PLUS_SERVERS                   = 65UC,
    TFTP_SERVER_NAME                   = 66UC,
    BOOTFILE_NAME                      = 67UC,
    MOBILE_IP_HOME_AGENT               = 68UC,
    SMTP_SERVER                        = 69UC,
    POP3_SERVER                        = 70UC,
    NNTP_SERVER                        = 71UC,
    DEFAULT_WWW_SERVER                 = 72UC,
    DEFAULT_FINGER_PROTOCOL_SERVER     = 73UC,
    DEFAULT_IRC_SERVER                 = 74UC,
    STREETTALK_SERVER                  = 75UC,
    STDA_SERVER                        = 76UC,
    TIME_ZONE                          = 100UC,
    CAPTIVE_PORTAL                     = 114UC,
    END_OF_TRANSMISSION                = 255UC
};
struct attribute(packed) dhcp_parameter
{
    dhcp_parameter_type type_code;
    // The length is always the first byte of this array if present; some types (such as padding and EOT) do not even have a length byte
    net8 parameter_octets[];
    constexpr net8& length() & noexcept { return parameter_octets[0]; }
    constexpr net8 const& length() const& noexcept { return parameter_octets[0]; }
    constexpr void* start() noexcept { return std::addressof(parameter_octets[1]); }
};
struct __pack dhcp_packet_base : udp_packet_base
{
    dhcp_operation_type operation;
    net8                hw_type = 0x1UC;
    net8                hw_len  = 0x6UC;
    net8                hops    = 0x0UC;
    net32               transaction_id;
    net16               seconds = 0USBE;
    net16               flags   = 0USBE;
    net32               client_ip;
    net32               your_ip;
    net32               server_ip;
    net32               relay_ip;
    net32               client_hw[4];
    char                server_name_optional[64];
    char                boot_file_name[128];
    net32               magic = dhcp_magic;
    dhcp_parameter      parameters[];
    dhcp_packet_base() noexcept;
    dhcp_packet_base(udp_packet_base const& that) noexcept;
    dhcp_packet_base(udp_packet_base&& that) noexcept;
};
constexpr size_t total_dhcp_size(size_t parameters_size) noexcept { return parameters_size + sizeof(dhcp_packet_base); }
#ifndef DHCP_INST
extern template struct abstract_packet<dhcp_packet_base>;
extern template abstract_packet<dhcp_packet_base>::abstract_packet(size_t, std::in_place_type_t<dhcp_packet_base>);
extern template abstract_packet<dhcp_packet_base>::abstract_packet(size_t, std::in_place_type_t<dhcp_packet_base>, dhcp_packet_base const&);
extern template abstract_packet<dhcp_packet_base>::abstract_packet(size_t, std::in_place_type_t<dhcp_packet_base>, dhcp_packet_base&&);
extern template abstract_packet<dhcp_packet_base>::abstract_packet(size_t, std::in_place_type_t<dhcp_packet_base>, udp_packet_base&&);
extern template abstract_packet<dhcp_packet_base>::abstract_packet(size_t, std::in_place_type_t<dhcp_packet_base>, udp_packet_base const&);
extern template abstract_packet<dhcp_packet_base>::abstract_packet(udp_packet_base&&);
extern template abstract_packet<dhcp_packet_base>::abstract_packet(udp_packet_base const&);
extern template abstract_packet<dhcp_packet_base>::abstract_packet(dhcp_packet_base&&);
extern template abstract_packet<dhcp_packet_base>::abstract_packet(dhcp_packet_base const&);
typedef abstract_packet<dhcp_packet_base> dhcp_packet;
#endif
struct protocol_dhcp : abstract_protocol_handler
{
    ipv4_addr_set* const ip_addrs;
    protocol_dhcp(protocol_udp* n);
    virtual ~protocol_dhcp();
    virtual std::type_info const& packet_type() const override;
    virtual int transmit(abstract_packet_base& p) override;
    virtual int receive(abstract_packet_base& p) override;
    int discover(std::vector<net8> const& param_requests);
};
#endif