#include "gdtoa.h"
void __ultod_d2a(uilong* u_l, uilong* bits, int exp, int k)
{
	switch(k & strtog_retmask)
	{
	case strtog_no_num:
	case strtog_zero: u_l[0] = u_l[1] = 0; break;
	case strtog_denormal:
		u_l[0]	= bits[0];
		u_l[1]	= bits[1];
		break;
	case strtog_normal:
	case strtog_nanbits:
		u_l[0]	= bits[0];
		u_l[1]	= (bits[1] & ~0x100000) | ((exp + 0x3FF + 52) << 20);
		break;
	case strtog_nomemory: *(__errno()) = 34;
	case strtog_infinite:
		u_l[1]	= 0x7FF00000;
		u_l[0]	= 0;
		break;
	case strog_nan: u_l[0] = 0x00000000; u_l[1] = 0x7FF80000;
	}
	if(k & strtog_neg) u_l[1] |= 0x80000000L;
}
int __strtord(const char* s, char** sp, int rounding, double* d)
{
	static fpi	fpi0 = { 53, -1074, 971, 1, 0 };
	fpi			*fpi, fpi1;
	uilong		bits[2];
	int			exp;
	int			k;
	fpi			= &fpi0;
	if(rounding != fpi_round_near)
	{
		fpi1			= fpi0;
		fpi1.rounding	= rounding;
		fpi				= &fpi1;
	}
	k			= __strtodg(s, sp, fpi, &exp, bits);
	__ultod_d2a((uilong*)d, bits, exp, k);
	return k;
}