#include "gdtoa.h"
void __rshift_d2a(big_int* b, int k)
{
	uilong *x, *x1, *xe, y;
	int		n;
	x		= x1 = b->x;
	n		= k >> 5;
	if(n < b->wds)
	{
		xe	= x + b->wds;
		x	+= n;
		if(k &= 31)
		{
			n	= 32 - k;
			y	= *x++ >> k;
			while(x < xe)
			{
				*x1++	= (y | (*x << n)) & 0xFFFFFFFF;
				y		= *x++ >> k;
			}
			if((*x1 = y) != 0) x1++;
		}
		else
			while(x < xe) *x1++ = *x++;
	}
	if((b->wds = x1 - b->x) == 0) b->x[0] = 0;
}
int __trailz_d2a(big_int* b)
{
	uilong u_l, *x, *xe;
	int    n	= 0;
	x			= b->x;
	xe			= x + b->wds;
	for(n = 0; x < xe && !*x; x++) n += 32;
	if(x < xe)
	{
		u_l		= *x;
		n		+= __lo0bits_d2a(&u_l);
	}
	return n;
}