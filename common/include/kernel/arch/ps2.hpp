#ifndef __PS2
#define __PS2
#include <kernel_api.hpp>
#include <arch/arch_amd64.h>
namespace ooos
{
	struct ps2_config_byte
	{
		bool ps2_p1_ien 	: 1;	// Interrupt enable, port 1
		bool ps2_p2_ien		: 1;	// Interrupt enable, port 2 (if present)
		bool ps2_sys		: 1;	// If this is false, something is wrong
		bool				: 1;
		bool ps2_p1_cdis	: 1;	// Clock disable, port 1
		bool ps2_p2_cdis	: 1;	// Clock disable, port 2 (if present)
		bool ps2_p1_xlat	: 1;	// Legacy scancode translation enable; this bit must be cleared to use a scanset other than set 1
		bool				: 1;
	};
	struct ps2_status_byte
	{
		bool ps2_out_avail	: 1;	// Indicates there is data to be read from IO port 0x60
		bool ps2_in_full	: 1;	// Indicates the input buffer on the controller is full; wait for this bit to be clear before writing to IO port 0x60 or 0x64
		bool ps2_sys		: 1;	// If this is false, something is wrong
		bool ps2_dtype		: 1;	// 0 = data for device, 1 = data for controller
		bool ps2_css0		: 1;	// Chipset-specific
		bool ps2_css1		: 1;	// Chipset-specific
		bool ps2_timeout	: 1;	// Indicates a timeout error
		bool ps2_perr		: 1;	// Indicates a parity error
	};
	enum ps2_command : uint8_t
	{
		PSC_MEM_READ		= 0x20UC,	// Read byte 0 from internal RAM; add N to this to read byte N (but these bytes are not standardized)
		PSC_MEM_WRITE		= 0x60UC,	// Write byte 0 of internal RAM; add N to this to write byte N (but these bytes are not standardized)
		PSC_P2DIS			= 0xA7UC,	// Disable the second port (only if 2 ports exist)
		PSC_P2EN			= 0xA8UC,	// Enable the second port (only if 2 ports exist)
		PSC_P2IST			= 0xA9UC,	// Internal self-test on port 2 (only if 2 ports exist)
		PSC_CIST			= 0xAAUC,	// Internal self-test on the controller
		PSC_P1IST			= 0xABUC,	// Internal self-test on port 1,
		PSC_P1DIS			= 0xADUC,	// Disable the first port
		PSC_P1EN			= 0xAEUC,	// Enable the first port
		PSC_P2LATCH			= 0xD4UC,	// Send the next byte to port 2 rather than port 1 (only if 2 ports exist)
	};
	constexpr word ps2_cmd_port 	= 0x64US;
	constexpr word ps2_data_port 	= 0x60US;
	enum class keyboard_scanset : uint8_t
    {
        SC_SET1 = 0x43UC,
        SC_SET2 = 0x41UC,
        SC_SET3 = 0x3FUC
    };
    enum class keycode : wchar_t
    {
        KC_MAX_CHAR     = 0x10FFFFU,        // Anything greater than this is a special key, an escape byte, or an unknown scan
        KC_F1           = 0x110001U,
        KC_F2           = 0x110002U,
        KC_F3           = 0x110003U,
        KC_F4           = 0x110004U,
        KC_F5           = 0x110005U,
        KC_F6           = 0x110006U,
        KC_F7           = 0x110007U,
        KC_F8           = 0x110008U,
        KC_F9           = 0x110009U,
        KC_F10          = 0x11000AU,
        KC_F11          = 0x11000BU,
        KC_F12          = 0x11000CU,
        KC_LCTRL        = 0x120001U,        // Left Ctrl
        KC_LSHFT        = 0x120002U,        // Left Shift
        KC_LALT         = 0x120003U,        // Left Alt
        KC_RCTRL        = 0x120004U,        // Right Ctrl
        KC_RSHFT        = 0x120005U,        // Right Shift
        KC_RALT         = 0x120006U,        // Right Alt
        KC_CAPS         = 0x120007U,        // Caps Lock
        KC_NUM          = 0x120008U,        // Num Lock
        KC_SCRLL        = 0x120009U,        // Scroll Lock
        KC_INS          = 0x12000AU,        // Insert
        KC_LGUI         = 0x130001U,        // Left Windows/GUI key
        KC_RGUI         = 0x130002U,        // Right Windows/GUI key
        KC_APPS         = 0x130003U,        // "Apps" key
        KC_PAUSE        = 0x130004U,        // Pause
        KC_PRTSC        = 0x130005U,        // Print Screen
        KC_END          = 0x140001U,
        KC_HOME         = 0x140002U,
        KC_PG_UP        = 0x140003U,        // Page Up
        KC_PG_DN        = 0x140004U,        // Page Down
        KC_LEFT         = 0x140005U,
        KC_RIGHT        = 0x140006U,
        KC_UP           = 0x140007U,
        KC_DOWN         = 0x140008U,
        KC_POWER        = 0x150001U,
        KC_SLEEP        = 0x150002U,
        KC_WAKE         = 0x150003U,
        KC_M1           = 0x160001U,        // Multimedia key 1 (Previous track)
        KC_M2           = 0x160002U,        // Multimedia key 2 (Next track)
        KC_M3           = 0x160003U,        // Multimedia key 3 (Mute)
        KC_M4           = 0x160004U,        // Multimedia key 4 (Calculator)
        KC_M5           = 0x160005U,        // Multimedia key 5 (Play)
        KC_M6           = 0x160006U,        // Multimedia key 6 (Stop)
		KC_M7           = 0x160007U,        // Multimedia key 7 (Volume down)
        KC_M8           = 0x160008U,        // Multimedia key 8 (Volume up)
        KC_M9           = 0x160009U,        // Multimedia key 9 (WWW home)
        KC_M10          = 0x16000AU,        // Multimedia key 10 (WWW search)
		KC_M11          = 0x16000BU,        // Multimedia key 11 (WWW favorites)
        KC_M12          = 0x16000CU,        // Multimedia key 12 (WWW refresh)
        KC_M13          = 0x16000DU,        // Multimedia key 13 (WWW stop)
        KC_M14          = 0x16000EU,        // Multimedia key 14 (WWW forward)
		KC_M15          = 0x16000FU,        // Multimedia key 15 (WWW back)
        KC_M16          = 0x160010U,        // Multimedia key 16 (My Computer)
        KC_M17          = 0x160011U,        // Multimedia key 17 (Email)
        KC_M18          = 0x160012U,        // Multimedia key 18 (Media Select)
        KC_BRB          = 0x1700F0U,        // Placeholder for a break code escape byte in scancode tables (generally 0xF0)
        KC_EXB          = 0x1700E0U,        // Placeholder for a general escape byte in scancode tables (generally 0xE0)
        KC_SEQB         = 0x1700E1U,        // Placeholder for an atomic sequence (i.e. no separate break sequence) escape byte in scancode tables (generally 0xE1)
        KC_SEQ_UNKNOWN  = 0x1E0000U,        // Placeholder for an unrecognized scan sequence; the first 2 scancodes received will be a little-endian word in the low 16 bits
        KC_UNKNOWN      = 0x1F0000U,        // Placeholder for an unrecognized single-byte scan; the scancode will be in the lowest 8 bits
        KC_EXT_UNKNOWN  = 0x1FE000U,        // Placeholder for an unrecognized escaped scan; the unrecognized byte (i.e. after the escape byte) will be in the lowest 8 bits
        // Note: Multimedia keys are not present on all keyboards. I am also uncertain as to the mappings' consistency, hence the use of generic numbers.
        // The mappings given are the ones mentioned on OSDev Wiki.
    };
	// This enum class is a bit like std::byte except it has some enum constants defined for its minimum, maximum, and default-on-reset values.
	enum class typematic_repeat_rate : uint8_t
	{
		RR_MIN 		= 0x00UC,
		RR_DEFAULT 	= 0x14UC,
		RR_MAX 		= 0x1FUC
	};
	enum class typematic_delay : uint8_t
	{
		TD_250 	= 0b00UC,
		TD_500 	= 0b01UC,
		TD_750 	= 0b10UC, 
		TD_1000 = 0b11UC
	};
	// The enum constants here correspond to known values I can find, but this list might not be exhaustive.
	enum class keyboard_id_bytes : uint16_t
	{
		KI_MF2_A 	= __be16(0xABUC, 0x83UC),
		KI_MF2_B	= __be16(0xABUC, 0xC1UC),
		KI_SHORT_KB	= __be16(0xABUC, 0x84UC),
		KI_NCD_N97	= __be16(0xABUC, 0x85UC),
		KI_STANDARD	= __be16(0xABUC, 0x86UC),
		KI_JP_G		= __be16(0xABUC, 0x90UC),
		KI_JP_P		= __be16(0xABUC, 0x91UC),
		KI_JP_A		= __be16(0xABUC, 0x92UC),
		KI_NCD_SUN	= __be16(0xACUC, 0xA1UC),
	};
	enum class keyboard_cmd_byte : uint8_t
	{
		KBC_SET_LEDS 	= 0xEDUC,	// Set LEDs; subcommand determines which
		KBC_ECHO		= 0xEEUC,	// Echo-test (response of 0xEE is expected)
		KBC_SCANSET		= 0xF0UC,	// Get/Set scanset; subcommand value 0 means get, or value 1/2/3 for the corresponding scanset
		KBC_IDENTIFY	= 0xF2UC,	// Get ID bytes
		KBC_TYPEMATIC	= 0xF3UC,	// Set the typematic byte (value as subcommand)
		KBC_SCEN		= 0xF4UC,	// Enable scanning
		KBC_SCDIS		= 0xF5UC,	// Disable scanning
		KBC_PARAMRST	= 0xF6UC,	// Reset to default parameters
		KBC_TMONLY_ALL	= 0xF7UC,	// Set all keys to typematic only (req. scanset 3)
		KBC_MRONLY_ALL	= 0xF8UC,	// Set all keys to make/release only (req. scanset 3)
		KBC_MKONLY_ALL	= 0xF9UC,	// Set all keys to make only (req. scanset 3)
		KBC_ALLSCAN_ALL	= 0xFAUC,	// Set all keys to typematic/make/release (req. scanset 3)
		KBC_TMONLY		= 0xFBUC,	// Set the key corresponding to the subcommand (scancode) to typematic only (req. scanset 3)
		KBC_MRONLY		= 0xFCUC,	// Set the key corresponding to the subcommand (scancode) to make/release only (req. scanset 3)
		KBC_MKONLY		= 0xFDUC,	// Set the key corresponding to the subcommand (scancode) to make only (req. scanset 3)
		KBC_RESEND		= 0xFEUC,	// Resend the last byte
		KBC_RESET		= 0xFFUC,	// Reset the keyboard and self-test
	};
	constexpr typematic_repeat_rate operator+(typematic_repeat_rate a, uint8_t b) noexcept { return static_cast<typematic_repeat_rate>(static_cast<uint8_t>(a) + b < 0x1FUC ? static_cast<uint8_t>(a) + b : 0x1FUC); }
	constexpr typematic_repeat_rate operator-(typematic_repeat_rate a, uint8_t b) noexcept { return static_cast<typematic_repeat_rate>(b < static_cast<uint8_t>(a) ? static_cast<uint8_t>(a) - b : 0UC); }
	constexpr typematic_repeat_rate& operator+=(typematic_repeat_rate& a, uint8_t b) noexcept { return (a = a + b); }
	constexpr typematic_repeat_rate& operator-=(typematic_repeat_rate& a, uint8_t b) noexcept { return (a = a - b); }
	struct __pack typematic_byte
	{
		typematic_repeat_rate repeat_rate 	: 	5 	= typematic_repeat_rate::RR_DEFAULT;
		typematic_delay delay_millis		:	2	= typematic_delay::TD_500;
		bool								:	1;
	};
	// Controller state variables
	struct __pack keyboard_cstate
	{
		keyboard_scanset 	scanset;
		typematic_byte		typematic;
		uint8_t				id_bytes[2];
	};
	// State-vector for the basic toggle-states of the keyboard.
    struct __pack keyboard_vstate
    {
        bool left_ctrl   : 1     = false;
        bool right_ctrl  : 1     = false;
        bool left_alt    : 1     = false;
        bool right_alt   : 1     = false;
        bool left_shift  : 1     = false;
        bool right_shift : 1     = false;
        bool caps_lock   : 1     = false;
        bool num_lock    : 1     = false;
		constexpr bool ctrl() const noexcept { return left_ctrl || right_ctrl; }
		constexpr bool alt() const noexcept { return left_alt || right_alt; }
		constexpr bool shift() const noexcept { return left_shift || right_shift; }
    };
	// State-vector for the LEDs on the keyboard.
	struct __pack keyboard_lstate
	{
		bool scroll_lock 	: 1 = false;
		bool num_lock		: 1 = false;
		bool caps_lock		: 1	= false;
		// Other bits may exist depending on the keyboard; might need to do more looking
		bool				: 5;
	};
    struct __pack keyboard_event
    {
        // The unicode codepoint of the character, if any, or the keycode for a special key (arrow keys, shift, caps lock, Fn keys, etc.).
        // The value of a special key will be out of range for unicode, so any such value indicates a special key.
        wchar_t kv_code     : 21;
        // If this bit is set, this is a break (key-up) event; if it is clear, it is a make (key-down) event.
        bool kv_release     : 1;
        // If this bit is set, the key is a num-pad numeral or symbol (the character code will remain the same).
        bool kv_numpad      : 1;
        // If this bit is set, the key generated multiple scancodes (other than escape bytes; e.g. BREAK or PAUSE in sets 1 and 2).
        bool kv_multiscan   : 1;
        // The state of the keyboard BEFORE the event — for example, if the key is right shift and the event is a key up, the right shift bit will still be set.
        // In prototype packets (the ones in the decode tables), the value will instead be an XOR mask to compute the state following the event.
        keyboard_vstate kv_vstate;
		// Extracting the character value from the packet requires some special handling to avoid accidental bit-field sign-extension.
		constexpr operator wchar_t() const noexcept { return static_cast<wchar_t>(kv_code & 0x1FFFFFU); }
		// Extracting the keycode value from the packet requires some special handling to avoid accidental bit-field sign-extension.
		constexpr operator keycode() const noexcept { return static_cast<keycode>(kv_code & 0x1FFFFFU); }
    };
	struct ps2_controller
	{
		bool ready;	// If this is false, the controller should not be used (or should be initialized first before being used)
		bool port1;	// Port 1 is functional (true) / not functional (false)
		bool port2;	// Port 2 exists and is functional (true) / doesn't exist or is nonfunctional (false)
		time_t io_timeout = 1000000UL;
		void io_wait();
		template<io_port_valid I> inline bool wait_for(I& i);
		template<io_port_valid I> inline bool operator>>(I& i) noexcept;
		template<io_port_valid I> inline ps2_controller& operator<<(I i);
		inline void dbump() noexcept { in(ps2_data_port); }
		inline ps2_status_byte status() const noexcept { return in<ps2_status_byte>(ps2_cmd_port); }
	};
	template<io_port_valid I>
	inline bool ps2_controller::operator>>(I& i) noexcept
	{
		ps2_status_byte status 		= in<ps2_status_byte>(ps2_cmd_port);
		if(status.ps2_out_avail) i 	= in<I>(ps2_data_port);
		return status.ps2_out_avail;
	}
	template<io_port_valid I>
	inline ps2_controller& ps2_controller::operator<<(I i)
	{
		this->io_wait();
		out(ps2_data_port, i);
		this->io_wait();
		return *this;
	}
	template<> 
	inline ps2_controller& ps2_controller::operator<<(ps2_command cmd)
	{
		this->io_wait();
		out(ps2_cmd_port, cmd);
		this->io_wait();
		return *this;
	}
	template<io_port_valid I>
	inline bool ps2_controller::wait_for(I& i)
	{
		bool read = false;
		for(time_t t = 0UL; t < io_timeout && !read; t++, read = (*this >> i));
		return read;	
	}
	__nointerrupts bool ps2_init(ps2_controller& ps2) noexcept;
}
#endif