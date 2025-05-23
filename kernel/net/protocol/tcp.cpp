#include "net/protocol/tcp.hpp"
tcp_session_buffer::tcp_session_buffer() = default;
tcp_header::tcp_header() noexcept { protocol = TCP; }
tcp_header::tcp_header(ipv4_standard_header const& that) noexcept : ipv4_standard_header(that) { protocol = TCP; }
tcp_header::tcp_header(ipv4_standard_header&& that) noexcept : ipv4_standard_header(std::move(that)) { protocol = TCP; }
addr_t tcp_header::payload_start() const { return addr_t(std::addressof(source_port)).plus(fields.data_offset * sizeof(net32)); }
addr_t tcp_header::payload_end() const { return addr_t(this).plus(static_cast<ptrdiff_t>(static_cast<uint16_t>(total_length))); }
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
    size_type avail = static_cast<size_type>(__in_region.__max - __in_region.__end);
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
    uint32_t intermediate_csum  = 0U;
    size_t tcp_length           = static_cast<size_t>(static_cast<uint16_t>(total_length)) - ihl * sizeof(net32);
    intermediate_csum           += source_addr.hi;
    intermediate_csum           += source_addr.lo;
    intermediate_csum           += destination_addr.hi;
    intermediate_csum           += destination_addr.lo;
    intermediate_csum           += protocol;
    intermediate_csum           += tcp_length;
    net16 const* words          = std::addressof(source_port);
    size_t num_words            = tcp_length / sizeof(net16);
    if((tcp_length % 2) != 0) 
        intermediate_csum       += payload_end().minus(1Z).ref<net8>();
    for(size_t i = 0; i < num_words; i++) intermediate_csum += words[i];
    dword dw_csum       = intermediate_csum;
    intermediate_csum   = dw_csum.hi + dw_csum.lo;
    dw_csum             = intermediate_csum;
    intermediate_csum   = dw_csum.hi + dw_csum.lo;
    tcp_checksum        = net16(~(static_cast<uint16_t>(intermediate_csum)));
}
bool tcp_header::verify_tcp_checksum() const
{
    uint32_t intermediate_csum  = 0U;
    size_t tcp_length           = static_cast<size_t>(static_cast<uint16_t>(total_length)) - ihl * sizeof(net32);
    intermediate_csum           += source_addr.hi;
    intermediate_csum           += source_addr.lo;
    intermediate_csum           += destination_addr.hi;
    intermediate_csum           += destination_addr.lo;
    intermediate_csum           += protocol;
    intermediate_csum           += tcp_length;
    net16 const* words          = std::addressof(source_port);
    size_t num_words            = tcp_length / sizeof(net16);
    if((tcp_length % 2) != 0) 
        intermediate_csum       += payload_end().minus(1Z).ref<net8>();
    for(size_t i = 0; i < num_words; i++) intermediate_csum += words[i];
    dword dw_csum       = intermediate_csum;
    intermediate_csum   = dw_csum.hi + dw_csum.lo;
    dw_csum             = intermediate_csum;
    intermediate_csum   = dw_csum.hi + dw_csum.lo;
    dw_csum             = intermediate_csum;
    return static_cast<uint16_t>(~(dw_csum.lo)) == 0US;
}