#include "gdtoa.h"
char *__dtoa_result_D2A;
char* __dtoa(double d0, int mode, int ndigits, int* decpt, int* sign, char** rve)
{
  int bbits, b2, b5, be, dig, i, ieps, ilim, ilim0, ilim1, j, j1, k, k0, k_check, leftright, m2, m5, s2, s5, spec_case, try_quick;
  int u_l;
  int denorm;
  uilong x;
  big_int *b, *b1, *delta, *mlo, *mhi, *S;
  udouble d, d2, eps;
  double ds;
  char *s, *s0;
  if (__dtoa_result_D2A) {
    __freedtoa(__dtoa_result_D2A);
    __dtoa_result_D2A = 0;
  }
  d.d = d0;
  if ((&d)->u_l[1] & 0x80000000) {
    *sign = 1;
    (&d)->u_l[1] &= ~0x80000000;
  } else
    *sign = 0;
  if (((&d)->u_l[1] & 0x7ff00000) == 0x7ff00000) {
    *decpt = 9999;
    if (!(&d)->u_l[0] && !((&d)->u_l[1] & 0xfffff))
      return __nrv_alloc_D2A("Infinity", rve, 8);
    return __nrv_alloc_D2A("NaN", rve, 3);
  }
  if (!(&d)->d) {
    *decpt = 1;
    return __nrv_alloc_D2A("0", rve, 1);
  }
  b = __d2b_d2a((&d)->d, &be, &bbits);
  if (b == NULL)
    return (NULL);
  if ((i = (int)((&d)->u_l[1] >> 20 & (0x7ff00000 >> 20))) != 0) {
    (&d2)->d = (&d)->d;
    (&d2)->u_l[1] &= 0xfffff;
    (&d2)->u_l[1] |= 0x3ff00000;
    i -= 1023;
    denorm = 0;
  } else {
    i = bbits + be + (1023 + (53 - 1) - 1);
    x = i > 32 ? (&d)->u_l[1] << (64 - i) | (&d)->u_l[0] >> (i - 32) : (&d)->u_l[0] << (32 - i);
    (&d2)->d = x;
    (&d2)->u_l[1] -= 31 * 0x100000;
    i -= (1023 + (53 - 1) - 1) + 1;
    denorm = 1;
  }
  ds = ((&d2)->d - 1.5) * 0.289529654602168 + 0.1760912590558 + i * 0.301029995663981;
  k = (int)ds;
  if (ds < 0. && ds != k)
    k--;
  k_check = 1;
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
  }
  leftright = 1;
  ilim = ilim1 = -1;
  switch (mode) {
  case 0:
  case 1:
    i = 18;
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
  s = s0 = __rv_alloc_d2a(i);
  if (s == NULL)
    return (NULL);
  if (ilim >= 0 && ilim <= 14 && try_quick) {
    i = 0;
    (&d2)->d = (&d)->d;
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
      (&d)->d /= ds;
    } else if ((j1 = -k) != 0) {
      (&d)->d *= __tens_D2A[j1 & 0xf];
      for (j = j1 >> 4; j; j >>= 1, i++)
        if (j & 1) {
          ieps++;
          (&d)->d *= __bigtens_D2A[i];
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
    (&eps)->u_l[1] -= (53 - 1) * 0x100000;
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
      (&eps)->d = 0.5 / __tens_D2A[ilim - 1] - (&eps)->d;
      for (i = 0;;) {
        u_l = (&d)->d;
        (&d)->d -= u_l;
        *s++ = '0' + (int)u_l;
        if ((&d)->d < (&eps)->d)
          goto ret1;
        if (1. - (&d)->d < (&eps)->d)
          goto bump_up;
        if (++i >= ilim)
          break;
        (&eps)->d *= 10.;
        (&d)->d *= 10.;
      }
    } else {
      (&eps)->d *= __tens_D2A[ilim - 1];
      for (i = 1;; i++, (&d)->d *= 10.) {
        u_l = (int)((&d)->d);
        if (!((&d)->d -= u_l))
          ilim = i;
        *s++ = '0' + (int)u_l;
        if (i == ilim) {
          if ((&d)->d > 0.5 + (&eps)->d)
            goto bump_up;
          else if ((&d)->d < 0.5 - (&eps)->d) {
            while (*--s == '0')
              ;
            s++;
            goto ret1;
          }
          break;
        }
      }
    }
  fast_failed:
    s = s0;
    (&d)->d = (&d2)->d;
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
      u_l = (int)((&d)->d / ds);
      (&d)->d -= u_l * ds;
      *s++ = '0' + (int)u_l;
      if (!(&d)->d) {
        break;
      }
      if (i == ilim) {
        (&d)->d += (&d)->d;
        if ((&d)->d > ds || ((&d)->d == ds && u_l & 1)) {
        bump_up:
          while (*--s == '9')
            if (s == s0) {
              k++;
              *s = '0';
              break;
            }
          ++*s++;
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
    i = denorm ? be + (1023 + (53 - 1) - 1 + 1) : 1 + 53 - bbits;
    b2 += i;
    s2 += i;
    mhi = __i2b_d2a(1);
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
        mhi = __pow5mult_d2a(mhi, m5);
        if (mhi == NULL)
          return (NULL);
        b1 = __mult_d2a(mhi, b);
        if (b1 == NULL)
          return (NULL);
        __bfree_d2a(b);
        b = b1;
      }
      if ((j = b5 - m5) != 0) {
        b = __pow5mult_d2a(b, j);
        if (b == NULL)
          return (NULL);
      }
    } else {
      b = __pow5mult_d2a(b, b5);
      if (b == NULL)
        return (NULL);
    }
  }
  S = __i2b_d2a(1);
  if (S == NULL)
    return (NULL);
  if (s5 > 0) {
    S = __pow5mult_d2a(S, s5);
    if (S == NULL)
      return (NULL);
  }
  spec_case = 0;
  if ((mode < 2 || leftright)) {
    if (!(&d)->u_l[0] && !((&d)->u_l[1] & 0xfffff) && (&d)->u_l[1] & (0x7ff00000 & ~0x100000)) {
      b2 += 1;
      s2 += 1;
      spec_case = 1;
    }
  }
  if ((i = ((s5 ? 32 - __hi0bits_d2a((uilong)(S->x[S->wds - 1])) : 1) + s2) & 0x1f) != 0)
    i = 32 - i;
  if (i > 4) {
    i -= 4;
    b2 += i;
    m2 += i;
    s2 += i;
  } else if (i < 4) {
    i += 28;
    b2 += i;
    m2 += i;
    s2 += i;
  }
  if (b2 > 0) {
    b = __lshift_d2a(b, b2);
    if (b == NULL)
      return (NULL);
  }
  if (s2 > 0) {
    S = __lshift_d2a(S, s2);
    if (S == NULL)
      return (NULL);
  }
  if (k_check) {
    if (__cmp_d2a(b, S) < 0) {
      k--;
      b = __multadd_d2a(b, 10, 0);
      if (b == NULL)
        return (NULL);
      if (leftright) {
        mhi = __multadd_d2a(mhi, 10, 0);
        if (mhi == NULL)
          return (NULL);
      }
      ilim = ilim1;
    }
  }
  if (ilim <= 0 && (mode == 3 || mode == 5)) {
    S = __multadd_d2a(S, 5, 0);
    if (S == NULL)
      return (NULL);
    if (ilim < 0 || __cmp_d2a(b, S) <= 0) {
    no_digits:
      k = -1 - ndigits;
      goto ret;
    }
  one_digit:
    *s++ = '1';
    k++;
    goto ret;
  }
  if (leftright) {
    if (m2 > 0) {
      mhi = __lshift_d2a(mhi, m2);
      if (mhi == NULL)
        return (NULL);
    }
    mlo = mhi;
    if (spec_case) {
      mhi = __balloc_d2a(mhi->k);
      if (mhi == NULL)
        return (NULL);
      memcpy(&mhi->sign, &mlo->sign, mlo->wds * sizeof(uilong) + 2 * sizeof(int));
      mhi = __lshift_d2a(mhi, 1);
      if (mhi == NULL)
        return (NULL);
    }
    for (i = 1;; i++) {
      dig = __quorem_D2A(b, S) + '0';
      j = __cmp_d2a(b, mlo);
      delta = __diff_d2a(S, mhi);
      if (delta == NULL)
        return (NULL);
      j1 = delta->sign ? 1 : __cmp_d2a(b, delta);
      __bfree_d2a(delta);
      if (j1 == 0 && mode != 1 && !((&d)->u_l[0] & 1)) {
        if (dig == '9')
          goto round_9_up;
        if (j > 0)
          dig++;
        *s++ = dig;
        goto ret;
      }
      if (j < 0 || (j == 0 && mode != 1 && !((&d)->u_l[0] & 1))) {
        if (!b->x[0] && b->wds <= 1) {
          goto accept_dig;
        }
        if (j1 > 0) {
          b = __lshift_d2a(b, 1);
          if (b == NULL)
            return (NULL);
          j1 = __cmp_d2a(b, S);
          if ((j1 > 0 || (j1 == 0 && dig & 1)) && dig++ == '9')
            goto round_9_up;
        }
      accept_dig:
        *s++ = dig;
        goto ret;
      }
      if (j1 > 0) {
        if (dig == '9') {
        round_9_up:
          *s++ = '9';
          goto roundoff;
        }
        *s++ = dig + 1;
        goto ret;
      }
      *s++ = dig;
      if (i == ilim)
        break;
      b = __multadd_d2a(b, 10, 0);
      if (b == NULL)
        return (NULL);
      if (mlo == mhi) {
        mlo = mhi = __multadd_d2a(mhi, 10, 0);
        if (mlo == NULL)
          return (NULL);
      } else {
        mlo = __multadd_d2a(mlo, 10, 0);
        if (mlo == NULL)
          return (NULL);
        mhi = __multadd_d2a(mhi, 10, 0);
        if (mhi == NULL)
          return (NULL);
      }
    }
  } else
    for (i = 1;; i++) {
      *s++ = dig = __quorem_D2A(b, S) + '0';
      if (!b->x[0] && b->wds <= 1) {
        goto ret;
      }
      if (i >= ilim)
        break;
      b = __multadd_d2a(b, 10, 0);
      if (b == NULL)
        return (NULL);
    }
  b = __lshift_d2a(b, 1);
  if (b == NULL)
    return (NULL);
  j = __cmp_d2a(b, S);
  if (j > 0 || (j == 0 && dig & 1)) {
  roundoff:
    while (*--s == '9')
      if (s == s0) {
        k++;
        *s++ = '1';
        goto ret;
      }
    ++*s++;
  } else {
    while (*--s == '0')
      ;
    s++;
  }
ret:
  __bfree_d2a(S);
  if (mhi) {
    if (mlo && mlo != mhi)
      __bfree_d2a(mlo);
    __bfree_d2a(mhi);
  }
ret1:
  __bfree_d2a(b);
  *s = 0;
  *decpt = k + 1;
  if (rve)
    *rve = s;
  return s0;
}