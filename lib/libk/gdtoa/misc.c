#include "gdtoa.h"
static Bigint* freelist[9 + 1];
static double private_mem[((2304 + sizeof(double) - 1) / sizeof(double))], *pmem_next = private_mem;
Bigint* __Balloc_D2A(int k)
{
  int x;
  Bigint* rv;
  unsigned int len;
  ;
  if (k <= 9 && (rv = freelist[k]) != 0) {
    freelist[k] = rv->next;
  } else {
    x = 1 << k;
    len = (sizeof(Bigint) + (x - 1) * sizeof(ULong) + sizeof(double) - 1) / sizeof(double);
    if (k <= 9 && pmem_next - private_mem + len <= ((2304 + sizeof(double) - 1) / sizeof(double))) {
      rv = (Bigint*)pmem_next;
      pmem_next += len;
    } else {
      rv = (Bigint*)malloc(len * sizeof(double));
      if (rv == NULL)
        return (NULL);
    }
    rv->k = k;
    rv->maxwds = x;
  };
  rv->sign = rv->wds = 0;
  return rv;
}
void __Bfree_D2A(Bigint* v)
{
  if (v) {
    if (v->k > 9)
      free(v);
    else {
      ;
      v->next = freelist[v->k];
      freelist[v->k] = v;
      ;
    }
  }
}
int __lo0bits_D2A(ULong* y)
{
  int k;
  ULong x = *y;
  if (x & 7) {
    if (x & 1)
      return 0;
    if (x & 2) {
      *y = x >> 1;
      return 1;
    }
    *y = x >> 2;
    return 2;
  }
  k = 0;
  if (!(x & 0xffff)) {
    k = 16;
    x >>= 16;
  }
  if (!(x & 0xff)) {
    k += 8;
    x >>= 8;
  }
  if (!(x & 0xf)) {
    k += 4;
    x >>= 4;
  }
  if (!(x & 0x3)) {
    k += 2;
    x >>= 2;
  }
  if (!(x & 1)) {
    k++;
    x >>= 1;
    if (!x)
      return 32;
  }
  *y = x;
  return k;
}
Bigint* __multadd_D2A(Bigint* b, int m, int a)
{
  int i, wds;
  ULong* x;
  unsigned long long carry, y;
  Bigint* b1;
  wds = b->wds;
  x = b->x;
  i = 0;
  carry = a;
  do {
    y = *x * (unsigned long long)m + carry;
    carry = y >> 32;
    *x++ = y & 0xffffffffUL;

  } while (++i < wds);
  if (carry) {
    if (wds >= b->maxwds) {
      b1 = __Balloc_D2A(b->k + 1);
      if (b1 == NULL)
        return (NULL);
      memcpy(&b1->sign, &b->sign, b->wds * sizeof(ULong) + 2 * sizeof(int));
      __Bfree_D2A(b);
      b = b1;
    }
    b->x[wds++] = carry;
    b->wds = wds;
  }
  return b;
}
int __hi0bits_D2A(ULong x)
{
  int k = 0;
  if (!(x & 0xffff0000)) {
    k = 16;
    x <<= 16;
  }
  if (!(x & 0xff000000)) {
    k += 8;
    x <<= 8;
  }
  if (!(x & 0xf0000000)) {
    k += 4;
    x <<= 4;
  }
  if (!(x & 0xc0000000)) {
    k += 2;
    x <<= 2;
  }
  if (!(x & 0x80000000)) {
    k++;
    if (!(x & 0x40000000))
      return 32;
  }
  return k;
}
Bigint* __i2b_D2A(int i)
{
  Bigint* b;
  b = __Balloc_D2A(1);
  if (b == NULL)
    return (NULL);
  b->x[0] = i;
  b->wds = 1;
  return b;
}
Bigint* __mult_D2A(Bigint* a, Bigint* b)
{
  Bigint* c;
  int k, wa, wb, wc;
  ULong *x, *xa, *xae, *xb, *xbe, *xc, *xc0;
  ULong y;
  unsigned long long carry, z;
  if (a->wds < b->wds) {
    c = a;
    a = b;
    b = c;
  }
  k = a->k;
  wa = a->wds;
  wb = b->wds;
  wc = wa + wb;
  if (wc > a->maxwds)
    k++;
  c = __Balloc_D2A(k);
  if (c == NULL)
    return (NULL);
  for (x = c->x, xa = x + wc; x < xa; x++)
    *x = 0;
  xa = a->x;
  xae = xa + wa;
  xb = b->x;
  xbe = xb + wb;
  xc0 = c->x;
  for (; xb < xbe; xc0++) {
    if ((y = *xb++) != 0) {
      x = xa;
      xc = xc0;
      carry = 0;
      do {
        z = *x++ * (unsigned long long)y + *xc + carry;
        carry = z >> 32;
        *xc++ = z & 0xffffffffUL;
      } while (x < xae);
      *xc = carry;
    }
  }

  for (xc0 = c->x, xc = xc0 + wc; wc > 0 && !*--xc; --wc)
    ;
  c->wds = wc;
  return c;
}
static Bigint* p5s;
Bigint* __pow5mult_D2A(Bigint* b, int k)
{
  Bigint *b1, *p5, *p51;
  int i;
  static int p05[3] = { 5, 25, 125 };
  if ((i = k & 3) != 0) {
    b = __multadd_D2A(b, p05[i - 1], 0);
    if (b == NULL)
      return (NULL);
  }
  if (!(k >>= 2))
    return b;
  if ((p5 = p5s) == 0) {

    p5 = p5s = __i2b_D2A(625);
    if (p5 == NULL)
      return NULL;
    p5->next = 0;
  }
  for (;;) {
    if (k & 1) {
      b1 = __mult_D2A(b, p5);
      if (b1 == NULL)
        return (NULL);
      __Bfree_D2A(b);
      b = b1;
    }
    if (!(k >>= 1))
      break;
    if ((p51 = p5->next) == 0) {

      p51 = p5->next = __mult_D2A(p5, p5);
      if (p51 == NULL)
        return (

            NULL);
      p51->next = 0;
    }
    p5 = p51;
  }
  return b;
}
Bigint* __lshift_D2A(Bigint* b, int k)
{
  int i, k1, n, n1;
  Bigint* b1;
  ULong *x, *x1, *xe, z;
  n = k >> 5;
  k1 = b->k;
  n1 = n + b->wds + 1;
  for (i = b->maxwds; n1 > i; i <<= 1)
    k1++;
  b1 = __Balloc_D2A(k1);
  if (b1 == NULL)
    return (NULL);
  x1 = b1->x;
  for (i = 0; i < n; i++)
    *x1++ = 0;
  x = b->x;
  xe = x + b->wds;
  if (k &= 31) {
    k1 = 32 - k;
    z = 0;
    do {
      *x1++ = *x << k | z;
      z = *x++ >> k1;
    } while (x < xe);
    if ((*x1 = z) != 0)
      ++n1;

  } else
    do
      *x1++ = *x++;
    while (x < xe);
  b1->wds = n1 - 1;
  __Bfree_D2A(b);
  return b1;
}
int __cmp_D2A(Bigint* a, Bigint* b)
{
  ULong *xa, *xa0, *xb, *xb0;
  int i, j;
  i = a->wds;
  j = b->wds;
  if (i -= j)
    return i;
  xa0 = a->x;
  xa = xa0 + j;
  xb0 = b->x;
  xb = xb0 + j;
  for (;;) {
    if (*--xa != *--xb)
      return *xa < *xb ? -1 : 1;
    if (xa <= xa0)
      break;
  }
  return 0;
}
Bigint* __diff_D2A(Bigint* a, Bigint* b)
{
  Bigint* c;
  int i, wa, wb;
  ULong *xa, *xae, *xb, *xbe, *xc;
  unsigned long long borrow, y;
  i = __cmp_D2A(a, b);
  if (!i) {
    c = __Balloc_D2A(0);
    if (c == NULL)
      return NULL;
    c->wds = 1;
    c->x[0] = 0;
    return c;
  }
  if (i < 0) {
    c = a;
    a = b;
    b = c;
    i = 1;
  } else
    i = 0;
  c = __Balloc_D2A(a->k);
  if (c ==NULL)
    return (NULL);
  c->sign = i;
  wa = a->wds;
  xa = a->x;
  xae = xa + wa;
  wb = b->wds;
  xb = b->x;
  xbe = xb + wb;
  xc = c->x;
  borrow = 0;
  do {
    y = (unsigned long long)*xa++ - *xb++ - borrow;
    borrow = y >> 32 & 1UL;
    *xc++ = y & 0xffffffffUL;
  } while (xb < xbe);
  while (xa < xae) {
    y = *xa++ - borrow;
    borrow = y >> 32 & 1UL;
    *xc++ = y & 0xffffffffUL;
  }

  while (!*--xc)
    wa--;
  c->wds = wa;
  return c;
}
double __b2d_D2A(Bigint* a, int* e)
{
  ULong *xa, *xa0, w, y, z;
  int k;
  U d;
  xa0 = a->x;
  xa = xa0 + a->wds;
  y = *--xa;
  k = __hi0bits_D2A((ULong)(y));
  *e = 32 - k;
  if (k < 11) {
    (&d)->L[1] = 0x3ff00000 | y >> (11 - k);
    w = xa > xa0 ? *--xa : 0;
    (&d)->L[0] = y << ((32 - 11) + k) | w >> (11 - k);
    goto ret_d;
  }
  z = xa > xa0 ? *--xa : 0;
  if (k -= 11) {
    (&d)->L[1] = 0x3ff00000 | y << k | z >> (32 - k);
    y = xa > xa0 ? *--xa : 0;
    (&d)->L[0] = z << k | y >> (32 - k);
  } else {
    (&d)->L[1] = 0x3ff00000 | y;
    (&d)->L[0] = z;
  }

ret_d:
  return (&d)->d;
}
Bigint* __d2b_D2A(double dd, int* e, int* bits)
{
  Bigint* b;
  U d;
  int i;
  int de, k;
  ULong *x, y, z;
  d.d = dd;
  b = __Balloc_D2A(1);
  if (b == NULL)
    return (NULL);
  x = b->x;
  z = (&d)->L[1] & 0xfffff;
  (&d)->L[1] &= 0x7fffffff;
  if ((de = (int)((&d)->L[1] >> 20)) != 0)
    z |= 0x100000;
  if ((y = (&d)->L[0]) != 0) {
    if ((k = __lo0bits_D2A(&y)) != 0) {
      x[0] = y | z << (32 - k);
      z >>= k;
    } else
      x[0] = y;
    i = b->wds = (x[1] = z) != 0 ? 2 : 1;
  } else {
    k = __lo0bits_D2A(&z);
    x[0] = z;
    i = b->wds = 1;
    k += 32;
  }
  if (de) {
    *e = de - 1023 - (53 - 1) + k;
    *bits = 53 - k;
  } else {
    *e = de - 1023 - (53 - 1) + 1 + k;
    *bits = 32 * i - __hi0bits_D2A((ULong)(x[i - 1]));
  }
  return b;
}
const double __bigtens_D2A[] = { 1e16, 1e32, 1e64, 1e128, 1e256 };
const double __tinytens_D2A[] = { 1e-16, 1e-32, 1e-64, 1e-128, 1e-256 };

const double __tens_D2A[] = { 1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9, 1e10, 1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19, 1e20, 1e21, 1e22 };