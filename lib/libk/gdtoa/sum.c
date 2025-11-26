#include "gdtoa.h"
big_int* __sum_d2a(big_int* a, big_int* b)
{
	big_int* c;
	uilong	carry, *xc, *xa, *xb, *xe, y;
	uilong	z;
	if(a->wds < b->wds)
	{
		c	= b;
		b	= a;
		a	= c;
	}
	c		= __balloc_d2a(a->k);
	if(c == NULL) return (NULL);
	c->wds	= a->wds;
	carry	= 0;
	xa		= a->x;
	xb		= b->x;
	xc		= c->x;
	xe		= xc + b->wds;
	do {
		y		= (*xa & 0xFFFF) + (*xb & 0xFFFF) + carry;
		carry	= (y & 0x10000) >> 16;
		z		= (*xa++ >> 16) + (*xb++ >> 16) + carry;
		carry	= (z & 0x10000) >> 16;
		(((unsigned short*)xc)[1] = (unsigned short)z, ((unsigned short*)xc)[0] = (unsigned short)y, xc++);
	} while(xc < xe);
	xe			+= a->wds - b->wds;
	while(xc < xe)
	{
		y		= (*xa & 0xFFFF) + carry;
		carry	= (y & 0x10000) >> 16;
		z		= (*xa++ >> 16) + carry;
		carry	= (z & 0x10000) >> 16;
		(((unsigned short*)xc)[1] = (unsigned short)z, ((unsigned short*)xc)[0] = (unsigned short)y, xc++);
	}
	if(carry)
	{
		if(c->wds == c->maxwds)
		{
			b	= __balloc_d2a(c->k + 1);
			if(b == NULL) return (NULL);
			memcpy(&b->sign, &c->sign, c->wds * sizeof(uilong) + 2 * sizeof(int));
			__bfree_d2a(c);
			c	= b;
		}
		c->x[c->wds++]	= 1;
	}
	return c;
}