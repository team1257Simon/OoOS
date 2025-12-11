#ifndef __XHCI
#define __XHCI
#include <arch/pci.hpp>
enum class slot_state : uint8_t
{
	SS_DISABLED_ENABLED	= 0UC,
	SS_DEFAULT			= 1UC,
	SS_ADDRESSED		= 2UC,
	SS_CONFIGURED		= 3UC,
};
enum class endpoint_state : uint8_t
{
	EPS_DISABLED		= 0UC,
	EPS_RUNNING			= 1UC,
	EPS_HALTED			= 2UC,
	EPS_STOPPED			= 3UC,
	EPS_ERROR			= 4UC,
};
enum class endpoint_type : uint8_t
{
	EPT_ISOCH_OUT		= 1UC,
	EPT_BULK_OUT		= 2UC,
	EPT_INTERRUPT_OUT	= 3UC,
	EPT_CONTROL			= 4UC,
	EPT_ISOCH_IN		= 5UC,
	EPT_BULK_IN			= 6UC,
	EPT_INTERRUPT_IN	= 7UC,
};
enum class stream_contex_type : uint8_t
{
	SCT_SECONDARY_RING	= 0UC,
	SCT_PRIMARY_RING	= 1UC,
	SCT_SSA_8			= 2UC,
	SCT_SSA_16			= 3UC,
	SCT_SSA_32			= 4UC,
	SCT_SSA_64			= 5UC,
	SCT_SSA_128			= 6UC,
	SCT_SSA_256			= 7UC
};
struct __pack xhci_dequeue_link
{
	bool dequeue_cycle_state	: 1;
	stream_contex_type sct		: 3;	// only valid in stream contexts; otherwise, must be 0
	uint64_t dequeue_ptr		: 60;
};
struct __pack xhci_slot_context
{
	uint8_t route_str_0			: 4;
	uint8_t route_str_1			: 4;
	uint8_t route_str_2			: 4;
	uint8_t route_str_3			: 4;
	uint8_t route_str_4			: 4;
	uint8_t speed_id			: 4;	// deprecated
	bool						: 1;
	bool multi_tt				: 1;
	bool hub					: 1;	// 0 -> function, 1 -> hub
	uint8_t entries				: 5;
	uint16_t max_exit_latency;
	uint8_t root_hub_port_num;
	uint8_t number_of_ports;
	uint8_t parent_hub_slot_id;
	uint8_t parent_port_number;
	uint8_t tt_think_time		: 2;	// (tt_think_time+1)*8 FS times of inter-transaction gap needed
	bool						: 4;
	uint16_t interrupter_target	: 10;
	uint8_t device_addr;
	bool						: 8;
	bool						: 8;
	bool						: 3;
	slot_state state			: 5;
	uint64_t					: 64;
	uint64_t					: 64;
};
struct __pack xhci_endpoint_context
{
	endpoint_state state			: 2;
	bool							: 5;
	uint8_t mult					: 2;	// maximum number of bursts within an interval supported; actual value is 1 higher
	uint8_t max_primary_streams		: 5;
	bool linear_stream_array		: 1;
	uint8_t interval;
	uint8_t max_esit_payload_hi;			// max endpoint service time interval payload, high-order 8 bits
	bool							: 1;
	uint8_t error_limit				: 2;	// if nonzero and more than this many bus errors are detected, the transaction is halted with an error event
	endpoint_type type				: 3;
	bool							: 1;
	bool host_initiate_disable		: 1;
	uint8_t max_burst_size;
	uint16_t max_packet_size;
	xhci_dequeue_link dequeue_ptr;
	uint16_t trb_average_length;
	uint16_t max_esit_payload_lo;			// max endpoint service time interval payload, low-order 16 bits
	uint32_t						: 32;
	uint64_t						: 64;
};
struct __pack xhci_stream_context
{
	xhci_dequeue_link dequeue_ptr;
	uint32_t stopped_edtla			: 24;
	uint8_t							: 8;
	uint32_t						: 32;
};
struct __pack xhci_input_control_context
{
	dword drop_flags;	// note: d0 and d1 are reserved
	dword add_flags;
	uint64_t 						: 64;
	uint64_t						: 64;
	uint32_t						: 32;
	uint8_t confifguration_value;
	uint8_t interface_number;
	uint8_t alternate_setting;
	uint8_t							: 8;
};
#endif