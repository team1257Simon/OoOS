#include "gdtoa.h"
void __ULtod_D2A(ULong* L, ULong* bits, int exp, int k)
{
  switch (k & STRTOG_Retmask) 
  {
  case STRTOG_NoNumber:
  case STRTOG_Zero:
    L[0] = L[1] = 0;
    break;
  case STRTOG_Denormal:
    L[0] = bits[0];
    L[1] = bits[1];
    break;
  case STRTOG_Normal:
  case STRTOG_NaNbits:
    L[0] = bits[0];
    L[1] = (bits[1] & ~0x100000) | ((exp + 0x3ff + 52) << 20);
    break;
  case STRTOG_NoMemory:
    *(__errno()) = 34;
  case STRTOG_Infinite:
    L[1] = 0x7ff00000;
    L[0] = 0;
    break;
  case STRTOG_NaN:
    L[0] = 0x00000000;
    L[1] = 0x7ff80000;
  }
  if (k & STRTOG_Neg)
    L[1] |= 0x80000000L;
}
int __strtord(const char* s, char** sp, int rounding, double* d)
{
  static FPI fpi0 = { 53, 1 - 1023 - 53 + 1, 2046 - 1023 - 53 + 1, 1, 0 };
  FPI *fpi, fpi1;
  ULong bits[2];
  int exp;
  int k;
  fpi = &fpi0;
  if (rounding != FPI_Round_near)
  {
    fpi1 = fpi0;
    fpi1.rounding = rounding;
    fpi = &fpi1;
  }
  k = __strtodg(s, sp, fpi, &exp, bits);
  __ULtod_D2A((ULong*)d, bits, exp, k);
  return k;
}