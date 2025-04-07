#include "gdtoa.h"
extern void panic(const char*);
static int roundup(char* s0, int ndigits)
{
  char* s;
  for (s = s0 + ndigits - 1; *s == 0xf; s--) {
    if (s == s0) {
      *s = 1;
      return (1);
    }
    *s = 0;
  }
  ++*s;
  return (0);
}
static void dorounding(char* s0, int ndigits, int sign, int* decpt)
{
  int adjust = 0;
  if ((s0[ndigits] > 8) || (s0[ndigits] == 8 && s0[ndigits + 1] & 1))
    adjust = roundup(s0, ndigits);
  if (adjust)
    *decpt += 4;
}
char* __hdtoa(double d, const char* xdigs, int ndigits, int* decpt, int* sign, char** rve)
{
  static const int sigfigs = (53 + 3) / 4;
  struct ieee_double* p = (struct ieee_double*)&d;
  char *s, *s0;
  int bufsize;
  *sign = p->dbl_sign;
  switch (__builtin_fpclassify(0x02, 0x01, 0x04, 0x08, 0x10, d)) {
  case 0x04:
    *decpt = p->dbl_exp - (1024 - 2 + ((53 - 1) % 4));
    break;
  case 0x10:
    *decpt = 1;
    return (__nrv_alloc_d2a("0", rve, 1));
  case 0x08:
    d *= 0x1p514;
    *decpt = p->dbl_exp - (514 + (1024 - 2 + ((53 - 1) % 4)));
    break;
  case 0x01:
    *decpt = 0x7fffffff;
    return (__nrv_alloc_d2a("Infinity", rve, sizeof("Infinity") - 1));
  case 0x02:
    *decpt = 0x7fffffff;
    return (__nrv_alloc_d2a("NaN", rve, sizeof("NaN") - 1));
  default:
    panic("hdtoa: classify error");
    abort();
  }
  if (ndigits == 0)
    ndigits = 1;
  bufsize = (sigfigs > ndigits) ? sigfigs : ndigits;
  s0 = __rv_alloc_d2a(bufsize);
  if (s0 == (NULL))
    return ((NULL));
  for (s = s0 + bufsize - 1; s > s0 + sigfigs - 1; s--)
    *s = 0;
  for (; s > s0 + sigfigs - (32 / 4) - 1 && s > s0; s--) {
    *s = p->dbl_fracl & 0xf;
    p->dbl_fracl >>= 4;
  }
  for (; s > s0; s--) {
    *s = p->dbl_frach & 0xf;
    p->dbl_frach >>= 4;
  }
  *s = p->dbl_frach | (1U << ((53 - 1) % 4));
  if (ndigits < 0) {
    for (ndigits = sigfigs; s0[ndigits - 1] == 0; ndigits--)
      ;
  }
  if (sigfigs > ndigits && s0[ndigits] != 0)
    dorounding(s0, ndigits, p->dbl_sign, decpt);
  s = s0 + ndigits;
  if (rve != NULL)
    *rve = s;
  *s-- = '\0';
  for (; s >= s0; s--)
    *s = xdigs[(unsigned int)*s];
  return (s0);
}
char* __hldtoa(long double e, const char* xdigs, int ndigits, int* decpt, int* sign, char** rve)
{
  static const int sigfigs = (64 + 3) / 4;
  struct ieee_ext* p = (struct ieee_ext*)&e;
  char *s, *s0;
  int bufsize;
  int fbits = 0;
  *sign = p->ext_sign;
  switch (__builtin_fpclassify(0x02, 0x01, 0x04, 0x08, 0x10, e)) {
  case 0x04:
    *decpt = p->ext_exp - (16384 - 2 + ((64 - 1) % 4));
    break;
  case 0x10:
    *decpt = 1;
    return (__nrv_alloc_d2a("0", rve, 1));
  case 0x08:
    e *= 0x1p514L;
    *decpt = p->ext_exp - (514 + (16384 - 2 + ((64 - 1) % 4)));
    break;
  case 0x01:
    *decpt = 0x7fffffff;
    return (__nrv_alloc_d2a("Infinity", rve, sizeof("Infinity") - 1));
  case 0x02:
    *decpt = 0x7fffffff;
    return (__nrv_alloc_d2a("NaN", rve, sizeof("NaN") - 1));
  default:
    panic("hldtoa: classify error");
    abort();
  }
  if (ndigits == 0)
    ndigits = 1;
  bufsize = (sigfigs > ndigits) ? sigfigs : ndigits;
  s0 = __rv_alloc_d2a(bufsize);
  if (s0 == NULL)
    return (NULL);
  for (s = s0 + bufsize - 1; s > s0 + sigfigs - 1; s--)
    *s = 0;
  for (fbits = 32 / 4; fbits > 0 && s > s0; s--, fbits--) {
    *s = p->ext_fracl & 0xf;
    p->ext_fracl >>= 4;
  }
  for (fbits = 16 / 4; fbits > 0 && s > s0; s--, fbits--) {
    *s = p->ext_frach & 0xf;
    p->ext_frach >>= 4;
  }
  *s = (p->ext_frach | (1U << ((64 - 1) % 4))) & 0xf;
  if (ndigits < 0) {
    for (ndigits = sigfigs; s0[ndigits - 1] == 0; ndigits--)
      ;
  }
  if (sigfigs > ndigits && s0[ndigits] != 0)
    dorounding(s0, ndigits, p->ext_sign, decpt);
  s = s0 + ndigits;
  if (rve != NULL)
    *rve = s;
  *s-- = '\0';
  for (; s >= s0; s--)
    *s = xdigs[(unsigned int)*s];
  return (s0);
}