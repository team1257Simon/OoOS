#ifndef __GDTOA_H
#define __GDTOA_H
#include "stdlib.h"
#include "string.h"
#include "sys/types.h"
#include "errno.h"
#include "float.h"
#ifdef __cplusplus
extern "C" {
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
typedef unsigned int ULong;
typedef unsigned short UShort;
enum 
{
	STRTOG_Zero		= 0x000,
	STRTOG_Normal	= 0x001,
	STRTOG_Denormal	= 0x002,
	STRTOG_Infinite	= 0x003,
	STRTOG_NaN		= 0x004,
	STRTOG_NaNbits	= 0x005,
	STRTOG_NoNumber	= 0x006,
	STRTOG_NoMemory = 0x007,
	STRTOG_Retmask	= 0x00f,
	STRTOG_Inexlo	= 0x010,
	STRTOG_Inexhi	= 0x020,
	STRTOG_Inexact	= 0x030,
	STRTOG_Underflow= 0x040,
	STRTOG_Overflow	= 0x080,
	STRTOG_Neg		= 0x100
};
typedef struct FPI 
{
	int nbits;
	int emin;
	int emax;
	int rounding;
	int sudden_underflow;
} FPI;
enum
{
	FPI_Round_zero = 0,
	FPI_Round_near = 1,
	FPI_Round_up = 2,
	FPI_Round_down = 3
};
typedef union { double d; ULong L[2]; } U;
typedef struct Bigint 
{
	struct Bigint *next;
	int k, maxwds, sign, wds;
	ULong x[1];
} Bigint;

extern char* __dtoa  (double d, int mode, int ndigits, int *decpt, int *sign, char **rve);
extern char* __gdtoa (FPI *fpi, int be, ULong *bits, int *kindp, int mode, int ndigits, int *decpt, char **rve);
extern void __freedtoa (char*);
extern float  strtof (const char *, char **);
extern double strtod (const char *, char **);
extern int __strtodg (const char*, char**, FPI*, int*, ULong*);
char	*__hdtoa(double, const char *, int, int *, int *, char **);
char	*__hldtoa(long double, const char *, int, int *, int *, char **);
char	*__ldtoa(long double *, int, int, int *, int *, char **);
extern char*	__g_ddfmt  (char*, double*, int, size_t);
extern char*	__g_dfmt   (char*, double*, int, size_t);
extern char*	__g_ffmt   (char*, float*,  int, size_t);
extern char*	__g_Qfmt   (char*, void*,   int, size_t);
extern char*	__g_xfmt   (char*, void*,   int, size_t);
extern char*	__g_xLfmt  (char*, void*,   int, size_t);
extern int	__strtoId  (const char*, char**, double*, double*);
extern int	__strtoIdd (const char*, char**, double*, double*);
extern int	__strtoIf  (const char*, char**, float*, float*);
extern int	__strtoIQ  (const char*, char**, void*, void*);
extern int	__strtoIx  (const char*, char**, void*, void*);
extern int	__strtoIxL (const char*, char**, void*, void*);
extern int	__strtord  (const char*, char**, int, double*);
extern int	__strtordd (const char*, char**, int, double*);
extern int	__strtorf  (const char*, char**, int, float*);
extern int	__strtorQ  (const char*, char**, int, void*);
extern int	__strtorx  (const char*, char**, int, void*);
extern int	__strtorxL (const char*, char**, int, void*);
extern int	__strtodI  (const char*, char**, double*);
extern int	__strtopd  (const char*, char**, double*);
extern int	__strtopdd (const char*, char**, double*);
extern int	__strtopf  (const char*, char**, float*);
extern int	__strtopQ  (const char*, char**, void*);
extern int	__strtopx  (const char*, char**, void*);
extern int	__strtopxL (const char*, char**, void*);
extern char *__dtoa_result_D2A;
extern const double __bigtens_D2A[], __tens_D2A[], __tinytens_D2A[];
extern unsigned char __hexdig_D2A[];
extern Bigint *__Balloc_D2A (int);
extern void __Bfree_D2A (Bigint*);
extern void __ULtof_D2A (ULong*, ULong*, int, int);
extern void __ULtod_D2A (ULong*, ULong*, int, int);
extern void __ULtodd_D2A (ULong*, ULong*, int, int);
extern void __ULtoQ_D2A (ULong*, ULong*, int, int);
extern void __ULtox_D2A (UShort*, ULong*, int, int);
extern void __ULtoxL_D2A (ULong*, ULong*, int, int);
extern ULong __any_on_D2A (Bigint*, int);
extern double __b2d_D2A (Bigint*, int*);
extern int __cmp_D2A (Bigint*, Bigint*);
extern void __copybits_D2A (ULong*, int, Bigint*);
extern Bigint *__d2b_D2A (double, int*, int*);
extern void __decrement_D2A (Bigint*);
extern Bigint *__diff_D2A (Bigint*, Bigint*);
extern char *__g__fmt_D2A (char*, char*, char*, int, ULong, size_t);
extern int __gethex_D2A (const char**, FPI*, int*, Bigint**, int);
extern void __hexdig_init_D2A(void);
extern int __hexnan_D2A (const char**, FPI*, ULong*);
extern int __hi0bits_D2A (ULong);
extern Bigint *__i2b_D2A (int);
extern Bigint *__increment_D2A (Bigint*);
extern int __lo0bits_D2A (ULong*);
extern Bigint *__lshift_D2A (Bigint*, int);
extern int __match_D2A (const char**, char*);
extern Bigint *__mult_D2A (Bigint*, Bigint*);
extern Bigint *__multadd_D2A (Bigint*, int, int);
extern char *__nrv_alloc_D2A (char*, char **, int);
extern Bigint *__pow5mult_D2A (Bigint*, int);
extern int __quorem_D2A (Bigint*, Bigint*);
extern double __ratio_D2A (Bigint*, Bigint*);
extern void __rshift_D2A (Bigint*, int);
extern char *__rv_alloc_D2A (int);
extern Bigint *__s2b_D2A (const char*, int, int, ULong, int);
extern Bigint *__set_ones_D2A (Bigint*, int);
extern char *__strcp_D2A (char*, const char*);
extern int __strtoIg_D2A (const char*, char**, FPI*, int*, Bigint**, int*);
extern Bigint *__sum_D2A (Bigint*, Bigint*);
extern int __trailz_D2A (Bigint*);
extern double __ulp_D2A (U*);
#ifdef __cplusplus
}
#endif
#endif
