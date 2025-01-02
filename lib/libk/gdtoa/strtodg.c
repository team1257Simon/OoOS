#include "gdtoa.h"
static const int fivesbits[] = { 0, 3, 5, 7, 10, 12, 14, 17, 19, 21, 24, 26, 28, 31, 33, 35, 38, 40, 42, 45, 47, 49, 52 };
Bigint* __increment_D2A(Bigint* b)
{
  ULong *x, *xe;
  Bigint* b1;
  x = b->x;
  xe = x + b->wds;
  do {
    if (*x < (ULong)0xffffffffL) {
      ++*x;
      return b;
    }
    *x++ = 0;
  } while (x < xe);
  {
    if (b->wds >= b->maxwds) {
      b1 = __Balloc_D2A(b->k + 1);
      if (b1 == NULL)
        return (NULL);
      memcpy(&b1->sign, &b->sign, b->wds * sizeof(ULong) + 2 * sizeof(int));
      __Bfree_D2A(b);
      b = b1;
    }
    b->x[b->wds++] = 1;
  }
  return b;
}
void __decrement_D2A(Bigint* b)
{
  ULong *x, *xe;
  x = b->x;
  xe = x + b->wds;
  do {
    if (*x) {
      --*x;
      break;
    }
    *x++ = 0xffffffffL;
  } while (x < xe);
}
static int all_on(Bigint* b, int n)
{
  ULong *x, *xe;
  x = b->x;
  xe = x + (n >> 5);
  while (x < xe)
    if ((*x++ & 0xffffffff) != 0xffffffff)
      return 0;
  if (n &= 31)
    return ((*x | (0xffffffff << n)) & 0xffffffff) == 0xffffffff;
  return 1;
}
Bigint* __set_ones_D2A(Bigint* b, int n)
{
  int k;
  ULong *x, *xe;
  k = (n + ((1 << 5) - 1)) >> 5;
  if (b->k < k) {
    __Bfree_D2A(b);
    b = __Balloc_D2A(k);
    if (b == NULL)
      return (NULL);
  }
  k = n >> 5;
  if (n &= 31)
    k++;
  b->wds = k;
  x = b->x;
  xe = x + k;
  while (x < xe)
    *x++ = 0xffffffff;
  if (n)
    x[-1] >>= 32 - n;
  return b;
}
static int rvOK(U* d, FPI* fpi, int* exp, ULong* bits, int exact, int rd, int* irv)
{
  Bigint* b;
  ULong carry, inex, lostbits;
  int bdif, e, j, k, k1, nb, rv;
  carry = rv = 0;
  b = __d2b_D2A((d)->d, &e, &bdif);
  if (b == NULL) {
    *irv = STRTOG_NoMemory;
    return (1);
  }
  bdif -= nb = fpi->nbits;
  e += bdif;
  if (bdif <= 0) {
    if (exact)
      goto trunc;
    goto ret;
  }
  if (53 == nb) {
    if (exact
        && fpi->rounding ==

            1

    )
      goto trunc;
    goto ret;
  }
  switch (rd) {
  case 1:
    goto trunc;
  case 2:
    break;
  default:
    k = bdif - 1;
    if (k < 0)
      goto trunc;
    if (!k) {
      if (!exact)
        goto ret;
      if (b->x[0] & 2)
        break;
      goto trunc;
    }
    if (b->x[k >> 5] & ((ULong)1 << (k & 31)))
      break;
    goto trunc;
  }
  carry = 1;
trunc:
  inex = lostbits = 0;
  if (bdif > 0) {
    if ((lostbits = __any_on_D2A(b, bdif)) != 0)
      inex = STRTOG_Inexlo;
    __rshift_D2A(b, bdif);
    if (carry) {
      inex = STRTOG_Inexhi;
      b = __increment_D2A(b);
      if (b == NULL) {
        *irv = STRTOG_NoMemory;
        return (1);
      }
      if ((j = nb & 31) != 0)
        j = 32 - j;
      if (__hi0bits_D2A((ULong)(b->x[b->wds - 1])) != j) {
        if (!lostbits)
          lostbits = b->x[0] & 1;
        __rshift_D2A(b, 1);
        e++;
      }
    }
  } else if (bdif < 0) {
    b = __lshift_D2A(b, -bdif);
    if (b == NULL) {
      *irv = STRTOG_NoMemory;
      return (1);
    }
  }
  if (e < fpi->emin) {
    k = fpi->emin - e;
    e = fpi->emin;
    if (k > nb || fpi->sudden_underflow) {
      b->wds = inex = 0;
      *irv = STRTOG_Underflow | STRTOG_Inexlo;
    } else {
      k1 = k - 1;
      if (k1 > 0 && !lostbits)
        lostbits = __any_on_D2A(b, k1);
      if (!lostbits && !exact)
        goto ret;
      lostbits |= carry = b->x[k1 >> 5] & (1 << (k1 & 31));
      __rshift_D2A(b, k);
      *irv = STRTOG_Denormal;
      if (carry) {
        b = __increment_D2A(b);
        if (b == NULL) {
          *irv = STRTOG_NoMemory;
          return (1);
        }
        inex = STRTOG_Inexhi | STRTOG_Underflow;
      } else if (lostbits)
        inex = STRTOG_Inexlo | STRTOG_Underflow;
    }
  } else if (e > fpi->emax) {
    e = fpi->emax + 1;
    *irv = STRTOG_Infinite | STRTOG_Overflow | STRTOG_Inexhi;
    *(__errno()) = 34;
    b->wds = inex = 0;
  }
  *exp = e;
  __copybits_D2A(bits, nb, b);
  *irv |= inex;
  rv = 1;
ret:
  __Bfree_D2A(b);
  return rv;
}
static int mantbits(U* d)
{
  ULong L;
  if ((L = (d)->L[0]) != 0)
    return 53 - __lo0bits_D2A(&L);
  L = (d)->L[1] | 0x100000;
  return 53 - 32 - __lo0bits_D2A(&L);
}
int __strtodg(const char* s00, char** se, FPI* fpi, int* exp, ULong* bits)
{
  int abe, abits, asub;
  int bb0, bb2, bb5, bbe, bd2, bd5, bbbits, bs2, c, decpt, denorm;
  int dsign, e, e1, e2, emin, esign, finished, i, inex, irv;
  int j, k, nbits, nd, nd0, nf, nz, nz0, rd, rvbits, rve, rve1, sign;
  int sudden_underflow;
  const char *s, *s0, *s1;
  double adj0, tol;
  int L;
  U adj, rv;
  ULong *b, *be, y, z;
  Bigint *ab, *bb, *bb1, *bd, *bd0, *bs, *delta, *rvb, *rvb0;
  irv = STRTOG_Zero;
  denorm = sign = nz0 = nz = 0;
  (&rv)->d = 0.;
  rvb = 0;
  nbits = fpi->nbits;
  for (s = s00;; s++)
    switch (*s) {
    case '-':
      sign = 1;
    case '+':
      if (*++s)
        goto break2;
    case 0:
      sign = 0;
      irv = STRTOG_NoNumber;
      s = s00;
      goto ret;
    case '\t':
    case '\n':
    case '\v':
    case '\f':
    case '\r':
    case ' ':
      continue;
    default:
      goto break2;
    }
break2:
  if (*s == '0') {
    switch (s[1]) {
    case 'x':
    case 'X':
      irv = __gethex_D2A(&s, fpi, exp, &rvb, sign);
      if (irv == STRTOG_NoMemory)
        return (STRTOG_NoMemory);
      if (irv == STRTOG_NoNumber) {
        s = s00;
        sign = 0;
      }
      goto ret;
    }
    nz0 = 1;
    while (*++s == '0')
      ;
    if (!*s)
      goto ret;
  }
  sudden_underflow = fpi->sudden_underflow;
  s0 = s;
  y = z = 0;
  for (decpt = nd = nf = 0; (c = *s) >= '0' && c <= '9'; nd++, s++)
    if (nd < 9)
      y = 10 * y + c - '0';
    else if (nd < 16)
      z = 10 * z + c - '0';
  nd0 = nd;
  if (c == '.') {
    c = *++s;
    decpt = 1;
    if (!nd) {
      for (; c == '0'; c = *++s)
        nz++;
      if (c > '0' && c <= '9') {
        s0 = s;
        nf += nz;
        nz = 0;
        goto have_dig;
      }
      goto dig_done;
    }
    for (; c >= '0' && c <= '9'; c = *++s) {
    have_dig:
      nz++;
      if (c -= '0') {
        nf += nz;
        for (i = 1; i < nz; i++)
          if (nd++ < 9)
            y *= 10;
          else if (nd <= 15 + 1)
            z *= 10;
        if (nd++ < 9)
          y = 10 * y + c;
        else if (nd <= 15 + 1)
          z = 10 * z + c;
        nz = 0;
      }
    }
  }
dig_done:
  e = 0;
  if (c == 'e' || c == 'E') {
    if (!nd && !nz && !nz0) {
      irv = STRTOG_NoNumber;
      s = s00;
      goto ret;
    }
    s00 = s;
    esign = 0;
    switch (c = *++s) {
    case '-':
      esign = 1;
    case '+':
      c = *++s;
    }
    if (c >= '0' && c <= '9') {
      while (c == '0')
        c = *++s;
      if (c > '0' && c <= '9') {
        L = c - '0';
        s1 = s;
        while ((c = *++s) >= '0' && c <= '9')
          L = 10 * L + c - '0';
        if (s - s1 > 8 || L > 19999)
          e = 19999;
        else
          e = (int)L;
        if (esign)
          e = -e;
      } else
        e = 0;
    } else
      s = s00;
  }
  if (!nd) {
    if (!nz && !nz0) {
      if (!decpt)
        switch (c) {
        case 'i':
        case 'I':
          if (__match_D2A(&s, "nf")) {
            --s;
            if (!__match_D2A(&s, "inity"))
              ++s;
            irv = STRTOG_Infinite;
            goto infnanexp;
          }
          break;
        case 'n':
        case 'N':
          if (__match_D2A(&s, "an")) {
            irv = STRTOG_NaN;
            *exp = fpi->emax + 1;
            if (*s == '(')
              irv = __hexnan_D2A(&s, fpi, bits);
            goto infnanexp;
          }
        }
      irv = STRTOG_NoNumber;
      s = s00;
    }
    goto ret;
  }
  irv = STRTOG_Normal;
  e1 = e -= nf;
  rd = 0;
  switch (fpi->rounding & 3) {
  case FPI_Round_up:
    rd = 2 - sign;
    break;
  case FPI_Round_zero:
    rd = 1;
    break;
  case FPI_Round_down:
    rd = 1 + sign;
  }
  if (!nd0)
    nd0 = nd;
  k = nd < 15 + 1 ? nd : 15 + 1;
  (&rv)->d = y;
  if (k > 9)
    (&rv)->d = __tens_D2A[k - 9] * (&rv)->d + z;
  bd0 = 0;
  if (nbits <= 53 && nd <= 15) {
    if (!e) {
      if (rvOK(&rv, fpi, exp, bits, 1, rd, &irv)) {
        if (irv == STRTOG_NoMemory)
          return (STRTOG_NoMemory);
        goto ret;
      }
    } else if (e > 0) {
      if (e <= 22) {
        i = fivesbits[e] + mantbits(&rv) <= 53;
        (&rv)->d *= __tens_D2A[e];
        if (rvOK(&rv, fpi, exp, bits, i, rd, &irv)) {
          if (irv == STRTOG_NoMemory)
            return (STRTOG_NoMemory);
          goto ret;
        }
        e1 -= e;
        goto rv_notOK;
      }
      i = 15 - nd;
      if (e <= 22 + i) {
        e2 = e - i;
        e1 -= i;
        (&rv)->d *= __tens_D2A[i];
        (&rv)->d *= __tens_D2A[e2];
        if (rvOK(&rv, fpi, exp, bits, 0, rd, &irv)) {
          if (irv == STRTOG_NoMemory)
            return (STRTOG_NoMemory);
          goto ret;
        }
        e1 -= e2;
      }
    } else if (e >= -22) {
      (&rv)->d /= __tens_D2A[-e];
      if (rvOK(&rv, fpi, exp, bits, 0, rd, &irv)) {
        if (irv == STRTOG_NoMemory)
          return (STRTOG_NoMemory);
        goto ret;
      }
      e1 -= e;
    }
  }
rv_notOK:
  e1 += nd - k;
  e2 = 0;
  if (e1 > 0) {
    if ((i = e1 & 15) != 0)
      (&rv)->d *= __tens_D2A[i];
    if (e1 &= ~15) {
      e1 >>= 4;
      while (e1 >= (1 << (5 - 1))) {
        e2 += (((&rv)->L[1] & 0x7ff00000) >> 20) - 1023;
        (&rv)->L[1] &= ~0x7ff00000;
        (&rv)->L[1] |= 1023 << 20;
        (&rv)->d *= __bigtens_D2A[5 - 1];
        e1 -= 1 << (5 - 1);
      }
      e2 += (((&rv)->L[1] & 0x7ff00000) >> 20) - 1023;
      (&rv)->L[1] &= ~0x7ff00000;
      (&rv)->L[1] |= 1023 << 20;
      for (j = 0; e1 > 0; j++, e1 >>= 1)
        if (e1 & 1)
          (&rv)->d *= __bigtens_D2A[j];
    }
  } else if (e1 < 0) {
    e1 = -e1;
    if ((i = e1 & 15) != 0)
      (&rv)->d /= __tens_D2A[i];
    if (e1 &= ~15) {
      e1 >>= 4;
      while (e1 >= (1 << (5 - 1))) {
        e2 += (((&rv)->L[1] & 0x7ff00000) >> 20) - 1023;
        (&rv)->L[1] &= ~0x7ff00000;
        (&rv)->L[1] |= 1023 << 20;
        (&rv)->d *= __tinytens_D2A[5 - 1];
        e1 -= 1 << (5 - 1);
      }
      e2 += (((&rv)->L[1] & 0x7ff00000) >> 20) - 1023;
      (&rv)->L[1] &= ~0x7ff00000;
      (&rv)->L[1] |= 1023 << 20;
      for (j = 0; e1 > 0; j++, e1 >>= 1)
        if (e1 & 1)
          (&rv)->d *= __tinytens_D2A[j];
    }
  }
  rvb = __d2b_D2A((&rv)->d, &rve, &rvbits);
  if (rvb == NULL)
    return (STRTOG_NoMemory);
  rve += e2;
  if ((j = rvbits - nbits) > 0) {
    __rshift_D2A(rvb, j);
    rvbits = nbits;
    rve += j;
  }
  bb0 = 0;
  e2 = rve + rvbits - nbits;
  if (e2 > fpi->emax + 1)
    goto huge;
  rve1 = rve + rvbits - nbits;
  if (e2 < (emin = fpi->emin)) {
    denorm = 1;
    j = rve - emin;
    if (j > 0) {
      rvb = __lshift_D2A(rvb, j);
      if (rvb == NULL)
        return (STRTOG_NoMemory);
      rvbits += j;
    } else if (j < 0) {
      rvbits += j;
      if (rvbits <= 0) {
        if (rvbits < -1) {
        ufl:
          rvb->wds = 0;
          rvb->x[0] = 0;
          *exp = emin;
          irv = STRTOG_Underflow | STRTOG_Inexlo;
          goto ret;
        }
        rvb->x[0] = rvb->wds = rvbits = 1;
      } else
        __rshift_D2A(rvb, -j);
    }
    rve = rve1 = emin;
    if (sudden_underflow && e2 + 1 < emin)
      goto ufl;
  }
  bd0 = __s2b_D2A(s0, nd0, nd, y, 1);
  if (bd0 == NULL)
    return (STRTOG_NoMemory);
  for (;;) {
    bd = __Balloc_D2A(bd0->k);
    if (bd == NULL)
      return (STRTOG_NoMemory);
    memcpy(&bd->sign, &bd0->sign, bd0->wds * sizeof(ULong) + 2 * sizeof(int));
    bb = __Balloc_D2A(rvb->k);
    if (bb == NULL)
      return (STRTOG_NoMemory);
    memcpy(&bb->sign, &rvb->sign, rvb->wds * sizeof(ULong) + 2 * sizeof(int));
    bbbits = rvbits - bb0;
    bbe = rve + bb0;
    bs = __i2b_D2A(1);
    if (bs == NULL)
      return (STRTOG_NoMemory);
    if (e >= 0) {
      bb2 = bb5 = 0;
      bd2 = bd5 = e;
    } else {
      bb2 = bb5 = -e;
      bd2 = bd5 = 0;
    }
    if (bbe >= 0)
      bb2 += bbe;
    else
      bd2 -= bbe;
    bs2 = bb2;
    j = nbits + 1 - bbbits;
    i = bbe + bbbits - nbits;
    if (i < emin)
      j += i - emin;
    bb2 += j;
    bd2 += j;
    i = bb2 < bd2 ? bb2 : bd2;
    if (i > bs2)
      i = bs2;
    if (i > 0) {
      bb2 -= i;
      bd2 -= i;
      bs2 -= i;
    }
    if (bb5 > 0) {
      bs = __pow5mult_D2A(bs, bb5);
      if (bs == NULL)
        return (STRTOG_NoMemory);
      bb1 = __mult_D2A(bs, bb);
      if (bb1 == NULL)
        return (STRTOG_NoMemory);
      __Bfree_D2A(bb);
      bb = bb1;
    }
    bb2 -= bb0;
    if (bb2 > 0) {
      bb = __lshift_D2A(bb, bb2);
      if (bb == NULL)
        return (STRTOG_NoMemory);
    } else if (bb2 < 0)
      __rshift_D2A(bb, -bb2);
    if (bd5 > 0) {
      bd = __pow5mult_D2A(bd, bd5);
      if (bd == NULL)
        return (STRTOG_NoMemory);
    }
    if (bd2 > 0) {
      bd = __lshift_D2A(bd, bd2);
      if (bd == NULL)
        return (STRTOG_NoMemory);
    }
    if (bs2 > 0) {
      bs = __lshift_D2A(bs, bs2);
      if (bs == NULL)
        return (STRTOG_NoMemory);
    }
    asub = 1;
    inex = STRTOG_Inexhi;
    delta = __diff_D2A(bb, bd);
    if (delta == NULL)
      return (STRTOG_NoMemory);
    if (delta->wds <= 1 && !delta->x[0])
      break;
    dsign = delta->sign;
    delta->sign = finished = 0;
    L = 0;
    i = __cmp_D2A(delta, bs);
    if (rd && i <= 0) {
      irv = STRTOG_Normal;
      if ((finished = dsign ^ (rd & 1)) != 0) {
        if (dsign != 0) {
          irv |= STRTOG_Inexhi;
          goto adj1;
        }
        irv |= STRTOG_Inexlo;
        if (rve1 == emin)
          goto adj1;
        for (i = 0, j = nbits; j >= 32; i++, j -= 32) {
          if (rvb->x[i] & 0xffffffff)
            goto adj1;
        }
        if (j > 1 && __lo0bits_D2A(rvb->x + i) < j - 1)
          goto adj1;
        rve = rve1 - 1;
        rvb = __set_ones_D2A(rvb, rvbits = nbits);
        if (rvb == NULL)
          return (STRTOG_NoMemory);
        break;
      }
      irv |= dsign ? STRTOG_Inexlo : STRTOG_Inexhi;
      break;
    }
    if (i < 0) {
      irv = dsign ? STRTOG_Normal | STRTOG_Inexlo : STRTOG_Normal | STRTOG_Inexhi;
      if (dsign || bbbits > 1 || denorm || rve1 == emin)
        break;
      delta = __lshift_D2A(delta, 1);
      if (delta == NULL)
        return (STRTOG_NoMemory);
      if (__cmp_D2A(delta, bs) > 0) {
        irv = STRTOG_Normal | STRTOG_Inexlo;
        goto drop_down;
      }
      break;
    }
    if (i == 0) {
      if (dsign) {
        if (denorm && all_on(rvb, rvbits)) {
          rvb->wds = 1;
          rvb->x[0] = 1;
          rve = emin + nbits - (rvbits = 1);
          irv = STRTOG_Normal | STRTOG_Inexhi;
          denorm = 0;
          break;
        }
        irv = STRTOG_Normal | STRTOG_Inexlo;
      } else if (bbbits == 1) {
        irv = STRTOG_Normal;
      drop_down:
        if (rve1 == emin) {
          irv = STRTOG_Normal | STRTOG_Inexhi;
          if (rvb->wds == 1 && rvb->x[0] == 1)
            sudden_underflow = 1;
          break;
        }
        rve -= nbits;
        rvb = __set_ones_D2A(rvb, rvbits = nbits);
        if (rvb == NULL)
          return (STRTOG_NoMemory);
        break;
      } else
        irv = STRTOG_Normal | STRTOG_Inexhi;
      if ((bbbits < nbits && !denorm) || !(rvb->x[0] & 1))
        break;
      if (dsign) {
        rvb = __increment_D2A(rvb);
        if (rvb == NULL)
          return (STRTOG_NoMemory);
        j = 31 & (32 - (rvbits & 31));
        if (__hi0bits_D2A((ULong)(rvb->x[rvb->wds - 1])) != j)
          rvbits++;
        irv = STRTOG_Normal | STRTOG_Inexhi;
      } else {
        if (bbbits == 1)
          goto undfl;
        __decrement_D2A(rvb);
        irv = STRTOG_Normal | STRTOG_Inexlo;
      }
      break;
    }
    if (((&adj)->d = __ratio_D2A(delta, bs)) <= 2.) {
    adj1:
      inex = STRTOG_Inexlo;
      if (dsign) {
        asub = 0;
        inex = STRTOG_Inexhi;
      } else if (denorm && bbbits <= 1) {
      undfl:
        rvb->wds = 0;
        rve = emin;
        irv = STRTOG_Underflow | STRTOG_Inexlo;
        break;
      }
      adj0 = (&adj)->d = 1.;
    } else {
      adj0 = (&adj)->d *= 0.5;
      if (dsign) {
        asub = 0;
        inex = STRTOG_Inexlo;
      }
      if ((&adj)->d < 2147483647.) {
        L = adj0;
        adj0 -= L;
        switch (rd) {
        case 0:
          if (adj0 >= .5)
            goto inc_L;
          break;
        case 1:
          if (asub && adj0 > 0.)
            goto inc_L;
          break;
        case 2:
          if (!asub && adj0 > 0.) {
          inc_L:
            L++;
            inex = STRTOG_Inexact - inex;
          }
        }
        (&adj)->d = L;
      }
    }
    y = rve + rvbits;
    if (!denorm && rvbits < nbits) {
      rvb = __lshift_D2A(rvb, j = nbits - rvbits);
      if (rvb == NULL)
        return (STRTOG_NoMemory);
      rve -= j;
      rvbits = nbits;
    }
    ab = __d2b_D2A((&adj)->d, &abe, &abits);
    if (ab == NULL)
      return (STRTOG_NoMemory);
    if (abe < 0)
      __rshift_D2A(ab, -abe);
    else if (abe > 0) {
      ab = __lshift_D2A(ab, abe);
      if (ab == NULL)
        return (STRTOG_NoMemory);
    }
    rvb0 = rvb;
    if (asub) {
      j = __hi0bits_D2A((ULong)(rvb->x[rvb->wds - 1]));
      rvb = __diff_D2A(rvb, ab);
      if (rvb == NULL)
        return (STRTOG_NoMemory);
      k = rvb0->wds - 1;
      if (denorm)
        ;
      else if (rvb->wds <= k || __hi0bits_D2A((ULong)(rvb->x[k])) > __hi0bits_D2A((ULong)(rvb0->x[k]))) {
        if (rve1 == emin) {
          --rvbits;
          denorm = 1;
        } else {
          rvb = __lshift_D2A(rvb, 1);
          if (rvb == NULL)
            return (STRTOG_NoMemory);
          --rve;
          --rve1;
          L = finished = 0;
        }
      }
    } else {
      rvb = __sum_D2A(rvb, ab);
      if (rvb == NULL)
        return (STRTOG_NoMemory);
      k = rvb->wds - 1;
      if (k >= rvb0->wds || __hi0bits_D2A((ULong)(rvb->x[k])) < __hi0bits_D2A((ULong)(rvb0->x[k]))) {
        if (denorm) {
          if (++rvbits == nbits)
            denorm = 0;
        } else {
          __rshift_D2A(rvb, 1);
          rve++;
          rve1++;
          L = 0;
        }
      }
    }
    __Bfree_D2A(ab);
    __Bfree_D2A(rvb0);
    if (finished)
      break;
    z = rve + rvbits;
    if (y == z && L) {
      tol = (&adj)->d * 5e-16;
      (&adj)->d = adj0 - .5;
      if ((&adj)->d < -tol) {
        if (adj0 > tol) {
          irv |= inex;
          break;
        }
      } else if ((&adj)->d > tol && adj0 < 1. - tol) {
        irv |= inex;
        break;
      }
    }
    bb0 = denorm ? 0 : __trailz_D2A(rvb);
    __Bfree_D2A(bb);
    __Bfree_D2A(bd);
    __Bfree_D2A(bs);
    __Bfree_D2A(delta);
  }
  if (!denorm && (j = nbits - rvbits)) {
    if (j > 0) {
      rvb = __lshift_D2A(rvb, j);
      if (rvb == NULL)
        return (STRTOG_NoMemory);
    } else
      __rshift_D2A(rvb, -j);
    rve -= j;
  }
  *exp = rve;
  __Bfree_D2A(bb);
  __Bfree_D2A(bd);
  __Bfree_D2A(bs);
  __Bfree_D2A(bd0);
  __Bfree_D2A(delta);
  if (rve > fpi->emax) {
    switch (fpi->rounding & 3) {
    case FPI_Round_near:
      goto huge;
    case FPI_Round_up:
      if (!sign)
        goto huge;
      break;
    case FPI_Round_down:
      if (sign)
        goto huge;
    }
    __Bfree_D2A(rvb);
    rvb = 0;
    irv = STRTOG_Normal | STRTOG_Inexlo;
    *exp = fpi->emax;
    b = bits;
    be = b + ((fpi->nbits + 31) >> 5);
    while (b < be)
      *b++ = -1;
    if ((j = fpi->nbits & 0x1f))
      *--be >>= (32 - j);
    goto ret;
  huge:
    rvb->wds = 0;
    irv = STRTOG_Infinite | STRTOG_Overflow | STRTOG_Inexhi;
    *(__errno()) = 34;
  infnanexp:
    *exp = fpi->emax + 1;
  }
ret:
  if (denorm) {
    if (sudden_underflow) {
      rvb->wds = 0;
      irv = STRTOG_Underflow | STRTOG_Inexlo;
      *(__errno()) = 34;
    } else {
      irv = (irv & ~STRTOG_Retmask) | (rvb->wds > 0 ? STRTOG_Denormal : STRTOG_Zero);
      if (irv & STRTOG_Inexact) {
        irv |= STRTOG_Underflow;
        *(__errno()) = 34;
      }
    }
  }
  if (se)
    *se = (char*)s;
  if (sign)
    irv |= STRTOG_Neg;
  if (rvb) {
    __copybits_D2A(bits, nbits, rvb);
    __Bfree_D2A(rvb);
  }
  return irv;
}