#include "gdtoa.h"
float strtof(const char* s, char** sp)
{
  static FPI fpi0 = { 24, 1 - 127 - 24 + 1, 254 - 127 - 24 + 1, 1, 0 };
  ULong bits[1];
  int exp;
  int k;
  union
  {
    ULong L[1];
    float f;
  } u;
  u.L[0] = 0;
  k = __strtodg(s, sp, &fpi0, &exp, bits);
  switch (k & STRTOG_Retmask) {
  case STRTOG_NoNumber:
  case STRTOG_Zero:
    u.L[0] = 0;
    break;
  case STRTOG_Normal:
  case STRTOG_NaNbits:
    u.L[0] = (bits[0] & 0x7fffff) | ((exp + 0x7f + 23) << 23);
    break;
  case STRTOG_Denormal:
    u.L[0] = bits[0];
    break;
  case STRTOG_NoMemory:
    *(__errno()) = 34;
  case STRTOG_Infinite:
    u.L[0] = 0x7f800000;
    break;
  case STRTOG_NaN:
    u.L[0] = 0x7fc00000;
  }
  if (k & STRTOG_Neg)
    u.L[0] |= 0x80000000L;
  return u.f;
}