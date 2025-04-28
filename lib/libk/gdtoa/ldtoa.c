#include "gdtoa.h"
extern void panic(const char* msg);
char*       __ldtoa(long double* ld, int mode, int ndigits, int* decpt, int* sign, char** rve)
{
    fpi              fpi = { 64, -16445, 16320, 1, 0 };
    int              be, kind;
    char*            ret;
    struct ieee_ext* p = (struct ieee_ext*)ld;
    uint32_t         bits[3];
    void*            vbits = bits;
    *sign                  = p->ext_sign;
    fpi.rounding ^= (fpi.rounding >> 1) & p->ext_sign;
    be        = p->ext_exp - 16446;
    (bits)[0] = (uint32_t)(p)->ext_fracl;
    (bits)[1] = (uint32_t)(p)->ext_frach;
    (bits)[2] = (uint32_t)(p)->ext_exp;
    switch(__builtin_fpclassify(0, 1, 4, 3, 2, *ld))
    {
    case 0x04:
        kind = strtog_normal;
        bits[2] |= 1 << ((64 - 1) % 32);
        break;
    case 0x10: kind = strtog_zero; break;
    case 0x08:
        kind = strtog_denormal;
        be++;
        break;
    case 0x01: kind = strtog_infinite; break;
    case 0x02: kind = strog_nan; break;
    default: panic("ldtoa: classify error"); abort();
    }
    ret = __gdtoa(&fpi, be, vbits, &kind, mode, ndigits, decpt, rve);
    if(*decpt == -32768) *decpt = 0x7FFFFFFF;
    return ret;
}