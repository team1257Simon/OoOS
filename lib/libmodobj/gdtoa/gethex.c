#include "gdtoa.h"
int __gethex_d2a(const char** sp, fpi* fpi, int* exp, big_int** bp, int sign)
{
	big_int*             b;
	const unsigned char *decpt, *s0, *s, *s1;
	int                  big, esign, havedig, irv, j, k, n, n0, nbits, up, zret;
	uilong               u_l, lostbits, *x;
	int                  e, e1;
	if(!__hexdig_d2a['0']) __hexdig_init_d2a();
	*bp     = 0;
	havedig = 0;
	s0      = *(const unsigned char**)sp + 2;
	while(s0[havedig] == '0') havedig++;
	s0 += havedig;
	s     = s0;
	decpt = 0;
	zret  = 0;
	e     = 0;
	if(__hexdig_d2a[*s])
		havedig++;
	else
	{
		zret = 1;
		if(*s != '.') goto pcheck;
		decpt = ++s;
		if(!__hexdig_d2a[*s]) goto pcheck;
		while(*s == '0') s++;
		if(__hexdig_d2a[*s]) zret = 0;
		havedig = 1;
		s0      = s;
	}
	while(__hexdig_d2a[*s]) s++;
	if(*s == '.' && !decpt)
	{
		decpt = ++s;
		while(__hexdig_d2a[*s]) s++;
	}
	if(decpt) e = -(((int)(s - decpt)) << 2);
pcheck:
	s1  = s;
	big = esign = 0;
	switch(*s)
	{
	case 'p':
	case 'P':
		switch(*++s)
		{
		case '-': esign = 1;
		case '+': s++;
		}
		if((n = __hexdig_d2a[*s]) == 0 || n > 0x19)
		{
			s = s1;
			break;
		}
		e1 = n - 0x10;
		while((n = __hexdig_d2a[*++s]) != 0 && n <= 0x19)
		{
			if(e1 & 0xF8000000) big = 1;
			e1 = 10 * e1 + n - 0x10;
		}
		if(esign) e1 = -e1;
		e += e1;
	}
	*sp = (char*)s;
	if(!havedig) *sp = (char*)s0 - 1;
	if(zret) return strtog_zero;
	if(big)
	{
		if(esign)
		{
			switch(fpi->rounding)
			{
			case fpi_round_up:
				if(sign) break;
				goto ret_tiny;
			case fpi_round_down:
				if(!sign) break;
				goto ret_tiny;
			}
			goto retz;
		ret_tiny:
			b = __balloc_d2a(0);
			if(b == NULL) return (strtog_nomemory);
			b->wds  = 1;
			b->x[0] = 1;
			goto dret;
		}
		switch(fpi->rounding)
		{
		case fpi_round_near: goto ovfl1;
		case fpi_round_up:
			if(!sign) goto ovfl1;
			goto ret_big;
		case fpi_round_down:
			if(sign) goto ovfl1;
			goto ret_big;
		}
	ret_big:
		nbits = fpi->nbits;
		n0 = n = nbits >> 5;
		if(nbits & 31) ++n;
		for(j = n, k = 0; j >>= 1; ++k)
			;
		*bp = b = __balloc_d2a(k);
		if(*bp == NULL) return (strtog_nomemory);
		b->wds = n;
		for(j = 0; j < n0; ++j) b->x[j] = 0xFFFFFFFF;
		if(n > n0) b->x[j] = 32 >> (32 - (nbits & 31));
		*exp = fpi->emin;
		return strtog_normal | strtog_inexlo;
	}
	n = s1 - s0 - 1;
	for(k = 0; n > (1 << 3) - 1; n >>= 1) k++;
	b = __balloc_d2a(k);
	if(b == NULL) return (strtog_nomemory);
	x   = b->x;
	n   = 0;
	u_l = 0;
	while(s1 > s0)
	{
		if(*--s1 == '.') continue;
		if(n == 32)
		{
			*x++ = u_l;
			u_l  = 0;
			n    = 0;
		}
		u_l |= (__hexdig_d2a[*s1] & 0xF) << n;
		n += 4;
	}
	*x++   = u_l;
	b->wds = n = x - b->x;
	n          = 32 * n - __hi0bits_d2a((uilong)(u_l));
	nbits      = fpi->nbits;
	lostbits   = 0;
	x          = b->x;
	if(n > nbits)
	{
		n -= nbits;
		if(__any_on_d2a(b, n))
		{
			lostbits = 1;
			k        = n - 1;
			if(x[k >> 5] & 1 << (k & 31))
			{
				lostbits = 2;
				if(k > 0 && __any_on_d2a(b, k)) lostbits = 3;
			}
		}
		__rshift_d2a(b, n);
		e += n;
	}
	else if(n < nbits)
	{
		n = nbits - n;
		b = __lshift_d2a(b, n);
		if(b == NULL) return (strtog_nomemory);
		e -= n;
		x = b->x;
	}
	if(e > fpi->emax)
	{
	ovfl:
		__bfree_d2a(b);
	ovfl1:
		*(__errno()) = 34;
		return strtog_infinite | strtog_oflow | strtog_inexhi;
	}
	irv = strtog_normal;
	if(e < fpi->emin)
	{
		irv = strtog_denormal;
		n   = fpi->emin - e;
		if(n >= nbits)
		{
			switch(fpi->rounding)
			{
			case fpi_round_near:
				if(n == nbits && (n < 2 || __any_on_d2a(b, n - 1))) goto one_bit;
				break;
			case fpi_round_up:
				if(!sign) goto one_bit;
				break;
			case fpi_round_down:
				if(sign)
				{
				one_bit:
					x[0] = b->wds = 1;
				dret:
					*bp          = b;
					*exp         = fpi->emin;
					*(__errno()) = 34;
					return strtog_denormal | strtog_inexhi | strtog_uflow;
				}
			}
			__bfree_d2a(b);
		retz:
			*(__errno()) = 34;
			return strtog_zero | strtog_inexlo | strtog_uflow;
		}
		k = n - 1;
		if(lostbits)
			lostbits = 1;
		else if(k > 0)
			lostbits = __any_on_d2a(b, k);
		if(x[k >> 5] & 1 << (k & 31)) lostbits |= 2;
		nbits -= n;
		__rshift_d2a(b, n);
		e = fpi->emin;
	}
	if(lostbits)
	{
		up = 0;
		switch(fpi->rounding)
		{
		case fpi_round_zero: break;
		case fpi_round_near:
			if(lostbits & 2 && (lostbits | x[0]) & 1) up = 1;
			break;
		case fpi_round_up: up = 1 - sign; break;
		case fpi_round_down: up = sign;
		}
		if(up)
		{
			k = b->wds;
			b = __increment_d2a(b);
			if(b == NULL) return (strtog_nomemory);
			x = b->x;
			if(irv == strtog_denormal)
			{
				if(nbits == fpi->nbits - 1 && x[nbits >> 5] & 1 << (nbits & 31)) irv = strtog_normal;
			}
			else if(b->wds > k || ((n = nbits & 31) != 0 && __hi0bits_d2a((uilong)(x[k - 1])) < 32 - n))
			{
				__rshift_d2a(b, 1);
				if(++e > fpi->emax) goto ovfl;
			}
			irv |= strtog_inexhi;
		}
		else
			irv |= strtog_inexlo;
	}
	*bp  = b;
	*exp = e;
	return irv;
}