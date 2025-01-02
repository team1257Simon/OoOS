#include "gdtoa.h"
int __gethex_D2A(const char** sp, FPI* fpi, int* exp, Bigint** bp, int sign)
{
  Bigint* b;
  const unsigned char *decpt, *s0, *s, *s1;
  int big, esign, havedig, irv, j, k, n, n0, nbits, up, zret;
  ULong L, lostbits, *x;
  int e, e1;
  if (!__hexdig_D2A['0'])
    __hexdig_init_D2A();
  *bp = 0;
  havedig = 0;
  s0 = *(const unsigned char**)sp + 2;
  while (s0[havedig] == '0')
    havedig++;
  s0 += havedig;
  s = s0;
  decpt = 0;
  zret = 0;
  e = 0;
  if (__hexdig_D2A[*s])
    havedig++;
  else {
    zret = 1;
    if (*s != '.')
      goto pcheck;
    decpt = ++s;
    if (!__hexdig_D2A[*s])
      goto pcheck;
    while (*s == '0')
      s++;
    if (__hexdig_D2A[*s])
      zret = 0;
    havedig = 1;
    s0 = s;
  }
  while (__hexdig_D2A[*s])
    s++;
  if (*s == '.' && !decpt) {
    decpt = ++s;
    while (__hexdig_D2A[*s])
      s++;
  }
  if (decpt)
    e = -(((int)(s - decpt)) << 2);
pcheck:
  s1 = s;
  big = esign = 0;
  switch (*s) {
  case 'p':
  case 'P':
    switch (*++s) {
    case '-':
      esign = 1;
    case '+':
      s++;
    }
    if ((n = __hexdig_D2A[*s]) == 0 || n > 0x19) {
      s = s1;
      break;
    }
    e1 = n - 0x10;
    while ((n = __hexdig_D2A[*++s]) != 0 && n <= 0x19) {
      if (e1 & 0xf8000000)
        big = 1;
      e1 = 10 * e1 + n - 0x10;
    }
    if (esign)
      e1 = -e1;
    e += e1;
  }
  *sp = (char*)s;
  if (!havedig)
    *sp = (char*)s0 - 1;
  if (zret)
    return STRTOG_Zero;
  if (big) {
    if (esign) {
      switch (fpi->rounding) {
      case FPI_Round_up:
        if (sign)
          break;
        goto ret_tiny;
      case FPI_Round_down:
        if (!sign)
          break;
        goto ret_tiny;
      }
      goto retz;
    ret_tiny:
      b = __Balloc_D2A(0);
      if (b == NULL)
        return (STRTOG_NoMemory);
      b->wds = 1;
      b->x[0] = 1;
      goto dret;
    }
    switch (fpi->rounding) {
    case FPI_Round_near:
      goto ovfl1;
    case FPI_Round_up:
      if (!sign)
        goto ovfl1;
      goto ret_big;
    case FPI_Round_down:
      if (sign)
        goto ovfl1;
      goto ret_big;
    }
  ret_big:
    nbits = fpi->nbits;
    n0 = n = nbits >> 5;
    if (nbits & 31)
      ++n;
    for (j = n, k = 0; j >>= 1; ++k)
      ;
    *bp = b = __Balloc_D2A(k);
    if (*bp == NULL)
      return (STRTOG_NoMemory);
    b->wds = n;
    for (j = 0; j < n0; ++j)
      b->x[j] = 0xffffffff;
    if (n > n0)
      b->x[j] = 32 >> (32 - (nbits & 31));
    *exp = fpi->emin;
    return STRTOG_Normal | STRTOG_Inexlo;
  }
  n = s1 - s0 - 1;
  for (k = 0; n > (1 << (5 - 2)) - 1; n >>= 1)
    k++;
  b = __Balloc_D2A(k);
  if (b == NULL)
    return (STRTOG_NoMemory);
  x = b->x;
  n = 0;
  L = 0;
  while (s1 > s0) {
    if (*--s1 == '.')
      continue;
    if (n == 32) {
      *x++ = L;
      L = 0;
      n = 0;
    }
    L |= (__hexdig_D2A[*s1] & 0x0f) << n;
    n += 4;
  }
  *x++ = L;
  b->wds = n = x - b->x;
  n = 32 * n - __hi0bits_D2A((ULong)(L));
  nbits = fpi->nbits;
  lostbits = 0;
  x = b->x;
  if (n > nbits) {
    n -= nbits;
    if (__any_on_D2A(b, n)) {
      lostbits = 1;
      k = n - 1;
      if (x[k >> 5] & 1 << (k & 31)) {
        lostbits = 2;
        if (k > 0 && __any_on_D2A(b, k))
          lostbits = 3;
      }
    }
    __rshift_D2A(b, n);
    e += n;
  } else if (n < nbits) {
    n = nbits - n;
    b = __lshift_D2A(b, n);
    if (b == NULL)
      return (STRTOG_NoMemory);
    e -= n;
    x = b->x;
  }
  if (e > fpi->emax) {
  ovfl:
    __Bfree_D2A(b);
  ovfl1:
    *(__errno()) = 34;
    return STRTOG_Infinite | STRTOG_Overflow | STRTOG_Inexhi;
  }
  irv = STRTOG_Normal;
  if (e < fpi->emin) {
    irv = STRTOG_Denormal;
    n = fpi->emin - e;
    if (n >= nbits) {
      switch (fpi->rounding) {
      case FPI_Round_near:
        if (n == nbits && (n < 2 || __any_on_D2A(b, n - 1)))
          goto one_bit;
        break;
      case FPI_Round_up:
        if (!sign)
          goto one_bit;
        break;
      case FPI_Round_down:
        if (sign) {
        one_bit:
          x[0] = b->wds = 1;
        dret:
          *bp = b;
          *exp = fpi->emin;
          *(__errno()) = 34;
          return STRTOG_Denormal | STRTOG_Inexhi | STRTOG_Underflow;
        }
      }
      __Bfree_D2A(b);
    retz:
      *(__errno()) = 34;
      return STRTOG_Zero | STRTOG_Inexlo | STRTOG_Underflow;
    }
    k = n - 1;
    if (lostbits)
      lostbits = 1;
    else if (k > 0)
      lostbits = __any_on_D2A(b, k);
    if (x[k >> 5] & 1 << (k & 31))
      lostbits |= 2;
    nbits -= n;
    __rshift_D2A(b, n);
    e = fpi->emin;
  }
  if (lostbits) {
    up = 0;
    switch (fpi->rounding) {
    case FPI_Round_zero:
      break;
    case FPI_Round_near:
      if (lostbits & 2 && (lostbits | x[0]) & 1)
        up = 1;
      break;
    case FPI_Round_up:
      up = 1 - sign;
      break;
    case FPI_Round_down:
      up = sign;
    }
    if (up) {
      k = b->wds;
      b = __increment_D2A(b);
      if (b == NULL)
        return (STRTOG_NoMemory);
      x = b->x;
      if (irv == STRTOG_Denormal) {
        if (nbits == fpi->nbits - 1 && x[nbits >> 5] & 1 << (nbits & 31))
          irv = STRTOG_Normal;
      } else if (b->wds > k || ((n = nbits & 31) != 0 && __hi0bits_D2A((ULong)(x[k - 1])) < 32 - n)) {
        __rshift_D2A(b, 1);
        if (++e > fpi->emax)
          goto ovfl;
      }
      irv |= STRTOG_Inexhi;
    } else
      irv |= STRTOG_Inexlo;
  }
  *bp = b;
  *exp = e;
  return irv;
}