#include "gdtoa.h"
void __ULtoQ_D2A(ULong* L, ULong* bits, int exp, int k)
{
  switch (k & STRTOG_Retmask) {
  case STRTOG_NoNumber:
  case STRTOG_Zero:
    L[0] = L[1] = L[2] = L[3] = 0;
    break;
  case STRTOG_Normal:
  case STRTOG_NaNbits:
    L[0] = bits[0];
    L[1] = bits[1];
    L[2] = bits[2];
    L[3] = (bits[3] & ~0x10000) | ((exp + 0x3fff + 112) << 16);
    break;
  case STRTOG_Denormal:
    L[0] = bits[0];
    L[1] = bits[1];
    L[2] = bits[2];
    L[3] = bits[3];
    break;
  case STRTOG_NoMemory:
    *(__errno()) = 34;
  case STRTOG_Infinite:
    L[3] = 0x7fff0000;
    L[2] = L[1] = L[0] = 0;
    break;
  case STRTOG_NaN:
    L[0] = 0x00000000;
    L[1] = 0x00000000;
    L[2] = 0x00000000;
    L[3] = 0x7fff8000;
  }
  if (k & STRTOG_Neg)
    L[3] |= 0x80000000L;
}
int __strtorQ(const char* s, char** sp, int rounding, void* L)
{
  static FPI fpi0 = { 113, 1 - 16383 - 113 + 1, 32766 - 16383 - 113 + 1, 1, 0 };
  FPI *fpi, fpi1;
  ULong bits[4];
  int exp;
  int k;
  fpi = &fpi0;
  if (rounding != FPI_Round_near) {
    fpi1 = fpi0;
    fpi1.rounding = rounding;
    fpi = &fpi1;
  }
  k = __strtodg(s, sp, fpi, &exp, bits);
  __ULtoQ_D2A((ULong*)L, bits, exp, k);
  return k;
}