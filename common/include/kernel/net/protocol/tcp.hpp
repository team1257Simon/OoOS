#ifndef __TRANSMISSION_CONTROL_PROTOCOL
#define __TRANSMISSION_CONTROL_PROTOCOL
#include "net/protocol/ipv4.hpp"
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
        net8 data_offset    : 4;    // offset of the payload in 32-bit dwords
        bool                : 4;
        bool cwr_flag       : 1;    // congestion window reduced
        bool ece_flag       : 1;    // explicit congestion notification enable
        bool urgent_flag    : 1;
        bool ack_flag       : 1;
        bool push_flag      : 1;
        bool reset_flag     : 1;
        bool syn_flag       : 1;
        bool finish_flag    : 1;
    };
};
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
    void compute_tcp_checksum();
    bool verify_tcp_checksum() const;
};
#endif