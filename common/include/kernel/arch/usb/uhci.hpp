#ifndef __UHCI
#define __UHCI
#include <arch/pci.hpp>
enum class usb_pid : uint8_t
{
	OUT		= 0xE1UC,
	IN		= 0x69UC,
	SOF		= 0xA5UC,
	SETUP	= 0x2DUC,
	DATA0	= 0xC3UC,
	DATA1	= 0x4BUC,
	DATA2	= 0x87UC,
	MDATA	= 0x0FUC,
	ACK		= 0xD2UC,
	NAK		= 0x5AUC,
	STALL	= 0x1EUC,
	NYET	= 0x96UC,
	PRE		= 0x3CUC,
	ERR		= 0x3CUC,
	SPLIT	= 0x78UC,
	PING	= 0xB4UC,
	RSVD	= 0xF0UC,
};
struct uhci_cmd
{
	bool run			: 1;
	bool reset			: 1;
	bool global_reset	: 1;
	bool global_suspend	: 1;
	bool global_resume	: 1;
	bool debug_mode		: 1;
	bool configure_flag	: 1;
	bool max_packet_64	: 1;	// 0 -> packet size 32 bytes, 1 -> packet size 64 bytes
	bool				: 8;
};
struct uhci_status
{
	bool completion_interrupt	: 1;
	bool error_interrupt		: 1;
	bool resume_detect			: 1;
	bool host_system_errof		: 1;
	bool host_process_error		: 1;
	bool halted					: 1;
	bool						: 2;
	bool						: 8;
};
struct uhci_ier
{
	bool timeout_crc	: 1;
	bool resume			: 1;
	bool on_complete	: 1;
	bool short_packet	: 1;
	bool				: 4;
	bool				: 8;
};
struct __pack uhci_link_ptr
{
	bool terminate		: 1;
	bool queue_head		: 1;
	bool depth_first	: 1;	// Only valid for TDs; 0 -> breadth first, 1 -> depth first
	bool				: 1;
	uint32_t ptr		: 28;
};
struct uhci_td
{
	uhci_link_ptr link_ptr;
	struct __pack
	{
		int16_t act_len				: 11;	// length - 1, meaning a value of -1 (0x7FFUS) indicates an empty packet
		bool						: 5;
		bool						: 1;
		bool bitstuff_error			: 1;	// received data stream has too many 1s in a row (more than 6)
		bool crc_error				: 1;	// CRC or time-out error
		bool nak_received			: 1;
		bool babble_error			: 1;	// received data after the end of stream was expected
		bool buffer_error			: 1;	// data buffer overflow or underflow on host controller
		bool stalled				: 1;
		bool active					: 1;
		bool ioc					: 1;	// interrupt on completion
		bool isochronous			: 1;
		bool low_speed				: 1;	// 0 -> full speed device, 1 -> low speed device
		uint8_t error_limit			: 2;	// 0 -> no limit, otherwise it's a counter that is decremented on an error and an interupt is issued when it reaches 0
		bool short_packet_detect	: 1;
		bool						: 2;
	};
	struct __pack
	{
		usb_pid pid;
		uint8_t device_addr : 7;
		uint8_t endpoint	: 4;
		bool data_toggle	: 1;
		bool				: 1;
		int16_t max_len		: 11;	// length - 1, meaning a value of -1 (0x7FFUS) indicates an empty packet
	};
	uint32_t buffer_ptr;
};
struct uhci_qh {
	uhci_link_ptr horizontal;
	uhci_link_ptr vertical;
};
#endif