#ifndef __PACKETS
#define __PACKETS
#include "typeindex"
#include "bits/in_place_t.hpp"
#include "bits/aligned_buffer.hpp"
#include "net/netstack_buffer.hpp"
#include "net/net_types.hpp"
struct generic_packet_base
{
    void* packet_data;
    std::ext::type_erasure packet_type;
    size_t packet_size;
    void (*release_fn)(void*, size_t);
    generic_packet_base(void* data, std::type_info const& type, size_t sz, void (*dealloc)(void*, size_t) = ::operator delete);
    ~generic_packet_base();
    // Nontrivial copy, move, and assign operations are required to manage ownership of (potentially variable-size) packet data.
    // Because the packet data should itself be trivially-copyable, all four of these operations are defined, but nontrivial, even if the specific type is unknown,
    // since the size value and type erasure are stored here.
    generic_packet_base(generic_packet_base const& that);
    generic_packet_base(generic_packet_base&& that);
    generic_packet_base& operator=(generic_packet_base const& that);
    generic_packet_base& operator=(generic_packet_base&& that);
    int read_from(netstack_buffer& buff);
    int write_to(netstack_buffer& buff) const;
    template<typename T> T* get_as() { return packet_type.template cast_to<T>(packet_data); }
    template<typename T> T const* get_as() const { return packet_type.template cast_to<T>(packet_data); }
};
template<typename T>
class generic_packet : public generic_packet_base
{
    constexpr static std::allocator<T> __alloc{};
    static void __deallocate(void* pkt, size_t) { __alloc.deallocate(static_cast<T*>(pkt), 1); }
public:
    template<typename ... Args> requires std::constructible_from<T, Args...> generic_packet(Args&& ... args) : generic_packet_base(std::construct_at(__alloc.allocate(1), std::forward<Args>(args)...), typeid(T), sizeof(T), __deallocate) {}
    template<typename ... Args> requires std::constructible_from<T, Args...> generic_packet(size_t sz, std::in_place_type_t<T>, Args&& ... args) : generic_packet_base(std::construct_at(static_cast<T*>(::operator new(sz)), std::forward<Args>(args)...), typeid(T), sz) {}
    constexpr T* operator->() noexcept { return static_cast<T*>(packet_data); }
    constexpr T const* operator->() const noexcept { return static_cast<T*>(packet_data); }
    constexpr T& operator*() noexcept { return *static_cast<T*>(packet_data); }
    constexpr T const& operator*() const noexcept { return *static_cast<T*>(packet_data); }
};
net16 ip_checksum(net16* words, size_t n);
#endif