#include "gdtoa.h"
void __ul_to_q_d2a(uilong* u_l, uilong* bits, int exp, int k)
{
  switch (k & strtog_retmask) {
  case strtog_no_num:
  case strtog_zero:
    u_l[0] = u_l[1] = u_l[2] = u_l[3] = 0;
    break;
  case strtog_normal:
  case strtog_nanbits:
    u_l[0] = bits[0];
    u_l[1] = bits[1];
    u_l[2] = bits[2];
    u_l[3] = (bits[3] & ~0x10000) | ((exp + 0x3fff + 112) << 16);
    break;
  case strtog_denormal:
    u_l[0] = bits[0];
    u_l[1] = bits[1];
    u_l[2] = bits[2];
    u_l[3] = bits[3];
    break;
  case strtog_nomemory:
    *(__errno()) = 34;
  case strtog_infinite:
    u_l[3] = 0x7fff0000;
    u_l[2] = u_l[1] = u_l[0] = 0;
    break;
  case strog_nan:
    u_l[0] = 0x00000000;
    u_l[1] = 0x00000000;
    u_l[2] = 0x00000000;
    u_l[3] = 0x7fff8000;
  }
  if (k & strtog_neg)
    u_l[3] |= 0x80000000L;
}
int __strtor_q(const char* s, char** sp, int rounding, void* u_l)
{
  static fpi fpi0 = { 113, 1 - 16383 - 113 + 1, 32766 - 16383 - 113 + 1, 1, 0 };
  fpi *fpi, fpi1;
  uilong bits[4];
  int exp;
  int k;
  fpi = &fpi0;
  if (rounding != fpi_round_near) {
    fpi1 = fpi0;
    fpi1.rounding = rounding;
    fpi = &fpi1;
  }
  k = __strtodg(s, sp, fpi, &exp, bits);
  __ul_to_q_d2a((uilong*)u_l, bits, exp, k);
  return k;
}