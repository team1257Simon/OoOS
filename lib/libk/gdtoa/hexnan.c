#include "gdtoa.h"
static void L_shift(ULong* x, ULong* x1, int i)
{
  int j;
  i = 8 - i;
  i <<= 2;
  j = 32 - i;
  do {
    *x |= x[1] << j;
    x[1] >>= i;
  } while (++x < x1);
}
int __hexnan_D2A(const char** sp, FPI* fpi, ULong* x0)
{
  ULong c, h, *x, *x1, *xe;
  const char* s;
  int havedig, hd0, i, nbits;
  if (!__hexdig_D2A['0'])
    __hexdig_init_D2A();
  nbits = fpi->nbits;
  x = x0 + (nbits >> 5);
  if (nbits & 31)
    x++;
  *--x = 0;
  x1 = xe = x;
  havedig = hd0 = i = 0;
  s = *sp;
  while ((c = *(const unsigned char*)(s + 1)) && c <= ' ')
    ++s;
  if (s[1] == '0' && (s[2] == 'x' || s[2] == 'X') && *(const unsigned char*)(s + 3) > ' ')
    s += 2;
  while ((c = *(const unsigned char*)++s)) {
    if (!(h = __hexdig_D2A[c])) {
      if (c <= ' ') {
        if (hd0 < havedig) {
          if (x < x1 && i < 8)
            L_shift(x, x1, i);
          if (x <= x0) {
            i = 8;
            continue;
          }
          hd0 = havedig;
          *--x = 0;
          x1 = x;
          i = 0;
        }
        while (*(const unsigned char*)(s + 1) <= ' ')
          ++s;
        if (s[1] == '0' && (s[2] == 'x' || s[2] == 'X') && *(const unsigned char*)(s + 3) > ' ')
          s += 2;
        continue;
      }
      if (c == ')' && havedig) {
        *sp = s + 1;
        break;
      }
      do {
        if (c == ')') {
          *sp = s + 1;
          break;
        }
      } while ((c = *++s));
      return STRTOG_NaN;
    }
    havedig++;
    if (++i > 8) {
      if (x <= x0)
        continue;
      i = 1;
      *--x = 0;
    }
    *x = (*x << 4) | (h & 0xf);
  }
  if (!havedig)
    return STRTOG_NaN;
  if (x < x1 && i < 8)
    L_shift(x, x1, i);
  if (x > x0) {
    x1 = x0;
    do
      *x1++ = *x++;
    while (x <= xe);
    do
      *x1++ = 0;
    while (x1 <= xe);
  } else {
    if ((i = nbits & (32 - 1)) != 0)
      *xe &= ((ULong)0xffffffff) >> (32 - i);
  }
  for (x1 = xe;; --x1) {
    if (*x1 != 0)
      break;
    if (x1 == x0) {
      *x1 = 1;
      break;
    }
  }
  return STRTOG_NaNbits;
}