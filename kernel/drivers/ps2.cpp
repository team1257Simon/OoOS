#include <arch/ps2.hpp>
#include <stdexcept>
namespace ooos
{
	void ps2_controller::io_wait()
	{
		ps2_status_byte status = in<ps2_status_byte>(ps2_cmd_port);
		for(time_t i = 0UL; i < io_timeout && status.ps2_in_full; i++, status = in<ps2_status_byte>(ps2_cmd_port));
		if(status.ps2_in_full) throw std::runtime_error("[PS2] controller timeout");
	}
	bool ps2_init(ps2_controller& ps2) noexcept try
	{
		(ps2 << PSC_P2DIS << PSC_P1DIS).dbump();
		ps2_config_byte cfg;
		ps2 << PSC_MEM_READ;
		if(!ps2.wait_for(cfg)) throw std::runtime_error("[PS2] controller timeout reading CFG[0]");
		cfg.ps2_p1_xlat	= false;
		cfg.ps2_p1_ien	= false;
		cfg.ps2_p1_cdis	= false;
		ps2 << PSC_MEM_WRITE << cfg << PSC_CIST;
		uint8_t response;
		if(!ps2.wait_for(response)) throw std::runtime_error("[PS2] controller timeout on selftest[0]");
		else if(response != 0x55UC) throw std::runtime_error("[PS2] controller self-test returned error code " + std::to_string(response, std::ext::hex));
		ps2 << PSC_MEM_WRITE << cfg << PSC_P2EN << PSC_MEM_READ;
		if(!ps2.wait_for(cfg)) throw std::runtime_error("[PS2] controller timeout writing CFG[1]");
		ps2.port2		= !cfg.ps2_p2_cdis;
		if(ps2.port2)
		{
			cfg.ps2_p2_cdis	= false;
			cfg.ps2_p2_ien	= false;
			ps2 << PSC_P2DIS << PSC_MEM_WRITE << cfg;
		}
		ps2 << PSC_P1IST;
		if(!ps2.wait_for(response)) throw std::runtime_error("[PS2] controller timeout on selftest[1]");
		ps2.port1			= !response;
		if(ps2.port2)
		{
			ps2 << PSC_P2IST;
			if(!ps2.wait_for(response)) throw std::runtime_error("[PS2] controller timeout on selftest[2]");
			ps2.port2	= !response;
		}
		if(!ps2.port1 && !ps2.port2) throw std::runtime_error("[PS2] no ports are functional");
		cfg.ps2_p1_cdis	= !ps2.port1;
		cfg.ps2_p1_ien	= ps2.port1;
		cfg.ps2_p2_cdis	= !ps2.port2;
		cfg.ps2_p2_ien	= ps2.port2;
		if(ps2.port1) ps2 << PSC_P1EN;
		if(ps2.port2) ps2 << PSC_P2EN;
		ps2 << PSC_MEM_WRITE << cfg;
		return true;
	}
	catch(std::exception& e) { panic(e.what()); return false; }
}
