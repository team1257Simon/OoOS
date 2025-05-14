#ifndef __PACKETS
#define __PACKETS
#include "typeindex"
#include "bits/aligned_buffer.hpp"
#include "net/netstack_buffer.hpp"
#include "net/net_types.hpp"
struct generic_packet_base
{
    void* packet_data;
    std::ext::type_erasure packet_type;
    size_t packet_size;
    generic_packet_base(void* data, std::type_info const& type, size_t sz);
    int read_from(netstack_buffer& buff);
    int write_to(netstack_buffer& buff) const;
    template<typename T> T* get_as() { return static_cast<T*>(packet_type.cast_to(packet_data, typeid(T))); }
    template<typename T> T const* get_as() const { return static_cast<T const*>(packet_type.cast_to(packet_data, typeid(T))); }
};
template<typename T>
class generic_packet : public generic_packet_base
{
    __impl::__aligned_buffer<T> __packet_data;
public:
    template<typename ... Args> requires std::constructible_from<T, Args...> generic_packet(Args&& ... args) : generic_packet_base(std::addressof(__packet_data.__my_storage), typeid(T), sizeof(T)), __packet_data{} { std::construct_at(__packet_data.__get_ptr(), std::forward<Args>(args)...); }
    constexpr T* operator->() noexcept { return __packet_data.__get_ptr(); }
    constexpr T const& operator->() const noexcept { return __packet_data.__get_ptr(); }
    constexpr T& operator*() noexcept { return *__packet_data.__get_ptr(); }
    constexpr T const& operator*() const noexcept { return *__packet_data.__get_ptr(); }
};
#endif