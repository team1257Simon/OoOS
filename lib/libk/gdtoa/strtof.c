#include <gdtoa.h>
float strtof(const char* s, char** sp)
{
	static fpi	fpi0 = { 24, -149, 104, 1, 0 };
	uilong		bits[1];
	int			exp;
	int			k;
	union
	{
		uilong	u_l[1];
		float	f;
	}	u;
	u.u_l[0]	= 0;
	k			= __strtodg(s, sp, &fpi0, &exp, bits);
	switch(k & strtog_retmask)
	{
	case strtog_no_num:
	case strtog_zero: u.u_l[0] = 0; break;
	case strtog_normal:
	case strtog_nanbits: u.u_l[0] = (bits[0] & 0x7FFFFF) | ((exp + 0x7F + 23) << 23); break;
	case strtog_denormal: u.u_l[0] = bits[0]; break;
	case strtog_nomemory: *(__errno()) = 34;
	case strtog_infinite: u.u_l[0] = 0x7F800000; break;
	case strog_nan: u.u_l[0] = 0x7FC00000;
	}
	if(k & strtog_neg) u.u_l[0] |= 0x80000000L;
	return u.f;
}