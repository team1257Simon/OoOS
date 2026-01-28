#ifndef __USB_ABSTRACT
#define __USB_ABSTRACT
#include <module.hpp>
#include <ranges>
#include <util/event_listener.hpp>
namespace ooos
{	
	typedef struct { volatile bool : 8; } empty_byte;
	typedef struct { volatile short : 16; } empty_word;
	typedef struct { volatile int : 32; } empty_dword;
	typedef struct { volatile long : 64; } empty_qword;
	enum class usb_descriptor_type : uint8_t
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
	enum class usb_test_selector : uint8_t
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
	enum class usb_setup_request_code : uint8_t
	{
		GET_STATUS			= 0UC,
		CLEAR_FEATURE		= 1UC,
		SET_FEATURE			= 3UC,
		SET_ADDRESS			= 5UC,
		GET_DESCRIPTOR		= 6UC,
		SET_DESCRIPTOR		= 7UC,
		GET_CONFIGURATION	= 8UC,
		SET_CONFIGURATION	= 9UC,
		GET_INTERFACE		= 10UC,
		SET_INTERFACE		= 11UC,
		SYNCH_FRAME			= 12UC,
	};
	enum class usb_transfer_type : uint8_t
	{
		CONTROL		= 0UC,
		ISOCHRONOUS	= 1UC,
		BULK_DATA	= 2UC,
		INTERRUPT	= 3UC,
	};
	enum class usb_request_target : uint8_t
	{
		DEVICE		= 0UC,
		INTERFACE	= 1UC,
		ENDPOINT	= 2UC,
		OTHER		= 3UC
	};
	enum class usb_request_type : uint8_t
	{
		STANDARD	= 0UC,
		CLASS	= 1UC,
		VENDOR	= 2UC,
	};
	enum class usb_transfer_direction : bool
	{
		OUT = false,
		IN	= true,
		H2D	= OUT,
		D2H	= IN,
	};
	enum class usb_feature_selector : uint16_t
	{
		REMOTE_WAKEUP	= 1US,
		ENDPOINT_HALT	= 0US,
		TEST_MODE		= 2US,
	};
	enum class usb_sync_type : uint8_t
	{
		NO_SYNC		= 0UC,
		ASYNC		= 1UC,
		ADAPTIVE	= 2UC,
		SYNC		= 3UC
	};
	enum class usb_isoch_usage_type : uint8_t
	{
		DATA					= 0UC,
		FEEDBACK				= 1UC,
		IMPLICIT_FEEDBACK_DATA	= 2UC,
	};
	enum class usb_lang_id : uint16_t
	{
		CHINESE_MACAU_SAR		=	0x1404US,
		CROATIAN				=	0x041AUS,
		CZECH					=	0x0405US,
		DANISH					=	0x0406US,
		DUTCH_NETHERLANDS		=	0x0413US,
		DUTCH_BELGIUM			=	0x0813US,
		ENGLISH_UNITED_STATES	=	0x0409US,
		ENGLISH_UNITED_KINGDOM	=	0x0809US,
		ENGLISH_AUSTRALIAN		=	0x0C09US,
		ENGLISH_CANADIAN		=	0x1009US,
		ENGLISH_NEW_ZEALAND		=	0x1409US,
		ENGLISH_IRELAND			=	0x1809US,
		ENGLISH_SOUTH_AFRICA	=	0x1C09US,
		ENGLISH_JAMAICA			=	0x2009US,
		ENGLISH_CARIBBEAN		=	0x2409US,
		ENGLISH_BELIZE			=	0x2809US,
		ENGLISH_TRINIDAD		=	0x2C09US,
		ENGLISH_ZIMBABWE		=	0x3009US,
		ENGLISH_PHILIPPINES		=	0x3409US,
		ESTONIAN				=	0x0425US,
		FAEROESE				=	0x0438US,
		FARSI					=	0x0429US,
		FINNISH					=	0x040BUS,
		FRENCH_STANDARD			=	0x040CUS,
		FRENCH_BELGIAN			=	0x080CUS,
		FRENCH_CANADIAN			=	0x0C0CUS,
		FRENCH_SWITZERLAND		=	0x100CUS,
		FRENCH_LUXEMBOURG		=	0x140CUS,
		FRENCH_MONACO			=	0x180CUS,
		GEORGIAN				=	0x0437US,
		GERMAN_STANDARD			=	0x0407US,
		GERMAN_SWITZERLAND		=	0x0807US,
		GERMAN_AUSTRIA			=	0x0C07US,
		GERMAN_LUXEMBOURG		=	0x1007US,
		GERMAN_LIECHTENSTEIN	=	0x1407US,
		GREEK					=	0x0408US,
		GUJARATI				=	0x0447US,
		HEBREW					=	0x040DUS,
		HINDI					=	0x0439US,
		HUNGARIAN				=	0x040EUS,
		ICELANDIC				=	0x040FUS,
		INDONESIAN				=	0x0421US,
		ITALIAN_STANDARD		=	0x0410US,
		ITALIAN_SWITZERLAND		=	0x0810US,
		JAPANESE				=	0x0411US,
		KANNADA					=	0x044BUS,
		KASHMIRI_INDIA			=	0x0860US,
		KAZAKH					=	0x043FUS,
		KONKANI					=	0x0457US,
		KOREAN					=	0x0412US,
		KOREAN_JOHAB			=	0x0812US,
		LATVIAN					=	0x0426US,
		LITHUANIAN				=	0x0427US,
		LITHUANIAN_CLASSIC		=	0x0827US,
		MACEDONIAN				=	0x042FUS,
		MALAY_MALAYSIAN			=	0x043EUS,
		MALAY_BRUNEI_DARUSSALAM	=	0x083EUS,
		MALAYALAM				=	0x044CUS,
		MANIPURI				=	0x0458US,
		MARATHI					=	0x044EUS,
		NEPALI_INDIA			=	0x0861US,
		NORWEGIAN_BOKMAL		=	0x0414US,
		NORWEGIAN_NYNORSK		=	0x0814US,
		ORIYA					=	0x0448US,
		POLISH					=	0x0415US,
		PORTUGUESE_BRAZIL		=	0x0416US,
		PORTUGUESE_STANDARD		=	0x0816US,
		PUNJABI					=	0x0446US,
		ROMANIAN				=	0x0418US,
		RUSSIAN					=	0x0419US,
		SANSKRIT				=	0x044FUS,
		SERBIAN_CYRILLIC		=	0x0C1AUS,
		SERBIAN_LATIN			=	0x081AUS,
		SINDHI					=	0x0459US,
		SLOVAK					=	0x041BUS,
		SLOVENIAN				=	0x0424US,
		SPANISH_TRADITIONAL		=	0x040AUS,
		SPANISH_MEXICAN			=	0x080AUS,
		SPANISH_MODERN			=	0x0C0AUS,
		SPANISH_GUATEMALA		=	0x100AUS,
		SPANISH_COSTA_RICA		=	0x140AUS,
		SPANISH_PANAMA			=	0x180AUS,
		SPANISH_DOMINICAN		=	0x1C0AUS,
		SPANISH_VENEZUELA		=	0x200AUS,
		SPANISH_COLOMBIA		=	0x240AUS,
		SPANISH_PERU			=	0x280AUS,
		SPANISH_ARGENTINA		=	0x2C0AUS,
		SPANISH_ECUADOR			=	0x300AUS,
		SPANISH_CHILE			=	0x340AUS,
		SPANISH_URUGUAY			=	0x380AUS,
		SPANISH_PARAGUAY		=	0x3C0AUS,
		SPANISH_BOLIVIA			=	0x400AUS,
		SPANISH_EL_SALVADOR		=	0x440AUS,
		SPANISH_HONDURAS		=	0x480AUS,
		SPANISH_NICARAGUA		=	0x4C0AUS,
		SPANISH_PUERTO_RICO		=	0x500AUS,
		SUTU					=	0x0430US,
		SWAHILI_KENYA			=	0x0441US,
		SWEDISH					=	0x041DUS,
		SWEDISH_FINLAND			=	0x081DUS,
		TAMIL					=	0x0449US,
		TATAR_TATARSTAN			=	0x0444US,
		TELUGU					=	0x044AUS,
		THAI					=	0x041EUS,
		TURKISH					=	0x041FUS,
		UKRAINIAN				=	0x0422US,
		URDU_PAKISTAN			=	0x0420US,
		URDU_INDIA				=	0x0820US,
		UZBEK_LATIN				=	0x0443US,
		UZBEK_CYRILLIC			=	0x0843US,
		VIETNAMESE				=	0x042AUS,
		HID_USAGE_DATA			=	0x04FFUS,
		HID_VENDOR_DEFINED_1	=	0xF0FFUS,
		HID_VENDOR_DEFINED_2	=	0xF4FFUS,
		HID_VENDOR_DEFINED_3	=	0xF8FFUS,
		HID_VENDOR_DEFINED_4	=	0xFCFFUS,
	};
	struct __align(64) usb_descriptor_base {
		uint8_t length;
		usb_descriptor_type type;
	};
	template<typename T> struct usb_descriptor;
	template<__internal::__can_inherit T> struct usb_descriptor<T> : usb_descriptor_base, T {};
	template<typename T> requires(std::is_integral_v<T> || std::is_enum_v<T>)
	struct usb_descriptor<T[]>
	{
		uint8_t length;
		usb_descriptor_type type;
		T desc_data[/* (length - 2) / sizeof(T) */];
		constexpr size_t size_bytes() const noexcept { return length; }
		constexpr size_t size() const noexcept { return static_cast<size_t>(length - 2Z) / sizeof(T); }
		constexpr T* begin() noexcept { return desc_data; }
		constexpr T const* begin() const noexcept { return desc_data; }
		constexpr T* end() noexcept { return desc_data + size(); }
		constexpr T const* end() const noexcept { return desc_data + size(); }
		constexpr bool empty() const noexcept { return !length; }
	};
	struct usb_device_desc_data
	{
		uint16_t usb_spec_version_bcd;	// BCD encoding of the version of USB to which this device complies
		uint8_t class_code;
		uint8_t subclass_code;
		uint8_t protocol_code;
		uint8_t ep0_max_packet_size;	// maximum packet size for endpoint 0
		uint16_t vendor_id;
		uint16_t product_id;
		uint16_t device_release_bcd;	// BCD encoding of the device's release number
		uint8_t manufacturer_string_idx;
		uint8_t serial_string_idx;
		uint8_t num_configurations;		// number of possible configurations
	};
	struct usb_device_qualifier_desc_data
	{
		uint16_t usb_spec_version_bcd;	// BCD encoding of the version of USB to which this device complies (at the alternate speed)
		uint8_t class_code;
		uint8_t subclass_code;
		uint8_t protocol_code;
		uint8_t ep0_max_packet_size;	// maximum packet size for endpoint 0 (at the alternate speed)
		uint8_t num_configurations;		// number of possible configurations (at the alternate speed)
		uint8_t rsvd0;
	};
	struct usb_configuration_desc_data
	{
		uint16_t total_bytes;			// total length of all the descriptors returned from this request
		uint8_t num_interfaces;			// number of interfaces supported by this configuration
		uint8_t config_id;				// use this value as an argument in set_configuration to assign the config described by this descriptor to the device
		uint8_t string_desc_idx;		// index of a string descriptor describing this configuration
		struct __pack
		{
			bool 				: 5;
			bool remote_wakeup	: 1;	// whether the device supports remote wakeup
			bool self_powered	: 1;	// whether the device has a local power source
			bool rsvd1			: 1;	// reserved; set to 1 for historical reasons
		};
		uint8_t max_power;				// expressed in increments of 2mA
	};
	struct usb_interface_desc_data
	{
		uint8_t interface_id;
		uint8_t alternate_setting_id;	// value used to select the settings described by this descriptor for the interface (zero means it is the default)
		uint8_t num_endpoints;			// number of endpoints used by the interface, not including endpoint 0
		uint8_t class_code;
		uint8_t subclass_code;
		uint8_t protocol_code;
		uint8_t string_desc_idx;		// index of a string descriptor describing this interface
	};
	struct usb_endpoint_desc_data
	{
		struct __pack
		{
			uint8_t endpoint_number				: 4;
			bool								: 3;
			usb_transfer_direction direction	: 1;
		};
		struct __pack
		{
			usb_transfer_type transfer_type		: 2;
			usb_sync_type sync_type				: 2;
			usb_isoch_usage_type usage_type		: 2;
			bool								: 2;
		};
		struct __pack
		{
			uint16_t max_packet_size_bytes				: 11;
			uint8_t added_transactions_per_microframe	: 2;
			bool										: 3;
		};
		uint8_t polling_interval;
	};
	struct usb_descriptor_request_value
	{
		uint8_t descriptor_index;
		usb_descriptor_type desc_type;
		constexpr operator uint16_t() const noexcept { return std::bit_cast<uint16_t>(*this); }
	};
	struct usb_config_request_value
	{
		uint8_t config_id;
		bool			: 8;
		constexpr usb_config_request_value& operator=(uint8_t val) noexcept { config_id = val; return *this; }
		constexpr operator uint16_t() const noexcept { return config_id; }
	};
	struct __pack usb_device_address_value
	{
		uint8_t address	: 7;
		uint16_t		: 9;
		constexpr usb_device_address_value& operator=(uint8_t val) noexcept { address = val & 0x7FUC; return *this; }
		constexpr operator uint16_t() const noexcept { return address; }
	};
	typedef uint16_t usb_alternate_setting;
	struct __pack usb_endpoint_idx
	{
		uint8_t endpoint_number				: 4;
		bool								: 3;
		usb_transfer_direction direction	: 1;
	};
	typedef uint8_t usb_interface_idx;
	struct __pack usb_control_transfer_setup
	{
		usb_transfer_direction direction	: 1;
		usb_request_type request_type		: 2;
		usb_request_target recipient		: 5;
		usb_setup_request_code request_code;
		union __pack [[gnu::may_alias]] usb_setup_value_field
		{
			empty_word empty_value;
			usb_descriptor_request_value descriptor_request;
			usb_device_address_value device_address;
			usb_config_request_value config_request;
			usb_feature_selector feature_selector;
			usb_alternate_setting alternate_setting_id;
		} value_field;
		union __pack [[gnu::may_alias]] usb_setup_index_field
		{
			empty_word empty_idx{};
			struct __pack
			{
				union __pack [[gnu::may_alias]]
				{
					empty_byte lo_idx_byte;
					usb_endpoint_idx endpoint;
					usb_interface_idx interface;
				};
				union __pack [[gnu::may_alias]] {
					empty_byte hi_idx_byte;
					usb_test_selector test_selector;
				};
			};
			usb_lang_id lang_id;
		} index_field;
		uint16_t length;
	};
	typedef usb_descriptor<usb_device_desc_data> usb_device_descriptor;
	typedef usb_descriptor<usb_device_qualifier_desc_data> usb_device_qualifier_descriptor;
	typedef usb_descriptor<usb_configuration_desc_data> usb_configuration_descriptor;
	typedef usb_descriptor<usb_interface_desc_data> usb_interface_descriptor;
	typedef usb_descriptor<usb_endpoint_desc_data> usb_endpoint_descriptor;
	typedef usb_descriptor<usb_lang_id[]> usb_langid_support_descriptor;					// requested as string descriptor 0
	typedef usb_descriptor<char16_t[]> usb_string_descriptor;
	struct __pack usb_message_base
	{
		uint16_t total_message_size;				// size of the overall message structure (not the data to be sent, but rather this object)
		int transfer_size					: 17;	// any negative value indicates the maximum of 64KB
		usb_transfer_type transfer_type		: 2;
		usb_transfer_direction direction	: 1;
		usb_request_target recipient		: 2;	// whether the message is for an endpoint, interface, device, or other, if applicable
		usb_request_type request_type		: 2;
		bool enable_ioc						: 1;	// interrupt on completion, if applicable
		bool enable_short_packet_detect		: 1;
		uint8_t bus_error_limit				: 2;	// corresponds to the error_limit fields in xhci / uhci
		uint8_t target_endpoint				: 4;	// endpoint number, if applicable
		uint8_t target_interface;					// interface number, if applicable
		usb_setup_request_code request_code;		// request code, if applicable (ignored otherwise; exists for alignment reasons)
	};
	struct usb_control_message : usb_message_base {
		usb_control_transfer_setup::usb_setup_value_field value;
		usb_control_transfer_setup::usb_setup_index_field index;
	};
	struct usb_rw_message : usb_message_base {
		typedef simple_buffer<char, module_mm_allocator<char>> buffer_type;
		std::ranges::ref_view<buffer_type> data_buffer;
	};
	typedef abstract_hub_module<usb_message_base, usb_control_message, usb_rw_message> usb_host_controller_module;
}
#endif