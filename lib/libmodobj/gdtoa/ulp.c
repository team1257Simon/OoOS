#include "gdtoa.h"
double __ulp_d2a(udouble* x)
{
    int     u_l;
    udouble a;
    u_l = ((x)->u_l[1] & 0x7FF00000) - (53 - 1) * 0x100000;
    if(u_l > 0)
    {
        (&a)->u_l[1] = u_l;
        (&a)->u_l[0] = 0;
    }
    else
    {
        u_l = -u_l >> 20;
        if(u_l < 20)
        {
            (&a)->u_l[1] = 0x80000 >> u_l;
            (&a)->u_l[0] = 0;
        }
        else
        {
            (&a)->u_l[1] = 0;
            u_l -= 20;
            (&a)->u_l[0] = u_l >= 31 ? 1 : 1 << (31 - u_l);
        }
    }
    return (&a)->d;
}