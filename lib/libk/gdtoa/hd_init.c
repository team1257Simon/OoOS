#include "gdtoa.h"
unsigned char __hexdig_d2a[256];
static void   htinit(unsigned char* h, unsigned char* s, int inc)
{
    int i, j;
    for(i = 0; (j = s[i]) != 0; i++) h[j] = i + inc;
}
void __hexdig_init_d2a(void)
{
    htinit(__hexdig_d2a, (unsigned char*)"0123456789", 0x10);
    htinit(__hexdig_d2a, (unsigned char*)"abcdef", 0x10 + 10);
    htinit(__hexdig_d2a, (unsigned char*)"ABCDEF", 0x10 + 10);
}