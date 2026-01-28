#ifndef __SERIAL
#define __SERIAL
#include <arch/arch_amd64.h>
#include <module.hpp>
constexpr word ier_port(word port) noexcept { return port + 1US; }
constexpr word iir_or_fifo_port(word port) noexcept { return port + 2US; }
constexpr word line_ctl_port(word port) noexcept { return port + 3US; }
constexpr word modem_ctl_port(word port) noexcept { return port + 4US; }
constexpr word line_status_port(word port) noexcept { return port + 5US; }
constexpr word port_com1				= 0x03F8US;
constexpr word port_com1_ier			= ier_port(port_com1);
constexpr word port_com1_iir			= iir_or_fifo_port(port_com1);
constexpr word port_com1_fifo_ctl		= iir_or_fifo_port(port_com1);
constexpr word port_com1_line_ctl		= line_ctl_port(port_com1);
constexpr word port_com1_modem_ctl		= modem_ctl_port(port_com1);
constexpr word port_com1_line_status	= line_status_port(port_com1);
enum data_len_t
{
	L5BIT	= 0b00UC,
	L6BIT	= 0b01UC,
	L7BIT	= 0b10UC,
	L8BIT	= 0b11UC
};
enum parity_bit_t
{
	NONE	=	0b000UC,
	ODD		=	0b001UC,
	EVEN	=	0b011UC,
	MARK	=	0b101UC,
	SPACE	=	0b111UC
};
enum trigger_level_t
{
	T1BYTE	= 0b00UC,
	T4BYTE	= 0b01UC,
	T8BYTE	= 0b10UC,
	T14BYTE = 0b11UC
};
enum buffer_state_t
{
	DISABLED = 0b00UC,
	UNUSABLE = 0b01UC,
	ENABLED	= 0b10UC
};
enum irq_state_t
{
	MODEM_STATUS			= 0b00UC,
	TRANSMIT_BUFFER_EMPTY	= 0b01UC,
	RECEIVE_DATA_AVAILABLE	= 0b10UC,
	RECEIVER_LINE_STATUS	= 0b11UC
};
typedef struct __line_ctl
{
	enum data_len_t data_len		: 2;
	bool extend_stop_bit			: 1;
	enum parity_bit_t parity_bit	: 3;
	bool break_enable				: 1;
	bool divisor_latch_access		: 1;
	constexpr __line_ctl(data_len_t len, bool extend_sb, parity_bit_t pb, bool enable_break, bool dla) noexcept : data_len(len), extend_stop_bit(extend_sb), parity_bit(pb), break_enable(enable_break), divisor_latch_access(dla) {}
	constexpr __line_ctl(u8 i) noexcept : __line_ctl(data_len_t(i & 0x03UC), i[2], parity_bit_t((i & 0x38) >> 3), i[6], i[7]) {}
	constexpr __line_ctl() noexcept = default;
	constexpr operator uint8_t() const noexcept {
		if consteval { return u8(false, false, extend_stop_bit, false, false, false, break_enable, divisor_latch_access) | data_len | (parity_bit << 3); }
		return __builtin_bit_cast(uint8_t, *this);
	}
} __pack line_ctl_byte;
constexpr line_ctl_byte dla_enable(0x80UC);
constexpr line_ctl_byte S8N1(0x03UC);
typedef struct __ier_reg
{
	bool receive_data			: 1;
	bool transmit_buffer_empty	: 1;
	bool receiver_line_status	: 1;
	bool modem_status			: 1;
	bool						: 4;
	constexpr __ier_reg(bool rcv, bool tbe, bool ls, bool ms) noexcept : receive_data(rcv), transmit_buffer_empty(tbe), receiver_line_status(ls), modem_status(ms) {}
	constexpr __ier_reg(u8 i) noexcept : __ier_reg(i[0], i[1], i[2], i[3]) {}
	constexpr __ier_reg() noexcept = default;
	constexpr operator uint8_t() const noexcept {
		if consteval { return u8(receive_data, transmit_buffer_empty, receiver_line_status, modem_status, false, false, false, false); }
		return __builtin_bit_cast(uint8_t, *this);
	}
} __pack serial_ier;
constexpr serial_ier send_recv(0x03UC);
typedef struct __modem_ctl
{
	bool dtr_enable			: 1; // Data Terminal Ready
	bool rts_enable			: 1; // Request To Send
	bool out1_enable		: 1;
	bool out2_enable		: 1;
	bool loopback_enable	: 1; // Enable Loopback-Test Mode
	bool					: 3;
	constexpr __modem_ctl(bool dtr, bool rts, bool out1, bool out2, bool loop) noexcept : dtr_enable(dtr), rts_enable(rts), out1_enable(out1), out2_enable(out2), loopback_enable(loop) {}
	constexpr __modem_ctl(u8 i) noexcept : __modem_ctl(i[0], i[1], i[2], i[3], i[4]) {}
	constexpr __modem_ctl() noexcept = default;
	constexpr operator uint8_t() const noexcept {
		if consteval { return u8(dtr_enable, rts_enable, out1_enable, out2_enable, loopback_enable, false, false, false); }
		return __builtin_bit_cast(uint8_t, *this);
	}
} __pack modem_ctl_byte;
typedef struct __fifo_ctl_reg
{
	bool enable							: 1;
	bool clear_transmit_buffer			: 1;
	bool clear_receive_buffer			: 1;
	bool dma_sel						: 1;
	bool								: 2;
	enum trigger_level_t trigger_level	: 2;
	constexpr __fifo_ctl_reg(bool irq, bool clt, bool clr, bool timeout, trigger_level_t buf) noexcept : enable(irq), clear_transmit_buffer(clt), clear_receive_buffer(clr), dma_sel(timeout), trigger_level(buf) {}
	constexpr __fifo_ctl_reg(u8 i) noexcept : __fifo_ctl_reg(i[0], i[1], i[2], i[3], trigger_level_t((i & 0xC0UC) >> 6)) {}
	constexpr operator uint8_t() const noexcept {
		if consteval { return u8((trigger_level << 6) | u8(enable, clear_transmit_buffer, clear_receive_buffer, dma_sel, false, false, false, false)); }
		return __builtin_bit_cast(uint8_t, *this);
	}
} __pack fifo_ctl_byte;
typedef struct __iir_reg
{
	bool irq_pending					: 1;
	enum irq_state_t irq_state			: 2;
	bool timeout_pending				: 1;
	bool								: 2;
	enum buffer_state_t buffer_state	: 2;
	constexpr __iir_reg(bool irq, irq_state_t state, bool timeout, buffer_state_t buf) noexcept : irq_pending(irq), irq_state(state), timeout_pending(timeout), buffer_state(buf) {}
	constexpr __iir_reg(u8 i) noexcept : __iir_reg(i[0], irq_state_t((i & 0x06UC) >> 1), i[3], buffer_state_t((i & 0xC0UC) >> 6)) {}
	constexpr operator uint8_t() const noexcept {
		if consteval { return u8(irq_pending, false, false, timeout_pending, false, false, false, false) | (buffer_state << 6) | (irq_state << 1); }
		return __builtin_bit_cast(uint8_t, *this);
	}
} __pack serial_iir;
typedef struct __line_status_reg
{
	bool data_ready					: 1;
	bool overrun_error				: 1;
	bool parity_error				: 1;
	bool framing_error				: 1;
	bool break_indicator			: 1;
	bool transmitter_buffer_empty	: 1;
	bool tranmitter_idle			: 1;
	bool impending_error			: 1;
	constexpr __line_status_reg(bool dr, bool oe, bool pe, bool fe, bool bi, bool te, bool ti, bool ie) noexcept :
		data_ready					{ dr },
		overrun_error				{ oe },
		parity_error				{ pe },
		framing_error				{ fe },
		break_indicator				{ bi },
		transmitter_buffer_empty	{ te },
		tranmitter_idle				{ ti },
		impending_error				{ ie }
									{}
	constexpr __line_status_reg(u8 i) noexcept :
		data_ready					{ i[0] },
		overrun_error				{ i[1] },
		parity_error				{ i[2] },
		framing_error				{ i[3] },
		break_indicator				{ i[4] },
		transmitter_buffer_empty	{ i[5] },
		tranmitter_idle				{ i[6] },
		impending_error				{ i[7] }
									{}
	constexpr __line_status_reg() noexcept = default;
	constexpr operator u8() const noexcept {
		if consteval { return u8(data_ready, overrun_error, parity_error, framing_error, break_indicator, transmitter_buffer_empty, tranmitter_idle, impending_error); }
		return __builtin_bit_cast(uint8_t, *this);
	}
} __pack line_status_byte;
/**
 * Config Parameters:
 *	#0 port				: word					— which IO port to use for the serial device
 *	#1 mode				: struct line_ctl_byte	— configuration byte to use for the line control register
 *	#2 trigger_level	: enum trigger_level_t	— value to set for the trigger level for the FIFO buffer on the serial port
 *	#3 baud_div			: u16					— value to set for the baud rate divisor on the serial line
 *	#4 trim_on_read		: bool					— whether to discard input bytes that were previously read each time a read occurs, rather than when the buffer is full
 */
constexpr auto serial_config()
{
	return ooos::create_config
	(
		ooos::parameter("port",				port_com1),
		ooos::parameter("mode",				S8N1),
		ooos::parameter("trigger_level",	T4BYTE),
		ooos::parameter("baud_div",			12US),
		ooos::parameter("trim_on_read",		false)
	);
}
struct amd64_serial final : ooos::io_module_base<char>
{
	typedef decltype(serial_config()) config_type;
private:
	static config_type __cfg;
	char* __input_pos;
	void __trim_old() noexcept;
public:
	virtual bool overflow(size_type needed) override;
	virtual ooos::generic_config_table& get_config() override;
	virtual bool initialize() override;
	virtual void finalize() override;
	virtual int sync() override;
	virtual size_type avail() const override;
	virtual size_type read(pointer dest, size_type n) override;
	amd64_serial();
};
#endif