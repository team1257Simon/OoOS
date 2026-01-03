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