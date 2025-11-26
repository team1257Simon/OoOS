#include "gdtoa.h"
static const int fivesbits[]	= { 0, 3, 5, 7, 10, 12, 14, 17, 19, 21, 24, 26, 28, 31, 33, 35, 38, 40, 42, 45, 47, 49, 52 };
big_int*		__increment_d2a(big_int* b)
{
	uilong * x, *xe;
	big_int* b1;
	x	= b->x;
	xe	= x + b->wds;
	do {
		if(*x < (uilong)0xFFFFFFFFL)
		{
			++*x;
			return b;
		}
		*x++	= 0;
	} while(x < xe);
	{
		if(b->wds >= b->maxwds)
		{
			b1	= __balloc_d2a(b->k + 1);
			if(b1 == NULL) return (NULL);
			memcpy(&b1->sign, &b->sign, b->wds * sizeof(uilong) + 2 * sizeof(int));
			__bfree_d2a(b);
			b	= b1;
		}
		b->x[b->wds++]	= 1;
	}
	return b;
}
void __decrement_d2a(big_int* b)
{
	uilong *x, *xe;
	x	= b->x;
	xe	= x + b->wds;
	do {
		if(*x)
		{
			--*x;
			break;
		}
		*x++	= 0xFFFFFFFFL;
	} while(x < xe);
}
static int all_on(big_int* b, int n)
{
	uilong *x, *xe;
	x	= b->x;
	xe	= x + (n >> 5);
	while(x < xe)
		if((*x++ & 0xFFFFFFFF) != 0xFFFFFFFF) return 0;
	if(n &= 31) return ((*x | (0xFFFFFFFF << n)) & 0xFFFFFFFF) == 0xFFFFFFFF;
	return 1;
}
big_int* __set_ones_d2a(big_int* b, int n)
{
	int	k;
	uilong *x, *xe;
	k	= (n + ((1 << 5) - 1)) >> 5;
	if(b->k < k)
	{
		__bfree_d2a(b);
		b	= __balloc_d2a(k);
		if(b == NULL) return (NULL);
	}
	k		= n >> 5;
	if(n &= 31) k++;
	b->wds	= k;
	x		= b->x;
	xe		= x + k;
	while(x < xe) *x++	= 0xFFFFFFFF;
	if(n) x[-1] >>= 32 - n;
	return b;
}
static int rv_ok(udouble* d, fpi* fpi, int* exp, uilong* bits, int exact, int rd, int* irv)
{
	big_int* b;
	uilong	carry, inex, lostbits;
	int		bdif, e, j, k, k1, nb, rv;
	carry	= rv	= 0;
	b		= __d2b_d2a((d)->d, &e, &bdif);
	if(b == NULL)
	{
		*irv	= strtog_nomemory;
		return (1);
	}
	bdif -= nb	= fpi->nbits;
	e += bdif;
	if(bdif <= 0)
	{
		if(exact) goto trunc;
		goto ret;
	}
	if(53 == nb)
	{
		if(exact && fpi->rounding == 1) goto trunc;
		goto ret;
	}
	switch(rd)
	{
	case 1: goto trunc;
	case 2: break;
	default:
		k	= bdif - 1;
		if(k < 0) goto trunc;
		if(!k)
		{
			if(!exact) goto ret;
			if(b->x[0] & 2) break;
			goto trunc;
		}
		if(b->x[k >> 5] & ((uilong)1 << (k & 31))) break;
		goto trunc;
	}
	carry	= 1;
trunc:
	inex	= lostbits	= 0;
	if(bdif > 0)
	{
		if((lostbits	= __any_on_d2a(b, bdif)) != 0) inex	= strtog_inexlo;
		__rshift_d2a(b, bdif);
		if(carry)
		{
			inex	= strtog_inexhi;
			b		= __increment_d2a(b);
			if(b == NULL)
			{
				*irv	= strtog_nomemory;
				return (1);
			}
			if((j	= nb & 31) != 0) j	= 32 - j;
			if(__hi0bits_d2a((uilong)(b->x[b->wds - 1])) != j)
			{
				if(!lostbits) lostbits	= b->x[0] & 1;
				__rshift_d2a(b, 1);
				e++;
			}
		}
	}
	else if(bdif < 0)
	{
		b	= __lshift_d2a(b, -bdif);
		if(b == NULL)
		{
			*irv	= strtog_nomemory;
			return (1);
		}
	}
	if(e < fpi->emin)
	{
		k	= fpi->emin - e;
		e	= fpi->emin;
		if(k > nb || fpi->sudden_underflow)
		{
			b->wds	= inex	= 0;
			*irv			= strtog_uflow | strtog_inexlo;
		}
		else
		{
			k1	= k - 1;
			if(k1 > 0 && !lostbits) lostbits	= __any_on_d2a(b, k1);
			if(!lostbits && !exact) goto ret;
			lostbits |= carry	= b->x[k1 >> 5] & (1 << (k1 & 31));
			__rshift_d2a(b, k);
			*irv	= strtog_denormal;
			if(carry)
			{
				b	= __increment_d2a(b);
				if(b == NULL)
				{
					*irv	= strtog_nomemory;
					return (1);
				}
				inex	= strtog_inexhi | strtog_uflow;
			}
			else if(lostbits)
				inex	= strtog_inexlo | strtog_uflow;
		}
	}
	else if(e > fpi->emax)
	{
		e			= fpi->emax + 1;
		*irv		= strtog_infinite | strtog_oflow | strtog_inexhi;
		*(__errno())	= 34;
		b->wds	= inex	= 0;
	}
	*exp	= e;
	__copybits_d2a(bits, nb, b);
	*irv |= inex;
	rv	= 1;
ret:
	__bfree_d2a(b);
	return rv;
}
static int mantbits(udouble* d)
{
	uilong u_l;
	if((u_l	= (d)->u_l[0]) != 0) return 53 - __lo0bits_d2a(&u_l);
	u_l	= (d)->u_l[1] | 0x100000;
	return 53 - 32 - __lo0bits_d2a(&u_l);
}
int __strtodg(const char* s00, char** se, fpi* fpi, int* exp, uilong* bits)
{
	int		abe, abits, asub;
	int		bb0, bb2, bb5, bbe, bd2, bd5, bbbits, bs2, c, decpt, denorm;
	int		dsign, e, e1, e2, emin, esign, finished, i, inex, irv;
	int		j, k, nbits, nd, nd0, nf, nz, nz0, rd, rvbits, rve, rve1, sign;
	int		sudden_underflow;
	const char *s, *s0, *s1;
	double		adj0, tol;
	int		u_l;
	udouble	adj, rv;
	uilong *	b, *be, y, z;
	big_int *	ab, *bb, *bb1, *bd, *bd0, *bs, *delta, *rvb, *rvb0;
	irv	= strtog_zero;
	denorm	= sign	= nz0	= nz	= 0;
	(&rv)->d				= 0.;
	rvb						= 0;
	nbits					= fpi->nbits;
	for(s	= s00;; s++) switch(*s)
		{
		case '-': sign	= 1;
		case '+':
			if(*++s) goto break2;
		case 0:
			sign	= 0;
			irv	= strtog_no_num;
			s	= s00;
			goto ret;
		case '\t':
		case '\n':
		case '\v':
		case '\f':
		case '\r':
		case ' ': continue;
		default: goto break2;
		}
break2:
	if(*s == '0')
	{
		switch(s[1])
		{
		case 'x':
		case 'X':
			irv	= __gethex_d2a(&s, fpi, exp, &rvb, sign);
			if(irv == strtog_nomemory) return (strtog_nomemory);
			if(irv == strtog_no_num)
			{
				s	= s00;
				sign	= 0;
			}
			goto ret;
		}
		nz0	= 1;
		while(*++s == '0')
			;
		if(!*s) goto ret;
	}
	sudden_underflow	= fpi->sudden_underflow;
	s0					= s;
	y					= z	= 0;
	for(decpt	= nd	= nf	= 0; (c	= *s) >= '0' && c <= '9'; nd++, s++)
		if(nd < 9)
			y	= 10 * y + c - '0';
		else if(nd < 16)
			z	= 10 * z + c - '0';
	nd0	= nd;
	if(c == '.')
	{
		c	= *++s;
		decpt	= 1;
		if(!nd)
		{
			for(; c == '0'; c	= *++s) nz++;
			if(c > '0' && c <= '9')
			{
				s0	= s;
				nf += nz;
				nz	= 0;
				goto have_dig;
			}
			goto dig_done;
		}
		for(; c >= '0' && c <= '9'; c	= *++s)
		{
		have_dig:
			nz++;
			if(c -= '0')
			{
				nf += nz;
				for(i	= 1; i < nz; i++)
					if(nd++ < 9)
						y *= 10;
					else if(nd <= 15 + 1)
						z *= 10;
				if(nd++ < 9)
					y	= 10 * y + c;
				else if(nd <= 15 + 1)
					z	= 10 * z + c;
				nz	= 0;
			}
		}
	}
dig_done:
	e	= 0;
	if(c == 'e' || c == 'E')
	{
		if(!nd && !nz && !nz0)
		{
			irv	= strtog_no_num;
			s	= s00;
			goto ret;
		}
		s00	= s;
		esign	= 0;
		switch(c	= *++s)
		{
		case '-': esign	= 1;
		case '+': c	= *++s;
		}
		if(c >= '0' && c <= '9')
		{
			while(c == '0') c	= *++s;
			if(c > '0' && c <= '9')
			{
				u_l	= c - '0';
				s1	= s;
				while((c	= *++s) >= '0' && c <= '9') u_l	= 10 * u_l + c - '0';
				if(s - s1 > 8 || u_l > 19999)
					e	= 19999;
				else
					e	= (int)u_l;
				if(esign) e	= -e;
			}
			else
				e	= 0;
		}
		else
			s	= s00;
	}
	if(!nd)
	{
		if(!nz && !nz0)
		{
			if(!decpt) switch(c)
				{
				case 'i':
				case 'I':
					if(__match_d2a(&s, "nf"))
					{
						--s;
						if(!__match_d2a(&s, "inity")) ++s;
						irv	= strtog_infinite;
						goto infnanexp;
					}
					break;
				case 'n':
				case 'N':
					if(__match_d2a(&s, "an"))
					{
						irv	= strog_nan;
						*exp	= fpi->emax + 1;
						if(*s == '(') irv	= __hexnan_d2a(&s, fpi, bits);
						goto infnanexp;
					}
				}
			irv	= strtog_no_num;
			s	= s00;
		}
		goto ret;
	}
	irv	= strtog_normal;
	e1	= e -= nf;
	rd	= 0;
	switch(fpi->rounding & 3)
	{
	case fpi_round_up: rd	= 2 - sign; break;
	case fpi_round_zero: rd	= 1; break;
	case fpi_round_down: rd	= 1 + sign;
	}
	if(!nd0) nd0	= nd;
	k		= nd < 15 + 1 ? nd : 15 + 1;
	(&rv)->d	= y;
	if(k > 9) (&rv)->d	= __tens_d2a[k - 9] * (&rv)->d + z;
	bd0	= 0;
	if(nbits <= 53 && nd <= 15)
	{
		if(!e)
		{
			if(rv_ok(&rv, fpi, exp, bits, 1, rd, &irv))
			{
				if(irv == strtog_nomemory) return (strtog_nomemory);
				goto ret;
			}
		}
		else if(e > 0)
		{
			if(e <= 22)
			{
				i	= fivesbits[e] + mantbits(&rv) <= 53;
				(&rv)->d *= __tens_d2a[e];
				if(rv_ok(&rv, fpi, exp, bits, i, rd, &irv))
				{
					if(irv == strtog_nomemory) return (strtog_nomemory);
					goto ret;
				}
				e1 -= e;
				goto rv_notOK;
			}
			i	= 15 - nd;
			if(e <= 22 + i)
			{
				e2	= e - i;
				e1 -= i;
				(&rv)->d *= __tens_d2a[i];
				(&rv)->d *= __tens_d2a[e2];
				if(rv_ok(&rv, fpi, exp, bits, 0, rd, &irv))
				{
					if(irv == strtog_nomemory) return (strtog_nomemory);
					goto ret;
				}
				e1 -= e2;
			}
		}
		else if(e >= -22)
		{
			(&rv)->d /= __tens_d2a[-e];
			if(rv_ok(&rv, fpi, exp, bits, 0, rd, &irv))
			{
				if(irv == strtog_nomemory) return (strtog_nomemory);
				goto ret;
			}
			e1 -= e;
		}
	}
rv_notOK:
	e1 += nd - k;
	e2	= 0;
	if(e1 > 0)
	{
		if((i	= e1 & 15) != 0) (&rv)->d *= __tens_d2a[i];
		if(e1 &= ~15)
		{
			e1 >>= 4;
			while(e1 >= (1 << (5 - 1)))
			{
				e2 += (((&rv)->u_l[1] & 0x7FF00000) >> 20) - 1023;
				(&rv)->u_l[1] &= ~0x7FF00000;
				(&rv)->u_l[1] |= 1023 << 20;
				(&rv)->d *= __bigtens_d2a[5 - 1];
				e1 -= 1 << (5 - 1);
			}
			e2 += (((&rv)->u_l[1] & 0x7FF00000) >> 20) - 1023;
			(&rv)->u_l[1] &= ~0x7FF00000;
			(&rv)->u_l[1] |= 1023 << 20;
			for(j	= 0; e1 > 0; j++, e1 >>= 1)
				if(e1 & 1) (&rv)->d *= __bigtens_d2a[j];
		}
	}
	else if(e1 < 0)
	{
		e1	= -e1;
		if((i	= e1 & 15) != 0) (&rv)->d /= __tens_d2a[i];
		if(e1 &= ~15)
		{
			e1 >>= 4;
			while(e1 >= (1 << (5 - 1)))
			{
				e2 += (((&rv)->u_l[1] & 0x7FF00000) >> 20) - 1023;
				(&rv)->u_l[1] &= ~0x7FF00000;
				(&rv)->u_l[1] |= 1023 << 20;
				(&rv)->d *= __tinytens_d2a[5 - 1];
				e1 -= 1 << (5 - 1);
			}
			e2 += (((&rv)->u_l[1] & 0x7FF00000) >> 20) - 1023;
			(&rv)->u_l[1] &= ~0x7FF00000;
			(&rv)->u_l[1] |= 1023 << 20;
			for(j	= 0; e1 > 0; j++, e1 >>= 1)
				if(e1 & 1) (&rv)->d *= __tinytens_d2a[j];
		}
	}
	rvb	= __d2b_d2a((&rv)->d, &rve, &rvbits);
	if(rvb == NULL) return (strtog_nomemory);
	rve += e2;
	if((j	= rvbits - nbits) > 0)
	{
		__rshift_d2a(rvb, j);
		rvbits	= nbits;
		rve += j;
	}
	bb0	= 0;
	e2	= rve + rvbits - nbits;
	if(e2 > fpi->emax + 1) goto huge;
	rve1	= rve + rvbits - nbits;
	if(e2 < (emin	= fpi->emin))
	{
		denorm	= 1;
		j		= rve - emin;
		if(j > 0)
		{
			rvb	= __lshift_d2a(rvb, j);
			if(rvb == NULL) return (strtog_nomemory);
			rvbits += j;
		}
		else if(j < 0)
		{
			rvbits += j;
			if(rvbits <= 0)
			{
				if(rvbits < -1)
				{
				ufl:
					rvb->wds	= 0;
					rvb->x[0]	= 0;
					*exp		= emin;
					irv		= strtog_uflow | strtog_inexlo;
					goto ret;
				}
				rvb->x[0]	= rvb->wds	= rvbits	= 1;
			}
			else
				__rshift_d2a(rvb, -j);
		}
		rve	= rve1	= emin;
		if(sudden_underflow && e2 + 1 < emin) goto ufl;
	}
	bd0	= __s2b_d2a(s0, nd0, nd, y, 1);
	if(bd0 == NULL) return (strtog_nomemory);
	for(;;)
	{
		bd	= __balloc_d2a(bd0->k);
		if(bd == NULL) return (strtog_nomemory);
		memcpy(&bd->sign, &bd0->sign, bd0->wds * sizeof(uilong) + 2 * sizeof(int));
		bb	= __balloc_d2a(rvb->k);
		if(bb == NULL) return (strtog_nomemory);
		memcpy(&bb->sign, &rvb->sign, rvb->wds * sizeof(uilong) + 2 * sizeof(int));
		bbbits	= rvbits - bb0;
		bbe	= rve + bb0;
		bs	= __i2b_d2a(1);
		if(bs == NULL) return (strtog_nomemory);
		if(e >= 0)
		{
			bb2	= bb5	= 0;
			bd2	= bd5	= e;
		}
		else
		{
			bb2	= bb5	= -e;
			bd2	= bd5	= 0;
		}
		if(bbe >= 0)
			bb2 += bbe;
		else
			bd2 -= bbe;
		bs2	= bb2;
		j	= nbits + 1 - bbbits;
		i	= bbe + bbbits - nbits;
		if(i < emin) j += i - emin;
		bb2 += j;
		bd2 += j;
		i	= bb2 < bd2 ? bb2 : bd2;
		if(i > bs2) i	= bs2;
		if(i > 0)
		{
			bb2 -= i;
			bd2 -= i;
			bs2 -= i;
		}
		if(bb5 > 0)
		{
			bs	= __pow5mult_d2a(bs, bb5);
			if(bs == NULL) return (strtog_nomemory);
			bb1	= __mult_d2a(bs, bb);
			if(bb1 == NULL) return (strtog_nomemory);
			__bfree_d2a(bb);
			bb	= bb1;
		}
		bb2 -= bb0;
		if(bb2 > 0)
		{
			bb	= __lshift_d2a(bb, bb2);
			if(bb == NULL) return (strtog_nomemory);
		}
		else if(bb2 < 0)
			__rshift_d2a(bb, -bb2);
		if(bd5 > 0)
		{
			bd	= __pow5mult_d2a(bd, bd5);
			if(bd == NULL) return (strtog_nomemory);
		}
		if(bd2 > 0)
		{
			bd	= __lshift_d2a(bd, bd2);
			if(bd == NULL) return (strtog_nomemory);
		}
		if(bs2 > 0)
		{
			bs	= __lshift_d2a(bs, bs2);
			if(bs == NULL) return (strtog_nomemory);
		}
		asub	= 1;
		inex	= strtog_inexhi;
		delta	= __diff_d2a(bb, bd);
		if(delta == NULL) return (strtog_nomemory);
		if(delta->wds <= 1 && !delta->x[0]) break;
		dsign		= delta->sign;
		delta->sign	= finished	= 0;
		u_l					= 0;
		i						= __cmp_d2a(delta, bs);
		if(rd && i <= 0)
		{
			irv	= strtog_normal;
			if((finished	= dsign ^ (rd & 1)) != 0)
			{
				if(dsign != 0)
				{
					irv |= strtog_inexhi;
					goto adj1;
				}
				irv |= strtog_inexlo;
				if(rve1 == emin) goto adj1;
				for(i	= 0, j	= nbits; j >= 32; i++, j -= 32)
				{
					if(rvb->x[i] & 0xFFFFFFFF) goto adj1;
				}
				if(j > 1 && __lo0bits_d2a(rvb->x + i) < j - 1) goto adj1;
				rve	= rve1 - 1;
				rvb	= __set_ones_d2a(rvb, rvbits	= nbits);
				if(rvb == NULL) return (strtog_nomemory);
				break;
			}
			irv |= dsign ? strtog_inexlo : strtog_inexhi;
			break;
		}
		if(i < 0)
		{
			irv	= dsign ? strtog_normal | strtog_inexlo : strtog_normal | strtog_inexhi;
			if(dsign || bbbits > 1 || denorm || rve1 == emin) break;
			delta	= __lshift_d2a(delta, 1);
			if(delta == NULL) return (strtog_nomemory);
			if(__cmp_d2a(delta, bs) > 0)
			{
				irv	= strtog_normal | strtog_inexlo;
				goto drop_down;
			}
			break;
		}
		if(i == 0)
		{
			if(dsign)
			{
				if(denorm && all_on(rvb, rvbits))
				{
					rvb->wds	= 1;
					rvb->x[0]	= 1;
					rve		= emin + nbits - (rvbits	= 1);
					irv		= strtog_normal | strtog_inexhi;
					denorm	= 0;
					break;
				}
				irv	= strtog_normal | strtog_inexlo;
			}
			else if(bbbits == 1)
			{
				irv	= strtog_normal;
			drop_down:
				if(rve1 == emin)
				{
					irv	= strtog_normal | strtog_inexhi;
					if(rvb->wds == 1 && rvb->x[0] == 1) sudden_underflow	= 1;
					break;
				}
				rve -= nbits;
				rvb	= __set_ones_d2a(rvb, rvbits	= nbits);
				if(rvb == NULL) return (strtog_nomemory);
				break;
			}
			else
				irv	= strtog_normal | strtog_inexhi;
			if((bbbits < nbits && !denorm) || !(rvb->x[0] & 1)) break;
			if(dsign)
			{
				rvb	= __increment_d2a(rvb);
				if(rvb == NULL) return (strtog_nomemory);
				j	= 31 & (32 - (rvbits & 31));
				if(__hi0bits_d2a((uilong)(rvb->x[rvb->wds - 1])) != j) rvbits++;
				irv	= strtog_normal | strtog_inexhi;
			}
			else
			{
				if(bbbits == 1) goto undfl;
				__decrement_d2a(rvb);
				irv	= strtog_normal | strtog_inexlo;
			}
			break;
		}
		if(((&adj)->d	= __ratio_d2a(delta, bs)) <= 2.)
		{
		adj1:
			inex	= strtog_inexlo;
			if(dsign)
			{
				asub	= 0;
				inex	= strtog_inexhi;
			}
			else if(denorm && bbbits <= 1)
			{
			undfl:
				rvb->wds	= 0;
				rve		= emin;
				irv		= strtog_uflow | strtog_inexlo;
				break;
			}
			adj0	= (&adj)->d	= 1.;
		}
		else
		{
			adj0	= (&adj)->d *= 0.5;
			if(dsign)
			{
				asub	= 0;
				inex	= strtog_inexlo;
			}
			if((&adj)->d < 2147483647.)
			{
				u_l	= adj0;
				adj0 -= u_l;
				switch(rd)
				{
				case 0:
					if(adj0 >= .5) goto inc_L;
					break;
				case 1:
					if(asub && adj0 > 0.) goto inc_L;
					break;
				case 2:
					if(!asub && adj0 > 0.)
					{
					inc_L:
						u_l++;
						inex	= strtog_inexact - inex;
					}
				}
				(&adj)->d	= u_l;
			}
		}
		y	= rve + rvbits;
		if(!denorm && rvbits < nbits)
		{
			rvb	= __lshift_d2a(rvb, j	= nbits - rvbits);
			if(rvb == NULL) return (strtog_nomemory);
			rve -= j;
			rvbits	= nbits;
		}
		ab	= __d2b_d2a((&adj)->d, &abe, &abits);
		if(ab == NULL) return (strtog_nomemory);
		if(abe < 0)
			__rshift_d2a(ab, -abe);
		else if(abe > 0)
		{
			ab	= __lshift_d2a(ab, abe);
			if(ab == NULL) return (strtog_nomemory);
		}
		rvb0	= rvb;
		if(asub)
		{
			j	= __hi0bits_d2a((uilong)(rvb->x[rvb->wds - 1]));
			rvb	= __diff_d2a(rvb, ab);
			if(rvb == NULL) return (strtog_nomemory);
			k	= rvb0->wds - 1;
			if(denorm)
				;
			else if(rvb->wds <= k || __hi0bits_d2a((uilong)(rvb->x[k])) > __hi0bits_d2a((uilong)(rvb0->x[k])))
			{
				if(rve1 == emin)
				{
					--rvbits;
					denorm	= 1;
				}
				else
				{
					rvb	= __lshift_d2a(rvb, 1);
					if(rvb == NULL) return (strtog_nomemory);
					--rve;
					--rve1;
					u_l	= finished	= 0;
				}
			}
		}
		else
		{
			rvb	= __sum_d2a(rvb, ab);
			if(rvb == NULL) return (strtog_nomemory);
			k	= rvb->wds - 1;
			if(k >= rvb0->wds || __hi0bits_d2a((uilong)(rvb->x[k])) < __hi0bits_d2a((uilong)(rvb0->x[k])))
			{
				if(denorm)
				{
					if(++rvbits == nbits) denorm	= 0;
				}
				else
				{
					__rshift_d2a(rvb, 1);
					rve++;
					rve1++;
					u_l	= 0;
				}
			}
		}
		__bfree_d2a(ab);
		__bfree_d2a(rvb0);
		if(finished) break;
		z	= rve + rvbits;
		if(y == z && u_l)
		{
			tol		= (&adj)->d * 5E-16;
			(&adj)->d	= adj0 - .5;
			if((&adj)->d < -tol)
			{
				if(adj0 > tol)
				{
					irv |= inex;
					break;
				}
			}
			else if((&adj)->d > tol && adj0 < 1. - tol)
			{
				irv |= inex;
				break;
			}
		}
		bb0	= denorm ? 0 : __trailz_d2a(rvb);
		__bfree_d2a(bb);
		__bfree_d2a(bd);
		__bfree_d2a(bs);
		__bfree_d2a(delta);
	}
	if(!denorm && (j	= nbits - rvbits))
	{
		if(j > 0)
		{
			rvb	= __lshift_d2a(rvb, j);
			if(rvb == NULL) return (strtog_nomemory);
		}
		else
			__rshift_d2a(rvb, -j);
		rve -= j;
	}
	*exp	= rve;
	__bfree_d2a(bb);
	__bfree_d2a(bd);
	__bfree_d2a(bs);
	__bfree_d2a(bd0);
	__bfree_d2a(delta);
	if(rve > fpi->emax)
	{
		switch(fpi->rounding & 3)
		{
		case fpi_round_near: goto huge;
		case fpi_round_up:
			if(!sign) goto huge;
			break;
		case fpi_round_down:
			if(sign) goto huge;
		}
		__bfree_d2a(rvb);
		rvb	= 0;
		irv	= strtog_normal | strtog_inexlo;
		*exp	= fpi->emax;
		b	= bits;
		be	= b + ((fpi->nbits + 31) >> 5);
		while(b < be) *b++	= -1;
		if((j	= fpi->nbits & 0x1f)) *--be >>= (32 - j);
		goto ret;
	huge:
		rvb->wds		= 0;
		irv				= strtog_infinite | strtog_oflow | strtog_inexhi;
		*(__errno())	= 34;
	infnanexp:
		*exp			= fpi->emax + 1;
	}
ret:
	if(denorm)
	{
		if(sudden_underflow)
		{
			rvb->wds		= 0;
			irv				= strtog_uflow | strtog_inexlo;
			*(__errno())	= 34;
		}
		else
		{
			irv	= (irv & ~strtog_retmask) | (rvb->wds > 0 ? strtog_denormal : strtog_zero);
			if(irv & strtog_inexact)
			{
				irv |= strtog_uflow;
				*(__errno())	= 34;
			}
		}
	}
	if(se) *se		= (char*)s;
	if(sign) irv	|= strtog_neg;
	if(rvb)
	{
		__copybits_d2a(bits, nbits, rvb);
		__bfree_d2a(rvb);
	}
	return irv;
}