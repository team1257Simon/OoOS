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
		PSC_DEV_SCDIS		= 0xF5UC,	// Disable scanning (to device)
		PSC_DEV_SCEN		= 0xF4UC,	// Enable scanning (to device)
	};
	constexpr word ps2_cmd_port 	= 0x64US;
	constexpr word ps2_data_port 	= 0x60US;
	struct ps2_controller
	{
		bool ready;	// If this is false, the controller should not be used (or should be initialized first before being used)
		bool port1;	// Port 1 is functional (true) / not functional (false)
		bool port2;	// Port 2 exists and is functional (true) / doesn't exist or is nonfunctional (false)
		time_t io_timeout = 1000000UL;
		void io_wait();
		template<io_port_valid I> inline bool operator>>(I& i) noexcept;
		template<io_port_valid I> inline ps2_controller& operator<<(I i);
		inline void dbump() noexcept { in(ps2_data_port); }
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
	attribute(nointerrupts) bool ps2_init(ps2_controller& ps2) noexcept;
}
#endif