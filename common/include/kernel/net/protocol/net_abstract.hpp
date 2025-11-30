#ifndef __NET_ABSTRACT
#define __NET_ABSTRACT
#include <ext/dynamic_ptr.hpp>
#include <bits/hash_map.hpp>
#include <bits/in_place_t.hpp>
#include <bits/aligned_buffer.hpp>
#include <net/netstack_buffer.hpp>
#include <net/net_types.hpp>
struct abstract_packet_base
{
	void* packet_data;
	std::ext::type_erasure packet_type;
	size_t packet_size;
	void (*release_fn)(void*, size_t);
	abstract_packet_base(void* data, std::type_info const& type, size_t sz, void (*dealloc)(void*, size_t) = ::operator delete);
	abstract_packet_base(netstack_buffer& buffer, std::type_info const& type);
	~abstract_packet_base();
	// Nontrivial copy, move, and assign operations are required to manage ownership of (potentially variable-size) packet data.
	// Because the packet data should itself be trivially-copyable, all four of these operations are defined, but nontrivial, even if the specific type is unknown,
	// since the size value and type erasure are stored here.
	abstract_packet_base(abstract_packet_base const& that);
	abstract_packet_base(abstract_packet_base&& that);
	abstract_packet_base& operator=(abstract_packet_base const& that);
	abstract_packet_base& operator=(abstract_packet_base&& that);
	int read_from(netstack_buffer& buff);
	int write_to(netstack_buffer& buff) const;
	template<typename T> T* get_as() { return packet_type.template cast_to<T>(packet_data); }
	template<typename T> T const* get_as() const { return packet_type.template cast_to<T>(packet_data); }
};
template<typename T>
class abstract_packet : public abstract_packet_base
{
	constexpr static std::allocator<T> __alloc{};
	static void __deallocate(void* pkt, size_t) { __alloc.deallocate(static_cast<T*>(pkt), 1); }
public:
	template<typename ... Args> requires(std::constructible_from<T, Args...>) abstract_packet(Args&& ... args) : abstract_packet_base(std::construct_at(__alloc.allocate(1UZ), std::forward<Args>(args)...), typeid(T), sizeof(T), __deallocate) {}
	template<typename ... Args> requires(std::constructible_from<T, Args...>) abstract_packet(size_t sz, std::in_place_type_t<T>, Args&& ... args) : abstract_packet_base(std::construct_at(static_cast<T*>(__builtin_memset(::operator new(sz), 0, sz)), std::forward<Args>(args)...), typeid(T), sz) {}
	abstract_packet(netstack_buffer& buff) : abstract_packet_base(buff, typeid(T)) {}
	abstract_packet(abstract_packet const& that) : abstract_packet_base(that) {}
	abstract_packet(abstract_packet&& that) : abstract_packet_base(std::move(that)) {}
	constexpr T* operator->() noexcept { return static_cast<T*>(packet_data); }
	constexpr T const* operator->() const noexcept { return static_cast<T*>(packet_data); }
	constexpr T& operator*() noexcept { return *static_cast<T*>(packet_data); }
	constexpr T const& operator*() const noexcept { return *static_cast<T*>(packet_data); }
};
typedef std::hash_map<ipv4_addr, mac_t, cast_t<ipv4_addr, size_t>, equals_t, std::allocator<std::pair<const ipv4_addr, mac_t>>> mac_resolve_map;
struct abstract_ip_resolver
{
	mac_resolve_map previously_resolved;
	abstract_ip_resolver();
	virtual ~abstract_ip_resolver();
	virtual mac_t& resolve(ipv4_addr addr)		= 0;
	virtual bool check_presence(ipv4_addr addr)	= 0;
	mac_t const& operator[](ipv4_addr addr);
};
struct protocol_ethernet;
struct abstract_protocol_handler
{
	abstract_protocol_handler* const next;
	protocol_ethernet* const base;
	virtual int transmit(abstract_packet_base& p);
	virtual int receive(abstract_packet_base& p);
	virtual std::type_info const& packet_type() const = 0;
	abstract_protocol_handler(abstract_protocol_handler* n);
	abstract_protocol_handler(abstract_protocol_handler* n, protocol_ethernet* b);
};
typedef std::ext::dynamic_ptr<abstract_protocol_handler> protocol_handler;
typedef std::ext::dynamic_ptr<abstract_protocol_handler> protocol_handler;
template<std::ext::__explicitly_convertible_to<size_t> T> using protocol_handler_map = std::hash_map<T, protocol_handler, cast_t<T, size_t>, equals_t, std::allocator<std::pair<const T, protocol_handler>>>;
struct protocol_ethernet : abstract_protocol_handler
{
	abstract_ip_resolver* const ip_resolver;
	std::function<int(abstract_packet_base&)> transmit_fn;
	protocol_handler_map<uint16_t> handlers;
	mac_t const& mac_addr;
	ipv4_config* ipv4_client_config;
	ethernet_header create_packet(mac_t const& dest);
	virtual std::type_info const& packet_type() const;
	virtual int transmit(abstract_packet_base& p);
	virtual int receive(abstract_packet_base& p);
	virtual ~protocol_ethernet();
	protocol_ethernet(abstract_ip_resolver* ip_res, std::function<int(abstract_packet_base&)>&& tx_fn, mac_t const& mac);
};
template<std::derived_from<abstract_protocol_handler> T, typename ... Args> requires(std::constructible_from<T, Args...>) protocol_handler create_handler(Args&& ... args) { return protocol_handler(std::move(std::ext::make_dynamic<T>(std::forward<Args>(args)...))); }
std::string stringify(mac_t const& mac);
std::string stringify(ipv4_addr ip);
#endif