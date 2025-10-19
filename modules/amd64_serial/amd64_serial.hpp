#ifndef __SERIAL
#define __SERIAL
#include <module.hpp>
#include <arch/arch_amd64.h>
constexpr word ier_port(word port) noexcept { return port + 1US; }
constexpr word iir_or_fifo_port(word port) noexcept { return port + 2US; }
constexpr word line_ctl_port(word port) noexcept { return port + 3US; }
constexpr word modem_ctl_port(word port) noexcept { return port + 4US; }
constexpr word line_status_port(word port) noexcept { return port + 5US; }
constexpr word port_com1                = 0x03F8US;
constexpr word port_com1_ier            = ier_port(port_com1);
constexpr word port_com1_iir            = iir_or_fifo_port(port_com1);
constexpr word port_com1_fifo_ctl       = iir_or_fifo_port(port_com1);
constexpr word port_com1_line_ctl       = line_ctl_port(port_com1);
constexpr word port_com1_modem_ctl      = modem_ctl_port(port_com1);
constexpr word port_com1_line_status    = line_status_port(port_com1);
enum data_len_t
{
    L5BIT = 0b00,
    L6BIT = 0b01,
    L7BIT = 0b10,
    L8BIT = 0b11
};
enum parity_bit_t
{
    NONE    =  0b000,
    ODD     =  0b001,
    EVEN    =  0b011,
    MARK    =  0b101,
    SPACE   =  0b111
};
enum trigger_level_t
{
    T1BYTE  = 0b00,
    T4BYTE  = 0b01,
    T8BYTE  = 0b10,
    T14BYTE = 0b11
};
enum buffer_state_t
{
    DISABLED = 0b00,
    UNUSABLE = 0b01,
    ENABLED  = 0b10
};
enum irq_state_t
{
    MODEM_STATUS            = 0b00,
    TRANSMIT_BUFFER_EMPTY   = 0b01,
    RECEIVE_DATA_AVAILABLE  = 0b10,
    RECEIVER_LINE_STATUS    = 0b11
};
typedef struct __line_ctl
{
    enum data_len_t data_len     : 2;
    bool extend_stop_bit         : 1;
    enum parity_bit_t parity_bit : 3;
    bool break_enable            : 1;
    bool divisor_latch_access    : 1;
    constexpr __line_ctl(data_len_t len, bool extend_sb, parity_bit_t pb, bool enable_break, bool dla) noexcept : data_len{ len }, extend_stop_bit{ extend_sb }, parity_bit{ pb }, break_enable{ enable_break }, divisor_latch_access{ dla } {}
    constexpr __line_ctl(byte i) noexcept : __line_ctl(data_len_t(i & 0x03UC), i[2], parity_bit_t((i & 0x38) >> 3), i[6], i[7]) {}
    constexpr operator byte() const noexcept { return byte(false, false, extend_stop_bit, false, false, false, break_enable, divisor_latch_access) | data_len | (parity_bit << 3); }
} __pack line_ctl_byte;
constexpr line_ctl_byte dla_enable(0x80UC);
constexpr line_ctl_byte S8N1(0x03UC);
typedef struct __ier_reg
{
    bool receive_data          : 1;
    bool transmit_buffer_empty : 1;
    bool receiver_line_status  : 1;
    bool modem_status          : 1;
    bool                       : 4;
    constexpr __ier_reg(bool rcv, bool tbe, bool ls, bool ms) noexcept : receive_data{ rcv }, transmit_buffer_empty{ tbe }, receiver_line_status{ ls }, modem_status{ ms } {}
    constexpr __ier_reg(byte i) noexcept : __ier_reg{ i[0], i[1], i[2], i[3] } {}
    constexpr operator byte() const noexcept { return byte{ receive_data, transmit_buffer_empty, receiver_line_status, modem_status, false, false, false, false }; }
} __pack serial_ier;
constexpr serial_ier send_recv(0x03UC);
typedef struct __modem_ctl
{
    bool dtr_enable      : 1; // Data Terminal Ready
    bool rts_enable      : 1; // Request To Send
    bool out1_enable     : 1;
    bool out2_enable     : 1;
    bool loopback_enable : 1; // Enable Loopback-Test Mode
    bool                 : 3;
    constexpr __modem_ctl(bool dtr, bool rts, bool out1, bool out2, bool loop) noexcept : dtr_enable{ dtr }, rts_enable{ rts }, out1_enable{ out1 }, out2_enable{ out2 }, loopback_enable{ loop } {}
    constexpr __modem_ctl(byte i) noexcept : __modem_ctl{ i[0], i[1], i[2], i[3], i[4] } {}
    constexpr operator byte() const noexcept { return byte{ dtr_enable, rts_enable, out1_enable, out2_enable, loopback_enable, false, false, false }; }
} __pack modem_ctl_byte;
typedef struct __fifo_ctl_reg
{
    bool enable                         : 1;
    bool clear_transmit_buffer          : 1;
	bool clear_receive_buffer           : 1;
    bool dma_sel                        : 1;
    bool                                : 2;
    enum trigger_level_t trigger_level  : 2;
    constexpr __fifo_ctl_reg(bool irq, bool clt, bool clr, bool timeout, trigger_level_t buf) noexcept : enable{ irq }, clear_transmit_buffer{ clt }, clear_receive_buffer{ clr }, dma_sel{ timeout }, trigger_level{ buf } {}
    constexpr __fifo_ctl_reg(byte i) noexcept : __fifo_ctl_reg(i[0], i[1], i[2], i[3], trigger_level_t((i & 0xC0UC) >> 6)) {}
    constexpr operator byte() const noexcept { return byte((trigger_level << 6) | byte{ enable, clear_transmit_buffer, clear_receive_buffer, dma_sel, false, false, false, false }); }
} __pack fifo_ctl_byte;
typedef struct __iir_reg
{
    bool irq_pending                    : 1;
    enum irq_state_t irq_state          : 2;
    bool timeout_pending                : 1;
    bool                                : 2;
    enum buffer_state_t buffer_state    : 2;
    constexpr __iir_reg(bool irq, irq_state_t state, bool timeout, buffer_state_t buf) noexcept : irq_pending{ irq }, irq_state{ state }, timeout_pending{ timeout }, buffer_state{ buf } {}
    constexpr __iir_reg(byte i) noexcept : __iir_reg(i[0], irq_state_t((i & 0x06UC) >> 1), i[3], buffer_state_t((i & 0xC0UC) >> 6)) {}
    constexpr operator byte() const noexcept { return byte(irq_pending, false, false, timeout_pending, false, false, false, false) | (buffer_state << 6) | (irq_state << 1); }
} __pack serial_iir;
typedef struct __line_status_reg
{
    bool data_ready                 : 1;
    bool overrun_error              : 1;
    bool parity_error               : 1;
    bool framing_error              : 1;
    bool break_indicator            : 1;
    bool transmitter_buffer_empty   : 1;
    bool tranmitter_idle            : 1;
    bool impending_error            : 1;
    constexpr __line_status_reg(bool dr, bool oe, bool pe, bool fe, bool bi, bool te, bool ti, bool ie) noexcept : 
        data_ready                  { dr }, 
        overrun_error               { oe }, 
        parity_error                { pe }, 
        framing_error               { fe }, 
        break_indicator             { bi }, 
        transmitter_buffer_empty    { te }, 
        tranmitter_idle             { ti }, 
        impending_error             { ie } 
                                    {}
    constexpr __line_status_reg(byte i) noexcept : 
        data_ready                  { i[0] }, 
        overrun_error               { i[1] }, 
        parity_error                { i[2] }, 
        framing_error               { i[3] }, 
        break_indicator             { i[4] }, 
        transmitter_buffer_empty    { i[5] }, 
        tranmitter_idle             { i[6] }, 
        impending_error             { i[7] } 
                                    {}
    constexpr __line_status_reg() noexcept : __line_status_reg(0UC) {}
    constexpr operator byte() const noexcept { return byte(data_ready, overrun_error, parity_error, framing_error, break_indicator, transmitter_buffer_empty, tranmitter_idle, impending_error); }
} __pack line_status_byte;
/**
 * Config Parameters:
 *  - port          : word                          — which IO port to use for the serial device
 *  - mode          : byte (struct line_ctl_byte)   — configuration byte to use for the line control register
 *  - trigger_level : byte (enum trigger_level_t)   — value to set for the trigger level for the FIFO buffer on the serial port
 *  - baud_div      : word                          — value to set for the baud rate divisor on the serial line
 *  - trim_on_read  : bool                          — if set to true, the internal buffer's get region will trim elements that were previously read each time a read occurs, rather than when the buffer is full
 */
constexpr auto serial_config() 
{
    using ooos_kernel_module::create_config;
    using ooos_kernel_module::parameter;
    using ooos_kernel_module::parameter_type;
    return create_config
    (
        parameter("port",           parameter_type<word>,               port_com1),
        parameter("mode",           parameter_type<line_ctl_byte>,      S8N1),
        parameter("trigger_level",  parameter_type<trigger_level_t>,    T4BYTE),
        parameter("baud_div",       parameter_type<word>,               12US),
        parameter("trim_on_read",   parameter_type<bool>,               false)
    );
}
class amd64_serial : public ooos_kernel_module::io_module_base<char>, public ooos_kernel_module::configurable_interface
{
    typedef decltype(serial_config()) __config_type;
    char* __input_pos;
    static __config_type __cfg;
    void __trim_old();
public:
    virtual bool overflow(size_type needed) override;
    virtual ooos_kernel_module::generic_config_table& get_config() override;
    virtual bool initialize() override;
    virtual void finalize() override;
    virtual int sync() override;
    virtual size_type avail() const override;
    virtual size_type read(pointer dest, size_type n) override;
    amd64_serial();
};
#endif