#include "gdtoa.h"
char* __rv_alloc_D2A(int i)
{
  int j, k, *r;
  j = sizeof(ULong);
  for (k = 0; sizeof(Bigint) - sizeof(ULong) - sizeof(int) + j <= i; j <<= 1)
    k++;
  r = (int*)__Balloc_D2A(k);
  if (r == (NULL))
    return (__dtoa_result_D2A = (NULL));
  *r = k;
  return __dtoa_result_D2A = (char*)(r + 1);
}
char* __nrv_alloc_D2A(char* s, char** rve, int n)
{
  char *rv, *t;
  t = rv = __rv_alloc_D2A(n);
  if (t == (NULL))
    return ((NULL));
  while ((*t = *s++) != 0)
    t++;
  if (rve)
    *rve = t;
  return rv;
}
void __freedtoa(char* s)
{
  Bigint* b = (Bigint*)((int*)s - 1);
  b->maxwds = 1 << (b->k = *(int*)b);
  __Bfree_D2A(b);
  if (s == __dtoa_result_D2A)
    __dtoa_result_D2A = 0;
}
int __quorem_D2A(Bigint* b, Bigint* S)
{
  int n;
  ULong *bx, *bxe, q, *sx, *sxe;
  unsigned long long borrow, carry, y, ys;
  n = S->wds;
  if (b->wds < n)
    return 0;
  sx = S->x;
  sxe = sx + --n;
  bx = b->x;
  bxe = bx + n;
  q = *bxe / (*sxe + 1);
  if (q) {
    borrow = 0;
    carry = 0;
    do {
      ys = *sx++ * (unsigned long long)q + carry;
      carry = ys >> 32;
      y = *bx - (ys & 0xffffffffUL) - borrow;
      borrow = y >> 32 & 1UL;
      *bx++ = y & 0xffffffffUL;
    } while (sx <= sxe);
    if (!*bxe) {
      bx = b->x;
      while (--bxe > bx && !*bxe)
        --n;
      b->wds = n;
    }
  }
  if (__cmp_D2A(b, S) >= 0) {
    q++;
    borrow = 0;
    carry = 0;
    bx = b->x;
    sx = S->x;
    do {
      ys = *sx++ + carry;
      carry = ys >> 32;
      y = *bx - (ys & 0xffffffffUL) - borrow;
      borrow = y >> 32 & 1UL;
      *bx++ = y & 0xffffffffUL;
    } while (sx <= sxe);
    bx = b->x;
    bxe = bx + n;
    if (!*bxe) {
      while (--bxe > bx && !*bxe)
        --n;
      b->wds = n;
    }
  }
  return q;
}