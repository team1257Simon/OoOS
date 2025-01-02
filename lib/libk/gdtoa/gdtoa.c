#include "gdtoa.h"
static Bigint* bitstob(ULong* bits, int nbits, int* bbits)
{
  int i, k;
  Bigint* b;
  ULong *be, *x, *x0;
  i = 32;
  k = 0;
  while (i < nbits) {
    i <<= 1;
    k++;
  }
  b = __Balloc_D2A(k);
  if (b == NULL)
    return (NULL);
  be = bits + ((nbits - 1) >> 5);
  x = x0 = b->x;
  do {
    *x++ = *bits & 0xffffffff;
  } while (++bits <= be);
  i = x - x0;
  while (!x0[--i])
    if (!i) {
      b->wds = 0;
      *bbits = 0;
      goto ret;
    }
  b->wds = i + 1;
  *bbits = i * 32 + 32 - __hi0bits_D2A((ULong)(b->x[i]));
ret:
  return b;
}
char* __gdtoa(FPI* fpi, int be, ULong* bits, int* kindp, int mode, int ndigits, int* decpt, char** rve)
{
  int bbits, b2, b5, be0, dig, i, ieps, ilim, ilim0, ilim1, inex;
  int j, j1, k, k0, k_check, kind, leftright, m2, m5, nbits;
  int rdir, s2, s5, spec_case, try_quick;
  int L;
  Bigint *b, *b1, *delta, *mlo, *mhi, *mhi1, *S;
  double d2, ds;
  char *s, *s0;
  U d, eps;
  if (__dtoa_result_D2A) {
    __freedtoa(__dtoa_result_D2A);
    __dtoa_result_D2A = 0;
  }
  inex = 0;
  kind = *kindp &= ~STRTOG_Inexact;
  switch (kind & STRTOG_Retmask) {
  case STRTOG_Zero:
    goto ret_zero;
  case STRTOG_Normal:
  case STRTOG_Denormal:
    break;
  case STRTOG_Infinite:
    *decpt = -32768;
    return __nrv_alloc_D2A("Infinity", rve, 8);
  case STRTOG_NaN:
    *decpt = -32768;
    return __nrv_alloc_D2A("NaN", rve, 3);
  default:
    return 0;
  }
  b = bitstob(bits, nbits = fpi->nbits, &bbits);
  if (b == NULL)
    return (NULL);
  be0 = be;
  if ((i = __trailz_D2A(b)) != 0) {
    __rshift_D2A(b, i);
    be += i;
    bbits -= i;
  }
  if (!b->wds) {
    __Bfree_D2A(b);
  ret_zero:
    *decpt = 1;
    return __nrv_alloc_D2A("0", rve, 1);
  }
  (&d)->d = __b2d_D2A(b, &i);
  i = be + bbits - 1;
  (&d)->L[1] &= 0xfffff;
  (&d)->L[1] |= 0x3ff00000;
  ds = ((&d)->d - 1.5) * 0.289529654602168 + 0.1760912590558 + i * 0.301029995663981;
  if ((j = i) < 0)
    j = -j;
  if ((j -= 1077) > 0)
    ds += j * 7e-17;
  k = (int)ds;
  if (ds < 0. && ds != k)
    k--;
  k_check = 1;
  (&d)->L[1] += (be + bbits - 1) << 20;
  if (k >= 0 && k <= 22) {
    if ((&d)->d < __tens_D2A[k])
      k--;
    k_check = 0;
  }
  j = bbits - i - 1;
  if (j >= 0) {
    b2 = 0;
    s2 = j;
  } else {
    b2 = -j;
    s2 = 0;
  }
  if (k >= 0) {
    b5 = 0;
    s5 = k;
    s2 += k;
  } else {
    b2 -= k;
    b5 = -k;
    s5 = 0;
  }
  if (mode < 0 || mode > 9)
    mode = 0;
  try_quick = 1;
  if (mode > 5) {
    mode -= 4;
    try_quick = 0;
  } else if (i >= -4 - (-1022) || i < (-1022))
    try_quick = 0;
  leftright = 1;
  ilim = ilim1 = -1;
  switch (mode) {
  case 0:
  case 1:
    i = (int)(nbits * .30103) + 3;
    ndigits = 0;
    break;
  case 2:
    leftright = 0;
  case 4:
    if (ndigits <= 0)
      ndigits = 1;
    ilim = ilim1 = i = ndigits;
    break;
  case 3:
    leftright = 0;
  case 5:
    i = ndigits + k + 1;
    ilim = i;
    ilim1 = i - 1;
    if (i <= 0)
      i = 1;
  }
  s = s0 = __rv_alloc_D2A(i);
  if (s == NULL)
    return (NULL);
  if ((rdir = fpi->rounding - 1) != 0) {
    if (rdir < 0)
      rdir = 2;
    if (kind & STRTOG_Neg)
      rdir = 3 - rdir;
  }
  if (ilim >= 0 && ilim <= 14 && try_quick && !rdir && k == 0) {
    i = 0;
    d2 = (&d)->d;
    k0 = k;
    ilim0 = ilim;
    ieps = 2;
    if (k > 0) {
      ds = __tens_D2A[k & 0xf];
      j = k >> 4;
      if (j & 0x10) {
        j &= 0x10 - 1;
        (&d)->d /= __bigtens_D2A[5 - 1];
        ieps++;
      }
      for (; j; j >>= 1, i++)
        if (j & 1) {
          ieps++;
          ds *= __bigtens_D2A[i];
        }
    } else {
      ds = 1.;
      if ((j1 = -k) != 0) {
        (&d)->d *= __tens_D2A[j1 & 0xf];
        for (j = j1 >> 4; j; j >>= 1, i++)
          if (j & 1) {
            ieps++;
            (&d)->d *= __bigtens_D2A[i];
          }
      }
    }
    if (k_check && (&d)->d < 1. && ilim > 0) {
      if (ilim1 <= 0)
        goto fast_failed;
      ilim = ilim1;
      k--;
      (&d)->d *= 10.;
      ieps++;
    }
    (&eps)->d = ieps * (&d)->d + 7.;
    (&eps)->L[1] -= (53 - 1) * 0x100000;
    if (ilim == 0) {
      S = mhi = 0;
      (&d)->d -= 5.;
      if ((&d)->d > (&eps)->d)
        goto one_digit;
      if ((&d)->d < -(&eps)->d)
        goto no_digits;
      goto fast_failed;
    }
    if (leftright) {
      (&eps)->d = ds * 0.5 / __tens_D2A[ilim - 1] - (&eps)->d;
      for (i = 0;;) {
        L = (int)((&d)->d / ds);
        (&d)->d -= L * ds;
        *s++ = '0' + (int)L;
        if ((&d)->d < (&eps)->d) {
          if ((&d)->d)
            inex = STRTOG_Inexlo;
          goto ret1;
        }
        if (ds - (&d)->d < (&eps)->d)
          goto bump_up;
        if (++i >= ilim)
          break;
        (&eps)->d *= 10.;
        (&d)->d *= 10.;
      }
    } else {
      (&eps)->d *= __tens_D2A[ilim - 1];
      for (i = 1;; i++, (&d)->d *= 10.) {
        if ((L = (int)((&d)->d / ds)) != 0)
          (&d)->d -= L * ds;
        *s++ = '0' + (int)L;
        if (i == ilim) {
          ds *= 0.5;
          if ((&d)->d > ds + (&eps)->d)
            goto bump_up;
          else if ((&d)->d < ds - (&eps)->d) {
            if ((&d)->d)
              inex = STRTOG_Inexlo;
            goto clear_trailing0;
          }
          break;
        }
      }
    }
  fast_failed:
    s = s0;
    (&d)->d = d2;
    k = k0;
    ilim = ilim0;
  }
  if (be >= 0 && k <= 14) {
    ds = __tens_D2A[k];
    if (ndigits < 0 && ilim <= 0) {
      S = mhi = 0;
      if (ilim < 0 || (&d)->d <= 5 * ds)
        goto no_digits;
      goto one_digit;
    }
    for (i = 1;; i++, (&d)->d *= 10.) {
      L = (&d)->d / ds;
      (&d)->d -= L * ds;
      *s++ = '0' + (int)L;
      if ((&d)->d == 0.)
        break;
      if (i == ilim) {
        if (rdir) {
          if (rdir == 1)
            goto bump_up;
          inex = STRTOG_Inexlo;
          goto ret1;
        }
        (&d)->d += (&d)->d;
        if ((&d)->d > ds || ((&d)->d == ds && L & 1)) {
        bump_up:
          inex = STRTOG_Inexhi;
          while (*--s == '9')
            if (s == s0) {
              k++;
              *s = '0';
              break;
            }
          ++*s++;
        } else {
          inex = STRTOG_Inexlo;
        clear_trailing0:
          while (*--s == '0') { }
          ++s;
        }
        break;
      }
    }
    goto ret1;
  }
  m2 = b2;
  m5 = b5;
  mhi = mlo = 0;
  if (leftright) {
    i = nbits - bbits;
    if (be - i++ < fpi->emin && mode != 3 && mode != 5) {
      i = be - fpi->emin + 1;
      if (mode >= 2 && ilim > 0 && ilim < i)
        goto small_ilim;
    } else if (mode >= 2) {
    small_ilim:
      j = ilim - 1;
      if (m5 >= j)
        m5 -= j;
      else {
        s5 += j -= m5;
        b5 += j;
        m5 = 0;
      }
      if ((i = ilim) < 0) {
        m2 -= i;
        i = 0;
      }
    }
    b2 += i;
    s2 += i;
    mhi = __i2b_D2A(1);
    if (mhi == NULL)
      return (NULL);
  }
  if (m2 > 0 && s2 > 0) {
    i = m2 < s2 ? m2 : s2;
    b2 -= i;
    m2 -= i;
    s2 -= i;
  }
  if (b5 > 0) {
    if (leftright) {
      if (m5 > 0) {
        mhi = __pow5mult_D2A(mhi, m5);
        if (mhi == NULL)
          return (NULL);
        b1 = __mult_D2A(mhi, b);
        if (b1 == NULL)
          return (NULL);
        __Bfree_D2A(b);
        b = b1;
      }
      if ((j = b5 - m5) != 0) {
        b = __pow5mult_D2A(b, j);
        if (b == NULL)
          return (NULL);
      }
    } else {
      b = __pow5mult_D2A(b, b5);
      if (b == NULL)
        return (NULL);
    }
  }
  S = __i2b_D2A(1);
  if (S == NULL)
    return (NULL);
  if (s5 > 0) {
    S = __pow5mult_D2A(S, s5);
    if (S == NULL)
      return (NULL);
  }
  spec_case = 0;
  if (mode < 2) {
    if (bbits == 1 && be0 > fpi->emin + 1) {
      b2++;
      s2++;
      spec_case = 1;
    }
  }
  i = ((s5 ? __hi0bits_D2A((ULong)(S->x[S->wds - 1])) : 32 - 1) - s2 - 4) & 31;
  m2 += i;
  if ((b2 += i) > 0) {
    b = __lshift_D2A(b, b2);
    if (b == NULL)
      return (NULL);
  }
  if ((s2 += i) > 0) {
    S = __lshift_D2A(S, s2);
    if (S == NULL)
      return (NULL);
  }
  if (k_check) {
    if (__cmp_D2A(b, S) < 0) {
      k--;
      b = __multadd_D2A(b, 10, 0);
      if (b == NULL)
        return (NULL);
      if (leftright) {
        mhi = __multadd_D2A(mhi, 10, 0);
        if (mhi == NULL)
          return (NULL);
      }
      ilim = ilim1;
    }
  }
  if (ilim <= 0 && mode > 2) {
    S = __multadd_D2A(S, 5, 0);
    if (S == NULL)
      return (NULL);
    if (ilim < 0 || __cmp_D2A(b, S) <= 0) {
    no_digits:
      k = -1 - ndigits;
      inex = STRTOG_Inexlo;
      goto ret;
    }
  one_digit:
    inex = STRTOG_Inexhi;
    *s++ = '1';
    k++;
    goto ret;
  }
  if (leftright) {
    if (m2 > 0) {
      mhi = __lshift_D2A(mhi, m2);
      if (mhi == NULL)
        return (NULL);
    }
    mlo = mhi;
    if (spec_case) {
      mhi = __Balloc_D2A(mhi->k);
      if (mhi == NULL)
        return (NULL);
      memcpy(&mhi->sign, &mlo->sign, mlo->wds * sizeof(ULong) + 2 * sizeof(int));
      mhi = __lshift_D2A(mhi, 1);
      if (mhi == NULL)
        return (NULL);
    }
    for (i = 1;; i++) {
      dig = __quorem_D2A(b, S) + '0';
      j = __cmp_D2A(b, mlo);
      delta = __diff_D2A(S, mhi);
      if (delta == NULL)
        return (NULL);
      j1 = delta->sign ? 1 : __cmp_D2A(b, delta);
      __Bfree_D2A(delta);
      if (j1 == 0 && !mode && !(bits[0] & 1) && !rdir) {
        if (dig == '9')
          goto round_9_up;
        if (j <= 0) {
          if (b->wds > 1 || b->x[0])
            inex = STRTOG_Inexlo;
        } else {
          dig++;
          inex = STRTOG_Inexhi;
        }
        *s++ = dig;
        goto ret;
      }
      if (j < 0 || (j == 0 && !mode && !(bits[0] & 1))) {
        if (rdir && (b->wds > 1 || b->x[0])) {
          if (rdir == 2) {
            inex = STRTOG_Inexlo;
            goto accept;
          }
          while (__cmp_D2A(S, mhi) > 0) {
            *s++ = dig;
            mhi1 = __multadd_D2A(mhi, 10, 0);
            if (mhi1 == NULL)
              return (NULL);
            if (mlo == mhi)
              mlo = mhi1;
            mhi = mhi1;
            b = __multadd_D2A(b, 10, 0);
            if (b == NULL)
              return (NULL);
            dig = __quorem_D2A(b, S) + '0';
          }
          if (dig++ == '9')
            goto round_9_up;
          inex = STRTOG_Inexhi;
          goto accept;
        }
        if (j1 > 0) {
          b = __lshift_D2A(b, 1);
          if (b == NULL)
            return (NULL);
          j1 = __cmp_D2A(b, S);
          if ((j1 > 0 || (j1 == 0 && dig & 1)) && dig++ == '9')
            goto round_9_up;
          inex = STRTOG_Inexhi;
        }
        if (b->wds > 1 || b->x[0])
          inex = STRTOG_Inexlo;
      accept:
        *s++ = dig;
        goto ret;
      }
      if (j1 > 0 && rdir != 2) {
        if (dig == '9') {
        round_9_up:
          *s++ = '9';
          inex = STRTOG_Inexhi;
          goto roundoff;
        }
        inex = STRTOG_Inexhi;
        *s++ = dig + 1;
        goto ret;
      }
      *s++ = dig;
      if (i == ilim)
        break;
      b = __multadd_D2A(b, 10, 0);
      if (b == NULL)
        return (NULL);
      if (mlo == mhi) {
        mlo = mhi = __multadd_D2A(mhi, 10, 0);
        if (mlo == NULL)
          return (NULL);
      } else {
        mlo = __multadd_D2A(mlo, 10, 0);
        if (mlo == NULL)
          return (NULL);
        mhi = __multadd_D2A(mhi, 10, 0);
        if (mhi == NULL)
          return (NULL);
      }
    }
  } else
    for (i = 1;; i++) {
      *s++ = dig = __quorem_D2A(b, S) + '0';
      if (i >= ilim)
        break;
      b = __multadd_D2A(b, 10, 0);
      if (b == NULL)
        return (NULL);
    }
  if (rdir) {
    if (rdir == 2 || (b->wds <= 1 && !b->x[0]))
      goto chopzeros;
    goto roundoff;
  }
  b = __lshift_D2A(b, 1);
  if (b == NULL)
    return (NULL);
  j = __cmp_D2A(b, S);
  if (j > 0 || (j == 0 && dig & 1)) {
  roundoff:
    inex = STRTOG_Inexhi;
    while (*--s == '9')
      if (s == s0) {
        k++;
        *s++ = '1';
        goto ret;
      }
    ++*s++;
  } else {
  chopzeros:
    if (b->wds > 1 || b->x[0])
      inex = STRTOG_Inexlo;
    while (*--s == '0') { }
    ++s;
  }
ret:
  __Bfree_D2A(S);
  if (mhi) {
    if (mlo && mlo != mhi)
      __Bfree_D2A(mlo);
    __Bfree_D2A(mhi);
  }
ret1:
  __Bfree_D2A(b);
  *s = 0;
  *decpt = k + 1;
  if (rve)
    *rve = s;
  *kindp |= inex;
  return s0;
}