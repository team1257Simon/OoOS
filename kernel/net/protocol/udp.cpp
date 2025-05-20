#include "net/protocol/udp.hpp"
void udp_packet_base::compute_udp_csum()
{
    udp_csum                    = 0USBE;
    uint32_t intermediate_csum  = 0U;
    intermediate_csum           += source_addr.hi;
    intermediate_csum           += source_addr.lo;
    intermediate_csum           += destination_addr.hi;
    intermediate_csum           += destination_addr.lo;
    intermediate_csum           += protocol;
    intermediate_csum           += udp_length;
    net16* words                = std::addressof(source_port);
    size_t num_words            = udp_length / sizeof(net16);
    for(size_t i = 0; i < num_words; i++) intermediate_csum += words[i];
    dword dw_csum       = intermediate_csum;
    intermediate_csum   = dw_csum.hi + dw_csum.lo;
    dw_csum             = intermediate_csum;
    intermediate_csum   = dw_csum.hi + dw_csum.lo;
    udp_csum            = net16(~(static_cast<uint16_t>(intermediate_csum)));
}