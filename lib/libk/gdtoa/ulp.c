#include "gdtoa.h"
double __ulp_D2A(U* x)
{
  int L;
  U a;
  L = ((x)->L[1] & 0x7ff00000) - (53 - 1) * 0x100000;
  if (L > 0) {
    (&a)->L[1] = L;
    (&a)->L[0] = 0;
  } else {
    L = -L >> 20;
    if (L < 20) {
      (&a)->L[1] = 0x80000 >> L;
      (&a)->L[0] = 0;
    } else {
      (&a)->L[1] = 0;
      L -= 20;
      (&a)->L[0] = L >= 31 ? 1 : 1 << (31 - L);
    }
  }
  return (&a)->d;
}