#include "gdtoa.h"
char* __rv_alloc_d2a(int i)
{
	int j, k, *r;
	j = sizeof(uilong);
	for(k = 0; sizeof(big_int) - sizeof(uilong) - sizeof(int) + j <= i; j <<= 1) k++;
	r = (int*)__balloc_d2a(k);
	if(r == NULL) return (__dtoa_result_d2a = NULL);
	*r                       = k;
	return __dtoa_result_d2a = (char*)(r + 1);
}
char* __nrv_alloc_d2a(char* s, char** rve, int n)
{
	char *rv, *t;
	t = rv = __rv_alloc_d2a(n);
	if(t == NULL) return (NULL);
	while((*t = *s++) != 0) t++;
	if(rve) *rve = t;
	return rv;
}
void __freedtoa(char* s)
{
	big_int* b = (big_int*)((int*)s - 1);
	b->maxwds  = 1 << (b->k = *(int*)b);
	__bfree_d2a(b);
	if(s == __dtoa_result_d2a) __dtoa_result_d2a = 0;
}
int __quorem_d2a(big_int* b, big_int* S)
{
	int                n;
	uilong *           bx, *bxe, q, *sx, *sxe;
	unsigned long long borrow, carry, y, ys;
	n = S->wds;
	if(b->wds < n) return 0;
	sx  = S->x;
	sxe = sx + --n;
	bx  = b->x;
	bxe = bx + n;
	q   = *bxe / (*sxe + 1);
	if(q)
	{
		borrow = 0;
		carry  = 0;
		do {
			ys     = *sx++ * (unsigned long long)q + carry;
			carry  = ys >> 32;
			y      = *bx - (ys & 0xFFFFFFFFUL) - borrow;
			borrow = y >> 32 & 1UL;
			*bx++  = y & 0xFFFFFFFFUL;
		} while(sx <= sxe);
		if(!*bxe)
		{
			bx = b->x;
			while(--bxe > bx && !*bxe) --n;
			b->wds = n;
		}
	}
	if(__cmp_d2a(b, S) >= 0)
	{
		q++;
		borrow = 0;
		carry  = 0;
		bx     = b->x;
		sx     = S->x;
		do {
			ys     = *sx++ + carry;
			carry  = ys >> 32;
			y      = *bx - (ys & 0xFFFFFFFFUL) - borrow;
			borrow = y >> 32 & 1UL;
			*bx++  = y & 0xFFFFFFFFUL;
		} while(sx <= sxe);
		bx  = b->x;
		bxe = bx + n;
		if(!*bxe)
		{
			while(--bxe > bx && !*bxe) --n;
			b->wds = n;
		}
	}
	return q;
}