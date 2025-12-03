#include <arch/arch_amd64.h>
extern "C"
{
	extern tss system_tss;
	void tss_init(addr_t k_rsp)
	{
		system_tss.rsp[0] 		= k_rsp;
		system_tss.rsp[1] 		= k_rsp;
		system_tss.rsp[2] 		= k_rsp;
		for(int i = 0; i < 7; i++)
			system_tss.ist[i] 	= k_rsp;
	}
}