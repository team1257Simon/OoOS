#ifndef __XHCI
#define __XHCI
// TODO: comment/document all this stuff
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
enum class setup_request_type : uint8_t
{
	RT_STANDARD	= 0UC,
	RT_CLASS	= 1UC,
	RT_VENDOR	= 2UC,
};
enum class setup_request_recipient : uint8_t
{
	RR_DEVICE		= 0UC,
	RR_INTERFACE	= 1UC,
	RR_ENDPOINT		= 2UC,
	RR_OTHER		= 3UC,
};
enum class setup_request_code : uint8_t
{
	RC_GET_STATUS			= 0UC,
	RC_CLEAR_FEATURE		= 1UC,
	RC_SET_FEATURE			= 3UC,
	RC_SET_ADDRESS			= 5UC,
	RC_GET_DESCRIPTOR		= 6UC,
	RC_SET_DESCRIPTOR		= 7UC,
	RC_GET_CONFIGURATION	= 8UC,
	RC_SET_CONFIGURATION	= 9UC,
	RC_GET_INTERFACE		= 10UC,
	RC_SET_INTERFACE		= 11UC,
	RC_SYNCH_FRAME			= 12UC,
};
enum class descriptor_type : uint8_t
{
	DDT_DEVICE				= 1UC,
	DDT_CONFIGURATION		= 2UC,
	DDT_STRING				= 3UC,
	DDT_INTERFACE			= 4UC,
	DDT_ENDPOINT			= 5UC,
	DDT_DEVICE_QUALIFIER	= 6UC,
	DDT_OTHER_SPEED_CONFIG	= 7UC,
	DDT_INTERFACE_POWER		= 8UC
};
enum class trb_type : uint8_t
{
	TRB_NORMAL						= 1UC,
	TRB_SETUP						= 2UC,
	TRB_DATA						= 3UC,
	TRB_STATUS						= 4UC,
	TRB_ISOCH						= 5UC,
	TRB_LINK						= 6UC,
	TRB_EVENT_DATA					= 7UC,
	TRB_NOOP						= 8UC,
	TRB_ENABLE_SLOT_CMD				= 9UC,
	TRB_DISABLE_SLOT_CMD			= 10UC,
	TRB_ADDRESS_DEVICE_CMD			= 11UC,
	TRB_CONFIGIRE_ENDPOINT_CMD		= 12UC,
	TRB_EVALUATE_CONTEXT_CMD		= 13UC,
	TRB_RESET_ENDPOINT_CMD			= 14UC,
	TRB_STOP_ENDPOINT_CMD			= 15UC,
	TRB_SET_DEQUEUE_PTR_CMD			= 16UC,
	TRB_RESET_DEVICE_CMD			= 17UC,
	TRB_FORCE_EVENT_CMD				= 18UC,
	TRB_NEGOTIATE_BANDWIDTH_CMD		= 19UC,
	TRB_SET_LATENCY_TOLERANCE_CMD	= 20UC,
	TRB_GET_PORT_BANDWIDTH_CMD		= 21UC,
	TRB_FORCE_HEADER_CMD			= 22UC,
	TRB_NOOP_CMD					= 23UC,
	TRB_GET_EXTENDED_PROPERTY_CMD	= 24UC,
	TRB_SET_EXTENDED_PROPERTY_CMD	= 25UC,
	TRB_TRANSFER_EVENT				= 32UC,
	TRB_COMMAND_COMPLETION_EVENT	= 33UC,
	TRB_PORT_STATUS_CHANGE_EVENT	= 34UC,
	TRB_BANDWIDTH_REQUEST_EVENT		= 35UC,
	TRB_DOORBELL_EVENT				= 36UC,
	TRB_HOST_CONTROLLER_EVENT		= 37UC,
	TRB_DEVICE_NOTIFICATION_EVENT	= 38UC,
	TRB_MFINDEX_WRAP_EVENT			= 39UC,
	TRB_VENDOR_MIN					= 48UC,
	TRB_VENDOR_MAX					= 63UC,
};
enum class control_transfer_type : uint8_t
{
	CTT_NO_DATA_STAGE				= 0UC,
	CTT_OUT_DATA_STAGE				= 2UC,
	CTT_IN_DATA_STAGE				= 3UC,
};
enum test_selector : uint8_t
{
	TEST_J				= 0x01UC,
	TEST_K				= 0x02UC,
	TEST_SE0_NAK		= 0x03UC,
	TEST_PACKET			= 0x04UC,
	TEST_FORCE_ENABLE	= 0x05UC,
	TEST_STD_MIN		= 0x06UC,
	TEST_STD_MAX		= 0x3FUC,
	TEST_VENDOR_MIN		= 0xC0UC,
	TEST_VENDOR_MAX		= 0xFFUC,
};
enum class feature_selector : uint16_t
{
	FEATURE_REMOTE_WAKEUP	= 1US,
	FEATURE_ENDPOINT_HALT	= 0US,
	FEATURE_TEST_MODE		= 2US,
};
enum completion_code : uint8_t
{
	CC_INVALID						= 0UC,
	CC_SUCCESS						= 1UC,
	CC_BUFFER_ERROR					= 2UC,
	CC_BABBLE						= 3UC,
	CC_TXN_ERROR					= 4UC,
	CC_TRB_ERROR					= 5UC,
	CC_STALL						= 6UC,
	CC_RSRC_ERROR					= 7UC,
	CC_BANDWIDTH_ERROR				= 8UC,
	CC_NO_SLOTS_AVAILABLE			= 9UC,
	CC_STREAM_TYPE_ERROR			= 10UC,
	CC_SLOT_NOT_ENABLED				= 11UC,
	CC_ENDPOINT_NOT_ENABLED			= 12UC,
	CC_SHORT_PACKET					= 13UC,
	CC_RING_UNDERRUN				= 14UC,
	CC_RING_OVERRUN					= 15UC,
	CC_VF_EVENT_RING_FULL			= 16UC,
	CC_PARAMETER_ERROR				= 17UC,
	CC_BANDWIDTH_OVERRUN			= 18UC,
	CC_CONTEXT_STATE_ERROR			= 19UC,
	CC_NO_PING_RESPONSE				= 20UC,
	CC_EVENT_RING_FULL				= 21UC,
	CC_INCOMPATIBLE_DEVICE			= 22UC,
	CC_MISSED_SERVICE				= 23UC,
	CC_COMMAND_RING_STOPPED			= 24UC,
	CC_COMMAND_ABORTED				= 25UC,
	CC_STOPPED						= 26UC,
	CC_LENGTH_INVALID				= 27UC,
	CC_STOPPED_SHORT_PACKET			= 28UC,
	CC_MAX_LATENCY_TOO_LARGE		= 29UC,
	CC_ISOCH_BUFFER_OVERFLOW		= 31UC,
	CC_EVENT_LOST					= 32UC,
	CC_UNDEFINED_ERROR				= 33UC,
	CC_INVALID_STREAM_ID			= 34UC,
	CC_SECONDARY_BANDWIDTH_ERROR	= 35UC,
	CC_SPLIT_TRANSACTION_ERROR		= 36UC,
	CC_VENDOR_ERROR_MIN				= 192UC,
	CC_VENDOR_ERROR_MAX				= 223UC,
	CC_VENDOR_INFO_MIN				= 224UC,
	CC_VENDOR_INFO_MAX				= 225UC,
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
struct xhci_descriptor_request_value {
	uint8_t descriptor_index;
	descriptor_type desc_type;
};
struct xhci_config_request_value {
	uint8_t value;
	bool			: 8;
};
struct __pack xhci_device_address_value {
	uint8_t address	: 7;
	uint16_t		: 9;
};
typedef uint16_t xhci_alternate_setting;
struct __pack xhci_index_for_endpoint
{
	uint8_t endpoint_number		: 4;
	bool						: 3;
	bool direction				: 1;	// 0 -> out, 1 -> in
};
typedef uint8_t xhci_index_for_interface;
typedef struct { volatile bool : 8; } empty_byte;
typedef struct { volatile short : 16; } empty_word;
typedef struct { volatile int : 32; } empty_dword;
typedef struct { volatile long : 64; } empty_qword;
struct force_header_data_dword { uint32_t header_data_hi; };
struct event_data_qword { uint64_t event_data; };
union [[gnu::may_alias]] trb_data_ptr
{
	addr_t ptr;
	struct __pack
	{
		bool dequeue_cycle_state	: 1;
		stream_contex_type sct		: 3;
		uintptr_t hi_ptr			: 60;
	};
};
struct __pack force_header_trb_data
{
	uint8_t packet_type		: 5;
	uint32_t header_info_lo	: 27;
	uint32_t header_info_mid;
};
struct __pack setup_stage_trb_data
{
	bool request_direction				: 1;	// 0 -> h2d, 1 -> d2h
	setup_request_type request_type		: 2;
	setup_request_recipient recipient	: 5;
	setup_request_code request_code;
	union [[gnu::may_alias]] value_field
	{
		empty_word empty{};
		xhci_descriptor_request_value descriptor;
		xhci_device_address_value device_address;
		xhci_config_request_value config;
		feature_selector feature;
		xhci_alternate_setting alternate_setting;
	} value;
	union [[gnu::may_alias]] low_index_field
	{
		empty_byte empty{};
		xhci_index_for_endpoint endpoint;
		xhci_index_for_interface interface;
		uint8_t lang_id_lo;
	} index;
	union [[gnu::may_alias]] high_index_field
	{
		empty_byte empty{};
		test_selector feature_set_test_selector;
		uint8_t lang_id_hi;
	} hi_index;
	uint16_t length;
};
struct __pack doorbell_event_trb_data
{
	uint8_t reason	: 5;
	uint32_t		: 27;
	uint32_t		: 32;
};
struct __pack port_status_change_event_trb_data
{
	uint32_t		: 24;
	uint8_t port_id;
	uint32_t		: 32;
};
struct __pack device_notification_event_trb_data
{
	uint8_t						: 4;
	uint8_t notification_type	: 4;
	uint64_t notification_data	: 56;
};
struct __pack transfer_trb_dw2
{
	uint32_t trb_transfer_length	: 17;	// bytes to send or size of receive buffer
	uint8_t td_size_tbc				: 5;	// td size or transfer burst count
	uint16_t interrupter_target		: 10;
};
struct __pack transfer_event_trb_dw2 {
	uint32_t residual_transfer_length	: 24;	// residual length of bytes not transferred
	completion_code code;
};
struct __pack completion_event_trb_dw2 {
	uint32_t param			: 24;	// specific to the command that was completed
	completion_code code;
};
struct __pack event_trb_dw2 {
	uint32_t				: 24;
	completion_code code;
};
struct __pack extended_property_cmd_trb_dw2
{
	bool audio_sideband_extended_capability	: 1;
	bool									: 7;
	bool									: 4;
	uint8_t vendor_defined					: 4;
	uint8_t capability_parameter;					// for set-property commands, a parameter specific to the extended capability; otherwise, zero
	bool									: 8;
};
struct __pack cmd_trb_dw2
{
	uint16_t				: 16;
	union [[gnu::may_alias]]
	{
		empty_byte empty{};
		uint16_t stream_id;
		struct __pack {
			bool							: 6;
			uint16_t vf_interrupter_target	: 10;
		};
	};
};
struct __pack other_trb_dw2 {
	uint32_t 						: 22;
	uint16_t interrupter_target		: 10;
};
struct __pack setup_trb_dw3hi {
	control_transfer_type transfer_type			: 2;
	uint16_t									: 14;
};
struct __pack data_trb_dw3hi {
	bool data_direction							: 1;
	uint16_t									: 15;
};
struct __pack isoch_trb_dw3hi
{
	uint8_t transfer_last_burst_packet_count	: 4;
	uint16_t frame_id							: 11;
	bool start_isoch_asap						: 1;
};
struct __pack transfer_event_trb_dw3hi
{
	uint8_t endpoint_id	: 5;
	bool				: 2;
	uint8_t slot_id;
};
struct __pack other_event_trb_dw3hi {
	uint8_t virtual_function_id;
	uint8_t slot_id;
};
struct __pack enable_slot_cmd_trb_dw3hi {
	uint8_t slot_type	: 5;
	uint16_t			: 11;
};
struct __pack force_event_cmd_trb_dw3hi {
	uint8_t virtual_function_id;
	bool							: 8;
};
struct __pack set_latency_tolerance_cmd_trb_dw3hi {
	uint16_t latency_tolerance_value	: 12;
	bool								: 4;
};
struct __pack get_port_bandwidth_cmd_trb_dw3hi
{
	uint8_t device_speed_id	: 4;
	uint8_t					: 4;
	uint8_t hub_slot_id;
};
struct __pack force_header_cmd_trb_dw3hi {
	bool							: 8;
	uint8_t root_hub_port_number;
};
struct __pack extended_property_cmd_trb_dw3hi
{
	uint8_t cmd_subtype		: 3;	// specific to the type of extended capability command
	uint8_t endpoint_id		: 5;	// ID of the target endpoint
	uint8_t slot_id;				// ID of the target slot
};
struct __pack other_cmd_trb_dw3hi
{
	uint8_t endpoint_id	: 5;	// for stop endpoint, reset endpoint, and set dequeue pointer commands, ID of the target endpoint; otherwise, zero
	bool				: 1;
	bool suspend		: 1;	// for stop endpoint commands, indicates that the endpoint will be suspended; otherwise, zero
	uint8_t slot_id;			// ID of the target slot
};
struct __pack xhci_trb_type_and_flags
{
	bool cycle						: 1;	// used to mark the Enqueue pointer of a command ring
	bool tc_or_ent					: 1;	// for links, toggle cycle; for transfers and event data, evaluate next; otherwise, zero
	bool ios_or_event_data			: 1;	// for transfer events, event data; for tranfers, interrupt-on-short-packet; otherwise, zero
	bool no_snoop					: 1;	// only for data and isochronous transfers; otherwise, zero
	bool chain						: 1;	// only for transfers, event data TRBs, and link TRBs; otherwise, zero
	bool interrupt_on_completion	: 1;	// only for transfers, event data TRBs, and link TRBs; otherwise; zero
	bool is_immediate				: 1;	// usually zero; 1 for any transfer where the data is immediate rather than a pointer (e.g. setup)
	uint8_t trb_status_tbc			: 2;	// for isochronous transfers, transfer burst count (if that field is not in DW2) or TD size; otherwise, zero
	bool block_or_dc				: 1;	// for configure commands, deconfigure; for event data and transfers, block event interrupt; for address device commands, block set-address; otherwise, zero
	trb_type type					: 6;
};
struct xhci_generic_trb
{
	uint64_t qw0;
	uint32_t dw2;
	struct __pack
	{
		uint16_t		: 10;
		trb_type value	: 6;
		constexpr operator trb_type() const noexcept { return value; }
	} type;
	uint16_t dw3hi;
};
template<qword_size Q0, dword_size D2, word_size D3H> struct xhci_trb : Q0, D2, xhci_trb_type_and_flags, D3H {};
template<word_size D3H, qword_size Q0 = trb_data_ptr, dword_size D2 = transfer_trb_dw2> using xhci_transfer_trb		= xhci_trb<Q0, D2, D3H>;
template<qword_size Q0, dword_size D2 = event_trb_dw2, word_size D3H = other_event_trb_dw3hi> using xhci_event_trb	= xhci_trb<Q0, D2, D3H>;
template<word_size D3H, qword_size Q0 = empty_qword, dword_size D2 = cmd_trb_dw2> using xhci_cmd_trb				= xhci_trb<Q0, D2, D3H>;
typedef xhci_transfer_trb<empty_word> xhci_normal_trb;
typedef xhci_transfer_trb<setup_trb_dw3hi, setup_stage_trb_data> xhci_setup_trb;
typedef xhci_transfer_trb<data_trb_dw3hi> xhci_data_trb;
typedef xhci_transfer_trb<data_trb_dw3hi, empty_qword, other_trb_dw2> xhci_status_trb;
typedef xhci_transfer_trb<isoch_trb_dw3hi> xhci_isoch_trb;
typedef xhci_trb<empty_qword, other_trb_dw2, empty_word> xhci_noop_trb;
typedef xhci_event_trb<trb_data_ptr, transfer_event_trb_dw2, transfer_event_trb_dw3hi> xhci_transfer_event_trb;
typedef xhci_event_trb<trb_data_ptr, completion_event_trb_dw2> xhci_completion_event_trb;
typedef xhci_event_trb<port_status_change_event_trb_data, event_trb_dw2, empty_word> xhci_port_status_change_event_trb;
typedef xhci_event_trb<empty_qword> xhci_bandwidth_request_event_trb;
typedef xhci_event_trb<doorbell_event_trb_data, event_trb_dw2> xhci_doorbell_event_trb;
typedef xhci_event_trb<empty_qword, event_trb_dw2, empty_word> xhci_host_controller_event_trb;
typedef xhci_event_trb<empty_qword, event_trb_dw2, empty_word> xhci_mfindex_wrap_event_trb;
typedef xhci_event_trb<device_notification_event_trb_data> xhci_device_notification_event_trb;
typedef xhci_cmd_trb<other_cmd_trb_dw3hi> xhci_noop_cmd_trb;
typedef xhci_cmd_trb<enable_slot_cmd_trb_dw3hi> xhci_enable_slot_cmd_trb;
typedef xhci_cmd_trb<other_cmd_trb_dw3hi> xhci_disable_slot_cmd_trb;
typedef xhci_cmd_trb<other_cmd_trb_dw3hi, trb_data_ptr>	xhci_address_device_cmd_trb;
typedef xhci_cmd_trb<other_cmd_trb_dw3hi, trb_data_ptr> xhci_configure_endpoint_cmd_trb;
typedef xhci_cmd_trb<other_cmd_trb_dw3hi, trb_data_ptr> xhci_evaluate_context_cmd_trb;
typedef xhci_cmd_trb<other_cmd_trb_dw3hi> xhci_reset_endpoint_cmd_trb;
typedef xhci_cmd_trb<other_cmd_trb_dw3hi> xhci_stop_endpoint_cmd_trb;
typedef xhci_cmd_trb<other_cmd_trb_dw3hi, trb_data_ptr> xhci_set_dequeue_ptr_cmd_trb;
typedef xhci_cmd_trb<other_cmd_trb_dw3hi> xhci_reset_device_cmd_trb;
typedef xhci_cmd_trb<force_event_cmd_trb_dw3hi, trb_data_ptr> xhci_force_event_cmd_trb;
typedef xhci_cmd_trb<other_cmd_trb_dw3hi> xhci_negotiate_bandwidth_cmd_trb;
typedef xhci_cmd_trb<set_latency_tolerance_cmd_trb_dw3hi> xhci_set_latency_tolerance_cmd_trb;
typedef xhci_cmd_trb<get_port_bandwidth_cmd_trb_dw3hi, trb_data_ptr> xhci_get_port_bandwidth_cmd_trb;
typedef xhci_cmd_trb<force_header_cmd_trb_dw3hi, force_header_trb_data, force_header_data_dword> xhci_force_header_cmd_trb;
typedef xhci_cmd_trb<extended_property_cmd_trb_dw3hi, empty_qword, extended_property_cmd_trb_dw2> xhci_extended_property_cmd;	// both get and set use this
typedef xhci_trb<trb_data_ptr, other_trb_dw2, empty_word> xhci_link_trb;
typedef xhci_trb<event_data_qword, other_trb_dw2, empty_word> xhci_event_data_trb;
#endif