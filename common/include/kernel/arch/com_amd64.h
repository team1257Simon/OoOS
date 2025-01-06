#ifndef __AMD64_SERIAL
#define __AMD64_SERIAL
#include "arch/arch_amd64.h"
constexpr uint16_t port_com1 = 0x03F8;
constexpr uint16_t port_com1_ier = port_com1 + 1;
constexpr uint16_t port_com1_iir = port_com1 + 2;
constexpr uint16_t port_com1_fifo_ctl = port_com1 + 2;
constexpr uint16_t port_com1_line_ctl = port_com1 + 3;
constexpr uint16_t port_com1_modem_ctl = port_com1 + 4;
constexpr uint16_t port_com1_line_status = port_com1 + 5;
typedef struct __line_ctl
{
    enum data_len_t
    {
        L5 = 0b00,
        L6 = 0b01,
        L7 = 0b10,
        L8 = 0b11
    } data_len                : 2;
    bool ext_stop_bit         : 1;
    enum parity_bit
    {
        NONE    =  0b000,
        ODD     =  0b001,
        EVEN    =  0b011,
        MARK    =  0b101,
        SPACE   =  0b111
    } parity                  : 3;
    bool break_enable         : 1;
    bool divisor_latch_access : 1;
#ifdef __cplusplus
    constexpr __line_ctl(data_len_t len, bool extend_sb, parity_bit pb, bool enable_break, bool dla) noexcept : data_len{ len }, ext_stop_bit{ extend_sb }, parity{ pb }, break_enable{ enable_break }, divisor_latch_access{ dla } {}
    constexpr __line_ctl(uint8_t i) noexcept : __line_ctl{ data_len_t(i & 0x03), (i & 0x04) != 0, parity_bit((i & 0x38) >> 3), (i & 0x40) != 0, (i & 0x80) != 0 } {}
    constexpr operator uint8_t() const noexcept { return uint8_t(data_len | (parity << 3) | (ext_stop_bit ? 0x04u : 0u) | (break_enable ? 0x40u : 0u) | (divisor_latch_access ? 0x80u : 0u)); }
#endif
} __pack line_ctl_byte;
#ifdef __cplusplus
constexpr line_ctl_byte dla_enable = 0x80;
constexpr line_ctl_byte S8N1 = 0x03;
#endif
typedef struct __ier_reg
{
    bool recv_data              : 1;
    bool transmit_buffer_empty  : 1;
    bool recv_line_status       : 1;
    bool modem_status           : 1;
    bool                        : 4;
#ifdef __cplusplus
    constexpr __ier_reg(bool rcv, bool tbe, bool ls, bool ms) noexcept : recv_data{ rcv }, transmit_buffer_empty{ tbe }, recv_line_status{ ls }, modem_status{ ms } {}
    constexpr __ier_reg(uint8_t i) noexcept : __ier_reg{(i & 0b0001) != 0, (i & 0b0010) != 0, (i & 0b0100) != 0, (i & 0b1000) != 0 } {}
    constexpr operator uint8_t() const noexcept { return uint8_t((recv_data ? 0b0001u : 0u) | (transmit_buffer_empty ? 0b0010u : 0u) | (recv_line_status ? 0b0100u : 0u) | (modem_status ? 0b1000u : 0u)); }
#endif
} __pack serial_ier;
#ifdef __cplusplus
constexpr serial_ier send_recv = 0b0011;
#endif
typedef struct __modem_ctl
{
    bool dtr_enable  : 1;
    bool rts_enable  : 1;
    bool out1_enable : 1;
    bool out2_enable : 1;
    bool loop_enable : 1;
    bool             : 3;
#ifdef __cplusplus
    constexpr __modem_ctl(bool dtr, bool rts, bool out1, bool out2, bool loop) noexcept : dtr_enable{ dtr }, rts_enable{ rts }, out1_enable{ out1 }, out2_enable{ out2 }, loop_enable{ loop } {}
    constexpr __modem_ctl(uint8_t i) noexcept : __modem_ctl{ (i & 0x01u) != 0, (i & 0x02u) != 0, (i & 0x04u) != 0, (i & 0x08u) != 0, (i & 0x10u) != 0 } {}
    constexpr operator uint8_t() const noexcept { return uint8_t((dtr_enable ? 0x01u : 0u) | (rts_enable ? 0x02u : 0) | (out1_enable ? 0x04u : 0) | (out2_enable ? 0x08u : 0) | (loop_enable ? 0x10u : 0));}
#endif
} __pack modem_ctl_byte;
typedef struct __fifo_ctl_reg
{
    bool enable : 1;
    bool clear_send : 1;
	bool clear_rcv : 1;
    bool dma_sel : 1;
    bool : 2;
    enum trigger_level
    {
		T1B  = 0b00,
        T4B  = 0b01,
        T8B  = 0b10,
		T14B = 0b11
    } trigger : 2;
#ifdef __cplusplus
    constexpr __fifo_ctl_reg(bool irq, bool clt, bool clr, bool timeout, trigger_level buf) noexcept : enable{ irq }, clear_send{ clt }, clear_rcv{ clr }, dma_sel{ timeout }, trigger{ buf } {}
    constexpr __fifo_ctl_reg(uint8_t i) noexcept : __fifo_ctl_reg{ (i & 0x01) != 0, (i & 0x02u) != 0, (i & 0x04u) != 0, (i & 0x08u) != 0, trigger_level((i & 0xC0u) >> 6) } {}
    constexpr operator uint8_t() const noexcept { return uint8_t((trigger << 6) | (dma_sel ? 0x08u : 0) | (clear_rcv ? 0x04u : 0) | (clear_send ? 0x02u : 0) | (enable ? 0x01u : 0)); }
#endif
} __pack fifo_ctl_byte;
typedef struct __iir_reg
{
    bool irq_pending : 1;
    enum irq_state_type
    {
        MODEM                   = 0b00,
        TRANSMIT_BUFFER_EMPTY   = 0b01,
        RECV_DATA_AVAIL         = 0b10,
        RECV_LINE_STATUS        = 0b11
    } irq_state : 2;
    bool timeout_pending : 1;
    bool : 2;
    enum buffer_state
    {
        DISABLED = 0b00,
        UNUSABLE = 0b01,
        ENABLED  = 0b10
    } buffer : 2;
#ifdef __cplusplus
    constexpr __iir_reg(bool irq, irq_state_type state, bool timeout, buffer_state buf) noexcept : irq_pending{ irq }, irq_state{ state }, timeout_pending{ timeout }, buffer{ buf } {}
    constexpr __iir_reg(uint8_t i) noexcept : __iir_reg{ (i & 0x01) != 0, irq_state_type((i & 0x06u) >> 1), (i & 0x08u) != 0, buffer_state((i & 0xC0u) >> 6) } {}
    constexpr operator uint8_t() const noexcept { return uint8_t((buffer << 6) | (timeout_pending ? 0x08u : 0) | (irq_state << 1) | (irq_pending ? 0x01u : 0)); }
#endif
} __pack serial_iir;
#endif