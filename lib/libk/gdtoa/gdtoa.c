#include <gdtoa.h>
static big_int* bitstob(uilong* bits, int nbits, int* bbits)
{
	int		i, k;
	big_int* b;
	uilong * be, *x, *x0;
	i	= 32;
	k	= 0;
	while(i < nbits)
	{
		i <<= 1;
		k++;
	}
	b	= __balloc_d2a(k);
	if(b == NULL) return (NULL);
	be	= bits + ((nbits - 1) >> 5);
	x	= x0 = b->x;
	do {
		*x++	= *bits & 0xFFFFFFFF;
	} while(++bits <= be);
	i	= x - x0;
	while(!x0[--i])
		if(!i)
		{
			b->wds	= 0;
			*bbits	= 0;
			goto ret;
		}
	b->wds	= i + 1;
	*bbits	= i * 32 + 32 - __hi0bits_d2a((uilong)(b->x[i]));
ret:
	return b;
}
char* __gdtoa(fpi* fpi, int be, uilong* bits, int* kindp, int mode, int ndigits, int* decpt, char** rve)
{
	int			bbits, b2, b5, be0, dig, i, ieps, ilim, ilim0, ilim1, inex;
	int			j, j1, k, k0, k_check, kind, leftright, m2, m5, nbits;
	int			rdir, s2, s5, spec_case, try_quick;
	int			u_l;
	big_int		*b, *b1, *delta, *mlo, *mhi, *mhi1, *S;
	double		d2, ds;
	char		*s, *s0;
	udouble 	d, eps;
	if(__dtoa_result_d2a)
	{
		__freedtoa(__dtoa_result_d2a);
		__dtoa_result_d2a	= 0;
	}
	inex					= 0;
	kind					= *kindp &= ~strtog_inexact;
	switch(kind & strtog_retmask)
	{
	case strtog_zero: goto ret_zero;
	case strtog_normal:
	case strtog_denormal: break;
	case strtog_infinite: *decpt	= -32768; return __nrv_alloc_d2a("Infinity", rve, 8);
	case strog_nan: *decpt			= -32768; return __nrv_alloc_d2a("NaN", rve, 3);
	default: return 0;
	}
	b	= bitstob(bits, nbits = fpi->nbits, &bbits);
	if(b == NULL) return (NULL);
	be0	= be;
	if((i = __trailz_d2a(b)) != 0)
	{
		__rshift_d2a(b, i);
		be		+= i;
		bbits	-= i;
	}
	if(!b->wds)
	{
		__bfree_d2a(b);
	ret_zero:
		*decpt	= 1;
		return __nrv_alloc_d2a("0", rve, 1);
	}
	(&d)->d		= __b2d_d2a(b, &i);
	i			= be + bbits - 1;
	(&d)->u_l[1] &= 0xFFFFF;
	(&d)->u_l[1] |= 0x3FF00000;
	ds = ((&d)->d - 1.5) * 0.289529654602168 + 0.1760912590558 + i * 0.301029995663981;
	if((j = i) < 0) j = -j;
	if((j -= 1077) > 0) ds += j * 7e-17;
	k		= (int)ds;
	if(ds < 0. && ds != k) k--;
	k_check	= 1;
	(&d)->u_l[1] += (be + bbits - 1) << 20;
	if(k >= 0 && k <= 22)
	{
		if((&d)->d < __tens_d2a[k]) k--;
		k_check	= 0;
	}
	j		= bbits - i - 1;
	if(j >= 0)
	{
		b2	= 0;
		s2	= j;
	}
	else
	{
		b2	= -j;
		s2	= 0;
	}
	if(k >= 0)
	{
		b5	= 0;
		s5	= k;
		s2	+= k;
	}
	else
	{
		b2	-= k;
		b5	= -k;
		s5	= 0;
	}
	if(mode < 0 || mode > 9) mode	= 0;
	try_quick	= 1;
	if(mode > 5)
	{
		mode		-= 4;
		try_quick	= 0;
	}
	else if(i >= 1018 || i < -1022)
		try_quick	= 0;
	leftright		= 1;
	ilim			= ilim1	= -1;
	switch(mode)
	{
	case 0:
	case 1:
		i		= (int)(nbits * .30103) + 3;
		ndigits	= 0;
		break;
	case 2: leftright	= 0;
	case 4:
		if(ndigits <= 0) ndigits	= 1;
		ilim						= ilim1	= i	= ndigits;
		break;
	case 3: leftright	= 0;
	case 5:
		i     			= ndigits + k + 1;
		ilim			= i;
		ilim1			= i - 1;
		if(i <= 0) i	= 1;
	}
	s					= s0		= __rv_alloc_d2a(i);
	if(s == NULL) return (NULL);
	if((rdir = fpi->rounding - 1) != 0)
	{
		if(rdir < 0) rdir			= 2;
		if(kind & strtog_neg) rdir	= 3 - rdir;
	}
	if(ilim >= 0 && ilim <= 14 && try_quick && !rdir && k == 0)
	{
		i		= 0;
		d2		= (&d)->d;
		k0		= k;
		ilim0	= ilim;
		ieps	= 2;
		if(k > 0)
		{
			ds	= __tens_d2a[k & 0xF];
			j	= k >> 4;
			if(j & 0x10)
			{
				j	&= 0xF;
				(&d)->d /= __bigtens_d2a[4];
				ieps++;
			}
			for(; j; j >>= 1, i++)
				if(j & 1)
				{
					ieps++;
					ds	*= __bigtens_d2a[i];
				}
		}
		else
		{
			ds	= 1.;
			if((j1 = -k) != 0)
			{
				(&d)->d	*= __tens_d2a[j1 & 0xF];
				for(j = j1 >> 4; j; j >>= 1, i++)
					if(j & 1)
					{
						ieps++;
						(&d)->d *= __bigtens_d2a[i];
					}
			}
		}
		if(k_check && (&d)->d < 1. && ilim > 0)
		{
			if(ilim1 <= 0) goto fast_failed;
			ilim	= ilim1;
			k--;
			(&d)->d	*= 10.;
			ieps++;
		}
		(&eps)->d		= ieps * (&d)->d + 7.;
		(&eps)->u_l[1]	-= (53 - 1) * 0x100000;
		if(ilim == 0)
		{
			S = mhi		= 0;
			(&d)->d		-= 5.;
			if((&d)->d > (&eps)->d) goto one_digit;
			if((&d)->d < -(&eps)->d) goto no_digits;
			goto fast_failed;
		}
		if(leftright)
		{
			(&eps)->d	= ds * 0.5 / __tens_d2a[ilim - 1] - (&eps)->d;
			for(i = 0;;)
			{
				u_l		= (int)((&d)->d / ds);
				(&d)->d	-= u_l * ds;
				*s++	= '0' + (int)u_l;
				if((&d)->d < (&eps)->d)
				{
					if((&d)->d) inex	= strtog_inexlo;
					goto ret1;
				}
				if(ds - (&d)->d < (&eps)->d) goto bump_up;
				if(++i >= ilim) break;
				(&eps)->d	*= 10.;
				(&d)->d		*= 10.;
			}
		}
		else
		{
			(&eps)->d		*= __tens_d2a[ilim - 1];
			for(i = 1;; i++, (&d)->d *= 10.)
			{
				if((u_l = (int)((&d)->d / ds)) != 0) (&d)->d -= u_l * ds;
				*s++		= '0' + (int)u_l;
				if(i == ilim)
				{
					ds		*= 0.5;
					if((&d)->d > ds + (&eps)->d)
						goto bump_up;
					else if((&d)->d < ds - (&eps)->d)
					{
						if((&d)->d) inex = strtog_inexlo;
						goto clear_trailing0;
					}
					break;
				}
			}
		}
	fast_failed:
		s		= s0;
		(&d)->d	= d2;
		k		= k0;
		ilim	= ilim0;
	}
	if(be >= 0 && k <= 14)
	{
		ds		= __tens_d2a[k];
		if(ndigits < 0 && ilim <= 0)
		{
			S	= mhi = 0;
			if(ilim < 0 || (&d)->d <= 5 * ds) goto no_digits;
			goto one_digit;
		}
		for(i = 1;; i++, (&d)->d *= 10.)
		{
			u_l	= (&d)->d / ds;
			(&d)->d	-= u_l * ds;
			*s++	= '0' + (int)u_l;
			if((&d)->d == 0.) break;
			if(i == ilim)
			{
				if(rdir)
				{
					if(rdir == 1) goto bump_up;
					inex	= strtog_inexlo;
					goto ret1;
				}
				(&d)->d += (&d)->d;
				if((&d)->d > ds || ((&d)->d == ds && u_l & 1))
				{
				bump_up:
					inex	= strtog_inexhi;
					while(*--s == '9')
						if(s == s0)
						{
							k++;
							*s	= '0';
							break;
						}
					++*s++;
				}
				else
				{
					inex	= strtog_inexlo;
				clear_trailing0:
					while(*--s == '0') { }
					++s;
				}
				break;
			}
		}
		goto ret1;
	}
	m2	= b2;
	m5	= b5;
	mhi	= mlo = 0;
	if(leftright)
	{
		i	= nbits - bbits;
		if(be - i++ < fpi->emin && mode != 3 && mode != 5)
		{
			i	= be - fpi->emin + 1;
			if(mode >= 2 && ilim > 0 && ilim < i) goto small_ilim;
		}
		else if(mode >= 2)
		{
		small_ilim:
			j	= ilim - 1;
			if(m5 >= j)
				m5 -= j;
			else
			{
				s5	+= j -= m5;
				b5	+= j;
				m5	= 0;
			}
			if((i = ilim) < 0)
			{
				m2	-= i;
				i	= 0;
			}
		}
		b2	+= i;
		s2	+= i;
		mhi	= __i2b_d2a(1);
		if(mhi == NULL) return (NULL);
	}
	if(m2 > 0 && s2 > 0)
	{
		i	= m2 < s2 ? m2 : s2;
		b2	-= i;
		m2	-= i;
		s2	-= i;
	}
	if(b5 > 0)
	{
		if(leftright)
		{
			if(m5 > 0)
			{
				mhi	= __pow5mult_d2a(mhi, m5);
				if(mhi == NULL) return (NULL);
				b1	= __mult_d2a(mhi, b);
				if(b1 == NULL) return (NULL);
				__bfree_d2a(b);
				b	= b1;
			}
			if((j = b5 - m5) != 0)
			{
				b	= __pow5mult_d2a(b, j);
				if(b == NULL) return (NULL);
			}
		}
		else
		{
			b = __pow5mult_d2a(b, b5);
			if(b == NULL) return (NULL);
		}
	}
	S		= __i2b_d2a(1);
	if(S == NULL) return (NULL);
	if(s5 > 0)
	{
		S	= __pow5mult_d2a(S, s5);
		if(S == NULL) return (NULL);
	}
	spec_case	= 0;
	if(mode < 2)
	{
		if(bbits == 1 && be0 > fpi->emin + 1)
		{
			b2++;
			s2++;
			spec_case	= 1;
		}
	}
	i					= ((s5 ? __hi0bits_d2a((uilong)(S->x[S->wds - 1])) : 32 - 1) - s2 - 4) & 31;
	m2					+= i;
	if((b2 += i) > 0)
	{
		b				= __lshift_d2a(b, b2);
		if(b == NULL) return (NULL);
	}
	if((s2 += i) > 0)
	{
		S				= __lshift_d2a(S, s2);
		if(S == NULL) return (NULL);
	}
	if(k_check)
	{
		if(__cmp_d2a(b, S) < 0)
		{
			k--;
			b			= __multadd_d2a(b, 10, 0);
			if(b == NULL) return (NULL);
			if(leftright)
			{
				mhi		= __multadd_d2a(mhi, 10, 0);
				if(mhi == NULL) return (NULL);
			}
			ilim		= ilim1;
		}
	}
	if(ilim <= 0 && mode > 2)
	{
		S				= __multadd_d2a(S, 5, 0);
		if(S == NULL) return (NULL);
		if(ilim < 0 || __cmp_d2a(b, S) <= 0)
		{
		no_digits:
			k		= -1 - ndigits;
			inex	= strtog_inexlo;
			goto ret;
		}
	one_digit:
		inex	= strtog_inexhi;
		*s++	= '1';
		k++;
		goto ret;
	}
	if(leftright)
	{
		if(m2 > 0)
		{
			mhi	= __lshift_d2a(mhi, m2);
			if(mhi == NULL) return (NULL);
		}
		mlo		= mhi;
		if(spec_case)
		{
			mhi	= __balloc_d2a(mhi->k);
			if(mhi == NULL) return (NULL);
			memcpy(&mhi->sign, &mlo->sign, mlo->wds * sizeof(uilong) + 2 * sizeof(int));
			mhi	= __lshift_d2a(mhi, 1);
			if(mhi == NULL) return (NULL);
		}
		for(i = 1;; i++)
		{
			dig		= __quorem_d2a(b, S) + '0';
			j		= __cmp_d2a(b, mlo);
			delta	= __diff_d2a(S, mhi);
			if(delta == NULL) return (NULL);
			j1		= delta->sign ? 1 : __cmp_d2a(b, delta);
			__bfree_d2a(delta);
			if(j1 == 0 && !mode && !(bits[0] & 1) && !rdir)
			{
				if(dig == '9') goto round_9_up;
				if(j <= 0)
				{
					if(b->wds > 1 || b->x[0]) inex = strtog_inexlo;
				}
				else
				{
					dig++;
					inex	= strtog_inexhi;
				}
				*s++		= dig;
				goto ret;
			}
			if(j < 0 || (j == 0 && !mode && !(bits[0] & 1)))
			{
				if(rdir && (b->wds > 1 || b->x[0]))
				{
					if(rdir == 2)
					{
						inex = strtog_inexlo;
						goto accept;
					}
					while(__cmp_d2a(S, mhi) > 0)
					{
						*s++	= dig;
						mhi1	= __multadd_d2a(mhi, 10, 0);
						if(mhi1 == NULL) return (NULL);
						if(mlo == mhi) mlo = mhi1;
						mhi		= mhi1;
						b		= __multadd_d2a(b, 10, 0);
						if(b == NULL) return (NULL);
						dig		= __quorem_d2a(b, S) + '0';
					}
					if(dig++ == '9') goto round_9_up;
					inex		= strtog_inexhi;
					goto accept;
				}
				if(j1 > 0)
				{
					b		= __lshift_d2a(b, 1);
					if(b == NULL) return (NULL);
					j1		= __cmp_d2a(b, S);
					if((j1 > 0 || (j1 == 0 && dig & 1)) && dig++ == '9') goto round_9_up;
					inex	= strtog_inexhi;
				}
				if(b->wds > 1 || b->x[0]) inex = strtog_inexlo;
			accept:
				*s++		= dig;
				goto ret;
			}
			if(j1 > 0 && rdir != 2)
			{
				if(dig == '9')
				{
				round_9_up:
					*s++	= '9';
					inex	= strtog_inexhi;
					goto roundoff;
				}
				inex		= strtog_inexhi;
				*s++		= dig + 1;
				goto ret;
			}
			*s++			= dig;
			if(i == ilim) break;
			b = __multadd_d2a(b, 10, 0);
			if(b == NULL) return (NULL);
			if(mlo == mhi)
			{
				mlo			= mhi	= __multadd_d2a(mhi, 10, 0);
				if(mlo == NULL) return (NULL);
			}
			else
			{
				mlo			= __multadd_d2a(mlo, 10, 0);
				if(mlo == NULL) return (NULL);
				mhi			= __multadd_d2a(mhi, 10, 0);
				if(mhi == NULL) return (NULL);
			}
		}
	}
	else
		for(i = 1;; i++)
		{
			*s++			= dig = __quorem_d2a(b, S) + '0';
			if(i >= ilim) break;
			b				= __multadd_d2a(b, 10, 0);
			if(b == NULL) return (NULL);
		}
	if(rdir)
	{
		if(rdir == 2 || (b->wds <= 1 && !b->x[0])) goto chopzeros;
		goto roundoff;
	}
	b	= __lshift_d2a(b, 1);
	if(b == NULL) return (NULL);
	j	= __cmp_d2a(b, S);
	if(j > 0 || (j == 0 && dig & 1))
	{
	roundoff:
		inex			= strtog_inexhi;
		while(*--s == '9')
			if(s == s0)
			{
				k++;
				*s++	= '1';
				goto ret;
			}
		++*s++;
	}
	else
	{
	chopzeros:
		if(b->wds > 1 || b->x[0]) inex = strtog_inexlo;
		while(*--s == '0') { }
		++s;
	}
ret:
	__bfree_d2a(S);
	if(mhi)
	{
		if(mlo && mlo != mhi) __bfree_d2a(mlo);
		__bfree_d2a(mhi);
	}
ret1:
	__bfree_d2a(b);
	*s				= 0;
	*decpt			= k + 1;
	if(rve) *rve	= s;
	*kindp			|= inex;
	return s0;
}