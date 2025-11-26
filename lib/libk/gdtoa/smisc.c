#include "gdtoa.h"
big_int* __s2b_d2a(const char* s, int nd0, int nd, uilong y9, int dplen)
{
	big_int* b;
	int		i, k;
	int		x, y;
	x		= (nd + 8) / 9;
	for(k = 0, y = 1; x > y; y <<= 1, k++);
	b		= __balloc_d2a(k);
	if(b == NULL) return (NULL);
	b->x[0] = y9;
	b->wds	= 1;
	i		= 9;
	if(9 < nd0)
	{
		s += 9;
		do {
			b = __multadd_d2a(b, 10, *s++ - '0');
			if(b == NULL) return (NULL);
		} while(++i < nd0);
		s += dplen;
	}
	else
		s += dplen + 9;
	for(; i < nd; i++)
	{
		b = __multadd_d2a(b, 10, *s++ - '0');
		if(b == NULL) return (NULL);
	}
	return b;
}
double __ratio_d2a(big_int* a, big_int* b)
{
	udouble	da, db;
	int		k, ka, kb;
	(&da)->d	= __b2d_d2a(a, &ka);
	(&db)->d	= __b2d_d2a(b, &kb);
	k			= ka - kb + 32 * (a->wds - b->wds);
	if(k > 0)
		(&da)->u_l[1] += k * 0x100000;
	else
	{
		k = -k;
		(&db)->u_l[1] += k * 0x100000;
	}
	return (&da)->d / (&db)->d;
}
int __match_d2a(const char** sp, char* t)
{
	int		c, d;
	const char* s = *sp;
	while((d = *t++) != 0)
	{
		if((c = *++s) >= 'A' && c <= 'Z') c += 'a' - 'A';
		if(c != d) return 0;
	}
	*sp		= s + 1;
	return 1;
}
void __copybits_d2a(uilong* c, int n, big_int* b)
{
	uilong *ce, *x, *xe;
	ce	= c + ((n - 1) >> 5) + 1;
	x	= b->x;
	xe	= x + b->wds;
	while(x < xe) *c++ = *x++;
	while(c < ce) *c++ = 0;
}
uilong __any_on_d2a(big_int* b, int k)
{
	int	n, nwds;
	uilong *x, *x0, x1, x2;
	x		= b->x;
	nwds	= b->wds;
	n		= k >> 5;
	if(n > nwds)
		n	= nwds;
	else if(n < nwds && (k &= 31))
	{
		x1	= x2	= x[n];
		x1 >>= k;
		x1 <<= k;
		if(x1 != x2) return 1;
	}
	x0		= x;
	x		+= n;
	while(x > x0)
		if(*--x) return 1;
	return 0;
}