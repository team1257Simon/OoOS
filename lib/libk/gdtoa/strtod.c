#include "gdtoa.h"
static const double tinytens[] = { 1e-16, 1e-32, 1e-64, 1e-128, 9007199254740992. * 9007199254740992.e-256 };
static double __sulp_D2A(udouble* x, int scale)
{
  udouble u;
  double rv;
  int i;
  rv = __ulp_d2a(x);
  if (!scale || (i = 2 * 53 + 1 - (((x)->u_l[1] & 0x7ff00000) >> 20)) <= 0)
    return rv;
  (&u)->u_l[1] = 0x3ff00000 + (i << 20);
  (&u)->u_l[0] = 0;
  return rv * u.d;
}
double strtod(const char* s00, char** se)
{
  int scale;
  int bb2, bb5, bbe, bd2, bd5, bbbits, bs2, c, decpt, dsign, e, e1, esign, i, j, k, nd, nd0, nf, nz, nz0, sign;
  const char *s, *s0, *s1;
  double aadj;
  int u_l;
  udouble adj, aadj1, rv, rv0;
  uilong y, z;
  big_int *bb = NULL, *bb1, *bd = NULL, *bd0 = NULL, *bs = NULL, *delta = NULL;
  uilong Lsb, Lsb1;
  sign = nz0 = nz = decpt = 0;
  (&rv)->d = 0.;
  for (s = s00;; s++)
    switch (*s) {
    case '-':
      sign = 1;
    case '+':
      if (*++s)
        goto break2;
    case 0:
      goto ret0;
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
    {
      static fpi fpi = { 53, 1 - 1023 - 53 + 1, 2046 - 1023 - 53 + 1, 1, 0 };
      int exp;
      uilong bits[2];
      switch (s[1]) {
      case 'x':
      case 'X': {
        switch (
            (i = __gethex_d2a(&s, &fpi, &exp, &bb, sign)) & strtog_retmask) {
        case strtog_nomemory:
          goto ovfl;
        case strtog_no_num:
          s = s00;
          sign = 0;
        case strtog_zero:
          break;
        default:
          if (bb) {
            __copybits_d2a(bits, fpi.nbits, bb);
            __bfree_d2a(bb);
          }
          __ultod_d2a(((udouble*)&rv)->u_l, bits, exp, i);
        }
      }
        goto ret;
      }
    }
    nz0 = 1;
    while (*++s == '0')
      ;
    if (!*s)
      goto ret;
  }
  s0 = s;
  y = z = 0;
  for (nd = nf = 0; (c = *s) >= '0' && c <= '9'; nd++, s++)
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
      goto ret0;
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
        u_l = c - '0';
        s1 = s;
        while ((c = *++s) >= '0' && c <= '9')
          u_l = 10 * u_l + c - '0';
        if (s - s1 > 8 || u_l > 19999)
          e = 19999;
        else
          e = (int)u_l;
        if (esign)
          e = -e;
      } else
        e = 0;
    } else
      s = s00;
  }
  if (!nd) {
    if (!nz && !nz0) {
      uilong bits[2];
      static fpi fpinan = { 52, 1 - 1023 - 53 + 1, 2046 - 1023 - 53 + 1, 1, 0 };
      if (!decpt)
        switch (c) {
        case 'i':
        case 'I':
          if (__match_d2a(&s, "nf")) {
            --s;
            if (!__match_d2a(&s, "inity"))
              ++s;
            (&rv)->u_l[1] = 0x7ff00000;
            (&rv)->u_l[0] = 0;
            goto ret;
          }
          break;
        case 'n':
        case 'N':
          if (__match_d2a(&s, "an")) {
            if (*s == '('
                && __hexnan_d2a(&s, &fpinan, bits) == strtog_nanbits) {
              (&rv)->u_l[1] = 0x7ff00000 | bits[1];
              (&rv)->u_l[0] = bits[0];
            } else {
              (&rv)->u_l[1] = 0x7ff80000;
              (&rv)->u_l[0] = 0x00000000;
            }
            goto ret;
          }
        }
    ret0:
      s = s00;
      sign = 0;
    }
    goto ret;
  }
  e1 = e -= nf;
  if (!nd0)
    nd0 = nd;
  k = nd < 15 + 1 ? nd : 15 + 1;
  (&rv)->d = y;
  if (k > 9) {
    (&rv)->d = __tens_d2a[k - 9] * (&rv)->d + z;
  }
  if (nd <= 15

      && 1 == 1) {
    if (!e)
      goto ret;
    if (e > 0) {
      if (e <= 22) {
        (&rv)->d *= __tens_d2a[e];
        goto ret;
      }
      i = 15 - nd;
      if (e <= 22 + i) {
        e -= i;
        (&rv)->d *= __tens_d2a[i];
        (&rv)->d *= __tens_d2a[e];
        goto ret;
      }
    } else if (e >= -22) {
      (&rv)->d /= __tens_d2a[-e];
      goto ret;
    }
  }
  e1 += nd - k;
  scale = 0;
  if (e1 > 0) {
    if ((i = e1 & 15) != 0)
      (&rv)->d *= __tens_d2a[i];
    if (e1 &= ~15) {
      if (e1 > 308) {
      ovfl:
        (&rv)->u_l[1] = 0x7ff00000;
        (&rv)->u_l[0] = 0;
      range_err:
        if (bd0) {
          __bfree_d2a(bb);
          __bfree_d2a(bd);
          __bfree_d2a(bs);
          __bfree_d2a(bd0);
          __bfree_d2a(delta);
        }
        *(__errno()) = 34;
        goto ret;
      }
      e1 >>= 4;
      for (j = 0; e1 > 1; j++, e1 >>= 1)
        if (e1 & 1)
          (&rv)->d *= __bigtens_D2A[j];
      (&rv)->u_l[1] -= 53 * 0x100000;
      (&rv)->d *= __bigtens_D2A[j];
      if ((z = (&rv)->u_l[1] & 0x7ff00000) > 0x100000 * (1024 + 1023 - 53))
        goto ovfl;
      if (z > 0x100000 * (1024 + 1023 - 1 - 53)) {
        (&rv)->u_l[1] = (0xfffff | 0x100000 * (1024 + 1023 - 1));
        (&rv)->u_l[0] = 0xffffffff;
      } else
        (&rv)->u_l[1] += 53 * 0x100000;
    }
  } else if (e1 < 0) {
    e1 = -e1;
    if ((i = e1 & 15) != 0)
      (&rv)->d /= __tens_d2a[i];
    if (e1 >>= 4) {
      if (e1 >= 1 << 5)
        goto undfl;
      if (e1 & 0x10)
        scale = 2 * 53;
      for (j = 0; e1 > 0; j++, e1 >>= 1)
        if (e1 & 1)
          (&rv)->d *= tinytens[j];
      if (scale && (j = 2 * 53 + 1 - (((&rv)->u_l[1] & 0x7ff00000) >> 20)) > 0) {
        if (j >= 32) {
          (&rv)->u_l[0] = 0;
          if (j >= 53)
            (&rv)->u_l[1] = (53 + 2) * 0x100000;
          else
            (&rv)->u_l[1] &= 0xffffffff << (j - 32);
        } else
          (&rv)->u_l[0] &= 0xffffffff << j;
      }
      if (!(&rv)->d) {
      undfl:
        (&rv)->d = 0.;
        goto range_err;
      }
    }
  }
  bd0 = __s2b_d2a(s0, nd0, nd, y, 1);
  if (bd0 == NULL)
    goto ovfl;
  for (;;) {
    bd = __balloc_d2a(bd0->k);
    if (bd == NULL)
      goto ovfl;
    memcpy(&bd->sign, &bd0->sign, bd0->wds * sizeof(uilong) + 2 * sizeof(int));
    bb = __d2b_d2a((&rv)->d, &bbe, &bbbits);
    if (bb == NULL)
      goto ovfl;
    bs = __i2b_d2a(1);
    if (bs == NULL)
      goto ovfl;
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
    Lsb = 1;
    Lsb1 = 0;
    j = bbe - scale;
    i = j + bbbits - 1;
    j = 53 + 1 - bbbits;
    if (i < (-1022)) {
      i = (-1022) - i;
      j -= i;
      if (i < 32)
        Lsb <<= i;
      else
        Lsb1 = Lsb << (i - 32);
    }
    bb2 += j;
    bd2 += j;
    bd2 += scale;
    i = bb2 < bd2 ? bb2 : bd2;
    if (i > bs2)
      i = bs2;
    if (i > 0) {
      bb2 -= i;
      bd2 -= i;
      bs2 -= i;
    }
    if (bb5 > 0) {
      bs = __pow5mult_d2a(bs, bb5);
      if (bs == NULL)
        goto ovfl;
      bb1 = __mult_d2a(bs, bb);
      if (bb1 == NULL)
        goto ovfl;
      __bfree_d2a(bb);
      bb = bb1;
    }
    if (bb2 > 0) {
      bb = __lshift_d2a(bb, bb2);
      if (bb == NULL)
        goto ovfl;
    }
    if (bd5 > 0) {
      bd = __pow5mult_d2a(bd, bd5);
      if (bd == NULL)
        goto ovfl;
    }
    if (bd2 > 0) {
      bd = __lshift_d2a(bd, bd2);
      if (bd == NULL)
        goto ovfl;
    }
    if (bs2 > 0) {
      bs = __lshift_d2a(bs, bs2);
      if (bs == NULL)
        goto ovfl;
    }
    delta = __diff_d2a(bb, bd);
    if (delta == NULL)
      goto ovfl;
    dsign = delta->sign;
    delta->sign = 0;
    i = __cmp_d2a(delta, bs);
    if (i < 0) {
      if (dsign || (&rv)->u_l[0] || (&rv)->u_l[1] & 0xfffff
          || ((&rv)->u_l[1] & 0x7ff00000) <= (2 * 53 + 1) * 0x100000) {
        break;
      }
      if (!delta->x[0] && delta->wds <= 1) {
        break;
      }
      delta = __lshift_d2a(delta, 1);
      if (delta == NULL)
        goto ovfl;
      if (__cmp_d2a(delta, bs) > 0)
        goto drop_down;
      break;
    }
    if (i == 0) {
      if (dsign) {
        if (((&rv)->u_l[1] & 0xfffff) == 0xfffff
            && (&rv)->u_l[0]
                == ((scale
                        && (y = (&rv)->u_l[1] & 0x7ff00000) <= 2 * 53 * 0x100000)
                        ? (0xffffffff
                              & (0xffffffff << (2 * 53 + 1 - (y >> 20))))
                        : 0xffffffff)) {
          if ((&rv)->u_l[1] == (0xfffff | 0x100000 * (1024 + 1023 - 1))
              && (&rv)->u_l[0] == 0xffffffff)
            goto ovfl;
          (&rv)->u_l[1] = ((&rv)->u_l[1] & 0x7ff00000) + 0x100000;
          (&rv)->u_l[0] = 0;
          dsign = 0;
          break;
        }
      } else if (!((&rv)->u_l[1] & 0xfffff) && !(&rv)->u_l[0]) {
      drop_down:
        if (scale) {
          u_l = (&rv)->u_l[1] & 0x7ff00000;
          if (u_l <= (2 * 53 + 1) * 0x100000) {
            if (u_l > (53 + 2) * 0x100000)
              break;
            goto undfl;
          }
        }
        u_l = ((&rv)->u_l[1] & 0x7ff00000) - 0x100000;
        (&rv)->u_l[1] = u_l | 0xfffff;
        (&rv)->u_l[0] = 0xffffffff;
        break;
      }
      if (Lsb1) {
        if (!((&rv)->u_l[1] & Lsb1))
          break;
      } else if (!((&rv)->u_l[0] & Lsb))
        break;
      if (dsign)
        (&rv)->d += __sulp_D2A(&rv, scale);
      else {
        (&rv)->d -= __sulp_D2A(&rv, scale);
        if (!(&rv)->d)
          goto undfl;
      }
      dsign = 1 - dsign;
      break;
    }
    if ((aadj = __ratio_d2a(delta, bs)) <= 2.) {
      if (dsign)
        aadj = (&aadj1)->d = 1.;
      else if ((&rv)->u_l[0] || (&rv)->u_l[1] & 0xfffff) {
        if ((&rv)->u_l[0] == 1 && !(&rv)->u_l[1])
          goto undfl;
        aadj = 1.;
        (&aadj1)->d = -1.;
      } else {
        if (aadj < 2. / 2)
          aadj = 1. / 2;
        else
          aadj *= 0.5;
        (&aadj1)->d = -aadj;
      }
    } else {
      aadj *= 0.5;
      (&aadj1)->d = dsign ? aadj : -aadj;
      if (1 == 0)
        (&aadj1)->d += 0.5;
    }
    y = (&rv)->u_l[1] & 0x7ff00000;
    if (y == 0x100000 * (1024 + 1023 - 1)) {
      (&rv0)->d = (&rv)->d;
      (&rv)->u_l[1] -= 53 * 0x100000;
      (&adj)->d = (&aadj1)->d * __ulp_d2a(&rv);
      (&rv)->d += (&adj)->d;
      if (((&rv)->u_l[1] & 0x7ff00000) >= 0x100000 * (1024 + 1023 - 53)) {
        if ((&rv0)->u_l[1] == (0xfffff | 0x100000 * (1024 + 1023 - 1))
            && (&rv0)->u_l[0] == 0xffffffff)
          goto ovfl;
        (&rv)->u_l[1] = (0xfffff | 0x100000 * (1024 + 1023 - 1));
        (&rv)->u_l[0] = 0xffffffff;
        goto cont;
      } else
        (&rv)->u_l[1] += 53 * 0x100000;
    } else {
      if (scale && y <= 2 * 53 * 0x100000) {
        if (aadj <= 0x7fffffff) {
          if ((z = aadj) <= 0)
            z = 1;
          aadj = z;
          (&aadj1)->d = dsign ? aadj : -aadj;
        }
        (&aadj1)->u_l[1] += (2 * 53 + 1) * 0x100000 - y;
      }
      (&adj)->d = (&aadj1)->d * __ulp_d2a(&rv);
      (&rv)->d += (&adj)->d;
    }
    z = (&rv)->u_l[1] & 0x7ff00000;
    if (!scale)
      if (y == z) {
        u_l = (int)aadj;
        aadj -= u_l;
        if (dsign || (&rv)->u_l[0] || (&rv)->u_l[1] & 0xfffff) {
          if (aadj < .4999999 || aadj > .5000001)
            break;
        } else if (aadj < .4999999 / 2)
          break;
      }
  cont:
    __bfree_d2a(bb);
    __bfree_d2a(bd);
    __bfree_d2a(bs);
    __bfree_d2a(delta);
  }
  __bfree_d2a(bb);
  __bfree_d2a(bd);
  __bfree_d2a(bs);
  __bfree_d2a(bd0);
  __bfree_d2a(delta);
  if (scale) {
    (&rv0)->u_l[1] = 0x3ff00000 - 2 * 53 * 0x100000;
    (&rv0)->u_l[0] = 0;
    (&rv)->d *= (&rv0)->d;
    if (!((&rv)->u_l[1] & 0x7ff00000))
      *(__errno()) = 34;
  }
ret:
  if (se)
    *se = (char*)s;
  return sign ? -(&rv)->d : (&rv)->d;
}