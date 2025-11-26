#ifndef __GDTOA_H
#define __GDTOA_H
#include "stdlib.h"
#include "string.h"
#include "sys/types.h"
#include "errno.h"
#include "float.h"
#ifdef __cplusplus
extern "C"
{
#endif
struct ieee_single
{
	unsigned sng_frac	: 23;
	unsigned sng_exp	: 8;
	unsigned sng_sign	: 1;
};
struct ieee_double
{
	unsigned dbl_fracl;
	unsigned dbl_frach	: 20;
	unsigned dbl_exp	: 11;
	unsigned dbl_sign	: 1;
};
struct ieee_ext
{
	unsigned ext_fracl;
	unsigned ext_frach;
	unsigned ext_exp	: 16;
	unsigned ext_sign	: 1;
};
typedef unsigned int uilong;
typedef unsigned short ushort;
enum
{
	strtog_zero		= 0x000,
	strtog_normal	= 0x001,
	strtog_denormal	= 0x002,
	strtog_infinite	= 0x003,
	strog_nan		= 0x004,
	strtog_nanbits	= 0x005,
	strtog_no_num	= 0x006,
	strtog_nomemory = 0x007,
	strtog_retmask	= 0x00F,
	strtog_inexlo	= 0x010,
	strtog_inexhi	= 0x020,
	strtog_inexact	= 0x030,
	strtog_uflow	= 0x040,
	strtog_oflow	= 0x080,
	strtog_neg		= 0x100
};
typedef struct fpi
{
	int nbits;
	int emin;
	int emax;
	int rounding;
	int sudden_underflow;
} fpi;
enum
{
	fpi_round_zero = 0,
	fpi_round_near = 1,
	fpi_round_up = 2,
	fpi_round_down = 3
};
typedef union { double d; uilong u_l[2]; } udouble;
typedef struct big_int
{
	struct big_int *next;
	int k, maxwds, sign, wds;
	uilong x[1];
} big_int;
extern char* __dtoa(double d, int mode, int ndigits, int* decpt, int* sign, char** rve);
extern char* __gdtoa(fpi* fpi, int be, uilong* bits, int* kindp, int mode, int ndigits, int* decpt, char** rve);
extern void __freedtoa(char*);
extern float  strtof(const char*, char**);
extern double strtod(const char*, char**);
extern int __strtodg(const char*, char**, fpi*, int*, uilong*);
char* __hdtoa(double, const char*, int, int*, int*, char**);
char* __hldtoa(long double, const char*, int, int*, int*, char**);
char* __ldtoa(long double*, int, int, int*, int*, char**);
extern int	__strtord(const char*, char**, int, double*);
extern int	__strtorq(const char*, char**, int, void*);
extern char* __dtoa_result_d2a;
extern const double __bigtens_d2a[], __tens_d2a[], __tinytens_d2a[];
extern unsigned char __hexdig_d2a[];
extern big_int *__balloc_d2a(int);
extern void __bfree_d2a(big_int*);
extern void __ultod_d2a(uilong*, uilong*, int, int);
extern void __ul_to_q_d2a(uilong*, uilong*, int, int);
extern uilong __any_on_d2a(big_int*, int);
extern double __b2d_d2a(big_int*, int*);
extern int __cmp_d2a(big_int*, big_int*);
extern void __copybits_d2a(uilong*, int, big_int*);
extern big_int* __d2b_d2a(double, int*, int*);
extern void __decrement_d2a(big_int*);
extern big_int* __diff_d2a(big_int*, big_int*);
extern int __gethex_d2a(const char**, fpi*, int*, big_int**, int);
extern void __hexdig_init_d2a(void);
extern int __hexnan_d2a (const char**, fpi*, uilong*);
extern int __hi0bits_d2a(uilong);
extern big_int* __i2b_d2a(int);
extern big_int* __increment_d2a(big_int*);
extern int __lo0bits_d2a(uilong*);
extern big_int* __lshift_d2a(big_int*, int);
extern int __match_d2a(const char**, char*);
extern big_int* __mult_d2a(big_int*, big_int*);
extern big_int* __multadd_d2a(big_int*, int, int);
extern char* __nrv_alloc_d2a(char*, char**, int);
extern big_int* __pow5mult_d2a(big_int*, int);
extern int __quorem_d2a(big_int*, big_int*);
extern double __ratio_d2a(big_int*, big_int*);
extern void __rshift_d2a(big_int*, int);
extern char* __rv_alloc_d2a(int);
extern big_int* __s2b_d2a(const char*, int, int, uilong, int);
extern big_int* __set_ones_d2a(big_int*, int);
extern big_int* __sum_d2a(big_int*, big_int*);
extern int __trailz_d2a(big_int*);
extern double __ulp_d2a(udouble*);
#ifdef __cplusplus
}
#endif
#endif
