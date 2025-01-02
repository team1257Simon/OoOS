#include "gdtoa.h"
extern void panic(const char* msg);
char* __ldtoa(long double* ld, int mode, int ndigits, int* decpt, int* sign, char** rve)
{
  FPI fpi = { 64, (-16381) - 64, 16384 - 64, 1, 0 };
  int be, kind;
  char* ret;
  struct ieee_ext* p = (struct ieee_ext*)ld;
  uint32_t bits[3];
  void* vbits = bits;
  *sign = p->ext_sign;
  fpi.rounding ^= (fpi.rounding >> 1) & p->ext_sign;
  be = p->ext_exp - (16384 - 1) - (64 - 1);
    (bits)[0] = (uint32_t)(p)->ext_fracl;
    (bits)[1] = (uint32_t)(p)->ext_frach;
    (bits)[2] = (uint32_t)(p)->ext_exp;
  switch (__builtin_fpclassify (0, 1, 4, 3, 2, *ld)) {
  case 0x04:
    kind = STRTOG_Normal;
    bits[2] |= 1 << ((64 - 1) % 32);
    break;
  case 0x10:
    kind = STRTOG_Zero;
    break;
  case 0x08:
    kind = STRTOG_Denormal;
    be++;
    break;
  case 0x01:
    kind = STRTOG_Infinite;
    break;
  case 0x02:
    kind = STRTOG_NaN;
    break;
  default:
    panic("Unrecognized classification");
    abort();
  }
  ret = __gdtoa(&fpi, be, vbits, &kind, mode, ndigits, decpt, rve);
  if (*decpt == -32768)
    *decpt = 0x7fffffff;
  return ret;
}