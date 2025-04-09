







typedef long int ptrdiff_t;
typedef long unsigned int size_t;
typedef int wchar_t;
typedef struct {
  long long __max_align_ll __attribute__((__aligned__(__alignof__(long long))));
  long double __max_align_ld __attribute__((__aligned__(__alignof__(long double))));
} max_align_t;








typedef signed char __int8_t;

typedef unsigned char __uint8_t;
typedef short int __int16_t;

typedef short unsigned int __uint16_t;
typedef int __int32_t;

typedef unsigned int __uint32_t;
typedef long int __int64_t;

typedef long unsigned int __uint64_t;
typedef signed char __int_least8_t;

typedef unsigned char __uint_least8_t;
typedef short int __int_least16_t;

typedef short unsigned int __uint_least16_t;
typedef int __int_least32_t;

typedef unsigned int __uint_least32_t;
typedef long int __int_least64_t;

typedef long unsigned int __uint_least64_t;
typedef long int __intmax_t;







typedef long unsigned int __uintmax_t;







typedef long int __intptr_t;

typedef long unsigned int __uintptr_t;
typedef unsigned long __blkcnt_t;
typedef unsigned long __blksize_t;
typedef unsigned long __fsblkcnt_t;
typedef unsigned int __fsfilcnt_t;
typedef long _off_t;
typedef int __pid_t;
typedef int __dev_t;
typedef unsigned long __uid_t;
typedef unsigned long __gid_t;
typedef unsigned int __id_t;
typedef unsigned long __ino_t;
typedef unsigned int __mode_t;
__extension__ typedef long long _off64_t;
typedef _off_t __off_t;
typedef _off64_t __loff_t;
typedef long __key_t;
typedef long _fpos_t;

typedef unsigned long ;

typedef signed long _ssize_t;
struct __lock;
typedef struct __lock *_flock_t;
typedef void *_iconv_t;
typedef unsigned long __clock_t;
typedef long __time_t;


typedef unsigned long __clockid_t;
typedef unsigned long __timer_t;
typedef unsigned char __sa_family_t;
typedef unsigned int __socklen_t;
typedef int __nl_item;
typedef unsigned long __nlink_t;
typedef long __suseconds_t;
typedef unsigned long __useconds_t;



typedef char * __va_list;


typedef unsigned int wint_t;

typedef struct
{
  int __count;
  union
  {
    unsigned int __wch;
    unsigned char __wchb[4];
  } __value;
} _mbstate_t;
typedef unsigned int __ULong;
struct _reent;

struct __locale_t;






struct _Bigint
{
  struct _Bigint *_next;
  int _k, _maxwds, _sign, _wds;
  __ULong _x[1];
};


struct __tm
{
  int __tm_sec;
  int __tm_min;
  int __tm_hour;
  int __tm_mday;
  int __tm_mon;
  int __tm_year;
  int __tm_wday;
  int __tm_yday;
  int __tm_isdst;
};







struct _on_exit_args {
 void * _fnargs[32];
 void * _dso_handle[32];

 __ULong _fntypes;


 __ULong _is_cxa;
};
struct _atexit {
 struct _atexit *_next;
 int _ind;

 void (*_fns[32])(void);
        struct _on_exit_args _on_exit_args;
};
struct __sbuf {
 unsigned char *_base;
 int _size;
};
struct __sFILE {
  unsigned char *_p;
  int _r;
  int _w;
  short _flags;
  short _file;
  struct __sbuf _bf;
  int _lbfsize;






  void * _cookie;

  int (*_read) (struct _reent *, void *,
        char *, int);
  int (*_write) (struct _reent *, void *,
         const char *,
         int);
  _fpos_t (*_seek) (struct _reent *, void *, _fpos_t, int);
  int (*_close) (struct _reent *, void *);


  struct __sbuf _ub;
  unsigned char *_up;
  int _ur;


  unsigned char _ubuf[3];
  unsigned char _nbuf[1];


  struct __sbuf _lb;


  int _blksize;
  _off_t _offset;


  struct _reent *_data;



  _flock_t _lock;

  _mbstate_t _mbstate;
  int _flags2;
};
typedef struct __sFILE __FILE;



struct _glue
{
  struct _glue *_next;
  int _niobs;
  __FILE *_iobs;
};
struct _rand48 {
  unsigned short _seed[3];
  unsigned short _mult[3];
  unsigned short _add;




};
struct _reent
{
  int _errno;




  __FILE *_stdin, *_stdout, *_stderr;

  int _inc;
  char _emergency[25];


  int _unspecified_locale_info;
  struct __locale_t *_locale;

  int __sdidinit;

  void (*__cleanup) (struct _reent *);


  struct _Bigint *_result;
  int _result_k;
  struct _Bigint *_p5s;
  struct _Bigint **_freelist;


  int _cvtlen;
  char *_cvtbuf;

  union
    {
      struct
        {
          unsigned int _unused_rand;
          char * _strtok_last;
          char _asctime_buf[26];
          struct __tm _localtime_buf;
          int _gamma_signgam;
          __extension__ unsigned long long _rand_next;
          struct _rand48 _r48;
          _mbstate_t _mblen_state;
          _mbstate_t _mbtowc_state;
          _mbstate_t _wctomb_state;
          char _l64a_buf[8];
          char _signal_buf[24];
          int _getdate_err;
          _mbstate_t _mbrlen_state;
          _mbstate_t _mbrtowc_state;
          _mbstate_t _mbsrtowcs_state;
          _mbstate_t _wcrtomb_state;
          _mbstate_t _wcsrtombs_state;
   int _h_errno;
        } _reent;



      struct
        {

          unsigned char * _nextf[30];
          unsigned int _nmalloc[30];
        } _unused;
    } _new;



  struct _atexit *_atexit;
  struct _atexit _atexit0;



  void (**(_sig_func))(int);




  struct _glue __sglue;

  __FILE __sf[3];

};
extern struct _reent *_impure_ptr ;
extern struct _reent *const _global_impure_ptr ;

void _reclaim_reent (struct _reent *);






struct stat;
struct tms;
struct timeval;
struct timezone;
extern int _close_r (struct _reent *, int);
extern int _execve_r (struct _reent *, const char *, char *const *, char *const *);
extern int _fcntl_r (struct _reent *, int, int, int);
extern int _fork_r (struct _reent *);
extern int _fstat_r (struct _reent *, int, struct stat *);
extern int _getpid_r (struct _reent *);
extern int _isatty_r (struct _reent *, int);
extern int _kill_r (struct _reent *, int, int);
extern int _link_r (struct _reent *, const char *, const char *);
extern _off_t _lseek_r (struct _reent *, int, _off_t, int);
extern int _mkdir_r (struct _reent *, const char *, int);
extern int _open_r (struct _reent *, const char *, int, int);
extern _ssize_t _read_r (struct _reent *, int, void *, size_t);
extern int _rename_r (struct _reent *, const char *, const char *);
extern void *_sbrk_r (struct _reent *, ptrdiff_t);
extern int _stat_r (struct _reent *, const char *, struct stat *);
extern unsigned long _times_r (struct _reent *, struct tms *);
extern int _unlink_r (struct _reent *, const char *);
extern int _wait_r (struct _reent *, int *);
extern _ssize_t _write_r (struct _reent *, int, const void *, size_t);


extern int _gettimeofday_r (struct _reent *, struct timeval *__tp, void *__tzp);







struct __locale_t;
typedef struct __locale_t *locale_t;





int bcmp(const void *, const void *, size_t) __attribute__((__pure__));
void bcopy(const void *, void *, size_t);
void bzero(void *, size_t);


void explicit_bzero(void *, size_t);


int ffs(int) __attribute__((__const__));


int ffsl(long) __attribute__((__const__));
int ffsll(long long) __attribute__((__const__));
int fls(int) __attribute__((__const__));
int flsl(long) __attribute__((__const__));
int flsll(long long) __attribute__((__const__));


char *index(const char *, int) __attribute__((__pure__));
char *rindex(const char *, int) __attribute__((__pure__));

int strcasecmp(const char *, const char *) __attribute__((__pure__));
int strncasecmp(const char *, const char *, size_t) __attribute__((__pure__));


int strcasecmp_l (const char *, const char *, locale_t);
int strncasecmp_l (const char *, const char *, size_t, locale_t);






void * memchr (const void *, int, size_t);
int memcmp (const void *, const void *, size_t);
void * memcpy (void *restrict, const void *restrict, size_t);
void * memmove (void *, const void *, size_t);
void * memset (void *, int, size_t);
char *strcat (char *restrict, const char *restrict);
char *strchr (const char *, int);
int strcmp (const char *, const char *);
int strcoll (const char *, const char *);
char *strcpy (char *restrict, const char *restrict);
size_t strcspn (const char *, const char *);
char *strerror (int);
size_t strlen (const char *);
char *strncat (char *restrict, const char *restrict, size_t);
int strncmp (const char *, const char *, size_t);
char *strncpy (char *restrict, const char *restrict, size_t);
char *strpbrk (const char *, const char *);
char *strrchr (const char *, int);
size_t strspn (const char *, const char *);
char *strstr (const char *, const char *);

char *strtok (char *restrict, const char *restrict);

size_t strxfrm (char *restrict, const char *restrict, size_t);


int strcoll_l (const char *, const char *, locale_t);
char *strerror_l (int, locale_t);
size_t strxfrm_l (char *restrict, const char *restrict, size_t, locale_t);


char *strtok_r (char *restrict, const char *restrict, char **restrict);


int timingsafe_bcmp (const void *, const void *, size_t);
int timingsafe_memcmp (const void *, const void *, size_t);


void * memccpy (void *restrict, const void *restrict, int, size_t);
char *stpcpy (char *restrict, const char *restrict);
char *stpncpy (char *restrict, const char *restrict, size_t);






char *strdup (const char *);

char *_strdup_r (struct _reent *, const char *);

char *strndup (const char *, size_t);

char *_strndup_r (struct _reent *, const char *, size_t);
int strerror_r (int, char *, size_t)

             __asm__ ("" "__xpg_strerror_r")

  ;







char * _strerror_r (struct _reent *, int, int, int *);


size_t strlcat (char *, const char *, size_t);
size_t strlcpy (char *, const char *, size_t);


size_t strnlen (const char *, size_t);


char *strsep (char **, const char *);


char *strnstr(const char *, const char *, size_t) __attribute__((__pure__));



char *strlwr (char *);
char *strupr (char *);



char *strsignal (int __signo);













typedef struct
{
  int quot;
  int rem;
} div_t;

typedef struct
{
  long quot;
  long rem;
} ldiv_t;


typedef struct
{
  long long int quot;
  long long int rem;
} lldiv_t;




typedef int (*__compar_fn_t) (const void *, const void *);







int __locale_mb_cur_max (void);



void abort (void) __attribute__ ((__noreturn__));
int abs (int);

__uint32_t arc4random (void);
__uint32_t arc4random_uniform (__uint32_t);
void arc4random_buf (void *, size_t);

int atexit (void (*__func)(void));
double atof (const char *__nptr);

float atoff (const char *__nptr);

int atoi (const char *__nptr);
int _atoi_r (struct _reent *, const char *__nptr);
long atol (const char *__nptr);
long _atol_r (struct _reent *, const char *__nptr);
void * bsearch (const void *__key,
         const void *__base,
         size_t __nmemb,
         size_t __size,
         __compar_fn_t _compar);
void *calloc(size_t, size_t) __attribute__((__malloc__)) __attribute__((__warn_unused_result__))
      __attribute__((__alloc_size__(1, 2))) ;
div_t div (int __numer, int __denom);
void exit (int __status) __attribute__ ((__noreturn__));
void free (void *) ;
char * getenv (const char *__string);
char * _getenv_r (struct _reent *, const char *__string);
char * _findenv (const char *, int *);
char * _findenv_r (struct _reent *, const char *, int *);

extern char *suboptarg;
int getsubopt (char **, char * const *, char **);

long labs (long);
ldiv_t ldiv (long __numer, long __denom);
void *malloc(size_t) __attribute__((__malloc__)) __attribute__((__warn_unused_result__)) __attribute__((__alloc_size__(1))) ;
int mblen (const char *, size_t);
int _mblen_r (struct _reent *, const char *, size_t, _mbstate_t *);
int mbtowc (wchar_t *restrict, const char *restrict, size_t);
int _mbtowc_r (struct _reent *, wchar_t *restrict, const char *restrict, size_t, _mbstate_t *);
int wctomb (char *, wchar_t);
int _wctomb_r (struct _reent *, char *, wchar_t, _mbstate_t *);
size_t mbstowcs (wchar_t *restrict, const char *restrict, size_t);
size_t _mbstowcs_r (struct _reent *, wchar_t *restrict, const char *restrict, size_t, _mbstate_t *);
size_t wcstombs (char *restrict, const wchar_t *restrict, size_t);
size_t _wcstombs_r (struct _reent *, char *restrict, const wchar_t *restrict, size_t, _mbstate_t *);


char * mkdtemp (char *);






int mkstemp (char *);


int mkstemps (char *, int);


char * mktemp (char *) __attribute__ ((__deprecated__("the use of `mktemp' is dangerous; use `mkstemp' instead")));


char * _mkdtemp_r (struct _reent *, char *);
int _mkostemp_r (struct _reent *, char *, int);
int _mkostemps_r (struct _reent *, char *, int, int);
int _mkstemp_r (struct _reent *, char *);
int _mkstemps_r (struct _reent *, char *, int);
char * _mktemp_r (struct _reent *, char *) __attribute__ ((__deprecated__("the use of `mktemp' is dangerous; use `mkstemp' instead")));
void qsort (void *__base, size_t __nmemb, size_t __size, __compar_fn_t _compar);
int rand (void);
void *realloc(void *, size_t) __attribute__((__warn_unused_result__)) __attribute__((__alloc_size__(2))) ;

void *reallocarray(void *, size_t, size_t) __attribute__((__warn_unused_result__)) __attribute__((__alloc_size__(2, 3)));
void *reallocf(void *, size_t) __attribute__((__warn_unused_result__)) __attribute__((__alloc_size__(2)));


char * realpath (const char *restrict path, char *restrict resolved_path);







void srand (unsigned __seed);
double strtod (const char *restrict __n, char **restrict __end_PTR);
double _strtod_r (struct _reent *,const char *restrict __n, char **restrict __end_PTR);

float strtof (const char *restrict __n, char **restrict __end_PTR);







long strtol (const char *restrict __n, char **restrict __end_PTR, int __base);
long _strtol_r (struct _reent *,const char *restrict __n, char **restrict __end_PTR, int __base);
unsigned long strtoul (const char *restrict __n, char **restrict __end_PTR, int __base);
unsigned long _strtoul_r (struct _reent *,const char *restrict __n, char **restrict __end_PTR, int __base);
int system (const char *__string);


long a64l (const char *__input);
char * l64a (long __input);
char * _l64a_r (struct _reent *,long __input);


int on_exit (void (*__func)(int, void *),void *__arg);


void _Exit (int __status) __attribute__ ((__noreturn__));


int putenv (char *__string);

int _putenv_r (struct _reent *, char *__string);
void * _reallocf_r (struct _reent *, void *, size_t);

int setenv (const char *__string, const char *__value, int __overwrite);

int _setenv_r (struct _reent *, const char *__string, const char *__value, int __overwrite);
char * __itoa (int, char *, int);
char * __utoa (unsigned, char *, int);

char * itoa (int, char *, int);
char * utoa (unsigned, char *, int);


int rand_r (unsigned *__seed);



double drand48 (void);
double _drand48_r (struct _reent *);
double erand48 (unsigned short [3]);
double _erand48_r (struct _reent *, unsigned short [3]);
long jrand48 (unsigned short [3]);
long _jrand48_r (struct _reent *, unsigned short [3]);
void lcong48 (unsigned short [7]);
void _lcong48_r (struct _reent *, unsigned short [7]);
long lrand48 (void);
long _lrand48_r (struct _reent *);
long mrand48 (void);
long _mrand48_r (struct _reent *);
long nrand48 (unsigned short [3]);
long _nrand48_r (struct _reent *, unsigned short [3]);
unsigned short *
       seed48 (unsigned short [3]);
unsigned short *
       _seed48_r (struct _reent *, unsigned short [3]);
void srand48 (long);
void _srand48_r (struct _reent *, long);


char * initstate (unsigned, char *, size_t);
long random (void);
char * setstate (char *);
void srandom (unsigned);


long long atoll (const char *__nptr);

long long _atoll_r (struct _reent *, const char *__nptr);

long long llabs (long long);
lldiv_t lldiv (long long __numer, long long __denom);
long long strtoll (const char *restrict __n, char **restrict __end_PTR, int __base);

long long _strtoll_r (struct _reent *, const char *restrict __n, char **restrict __end_PTR, int __base);

unsigned long long strtoull (const char *restrict __n, char **restrict __end_PTR, int __base);

unsigned long long _strtoull_r (struct _reent *, const char *restrict __n, char **restrict __end_PTR, int __base);



void cfree (void *);


int unsetenv (const char *__string);

int _unsetenv_r (struct _reent *, const char *__string);



int __attribute__((__nonnull__ (1))) posix_memalign (void **, size_t, size_t);


char * _dtoa_r (struct _reent *, double, int, int, int *, int*, char**);

void * _malloc_r (struct _reent *, size_t) ;
void * _calloc_r (struct _reent *, size_t, size_t) ;
void _free_r (struct _reent *, void *) ;
void * _realloc_r (struct _reent *, void *, size_t) ;
void _mstats_r (struct _reent *, char *);

int _system_r (struct _reent *, const char *);

void __eprintf (const char *, const char *, unsigned int, const char *);
void qsort_r (void *__base, size_t __nmemb, size_t __size, void *__thunk, int (*_compar)(void *, const void *, const void *))
             __asm__ ("" "__bsd_qsort_r");
extern long double _strtold_r (struct _reent *, const char *restrict, char **restrict);

extern long double strtold (const char *restrict, char **restrict);







void * aligned_alloc(size_t, size_t) __attribute__((__malloc__)) __attribute__((__alloc_align__(1)))
     __attribute__((__alloc_size__(2)));
int at_quick_exit(void (*)(void));
_Noreturn void
 quick_exit(int);










typedef union
{
  double value;
  struct
  {






    unsigned int fraction1:32;
    unsigned int fraction0:20;

    unsigned int exponent :11;
    unsigned int sign : 1;
  } number;
  struct
  {






    unsigned int function1:32;
    unsigned int function0:19;

    unsigned int quiet:1;
    unsigned int exponent: 11;
    unsigned int sign : 1;
  } nan;
  struct
  {
    unsigned long lsw;
    unsigned long msw;
  } parts;

  long aslong[2];

} __ieee_double_shape_type;
typedef union
{
  float value;
  struct
  {
    unsigned int fraction0: 7;
    unsigned int fraction1: 16;
    unsigned int exponent: 8;
    unsigned int sign : 1;
  } number;
  struct
  {
    unsigned int function1:16;
    unsigned int function0:6;
    unsigned int quiet:1;
    unsigned int exponent:8;
    unsigned int sign:1;
  } nan;
  long p1;

} __ieee_float_shape_type;
typedef struct ieee_ext
{
  unsigned int ext_fracl : 32;
  unsigned int ext_frach : 32;
  unsigned int ext_exp : 15;
  unsigned int ext_sign : 1;
} ieee_ext;

typedef union ieee_ext_u
{
  long double extu_ld;
  struct ieee_ext extu_ext;
} ieee_ext_u;






typedef int fp_rnd;





fp_rnd fpgetround (void);
fp_rnd fpsetround (fp_rnd);



typedef int fp_except;






fp_except fpgetmask (void);
fp_except fpsetmask (fp_except);
fp_except fpgetsticky (void);
fp_except fpsetsticky (fp_except);



typedef int fp_rdi;



fp_rdi fpgetroundtoi (void);
fp_rdi fpsetroundtoi (fp_rdi);










extern double atan (double);
extern double cos (double);
extern double sin (double);
extern double tan (double);
extern double tanh (double);
extern double frexp (double, int *);
extern double modf (double, double *);
extern double ceil (double);
extern double fabs (double);
extern double floor (double);






extern double acos (double);
extern double asin (double);
extern double atan2 (double, double);
extern double cosh (double);
extern double sinh (double);
extern double exp (double);
extern double ldexp (double, int);
extern double log (double);
extern double log10 (double);
extern double pow (double, double);
extern double sqrt (double);
extern double fmod (double, double);




extern int finite (double);
extern int finitef (float);
extern int finitel (long double);
extern int isinff (float);
extern int isnanf (float);





extern int isinf (double);




extern int isnan (double);
    typedef float float_t;
    typedef double double_t;
extern int __isinff (float x);
extern int __isinfd (double x);
extern int __isnanf (float x);
extern int __isnand (double x);
extern int __fpclassifyf (float x);
extern int __fpclassifyd (double x);
extern int __signbitf (float x);
extern int __signbitd (double x);
extern double infinity (void);
extern double nan (const char *);
extern double copysign (double, double);
extern double logb (double);
extern int ilogb (double);

extern double asinh (double);
extern double cbrt (double);
extern double nextafter (double, double);
extern double rint (double);
extern double scalbn (double, int);

extern double exp2 (double);
extern double scalbln (double, long int);
extern double tgamma (double);
extern double nearbyint (double);
extern long int lrint (double);
extern long long int llrint (double);
extern double round (double);
extern long int lround (double);
extern long long int llround (double);
extern double trunc (double);
extern double remquo (double, double, int *);
extern double fdim (double, double);
extern double fmax (double, double);
extern double fmin (double, double);
extern double fma (double, double, double);


extern double log1p (double);
extern double expm1 (double);



extern double acosh (double);
extern double atanh (double);
extern double remainder (double, double);
extern double gamma (double);
extern double lgamma (double);
extern double erf (double);
extern double erfc (double);
extern double log2 (double);





extern double hypot (double, double);






extern float atanf (float);
extern float cosf (float);
extern float sinf (float);
extern float tanf (float);
extern float tanhf (float);
extern float frexpf (float, int *);
extern float modff (float, float *);
extern float ceilf (float);
extern float fabsf (float);
extern float floorf (float);


extern float acosf (float);
extern float asinf (float);
extern float atan2f (float, float);
extern float coshf (float);
extern float sinhf (float);
extern float expf (float);
extern float ldexpf (float, int);
extern float logf (float);
extern float log10f (float);
extern float powf (float, float);
extern float sqrtf (float);
extern float fmodf (float, float);




extern float exp2f (float);
extern float scalblnf (float, long int);
extern float tgammaf (float);
extern float nearbyintf (float);
extern long int lrintf (float);
extern long long int llrintf (float);
extern float roundf (float);
extern long int lroundf (float);
extern long long int llroundf (float);
extern float truncf (float);
extern float remquof (float, float, int *);
extern float fdimf (float, float);
extern float fmaxf (float, float);
extern float fminf (float, float);
extern float fmaf (float, float, float);

extern float infinityf (void);
extern float nanf (const char *);
extern float copysignf (float, float);
extern float logbf (float);
extern int ilogbf (float);

extern float asinhf (float);
extern float cbrtf (float);
extern float nextafterf (float, float);
extern float rintf (float);
extern float scalbnf (float, int);
extern float log1pf (float);
extern float expm1f (float);


extern float acoshf (float);
extern float atanhf (float);
extern float remainderf (float, float);
extern float gammaf (float);
extern float lgammaf (float);
extern float erff (float);
extern float erfcf (float);
extern float log2f (float);
extern float hypotf (float, float);
extern long double hypotl (long double, long double);
extern long double sqrtl (long double);
extern double drem (double, double);
extern float dremf (float, float);



extern double gamma_r (double, int *);
extern double lgamma_r (double, int *);
extern float gammaf_r (float, int *);
extern float lgammaf_r (float, int *);



extern double y0 (double);
extern double y1 (double);
extern double yn (int, double);
extern double j0 (double);
extern double j1 (double);
extern double jn (int, double);



extern float y0f (float);
extern float y1f (float);
extern float ynf (int, float);
extern float j0f (float);
extern float j1f (float);
extern float jnf (int, float);
extern int *__signgam (void);
struct exception

{
  int type;
  char *name;
  double arg1;
  double arg2;
  double retval;
  int err;
};




extern int matherr (struct exception *e);
enum __fdlibm_version
{
  __fdlibm_ieee = -1,
  __fdlibm_svid,
  __fdlibm_xopen,
  __fdlibm_posix
};




extern enum __fdlibm_version __fdlib_version;






typedef int error_t;



extern int *__errno (void);




extern const char * const _sys_errlist[];
extern int _sys_nerr;

typedef __uint8_t u_int8_t;


typedef __uint16_t u_int16_t;


typedef __uint32_t u_int32_t;


typedef __uint64_t u_int64_t;

typedef int register_t;

typedef __int8_t int8_t ;



typedef __uint8_t uint8_t ;







typedef __int16_t int16_t ;



typedef __uint16_t uint16_t ;







typedef __int32_t int32_t ;



typedef __uint32_t uint32_t ;







typedef __int64_t int64_t ;



typedef __uint64_t uint64_t ;






typedef __intmax_t intmax_t;




typedef __uintmax_t uintmax_t;




typedef __intptr_t intptr_t;




typedef __uintptr_t uintptr_t;







typedef unsigned long __sigset_t;
typedef __suseconds_t suseconds_t;




typedef long time_t;
struct timeval {
 time_t tv_sec;
 suseconds_t tv_usec;
};
struct timespec {
 time_t tv_sec;
 long tv_nsec;
};
struct itimerspec {
 struct timespec it_interval;
 struct timespec it_value;
};



typedef __sigset_t sigset_t;
typedef unsigned long fd_mask;







typedef struct _types_fd_set {
 fd_mask fds_bits[(((64)+(((sizeof (fd_mask) * 8))-1))/((sizeof (fd_mask) * 8)))];
} _types_fd_set;


int select (int __n, _types_fd_set *__readfds, _types_fd_set *__writefds, _types_fd_set *__exceptfds, struct timeval *__timeout)
                                                   ;

int pselect (int __n, _types_fd_set *__readfds, _types_fd_set *__writefds, _types_fd_set *__exceptfds, const struct timespec *__timeout, const sigset_t *__set)

                           ;







typedef __uint32_t in_addr_t;




typedef __uint16_t in_port_t;
typedef unsigned char u_char;



typedef unsigned short u_short;



typedef unsigned int u_int;



typedef unsigned long u_long;







typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;



typedef __blkcnt_t blkcnt_t;




typedef __blksize_t blksize_t;




typedef unsigned long clock_t;
typedef long daddr_t;



typedef char * caddr_t;




typedef __fsblkcnt_t fsblkcnt_t;
typedef __fsfilcnt_t fsfilcnt_t;




typedef __id_t id_t;




typedef __ino_t ino_t;
typedef __off_t off_t;



typedef __dev_t dev_t;



typedef __uid_t uid_t;



typedef __gid_t gid_t;




typedef __pid_t pid_t;




typedef __key_t key_t;




typedef _ssize_t ssize_t;




typedef __mode_t mode_t;




typedef __nlink_t nlink_t;




typedef __clockid_t clockid_t;





typedef __timer_t timer_t;





typedef __useconds_t useconds_t;
typedef __int64_t sbintime_t;


struct sched_param {
  int sched_priority;
};
typedef __uint32_t pthread_t;
typedef struct {
  int is_initialized;
  void *stackaddr;
  int stacksize;
  int contentionscope;
  int inheritsched;
  int schedpolicy;
  struct sched_param schedparam;





  int detachstate;
} pthread_attr_t;
typedef __uint32_t pthread_mutex_t;

typedef struct {
  int is_initialized;
  int recursive;
} pthread_mutexattr_t;






typedef __uint32_t pthread_cond_t;



typedef struct {
  int is_initialized;
  clock_t clock;



} pthread_condattr_t;



typedef __uint32_t pthread_key_t;

typedef struct {
  int is_initialized;
  int init_executed;
} pthread_once_t;







typedef __builtin_va_list __gnuc_va_list;





typedef __gnuc_va_list va_list;





typedef __FILE FILE;






struct tm;



typedef _mbstate_t mbstate_t;


wint_t btowc (int);
int wctob (wint_t);
size_t mbrlen (const char *restrict, size_t, mbstate_t *restrict);
size_t mbrtowc (wchar_t *restrict, const char *restrict, size_t,
      mbstate_t *restrict);
size_t _mbrtowc_r (struct _reent *, wchar_t * , const char * ,
   size_t, mbstate_t *);
int mbsinit (const mbstate_t *);

size_t mbsnrtowcs (wchar_t *restrict, const char **restrict,
    size_t, size_t, mbstate_t *restrict);

size_t _mbsnrtowcs_r (struct _reent *, wchar_t * , const char ** ,
   size_t, size_t, mbstate_t *);
size_t mbsrtowcs (wchar_t *restrict, const char **restrict, size_t,
    mbstate_t *restrict);
size_t _mbsrtowcs_r (struct _reent *, wchar_t * , const char ** , size_t, mbstate_t *);
size_t wcrtomb (char *restrict, wchar_t, mbstate_t *restrict);
size_t _wcrtomb_r (struct _reent *, char * , wchar_t, mbstate_t *);

size_t wcsnrtombs (char *restrict, const wchar_t **restrict,
    size_t, size_t, mbstate_t *restrict);

size_t _wcsnrtombs_r (struct _reent *, char * , const wchar_t ** ,
   size_t, size_t, mbstate_t *);
size_t wcsrtombs (char *restrict, const wchar_t **restrict,
    size_t, mbstate_t *restrict);
size_t _wcsrtombs_r (struct _reent *, char * , const wchar_t ** ,
   size_t, mbstate_t *);

int wcscasecmp (const wchar_t *, const wchar_t *);

wchar_t *wcscat (wchar_t *restrict, const wchar_t *restrict);
wchar_t *wcschr (const wchar_t *, wchar_t);
int wcscmp (const wchar_t *, const wchar_t *);
int wcscoll (const wchar_t *, const wchar_t *);
wchar_t *wcscpy (wchar_t *restrict, const wchar_t *restrict);

wchar_t *wcpcpy (wchar_t *restrict,
     const wchar_t *restrict);
wchar_t *wcsdup (const wchar_t *);

wchar_t *_wcsdup_r (struct _reent *, const wchar_t * );
size_t wcscspn (const wchar_t *, const wchar_t *);
size_t wcsftime (wchar_t *restrict, size_t,
    const wchar_t *restrict, const struct tm *restrict);




size_t wcslcat (wchar_t *, const wchar_t *, size_t);
size_t wcslcpy (wchar_t *, const wchar_t *, size_t);
size_t wcslen (const wchar_t *);

int wcsncasecmp (const wchar_t *, const wchar_t *, size_t);

wchar_t *wcsncat (wchar_t *restrict,
     const wchar_t *restrict, size_t);
int wcsncmp (const wchar_t *, const wchar_t *, size_t);
wchar_t *wcsncpy (wchar_t *restrict,
     const wchar_t *restrict, size_t);

wchar_t *wcpncpy (wchar_t *restrict,
     const wchar_t *restrict, size_t);
size_t wcsnlen (const wchar_t *, size_t);

wchar_t *wcspbrk (const wchar_t *, const wchar_t *);
wchar_t *wcsrchr (const wchar_t *, wchar_t);
size_t wcsspn (const wchar_t *, const wchar_t *);
wchar_t *wcsstr (const wchar_t *restrict,
     const wchar_t *restrict);
wchar_t *wcstok (wchar_t *restrict, const wchar_t *restrict,
     wchar_t **restrict);
double wcstod (const wchar_t *restrict, wchar_t **restrict);
double _wcstod_r (struct _reent *, const wchar_t *, wchar_t **);

float wcstof (const wchar_t *restrict, wchar_t **restrict);

float _wcstof_r (struct _reent *, const wchar_t *, wchar_t **);



size_t wcsxfrm (wchar_t *restrict, const wchar_t *restrict,
    size_t);

extern int wcscasecmp_l (const wchar_t *, const wchar_t *, locale_t);
extern int wcsncasecmp_l (const wchar_t *, const wchar_t *, size_t, locale_t);
extern int wcscoll_l (const wchar_t *, const wchar_t *, locale_t);
extern size_t wcsxfrm_l (wchar_t *restrict, const wchar_t *restrict, size_t,
    locale_t);





wchar_t *wmemchr (const wchar_t *, wchar_t, size_t);
int wmemcmp (const wchar_t *, const wchar_t *, size_t);
wchar_t *wmemcpy (wchar_t *restrict, const wchar_t *restrict,
     size_t);
wchar_t *wmemmove (wchar_t *, const wchar_t *, size_t);




wchar_t *wmemset (wchar_t *, wchar_t, size_t);

long wcstol (const wchar_t *restrict, wchar_t **restrict, int);

long long wcstoll (const wchar_t *restrict, wchar_t **restrict,
      int);

unsigned long wcstoul (const wchar_t *restrict, wchar_t **restrict,
       int);

unsigned long long wcstoull (const wchar_t *restrict,
         wchar_t **restrict, int);

long _wcstol_r (struct _reent *, const wchar_t *, wchar_t **, int);
long long _wcstoll_r (struct _reent *, const wchar_t *, wchar_t **, int);
unsigned long _wcstoul_r (struct _reent *, const wchar_t *, wchar_t **, int);
unsigned long long _wcstoull_r (struct _reent *, const wchar_t *, wchar_t **, int);

long double wcstold (const wchar_t *, wchar_t **);
wint_t fgetwc (__FILE *);
wchar_t *fgetws (wchar_t *restrict, int, __FILE *restrict);
wint_t fputwc (wchar_t, __FILE *);
int fputws (const wchar_t *restrict, __FILE *restrict);

int fwide (__FILE *, int);

wint_t getwc (__FILE *);
wint_t getwchar (void);
wint_t putwc (wchar_t, __FILE *);
wint_t putwchar (wchar_t);
wint_t ungetwc (wint_t wc, __FILE *);

wint_t _fgetwc_r (struct _reent *, __FILE *);
wint_t _fgetwc_unlocked_r (struct _reent *, __FILE *);
wchar_t *_fgetws_r (struct _reent *, wchar_t *, int, __FILE *);
wchar_t *_fgetws_unlocked_r (struct _reent *, wchar_t *, int, __FILE *);
wint_t _fputwc_r (struct _reent *, wchar_t, __FILE *);
wint_t _fputwc_unlocked_r (struct _reent *, wchar_t, __FILE *);
int _fputws_r (struct _reent *, const wchar_t *, __FILE *);
int _fputws_unlocked_r (struct _reent *, const wchar_t *, __FILE *);
int _fwide_r (struct _reent *, __FILE *, int);
wint_t _getwc_r (struct _reent *, __FILE *);
wint_t _getwc_unlocked_r (struct _reent *, __FILE *);
wint_t _getwchar_r (struct _reent *ptr);
wint_t _getwchar_unlocked_r (struct _reent *ptr);
wint_t _putwc_r (struct _reent *, wchar_t, __FILE *);
wint_t _putwc_unlocked_r (struct _reent *, wchar_t, __FILE *);
wint_t _putwchar_r (struct _reent *, wchar_t);
wint_t _putwchar_unlocked_r (struct _reent *, wchar_t);
wint_t _ungetwc_r (struct _reent *, wint_t wc, __FILE *);
__FILE *open_wmemstream (wchar_t **, size_t *);

__FILE *_open_wmemstream_r (struct _reent *, wchar_t **, size_t *);
int fwprintf (__FILE *restrict, const wchar_t *restrict, ...);
int swprintf (wchar_t *restrict, size_t,
   const wchar_t *restrict, ...);
int vfwprintf (__FILE *restrict, const wchar_t *restrict,
   __gnuc_va_list);
int vswprintf (wchar_t *restrict, size_t,
   const wchar_t *restrict, __gnuc_va_list);
int vwprintf (const wchar_t *restrict, __gnuc_va_list);
int wprintf (const wchar_t *restrict, ...);


int _fwprintf_r (struct _reent *, __FILE *, const wchar_t *, ...);
int _swprintf_r (struct _reent *, wchar_t *, size_t, const wchar_t *, ...);
int _vfwprintf_r (struct _reent *, __FILE *, const wchar_t *, __gnuc_va_list);
int _vswprintf_r (struct _reent *, wchar_t *, size_t, const wchar_t *, __gnuc_va_list);
int _vwprintf_r (struct _reent *, const wchar_t *, __gnuc_va_list);
int _wprintf_r (struct _reent *, const wchar_t *, ...);


int fwscanf (__FILE *restrict, const wchar_t *restrict, ...);
int swscanf (const wchar_t *restrict,
   const wchar_t *restrict, ...);
int vfwscanf (__FILE *restrict, const wchar_t *restrict,
   __gnuc_va_list);
int vswscanf (const wchar_t *restrict, const wchar_t *restrict,
   __gnuc_va_list);
int vwscanf (const wchar_t *restrict, __gnuc_va_list);
int wscanf (const wchar_t *restrict, ...);


int _fwscanf_r (struct _reent *, __FILE *, const wchar_t *, ...);
int _swscanf_r (struct _reent *, const wchar_t *, const wchar_t *, ...);
int _vfwscanf_r (struct _reent *, __FILE *, const wchar_t *, __gnuc_va_list);
int _vswscanf_r (struct _reent *, const wchar_t *, const wchar_t *, __gnuc_va_list);
int _vwscanf_r (struct _reent *, const wchar_t *, __gnuc_va_list);
int _wscanf_r (struct _reent *, const wchar_t *, ...);



struct lconv
{
  char *decimal_point;
  char *thousands_sep;
  char *grouping;
  char *int_curr_symbol;
  char *currency_symbol;
  char *mon_decimal_point;
  char *mon_thousands_sep;
  char *mon_grouping;
  char *positive_sign;
  char *negative_sign;
  char int_frac_digits;
  char frac_digits;
  char p_cs_precedes;
  char p_sep_by_space;
  char n_cs_precedes;
  char n_sep_by_space;
  char p_sign_posn;
  char n_sign_posn;
  char int_n_cs_precedes;
  char int_n_sep_by_space;
  char int_n_sign_posn;
  char int_p_cs_precedes;
  char int_p_sep_by_space;
  char int_p_sign_posn;
};

struct _reent;
char *_setlocale_r (struct _reent *, int, const char *);
struct lconv *_localeconv_r (struct _reent *);

struct __locale_t *_newlocale_r (struct _reent *, int, const char *,
     struct __locale_t *);
void _freelocale_r (struct _reent *, struct __locale_t *);
struct __locale_t *_duplocale_r (struct _reent *, struct __locale_t *);
struct __locale_t *_uselocale_r (struct _reent *, struct __locale_t *);



char *setlocale (int, const char *);
struct lconv *localeconv (void);


locale_t newlocale (int, const char *, locale_t);
void freelocale (locale_t);
locale_t duplocale (locale_t);
locale_t uselocale (locale_t);








struct lc_ctype_T
{
  const char *codeset;
  const char *mb_cur_max;




};
extern const struct lc_ctype_T _C_ctype_locale;

struct lc_monetary_T
{
  const char *int_curr_symbol;
  const char *currency_symbol;
  const char *mon_decimal_point;
  const char *mon_thousands_sep;
  const char *mon_grouping;
  const char *positive_sign;
  const char *negative_sign;
  const char *int_frac_digits;
  const char *frac_digits;
  const char *p_cs_precedes;
  const char *p_sep_by_space;
  const char *n_cs_precedes;
  const char *n_sep_by_space;
  const char *p_sign_posn;
  const char *n_sign_posn;
};
extern const struct lc_monetary_T _C_monetary_locale;

struct lc_numeric_T
{
  const char *decimal_point;
  const char *thousands_sep;
  const char *grouping;





};
extern const struct lc_numeric_T _C_numeric_locale;

struct lc_time_T
{
  const char *mon[12];
  const char *month[12];
  const char *wday[7];
  const char *weekday[7];
  const char *X_fmt;
  const char *x_fmt;
  const char *c_fmt;
  const char *am_pm[2];
  const char *date_fmt;
  const char *alt_month[12];
  const char *md_order;
  const char *ampm_fmt;
  const char *era;
  const char *era_d_fmt;
  const char *era_d_t_fmt;
  const char *era_t_fmt;
  const char *alt_digits;
};
extern const struct lc_time_T _C_time_locale;

struct lc_messages_T
{
  const char *yesexpr;
  const char *noexpr;
  const char *yesstr;
  const char *nostr;







};
extern const struct lc_messages_T _C_messages_locale;

struct __lc_cats
{
  const void *ptr;
  char *buf;
};

struct __locale_t
{
  char categories[7][31 + 1];
  int (*wctomb) (struct _reent *, char *, wchar_t,
       mbstate_t *);
  int (*mbtowc) (struct _reent *, wchar_t *,
       const char *, size_t, mbstate_t *);
  int cjk_lang;
  char *ctype_ptr;
  struct lconv lconv;

  char mb_cur_max[2];
  char ctype_codeset[31 + 1];
  char message_codeset[31 + 1];



};






extern struct lconv *__localeconv_l (struct __locale_t *locale);

extern size_t _wcsnrtombs_l (struct _reent *, char *, const wchar_t **,
        size_t, size_t, mbstate_t *, struct __locale_t *);




static __inline__ 
                struct __locale_t *
__get_global_locale ()
{
  extern struct __locale_t __global_locale;
  return &__global_locale;
}



static __inline__ 
                struct __locale_t *
__get_locale_r (struct _reent *r)
{



  return __get_global_locale();

}






static __inline__ 
                struct __locale_t *
__get_current_locale (void)
{



  return __get_global_locale();

}




static __inline__ 
                struct __locale_t *
__get_C_locale (void)
{

  return __get_global_locale ();




}

static __inline__ 
                int
__locale_mb_cur_max_l (struct __locale_t *locale)
{



  return locale->mb_cur_max[0];

}

static __inline__ 
                const struct lc_monetary_T *
__get_monetary_locale (struct __locale_t *locale)
{
  return &_C_monetary_locale;
}


static __inline__ 
                const struct lc_monetary_T *
__get_current_monetary_locale (void)
{
  return &_C_monetary_locale;
}


static __inline__ 
                const struct lc_numeric_T *
__get_numeric_locale (struct __locale_t *locale)
{
  return &_C_numeric_locale;
}


static __inline__ 
                const struct lc_numeric_T *
__get_current_numeric_locale (void)
{
  return &_C_numeric_locale;
}


static __inline__ 
                const struct lc_time_T *
__get_time_locale (struct __locale_t *locale)
{
  return &_C_time_locale;
}


static __inline__ 
                const struct lc_time_T *
__get_current_time_locale (void)
{
  return &_C_time_locale;
}


static __inline__ 
                const struct lc_messages_T *
__get_messages_locale (struct __locale_t *locale)
{
  return &_C_messages_locale;
}


static __inline__ 
                const struct lc_messages_T *
__get_current_messages_locale (void)
{
  return &_C_messages_locale;
}



static __inline__ 
                const char *
__locale_charset (struct __locale_t *locale)
{



  return locale->ctype_codeset;

}


static __inline__ 
                const char *
__current_locale_charset (void)
{



  return __get_current_locale ()->ctype_codeset;

}


static __inline__ 
                const char *
__locale_msgcharset (void)
{



  return (char *) __get_current_locale ()->message_codeset;

}


static __inline__ 
                int
__locale_cjk_lang (void)
{
  return __get_current_locale ()->cjk_lang;
}

int __ctype_load_locale (struct __locale_t *, const char *, void *,
    const char *, int);
int __monetary_load_locale (struct __locale_t *, const char *, void *,
       const char *);
int __numeric_load_locale (struct __locale_t *, const char *, void *,
      const char *);
int __time_load_locale (struct __locale_t *, const char *, void *,
   const char *);
int __messages_load_locale (struct __locale_t *, const char *, void *,
       const char *);

union double_union
{
  double d;
  __uint32_t i[2];
};
typedef __int32_t Long;




typedef union { double d; __ULong i[2]; } U;
typedef struct _Bigint _Bigint;
struct _reent ;
struct FPI;
double __ulp (double x);
double __b2d (_Bigint *a , int *e);
_Bigint * _Balloc (struct _reent *p, int k);
void _Bfree (struct _reent *p, _Bigint *v);
_Bigint * __multadd (struct _reent *p, _Bigint *, int, int);
_Bigint * __s2b (struct _reent *, const char*, int, int, __ULong);
_Bigint * __i2b (struct _reent *,int);
_Bigint * __multiply (struct _reent *, _Bigint *, _Bigint *);
_Bigint * __pow5mult (struct _reent *, _Bigint *, int k);
int __hi0bits (__ULong);
int __lo0bits (__ULong *);
_Bigint * __d2b (struct _reent *p, double d, int *e, int *bits);
_Bigint * __lshift (struct _reent *p, _Bigint *b, int k);
int __match (const char**, char*);
_Bigint * __mdiff (struct _reent *p, _Bigint *a, _Bigint *b);
int __mcmp (_Bigint *a, _Bigint *b);
int __gethex (struct _reent *p, const char **sp, const struct FPI *fpi, Long *exp, _Bigint **bp, int sign, locale_t loc);
double __ratio (_Bigint *a, _Bigint *b);
__ULong __any_on (_Bigint *b, int k);
void __copybits (__ULong *c, int n, _Bigint *b);
double _strtod_l (struct _reent *ptr, const char *
                                                  restrict 
                                                             s00,
      char **
            restrict 
                       se, locale_t loc);

int _strtorx_l (struct _reent *, const char *, char **, int,
       void *, locale_t);
int _strtodg_l (struct _reent *p, const char *s00, char **se,
       struct FPI *fpi, Long *exp, __ULong *bits,
       locale_t);






int __hexnan (const char **sp, const struct FPI *fpi, __ULong *x0);




extern const double __mprec_tinytens[];
extern const double __mprec_bigtens[];
extern const double __mprec_tens[];

extern const unsigned char __hexdig[];



double _mprec_log10 (int);



long double _strtold (char *, char **);
char *_ldtoa_r (struct _reent *, long double, int, int, int *, int *,
  char **);
int _ldcheck (long double *);
typedef struct
{
  int rlast;
  int rndprc;
  int rw;
  int re;
  int outexpon;
  unsigned short rmsk;
  unsigned short rmbit;
  unsigned short rebit;
  unsigned short rbit[(10 +3)];
  unsigned short equot[(10 +3)];
} LDPARMS;

static void esub (const short unsigned int *a, const short unsigned int *b,
    short unsigned int *c, LDPARMS * ldp);
static void emul (const short unsigned int *a, const short unsigned int *b,
    short unsigned int *c, LDPARMS * ldp);
static void ediv (const short unsigned int *a, const short unsigned int *b,
    short unsigned int *c, LDPARMS * ldp);
static int ecmp (const short unsigned int *a, const short unsigned int *b);
static int enormlz (short unsigned int *x);
static int eshift (short unsigned int *x, int sc);
static void eshup1 (register short unsigned int *x);
static void eshup8 (register short unsigned int *x);
static void eshup6 (register short unsigned int *x);
static void eshdn1 (register short unsigned int *x);
static void eshdn8 (register short unsigned int *x);
static void eshdn6 (register short unsigned int *x);
static void eneg (short unsigned int *x);
static void emov (register const short unsigned int *a,
    register short unsigned int *b);
static void eclear (register short unsigned int *x);
static void einfin (register short unsigned int *x, register LDPARMS * ldp);
static void efloor (short unsigned int *x, short unsigned int *y,
      LDPARMS * ldp);
static void etoasc (short unsigned int *x, char *string, int ndigs,
      int outformat, LDPARMS * ldp);

union uconv
{
  unsigned short pe;
  long double d;
};
static void e64toe (short unsigned int *pe, short unsigned int *y,
      LDPARMS * ldp);
static const unsigned short ezero[10] = { 0x0000, 0x0000, 0x0000, 0x0000,
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
};


static const unsigned short eone[10] = { 0x0000, 0x0000, 0x0000, 0x0000,
  0x0000, 0x0000, 0x0000, 0x0000, 0x8000, 0x3fff,
};





typedef _fpos_t fpos_t;








typedef int _LOCK_T;
typedef int _LOCK_RECURSIVE_T;
char * ctermid (char *);




FILE * tmpfile (void);
char * tmpnam (char *);

char * tempnam (const char *, const char *);

int fclose (FILE *);
int fflush (FILE *);
FILE * freopen (const char *restrict, const char *restrict, FILE *restrict);
void setbuf (FILE *restrict, char *restrict);
int setvbuf (FILE *restrict, char *restrict, int, size_t);
int fprintf (FILE *restrict, const char *restrict, ...)
               __attribute__ ((__format__ (__printf__, 2, 3)));
int fscanf (FILE *restrict, const char *restrict, ...)
               __attribute__ ((__format__ (__scanf__, 2, 3)));
int printf (const char *restrict, ...)
               __attribute__ ((__format__ (__printf__, 1, 2)));
int scanf (const char *restrict, ...)
               __attribute__ ((__format__ (__scanf__, 1, 2)));
int sscanf (const char *restrict, const char *restrict, ...)
               __attribute__ ((__format__ (__scanf__, 2, 3)));
int vfprintf (FILE *restrict, const char *restrict, __gnuc_va_list)
               __attribute__ ((__format__ (__printf__, 2, 0)));
int vprintf (const char *, __gnuc_va_list)
               __attribute__ ((__format__ (__printf__, 1, 0)));
int vsprintf (char *restrict, const char *restrict, __gnuc_va_list)
               __attribute__ ((__format__ (__printf__, 2, 0)));
int fgetc (FILE *);
char * fgets (char *restrict, int, FILE *restrict);
int fputc (int, FILE *);
int fputs (const char *restrict, FILE *restrict);
int getc (FILE *);
int getchar (void);
char * gets (char *);
int putc (int, FILE *);
int putchar (int);
int puts (const char *);
int ungetc (int, FILE *);
size_t fread (void *restrict, size_t _size, size_t _n, FILE *restrict);
size_t fwrite (const void *restrict , size_t _size, size_t _n, FILE *);



int fgetpos (FILE *restrict, fpos_t *restrict);

int fseek (FILE *, long, int);



int fsetpos (FILE *, const fpos_t *);

long ftell ( FILE *);
void rewind (FILE *);
void clearerr (FILE *);
int feof (FILE *);
int ferror (FILE *);
void perror (const char *);

FILE * fopen (const char *restrict _name, const char *restrict _type);
int sprintf (char *restrict, const char *restrict, ...)
               __attribute__ ((__format__ (__printf__, 2, 3)));
int remove (const char *);
int rename (const char *, const char *);
int fseeko (FILE *, off_t, int);
off_t ftello (FILE *);







int snprintf (char *restrict, size_t, const char *restrict, ...)
               __attribute__ ((__format__ (__printf__, 3, 4)));
int vsnprintf (char *restrict, size_t, const char *restrict, __gnuc_va_list)
               __attribute__ ((__format__ (__printf__, 3, 0)));
int vfscanf (FILE *restrict, const char *restrict, __gnuc_va_list)
               __attribute__ ((__format__ (__scanf__, 2, 0)));
int vscanf (const char *, __gnuc_va_list)
               __attribute__ ((__format__ (__scanf__, 1, 0)));
int vsscanf (const char *restrict, const char *restrict, __gnuc_va_list)
               __attribute__ ((__format__ (__scanf__, 2, 0)));
int asiprintf (char **, const char *, ...)
               __attribute__ ((__format__ (__printf__, 2, 3)));
char * asniprintf (char *, size_t *, const char *, ...)
               __attribute__ ((__format__ (__printf__, 3, 4)));
char * asnprintf (char *restrict, size_t *restrict, const char *restrict, ...)
               __attribute__ ((__format__ (__printf__, 3, 4)));

int diprintf (int, const char *, ...)
               __attribute__ ((__format__ (__printf__, 2, 3)));

int fiprintf (FILE *, const char *, ...)
               __attribute__ ((__format__ (__printf__, 2, 3)));
int fiscanf (FILE *, const char *, ...)
               __attribute__ ((__format__ (__scanf__, 2, 3)));
int iprintf (const char *, ...)
               __attribute__ ((__format__ (__printf__, 1, 2)));
int iscanf (const char *, ...)
               __attribute__ ((__format__ (__scanf__, 1, 2)));
int siprintf (char *, const char *, ...)
               __attribute__ ((__format__ (__printf__, 2, 3)));
int siscanf (const char *, const char *, ...)
               __attribute__ ((__format__ (__scanf__, 2, 3)));
int sniprintf (char *, size_t, const char *, ...)
               __attribute__ ((__format__ (__printf__, 3, 4)));
int vasiprintf (char **, const char *, __gnuc_va_list)
               __attribute__ ((__format__ (__printf__, 2, 0)));
char * vasniprintf (char *, size_t *, const char *, __gnuc_va_list)
               __attribute__ ((__format__ (__printf__, 3, 0)));
char * vasnprintf (char *, size_t *, const char *, __gnuc_va_list)
               __attribute__ ((__format__ (__printf__, 3, 0)));
int vdiprintf (int, const char *, __gnuc_va_list)
               __attribute__ ((__format__ (__printf__, 2, 0)));
int vfiprintf (FILE *, const char *, __gnuc_va_list)
               __attribute__ ((__format__ (__printf__, 2, 0)));
int vfiscanf (FILE *, const char *, __gnuc_va_list)
               __attribute__ ((__format__ (__scanf__, 2, 0)));
int viprintf (const char *, __gnuc_va_list)
               __attribute__ ((__format__ (__printf__, 1, 0)));
int viscanf (const char *, __gnuc_va_list)
               __attribute__ ((__format__ (__scanf__, 1, 0)));
int vsiprintf (char *, const char *, __gnuc_va_list)
               __attribute__ ((__format__ (__printf__, 2, 0)));
int vsiscanf (const char *, const char *, __gnuc_va_list)
               __attribute__ ((__format__ (__scanf__, 2, 0)));
int vsniprintf (char *, size_t, const char *, __gnuc_va_list)
               __attribute__ ((__format__ (__printf__, 3, 0)));
FILE * fdopen (int, const char *);

int fileno (FILE *);


int pclose (FILE *);
FILE * popen (const char *, const char *);



void setbuffer (FILE *, char *, int);
int setlinebuf (FILE *);



int getw (FILE *);
int putw (int, FILE *);


int getc_unlocked (FILE *);
int getchar_unlocked (void);
void flockfile (FILE *);
int ftrylockfile (FILE *);
void funlockfile (FILE *);
int putc_unlocked (int, FILE *);
int putchar_unlocked (int);
int dprintf (int, const char *restrict, ...)
               __attribute__ ((__format__ (__printf__, 2, 3)));

FILE * fmemopen (void *restrict, size_t, const char *restrict);


FILE * open_memstream (char **, size_t *);
int vdprintf (int, const char *restrict, __gnuc_va_list)
               __attribute__ ((__format__ (__printf__, 2, 0)));



int renameat (int, const char *, int, const char *);
int _asiprintf_r (struct _reent *, char **, const char *, ...)
               __attribute__ ((__format__ (__printf__, 3, 4)));
char * _asniprintf_r (struct _reent *, char *, size_t *, const char *, ...)
               __attribute__ ((__format__ (__printf__, 4, 5)));
char * _asnprintf_r (struct _reent *, char *restrict, size_t *restrict, const char *restrict, ...)
               __attribute__ ((__format__ (__printf__, 4, 5)));
int _asprintf_r (struct _reent *, char **restrict, const char *restrict, ...)
               __attribute__ ((__format__ (__printf__, 3, 4)));
int _diprintf_r (struct _reent *, int, const char *, ...)
               __attribute__ ((__format__ (__printf__, 3, 4)));
int _dprintf_r (struct _reent *, int, const char *restrict, ...)
               __attribute__ ((__format__ (__printf__, 3, 4)));
int _fclose_r (struct _reent *, FILE *);
int _fcloseall_r (struct _reent *);
FILE * _fdopen_r (struct _reent *, int, const char *);
int _fflush_r (struct _reent *, FILE *);
int _fgetc_r (struct _reent *, FILE *);
int _fgetc_unlocked_r (struct _reent *, FILE *);
char * _fgets_r (struct _reent *, char *restrict, int, FILE *restrict);
char * _fgets_unlocked_r (struct _reent *, char *restrict, int, FILE *restrict);




int _fgetpos_r (struct _reent *, FILE *, fpos_t *);
int _fsetpos_r (struct _reent *, FILE *, const fpos_t *);

int _fiprintf_r (struct _reent *, FILE *, const char *, ...)
               __attribute__ ((__format__ (__printf__, 3, 4)));
int _fiscanf_r (struct _reent *, FILE *, const char *, ...)
               __attribute__ ((__format__ (__scanf__, 3, 4)));
FILE * _fmemopen_r (struct _reent *, void *restrict, size_t, const char *restrict);
FILE * _fopen_r (struct _reent *, const char *restrict, const char *restrict);
FILE * _freopen_r (struct _reent *, const char *restrict, const char *restrict, FILE *restrict);
int _fprintf_r (struct _reent *, FILE *restrict, const char *restrict, ...)
               __attribute__ ((__format__ (__printf__, 3, 4)));
int _fpurge_r (struct _reent *, FILE *);
int _fputc_r (struct _reent *, int, FILE *);
int _fputc_unlocked_r (struct _reent *, int, FILE *);
int _fputs_r (struct _reent *, const char *restrict, FILE *restrict);
int _fputs_unlocked_r (struct _reent *, const char *restrict, FILE *restrict);
size_t _fread_r (struct _reent *, void *restrict, size_t _size, size_t _n, FILE *restrict);
size_t _fread_unlocked_r (struct _reent *, void *restrict, size_t _size, size_t _n, FILE *restrict);
int _fscanf_r (struct _reent *, FILE *restrict, const char *restrict, ...)
               __attribute__ ((__format__ (__scanf__, 3, 4)));
int _fseek_r (struct _reent *, FILE *, long, int);
int _fseeko_r (struct _reent *, FILE *, _off_t, int);
long _ftell_r (struct _reent *, FILE *);
_off_t _ftello_r (struct _reent *, FILE *);
void _rewind_r (struct _reent *, FILE *);
size_t _fwrite_r (struct _reent *, const void *restrict, size_t _size, size_t _n, FILE *restrict);
size_t _fwrite_unlocked_r (struct _reent *, const void *restrict, size_t _size, size_t _n, FILE *restrict);
int _getc_r (struct _reent *, FILE *);
int _getc_unlocked_r (struct _reent *, FILE *);
int _getchar_r (struct _reent *);
int _getchar_unlocked_r (struct _reent *);
char * _gets_r (struct _reent *, char *);
int _iprintf_r (struct _reent *, const char *, ...)
               __attribute__ ((__format__ (__printf__, 2, 3)));
int _iscanf_r (struct _reent *, const char *, ...)
               __attribute__ ((__format__ (__scanf__, 2, 3)));
FILE * _open_memstream_r (struct _reent *, char **, size_t *);
void _perror_r (struct _reent *, const char *);
int _printf_r (struct _reent *, const char *restrict, ...)
               __attribute__ ((__format__ (__printf__, 2, 3)));
int _putc_r (struct _reent *, int, FILE *);
int _putc_unlocked_r (struct _reent *, int, FILE *);
int _putchar_unlocked_r (struct _reent *, int);
int _putchar_r (struct _reent *, int);
int _puts_r (struct _reent *, const char *);
int _remove_r (struct _reent *, const char *);
int _rename_r (struct _reent *,
      const char *_old, const char *_new);
int _scanf_r (struct _reent *, const char *restrict, ...)
               __attribute__ ((__format__ (__scanf__, 2, 3)));
int _siprintf_r (struct _reent *, char *, const char *, ...)
               __attribute__ ((__format__ (__printf__, 3, 4)));
int _siscanf_r (struct _reent *, const char *, const char *, ...)
               __attribute__ ((__format__ (__scanf__, 3, 4)));
int _sniprintf_r (struct _reent *, char *, size_t, const char *, ...)
               __attribute__ ((__format__ (__printf__, 4, 5)));
int _snprintf_r (struct _reent *, char *restrict, size_t, const char *restrict, ...)
               __attribute__ ((__format__ (__printf__, 4, 5)));
int _sprintf_r (struct _reent *, char *restrict, const char *restrict, ...)
               __attribute__ ((__format__ (__printf__, 3, 4)));
int _sscanf_r (struct _reent *, const char *restrict, const char *restrict, ...)
               __attribute__ ((__format__ (__scanf__, 3, 4)));
char * _tempnam_r (struct _reent *, const char *, const char *);
FILE * _tmpfile_r (struct _reent *);
char * _tmpnam_r (struct _reent *, char *);
int _ungetc_r (struct _reent *, int, FILE *);
int _vasiprintf_r (struct _reent *, char **, const char *, __gnuc_va_list)
               __attribute__ ((__format__ (__printf__, 3, 0)));
char * _vasniprintf_r (struct _reent*, char *, size_t *, const char *, __gnuc_va_list)
               __attribute__ ((__format__ (__printf__, 4, 0)));
char * _vasnprintf_r (struct _reent*, char *, size_t *, const char *, __gnuc_va_list)
               __attribute__ ((__format__ (__printf__, 4, 0)));
int _vasprintf_r (struct _reent *, char **, const char *, __gnuc_va_list)
               __attribute__ ((__format__ (__printf__, 3, 0)));
int _vdiprintf_r (struct _reent *, int, const char *, __gnuc_va_list)
               __attribute__ ((__format__ (__printf__, 3, 0)));
int _vdprintf_r (struct _reent *, int, const char *restrict, __gnuc_va_list)
               __attribute__ ((__format__ (__printf__, 3, 0)));
int _vfiprintf_r (struct _reent *, FILE *, const char *, __gnuc_va_list)
               __attribute__ ((__format__ (__printf__, 3, 0)));
int _vfiscanf_r (struct _reent *, FILE *, const char *, __gnuc_va_list)
               __attribute__ ((__format__ (__scanf__, 3, 0)));
int _vfprintf_r (struct _reent *, FILE *restrict, const char *restrict, __gnuc_va_list)
               __attribute__ ((__format__ (__printf__, 3, 0)));
int _vfscanf_r (struct _reent *, FILE *restrict, const char *restrict, __gnuc_va_list)
               __attribute__ ((__format__ (__scanf__, 3, 0)));
int _viprintf_r (struct _reent *, const char *, __gnuc_va_list)
               __attribute__ ((__format__ (__printf__, 2, 0)));
int _viscanf_r (struct _reent *, const char *, __gnuc_va_list)
               __attribute__ ((__format__ (__scanf__, 2, 0)));
int _vprintf_r (struct _reent *, const char *restrict, __gnuc_va_list)
               __attribute__ ((__format__ (__printf__, 2, 0)));
int _vscanf_r (struct _reent *, const char *restrict, __gnuc_va_list)
               __attribute__ ((__format__ (__scanf__, 2, 0)));
int _vsiprintf_r (struct _reent *, char *, const char *, __gnuc_va_list)
               __attribute__ ((__format__ (__printf__, 3, 0)));
int _vsiscanf_r (struct _reent *, const char *, const char *, __gnuc_va_list)
               __attribute__ ((__format__ (__scanf__, 3, 0)));
int _vsniprintf_r (struct _reent *, char *, size_t, const char *, __gnuc_va_list)
               __attribute__ ((__format__ (__printf__, 4, 0)));
int _vsnprintf_r (struct _reent *, char *restrict, size_t, const char *restrict, __gnuc_va_list)
               __attribute__ ((__format__ (__printf__, 4, 0)));
int _vsprintf_r (struct _reent *, char *restrict, const char *restrict, __gnuc_va_list)
               __attribute__ ((__format__ (__printf__, 3, 0)));
int _vsscanf_r (struct _reent *, const char *restrict, const char *restrict, __gnuc_va_list)
               __attribute__ ((__format__ (__scanf__, 3, 0)));



int fpurge (FILE *);
ssize_t __getdelim (char **, size_t *, int, FILE *);
ssize_t __getline (char **, size_t *, FILE *);


void clearerr_unlocked (FILE *);
int feof_unlocked (FILE *);
int ferror_unlocked (FILE *);
int fileno_unlocked (FILE *);
int fflush_unlocked (FILE *);
int fgetc_unlocked (FILE *);
int fputc_unlocked (int, FILE *);
size_t fread_unlocked (void *restrict, size_t _size, size_t _n, FILE *restrict);
size_t fwrite_unlocked (const void *restrict , size_t _size, size_t _n, FILE *);
int __srget_r (struct _reent *, FILE *);
int __swbuf_r (struct _reent *, int, FILE *);
FILE *funopen (const void *__cookie,
  int (*__readfn)(void *__cookie, char *__buf,
    int __n),
  int (*__writefn)(void *__cookie, const char *__buf,
     int __n),
  fpos_t (*__seekfn)(void *__cookie, fpos_t __off, int __whence),
  int (*__closefn)(void *__cookie));
FILE *_funopen_r (struct _reent *, const void *__cookie,
  int (*__readfn)(void *__cookie, char *__buf,
    int __n),
  int (*__writefn)(void *__cookie, const char *__buf,
     int __n),
  fpos_t (*__seekfn)(void *__cookie, fpos_t __off, int __whence),
  int (*__closefn)(void *__cookie));
static __inline__ int __sputc_r(struct _reent *_ptr, int _c, FILE *_p) {




 if (--_p->_w >= 0 || (_p->_w >= _p->_lbfsize && (char)_c != '\n'))
  return (*_p->_p++ = _c);
 else
  return (__swbuf_r(_ptr, _c, _p));
}
static __inline int
_getchar_unlocked(void)
{
 struct _reent *_ptr;

 _ptr = _impure_ptr;
 return ((--(((_ptr)->_stdin))->_r < 0 ? __srget_r(_ptr, ((_ptr)->_stdin)) : (int)(*(((_ptr)->_stdin))->_p++)));
}

static __inline int
_putchar_unlocked(int _c)
{
 struct _reent *_ptr;

 _ptr = _impure_ptr;
 return (__sputc_r(_ptr, _c, ((_ptr)->_stdout)));
}


typedef struct
{
  double r;
  double i;
} cmplx;
static void eaddm (short unsigned int *x, short unsigned int *y);
static void esubm (short unsigned int *x, short unsigned int *y);
static void emdnorm (short unsigned int *s, int lost, int subflg,
       long int exp, int rcntrl, LDPARMS * ldp);




static void enan (short unsigned int *nan, int size);





static void toe64 (short unsigned int *a, short unsigned int *b);



static void eiremain (short unsigned int *den, short unsigned int *num,
        LDPARMS * ldp);
static int ecmpm (register short unsigned int *a,
    register short unsigned int *b);
static int edivm (short unsigned int *den, short unsigned int *num,
    LDPARMS * ldp);
static int emulm (short unsigned int *a, short unsigned int *b,
    LDPARMS * ldp);
static int eisneg (const short unsigned int *x);
static int eisinf (const short unsigned int *x);
static void emovi (const short unsigned int *a, short unsigned int *b);
static void emovo (short unsigned int *a, short unsigned int *b,
     LDPARMS * ldp);
static void emovz (register short unsigned int *a,
     register short unsigned int *b);
static void ecleaz (register short unsigned int *xi);
static void eadd1 (const short unsigned int *a, const short unsigned int *b,
     short unsigned int *c, int subflg, LDPARMS * ldp);
static int eisnan (const short unsigned int *x);
static int eiisnan (short unsigned int *x);
static void
eclear (register short unsigned int *x)
{
  register int i;

  for (i = 0; i < 10; i++)
    *x++ = 0;
}
static void
emov (register const short unsigned int *a, register short unsigned int *b)
{
  register int i;

  for (i = 0; i < 10; i++)
    *b++ = *a++;
}
static void
eneg (short unsigned int *x)
{


  if (eisnan (x))
    return;

  x[10 - 1] ^= 0x8000;
}






static int
eisneg (const short unsigned int *x)
{


  if (eisnan (x))
    return (0);

  if (x[10 - 1] & 0x8000)
    return (1);
  else
    return (0);
}





static int
eisinf (const short unsigned int *x)
{

  if ((x[10 - 1] & 0x7fff) == 0x7fff)
    {

      if (eisnan (x))
 return (0);

      return (1);
    }
  else
    return (0);
}



static int
eisnan (const short unsigned int *x)
{


  int i;

  if ((x[10 - 1] & 0x7fff) != 0x7fff)
    return (0);

  for (i = 0; i < 10 - 1; i++)
    {
      if (*x++ != 0)
 return (1);
    }

  return (0);
}
static void
einfin (register short unsigned int *x, register LDPARMS * ldp)
{
  register int i;


  for (i = 0; i < 10 - 1; i++)
    *x++ = 0;
  *x |= 32767;
  ldp = ldp;
}




static void
emovi (const short unsigned int *a, short unsigned int *b)
{
  register const unsigned short *p;
  register unsigned short *q;
  int i;

  q = b;
  p = a + (10 - 1);

  if (*p & 0x8000)
    *q++ = 0xffff;
  else
    *q++ = 0;

  *q = *p--;
  *q++ &= 0x7fff;

  if ((*(q - 1) & 0x7fff) == 0x7fff)
    {

      if (eisnan (a))
 {
   *q++ = 0;
   for (i = 3; i < (10 +3); i++)
     *q++ = *p--;
   return;
 }

      for (i = 2; i < (10 +3); i++)
 *q++ = 0;
      return;
    }


  *q++ = 0;

  for (i = 0; i < 10 - 1; i++)
    *q++ = *p--;

  *q = 0;
}





static void
emovo (short unsigned int *a, short unsigned int *b, LDPARMS * ldp)
{
  register unsigned short *p, *q;
  unsigned short i;

  p = a;
  q = b + (10 - 1);

  i = *p++;
  if (i)
    *q-- = *p++ | 0x8000;
  else
    *q-- = *p++;

  if (*(p - 1) == 0x7fff)
    {

      if (eiisnan (a))
 {
   enan (b, (((10 +3)-4)*16));
   return;
 }

      einfin (b, ldp);
      return;
    }


  ++p;

  for (i = 0; i < 10 - 1; i++)
    *q-- = *p++;
}





static void
ecleaz (register short unsigned int *xi)
{
  register int i;

  for (i = 0; i < (10 +3); i++)
    *xi++ = 0;
}



static void
ecleazs (register short unsigned int *xi)
{
  register int i;

  ++xi;
  for (i = 0; i < (10 +3) - 1; i++)
    *xi++ = 0;
}






static void
emovz (register short unsigned int *a, register short unsigned int *b)
{
  register int i;

  for (i = 0; i < (10 +3) - 1; i++)
    *b++ = *a++;

  *b = 0;
}




static int
eiisnan (short unsigned int *x)
{
  int i;

  if ((x[1] & 0x7fff) == 0x7fff)
    {
      for (i = 2 + 1; i < (10 +3); i++)
 {
   if (x[i] != 0)
     return (1);
 }
    }
  return (0);
}




static int
eiisinf (unsigned short x[])
{


  if (eiisnan (x))
    return (0);

  if ((x[1] & 0x7fff) == 0x7fff)
    return (1);
  return (0);
}
static int
ecmpm (register short unsigned int *a, register short unsigned int *b)
{
  int i;

  a += 2;
  b += 2;
  for (i = 2; i < (10 +3); i++)
    {
      if (*a++ != *b++)
 goto difrnt;
    }
  return (0);

difrnt:
  if (*(--a) > *(--b))
    return (1);
  else
    return (-1);
}






static void
eshdn1 (register short unsigned int *x)
{
  register unsigned short bits;
  int i;

  x += 2;

  bits = 0;
  for (i = 2; i < (10 +3); i++)
    {
      if (*x & 1)
 bits |= 1;
      *x >>= 1;
      if (bits & 2)
 *x |= 0x8000;
      bits <<= 1;
      ++x;
    }
}







static void
eshup1 (register short unsigned int *x)
{
  register unsigned short bits;
  int i;

  x += (10 +3) - 1;
  bits = 0;

  for (i = 2; i < (10 +3); i++)
    {
      if (*x & 0x8000)
 bits |= 1;
      *x <<= 1;
      if (bits & 2)
 *x |= 1;
      bits <<= 1;
      --x;
    }
}







static void
eshdn8 (register short unsigned int *x)
{
  register unsigned short newbyt, oldbyt;
  int i;

  x += 2;
  oldbyt = 0;
  for (i = 2; i < (10 +3); i++)
    {
      newbyt = *x << 8;
      *x >>= 8;
      *x |= oldbyt;
      oldbyt = newbyt;
      ++x;
    }
}





static void
eshup8 (register short unsigned int *x)
{
  int i;
  register unsigned short newbyt, oldbyt;

  x += (10 +3) - 1;
  oldbyt = 0;

  for (i = 2; i < (10 +3); i++)
    {
      newbyt = *x >> 8;
      *x <<= 8;
      *x |= oldbyt;
      oldbyt = newbyt;
      --x;
    }
}





static void
eshup6 (register short unsigned int *x)
{
  int i;
  register unsigned short *p;

  p = x + 2;
  x += 2 + 1;

  for (i = 2; i < (10 +3) - 1; i++)
    *p++ = *x++;

  *p = 0;
}





static void
eshdn6 (register short unsigned int *x)
{
  int i;
  register unsigned short *p;

  x += (10 +3) - 1;
  p = x + 1;

  for (i = 2; i < (10 +3) - 1; i++)
    *(--p) = *(--x);

  *(--p) = 0;
}






static void
eaddm (short unsigned int *x, short unsigned int *y)
{
  register unsigned long a;
  int i;
  unsigned int carry;

  x += (10 +3) - 1;
  y += (10 +3) - 1;
  carry = 0;
  for (i = 2; i < (10 +3); i++)
    {
      a = (unsigned long) (*x) + (unsigned long) (*y) + carry;
      if (a & 0x10000)
 carry = 1;
      else
 carry = 0;
      *y = (unsigned short) a;
      --x;
      --y;
    }
}






static void
esubm (short unsigned int *x, short unsigned int *y)
{
  unsigned long a;
  int i;
  unsigned int carry;

  x += (10 +3) - 1;
  y += (10 +3) - 1;
  carry = 0;
  for (i = 2; i < (10 +3); i++)
    {
      a = (unsigned long) (*y) - (unsigned long) (*x) - carry;
      if (a & 0x10000)
 carry = 1;
      else
 carry = 0;
      *y = (unsigned short) a;
      --x;
      --y;
    }
}
static void
m16m (short unsigned int a, short unsigned int *b, short unsigned int *c)
{
  register unsigned short *pp;
  register unsigned long carry;
  unsigned short *ps;
  unsigned short p[(10 +3)];
  unsigned long aa, m;
  int i;

  aa = a;
  pp = &p[(10 +3) - 2];
  *pp++ = 0;
  *pp = 0;
  ps = &b[(10 +3) - 1];

  for (i = 2 + 1; i < (10 +3); i++)
    {
      if (*ps == 0)
 {
   --ps;
   --pp;
   *(pp - 1) = 0;
 }
      else
 {
   m = (unsigned long) aa **ps--;
   carry = (m & 0xffff) + *pp;
   *pp-- = (unsigned short) carry;
   carry = (carry >> 16) + (m >> 16) + *pp;
   *pp = (unsigned short) carry;
   *(pp - 1) = carry >> 16;
 }
    }
  for (i = 2; i < (10 +3); i++)
    c[i] = p[i];
}






static int
edivm (short unsigned int *den, short unsigned int *num, LDPARMS * ldp)
{
  int i;
  register unsigned short *p;
  unsigned long tnum;
  unsigned short j, tdenm, tquot;
  unsigned short tprod[(10 +3) + 1];
  unsigned short *equot = ldp->equot;

  p = &equot[0];
  *p++ = num[0];
  *p++ = num[1];

  for (i = 2; i < (10 +3); i++)
    {
      *p++ = 0;
    }
  eshdn1 (num);
  tdenm = den[2 + 1];
  for (i = 2; i < (10 +3); i++)
    {

      tnum = (((unsigned long) num[2]) << 16) + num[2 + 1];


      if ((tdenm * 0xffffUL) < tnum)
 tquot = 0xffff;
      else
 tquot = tnum / tdenm;
      m16m (tquot, den, tprod);

      if (ecmpm (tprod, num) > 0)
 {
   tquot -= 1;
   esubm (den, tprod);
   if (ecmpm (tprod, num) > 0)
     {
       tquot -= 1;
       esubm (den, tprod);
     }
 }
      esubm (tprod, num);
      equot[i] = tquot;
      eshup6 (num);
    }

  p = &num[2];
  j = 0;
  for (i = 2; i < (10 +3); i++)
    {
      j |= *p++;
    }
  if (j)
    j = 1;

  for (i = 0; i < (10 +3); i++)
    num[i] = equot[i];

  return ((int) j);
}




static int
emulm (short unsigned int *a, short unsigned int *b, LDPARMS * ldp)
{
  unsigned short *p, *q;
  unsigned short pprod[(10 +3)];
  unsigned short j;
  int i;
  unsigned short *equot = ldp->equot;

  equot[0] = b[0];
  equot[1] = b[1];
  for (i = 2; i < (10 +3); i++)
    equot[i] = 0;

  j = 0;
  p = &a[(10 +3) - 1];
  q = &equot[(10 +3) - 1];
  for (i = 2 + 1; i < (10 +3); i++)
    {
      if (*p == 0)
 {
   --p;
 }
      else
 {
   m16m (*p--, b, pprod);
   eaddm (pprod, equot);
 }
      j |= *q;
      eshdn6 (equot);
    }

  for (i = 0; i < (10 +3); i++)
    b[i] = equot[i];


  return ((int) j);
}
static void
emdnorm (short unsigned int *s, int lost, int subflg, long int exp,
  int rcntrl, LDPARMS * ldp)
{
  int i, j;
  unsigned short r;


  j = enormlz (s);
  exp -= j;




  if ((j > (((10 +3)-4)*16)) && (exp < 32767L))
    {
      ecleazs (s);
      return;
    }

  if (exp < 0L)
    {
      if (exp > (long) (-(((10 +3)-4)*16) - 1))
 {
   j = (int) exp;
   i = eshift (s, j);
   if (i)
     lost = 1;
 }
      else
 {
   ecleazs (s);
   return;
 }
    }

  if (rcntrl == 0)
    goto mdfin;

  if (ldp->rndprc != ldp->rlast)
    {
      ecleaz (ldp->rbit);
      switch (ldp->rndprc)
 {
 default:
 case (((10 +3)-4)*16):
   ldp->rw = (10 +3) - 1;
   ldp->rmsk = 0xffff;
   ldp->rmbit = 0x8000;
   ldp->rebit = 1;
   ldp->re = ldp->rw - 1;
   break;
 case 113:
   ldp->rw = 10;
   ldp->rmsk = 0x7fff;
   ldp->rmbit = 0x4000;
   ldp->rebit = 0x8000;
   ldp->re = ldp->rw;
   break;
 case 64:
   ldp->rw = 7;
   ldp->rmsk = 0xffff;
   ldp->rmbit = 0x8000;
   ldp->rebit = 1;
   ldp->re = ldp->rw - 1;
   break;

 case 56:
   ldp->rw = 6;
   ldp->rmsk = 0xff;
   ldp->rmbit = 0x80;
   ldp->rebit = 0x100;
   ldp->re = ldp->rw;
   break;
 case 53:
   ldp->rw = 6;
   ldp->rmsk = 0x7ff;
   ldp->rmbit = 0x0400;
   ldp->rebit = 0x800;
   ldp->re = ldp->rw;
   break;
 case 24:
   ldp->rw = 4;
   ldp->rmsk = 0xff;
   ldp->rmbit = 0x80;
   ldp->rebit = 0x100;
   ldp->re = ldp->rw;
   break;
 }
      ldp->rbit[ldp->re] = ldp->rebit;
      ldp->rlast = ldp->rndprc;
    }







  if ((exp <= 0) && (ldp->rndprc != (((10 +3)-4)*16)))



    {
      lost |= s[(10 +3) - 1] & 1;
      eshdn1 (s);
    }



  r = s[ldp->rw] & ldp->rmsk;
  if (ldp->rndprc < (((10 +3)-4)*16))
    {
      i = ldp->rw + 1;
      while (i < (10 +3))
 {
   if (s[i])
     r |= 1;
   s[i] = 0;
   ++i;
 }
    }
  s[ldp->rw] &= ~ldp->rmsk;
  if ((r & ldp->rmbit) != 0)
    {
      if (r == ldp->rmbit)
 {
   if (lost == 0)
     {
       if ((s[ldp->re] & ldp->rebit) == 0)
  goto mddone;
     }
   else
     {
       if (subflg != 0)
  goto mddone;
     }
 }
      eaddm (ldp->rbit, s);
    }
mddone:

  if ((exp <= 0) && (ldp->rndprc != (((10 +3)-4)*16)))



    {
      eshup1 (s);
    }
  if (s[2] != 0)
    {
      eshdn1 (s);
      exp += 1;
    }
mdfin:
  s[(10 +3) - 1] = 0;
  if (exp >= 32767L)
    {




      s[1] = 32767;
      for (i = 2; i < (10 +3) - 1; i++)
 s[i] = 0;
      return;
    }
  if (exp < 0)
    s[1] = 0;
  else
    s[1] = (unsigned short) exp;
}
static void
esub (const short unsigned int *a, const short unsigned int *b,
      short unsigned int *c, LDPARMS * ldp)
{


  if (eisnan (a))
    {
      emov (a, c);
      return;
    }
  if (eisnan (b))
    {
      emov (b, c);
      return;
    }



  if (eisinf (a) && eisinf (b) && ((eisneg (a) ^ eisneg (b)) == 0))
    {
      ;
      enan (c, (((10 +3)-4)*16));
      return;
    }

  eadd1 (a, b, c, 1, ldp);
}



static void
eadd1 (const short unsigned int *a, const short unsigned int *b,
       short unsigned int *c, int subflg, LDPARMS * ldp)
{
  unsigned short ai[(10 +3)], bi[(10 +3)], ci[(10 +3)];
  int i, lost, j, k;
  long lt, lta, ltb;


  if (eisinf (a))
    {
      emov (a, c);
      if (subflg)
 eneg (c);
      return;
    }
  if (eisinf (b))
    {
      emov (b, c);
      return;
    }

  emovi (a, ai);
  emovi (b, bi);
  if (subflg)
    ai[0] = ~ai[0];


  lta = ai[1];
  ltb = bi[1];
  lt = lta - ltb;
  if (lt > 0L)
    {
      emovz (bi, ci);
      emovz (ai, bi);
      emovz (ci, ai);
      ltb = bi[1];
      lt = -lt;
    }
  lost = 0;
  if (lt != 0L)
    {
      if (lt < (long) (-(((10 +3)-4)*16) - 1))
 goto done;
      k = (int) lt;
      lost = eshift (ai, k);
    }
  else
    {

      i = ecmpm (ai, bi);
      if (i == 0)
 {

   if (ai[0] != bi[0])
     {
       eclear (c);
       return;
     }


   if ((bi[1] == 0) && ((bi[3] & 0x8000) == 0))
     {
       eshup1 (bi);
       goto done;
     }

   for (j = 1; j < (10 +3) - 1; j++)
     {
       if (bi[j] != 0)
  {

    ltb += 1;
    break;
  }
     }
   bi[1] = (unsigned short) ltb;
   goto done;
 }
      if (i > 0)
 {
   emovz (bi, ci);
   emovz (ai, bi);
   emovz (ci, ai);
 }
    }
  if (ai[0] == bi[0])
    {
      eaddm (ai, bi);
      subflg = 0;
    }
  else
    {
      esubm (ai, bi);
      subflg = 1;
    }
  emdnorm (bi, lost, subflg, ltb, 64, ldp);

done:
  emovo (bi, c, ldp);
}
static void
ediv (const short unsigned int *a, const short unsigned int *b,
      short unsigned int *c, LDPARMS * ldp)
{
  unsigned short ai[(10 +3)], bi[(10 +3)];
  int i;
  long lt, lta, ltb;



  if (eisnan (a))
    {
      emov (a, c);
      return;
    }
  if (eisnan (b))
    {
      emov (b, c);
      return;
    }

  if (((ecmp (a, ezero) == 0) && (ecmp (b, ezero) == 0))
      || (eisinf (a) && eisinf (b)))
    {
      ;
      enan (c, (((10 +3)-4)*16));
      return;
    }



  if (eisinf (b))
    {
      if (eisneg (a) ^ eisneg (b))
 *(c + (10 - 1)) = 0x8000;
      else
 *(c + (10 - 1)) = 0;
      einfin (c, ldp);
      return;
    }
  if (eisinf (a))
    {
      eclear (c);
      return;
    }

  emovi (a, ai);
  emovi (b, bi);
  lta = ai[1];
  ltb = bi[1];
  if (bi[1] == 0)
    {
      for (i = 1; i < (10 +3) - 1; i++)
 {
   if (bi[i] != 0)
     {
       ltb -= enormlz (bi);
       goto dnzro1;
     }
 }
      eclear (c);
      return;
    }
dnzro1:

  if (ai[1] == 0)
    {
      for (i = 1; i < (10 +3) - 1; i++)
 {
   if (ai[i] != 0)
     {
       lta -= enormlz (ai);
       goto dnzro2;
     }
 }
      if (ai[0] == bi[0])
 *(c + (10 - 1)) = 0;
      else
 *(c + (10 - 1)) = 0x8000;
      einfin (c, ldp);
      ;
      return;
    }
dnzro2:

  i = edivm (ai, bi, ldp);

  lt = ltb - lta + (0x3fff);
  emdnorm (bi, i, 0, lt, 64, ldp);

  if (ai[0] == bi[0])
    bi[0] = 0;
  else
    bi[0] = 0Xffff;
  emovo (bi, c, ldp);
}
static void
emul (const short unsigned int *a, const short unsigned int *b,
      short unsigned int *c, LDPARMS * ldp)
{
  unsigned short ai[(10 +3)], bi[(10 +3)];
  int i, j;
  long lt, lta, ltb;



  if (eisnan (a))
    {
      emov (a, c);
      return;
    }
  if (eisnan (b))
    {
      emov (b, c);
      return;
    }

  if ((eisinf (a) && (ecmp (b, ezero) == 0))
      || (eisinf (b) && (ecmp (a, ezero) == 0)))
    {
      ;
      enan (c, (((10 +3)-4)*16));
      return;
    }



  if (eisinf (a) || eisinf (b))
    {
      if (eisneg (a) ^ eisneg (b))
 *(c + (10 - 1)) = 0x8000;
      else
 *(c + (10 - 1)) = 0;
      einfin (c, ldp);
      return;
    }

  emovi (a, ai);
  emovi (b, bi);
  lta = ai[1];
  ltb = bi[1];
  if (ai[1] == 0)
    {
      for (i = 1; i < (10 +3) - 1; i++)
 {
   if (ai[i] != 0)
     {
       lta -= enormlz (ai);
       goto mnzer1;
     }
 }
      eclear (c);
      return;
    }
mnzer1:

  if (bi[1] == 0)
    {
      for (i = 1; i < (10 +3) - 1; i++)
 {
   if (bi[i] != 0)
     {
       ltb -= enormlz (bi);
       goto mnzer2;
     }
 }
      eclear (c);
      return;
    }
mnzer2:


  j = emulm (ai, bi, ldp);

  lt = lta + ltb - ((0x3fff) - 1);
  emdnorm (bi, j, 0, lt, 64, ldp);

  if (ai[0] == bi[0])
    bi[0] = 0;
  else
    bi[0] = 0xffff;
  emovo (bi, c, ldp);
}
static void
e64toe (short unsigned int *pe, short unsigned int *y, LDPARMS * ldp)
{
  unsigned short yy[(10 +3)];
  unsigned short *p, *q, *e;
  int i;

  e = pe;
  p = yy;

  for (i = 0; i < 10 - 5; i++)
    *p++ = 0;

  for (i = 0; i < 5; i++)
    *p++ = *e++;
  if ((yy[10 - 1] & 0x7fff) == 0 && (yy[10 - 2] & 0x8000) == 0)
    {
      unsigned short temp[(10 +3) + 1];
      emovi (yy, temp);
      eshup1 (temp);
      emovo (temp, y, ldp);
      return;
    }



  p = &yy[10 - 1];
  if ((*p & 0x7fff) == 0x7fff)
    {


      for (i = 0; i < 4; i++)
 {
   if ((i != 3 && pe[i] != 0)

       || (i == 3 && pe[i] != 0x8000))
     {
       enan (y, (((10 +3)-4)*16));
       return;
     }
 }
      eclear (y);
      einfin (y, ldp);
      if (*p & 0x8000)
 eneg (y);
      return;
    }

  p = yy;
  q = y;
  for (i = 0; i < 10; i++)
    *q++ = *p++;
}


static void
__attribute__ ((__unused__))
toe64 (short unsigned int *a, short unsigned int *b)
{
  register unsigned short *p, *q;
  unsigned short i;


  if (eiisnan (a))
    {
      enan (b, 64);
      return;
    }



  if (a[1] == 0)
    eshdn1 (a);

  p = a;



  q = b + 4;

  *(q + 1) = 0;



  i = *p++;







  if (i)
    *q-- = *p++ | 0x8000;
  else
    *q-- = *p++;


  ++p;







  if (eiisinf (a))
    {

      *q-- = 0x8000;
      *q-- = 0;
      *q-- = 0;
      *q = 0;
      return;
    }


  for (i = 0; i < 4; i++)
    *q-- = *p++;

}
static int
ecmp (const short unsigned int *a, const short unsigned int *b)
{
  unsigned short ai[(10 +3)], bi[(10 +3)];
  register unsigned short *p, *q;
  register int i;
  int msign;


  if (eisnan (a) || eisnan (b))
    return (-2);

  emovi (a, ai);
  p = ai;
  emovi (b, bi);
  q = bi;

  if (*p != *q)
    {

      for (i = 1; i < (10 +3) - 1; i++)
 {
   if (ai[i] != 0)
     goto nzro;
   if (bi[i] != 0)
     goto nzro;
 }
      return (0);
    nzro:
      if (*p == 0)
 return (1);
      else
 return (-1);
    }

  if (*p == 0)
    msign = 1;
  else
    msign = -1;
  i = (10 +3) - 1;
  do
    {
      if (*p++ != *q++)
 {
   goto __mdiff;
 }
    }
  while (--i > 0);

  return (0);



__mdiff:

  if (*(--p) > *(--q))
    return (msign);
  else
    return (-msign);
}
static int
eshift (short unsigned int *x, int sc)
{
  unsigned short lost;
  unsigned short *p;

  if (sc == 0)
    return (0);

  lost = 0;
  p = x + (10 +3) - 1;

  if (sc < 0)
    {
      sc = -sc;
      while (sc >= 16)
 {
   lost |= *p;
   eshdn6 (x);
   sc -= 16;
 }

      while (sc >= 8)
 {
   lost |= *p & 0xff;
   eshdn8 (x);
   sc -= 8;
 }

      while (sc > 0)
 {
   lost |= *p & 1;
   eshdn1 (x);
   sc -= 1;
 }
    }
  else
    {
      while (sc >= 16)
 {
   eshup6 (x);
   sc -= 16;
 }

      while (sc >= 8)
 {
   eshup8 (x);
   sc -= 8;
 }

      while (sc > 0)
 {
   eshup1 (x);
   sc -= 1;
 }
    }
  if (lost)
    lost = 1;
  return ((int) lost);
}
static int
enormlz (short unsigned int *x)
{
  register unsigned short *p;
  int sc;

  sc = 0;
  p = &x[2];
  if (*p != 0)
    goto normdn;
  ++p;
  if (*p & 0x8000)
    return (0);
  while (*p == 0)
    {
      eshup6 (x);
      sc += 16;



      if (sc > (((10 +3)-4)*16))
 return (sc);
    }

  while ((*p & 0xff00) == 0)
    {
      eshup8 (x);
      sc += 8;
    }

  while ((*p & 0x8000) == 0)
    {
      eshup1 (x);
      sc += 1;
      if (sc > ((((10 +3)-4)*16) + 16))
 {
   ;
   return (sc);
 }
    }
  return (sc);



normdn:

  if (*p & 0xff00)
    {
      eshdn8 (x);
      sc -= 8;
    }
  while (*p != 0)
    {
      eshdn1 (x);
      sc -= 1;

      if (sc < -(((10 +3)-4)*16))
 {
   ;
   return (sc);
 }
    }
  return (sc);
}
static const unsigned short etens[12 + 1][10] = {
  {0x6576, 0x4a92, 0x804a, 0x153f,
   0xc94c, 0x979a, 0x8a20, 0x5202, 0xc460, 0x7525,},
  {0x6a32, 0xce52, 0x329a, 0x28ce,
   0xa74d, 0x5de4, 0xc53d, 0x3b5d, 0x9e8b, 0x5a92,},
  {0x526c, 0x50ce, 0xf18b, 0x3d28,
   0x650d, 0x0c17, 0x8175, 0x7586, 0xc976, 0x4d48,},
  {0x9c66, 0x58f8, 0xbc50, 0x5c54,
   0xcc65, 0x91c6, 0xa60e, 0xa0ae, 0xe319, 0x46a3,},
  {0x851e, 0xeab7, 0x98fe, 0x901b,
   0xddbb, 0xde8d, 0x9df9, 0xebfb, 0xaa7e, 0x4351,},
  {0x0235, 0x0137, 0x36b1, 0x336c,
   0xc66f, 0x8cdf, 0x80e9, 0x47c9, 0x93ba, 0x41a8,},
  {0x50f8, 0x25fb, 0xc76b, 0x6b71,
   0x3cbf, 0xa6d5, 0xffcf, 0x1f49, 0xc278, 0x40d3,},
  {0x0000, 0x0000, 0x0000, 0x0000,
   0xf020, 0xb59d, 0x2b70, 0xada8, 0x9dc5, 0x4069,},
  {0x0000, 0x0000, 0x0000, 0x0000,
   0x0000, 0x0000, 0x0400, 0xc9bf, 0x8e1b, 0x4034,},
  {0x0000, 0x0000, 0x0000, 0x0000,
   0x0000, 0x0000, 0x0000, 0x2000, 0xbebc, 0x4019,},
  {0x0000, 0x0000, 0x0000, 0x0000,
   0x0000, 0x0000, 0x0000, 0x0000, 0x9c40, 0x400c,},
  {0x0000, 0x0000, 0x0000, 0x0000,
   0x0000, 0x0000, 0x0000, 0x0000, 0xc800, 0x4005,},
  {0x0000, 0x0000, 0x0000, 0x0000,
   0x0000, 0x0000, 0x0000, 0x0000, 0xa000, 0x4002,},
};

static const unsigned short emtens[12 + 1][10] = {
  {0x2030, 0xcffc, 0xa1c3, 0x8123,
   0x2de3, 0x9fde, 0xd2ce, 0x04c8, 0xa6dd, 0x0ad8,},
  {0x8264, 0xd2cb, 0xf2ea, 0x12d4,
   0x4925, 0x2de4, 0x3436, 0x534f, 0xceae, 0x256b,},
  {0xf53f, 0xf698, 0x6bd3, 0x0158,
   0x87a6, 0xc0bd, 0xda57, 0x82a5, 0xa2a6, 0x32b5,},
  {0xe731, 0x04d4, 0xe3f2, 0xd332,
   0x7132, 0xd21c, 0xdb23, 0xee32, 0x9049, 0x395a,},
  {0xa23e, 0x5308, 0xfefb, 0x1155,
   0xfa91, 0x1939, 0x637a, 0x4325, 0xc031, 0x3cac,},
  {0xe26d, 0xdbde, 0xd05d, 0xb3f6,
   0xac7c, 0xe4a0, 0x64bc, 0x467c, 0xddd0, 0x3e55,},
  {0x2a20, 0x6224, 0x47b3, 0x98d7,
   0x3f23, 0xe9a5, 0xa539, 0xea27, 0xa87f, 0x3f2a,},
  {0x0b5b, 0x4af2, 0xa581, 0x18ed,
   0x67de, 0x94ba, 0x4539, 0x1ead, 0xcfb1, 0x3f94,},
  {0xbf71, 0xa9b3, 0x7989, 0xbe68,
   0x4c2e, 0xe15b, 0xc44d, 0x94be, 0xe695, 0x3fc9,},
  {0x3d4d, 0x7c3d, 0x36ba, 0x0d2b,
   0xfdc2, 0xcefc, 0x8461, 0x7711, 0xabcc, 0x3fe4,},
  {0xc155, 0xa4a8, 0x404e, 0x6113,
   0xd3c3, 0x652b, 0xe219, 0x1758, 0xd1b7, 0x3ff1,},
  {0xd70a, 0x70a3, 0x0a3d, 0xa3d7,
   0x3d70, 0xd70a, 0x70a3, 0x0a3d, 0xa3d7, 0x3ff8,},
  {0xcccd, 0xcccc, 0xcccc, 0xcccc,
   0xcccc, 0xcccc, 0xcccc, 0xcccc, 0xcccc, 0x3ffb,},
};
char *
_ldtoa_r (struct _reent *ptr, long double d, int mode, int ndigits,
   int *decpt, int *sign, char **rve)
{
  unsigned short e[(10 +3)];
  char *s, *p;
  int i, j, k;
  int orig_ndigits;
  LDPARMS rnd;
  LDPARMS *ldp = &rnd;
  char *outstr;
  char outbuf[((((10 +3)-4)*16)*8/27) + 5 + 10];
  union uconv du;
  du.d = d;

  orig_ndigits = ndigits;
  rnd.rlast = -1;
  rnd.rndprc = (((10 +3)-4)*16);

  ;


  if (
     ((
     ptr
     )->_result)
                           )
    {
      
     ((
     ptr
     )->_result)
                           ->_k = 
                                  ((
                                  ptr
                                  )->_result_k)
                                                          ;
      
     ((
     ptr
     )->_result)
                           ->_maxwds = 1 << 
                                            ((
                                            ptr
                                            )->_result_k)
                                                                    ;
      _Bfree (ptr, 
                 ((
                 ptr
                 )->_result)
                                       );
      
     ((
     ptr
     )->_result) 
                            = 0;
    }






  e64toe (&du.pe, e, ldp);




  if (eisneg (e))
    *sign = 1;
  else
    *sign = 0;

  if (mode != 3)
    ndigits -= 1;



  if (mode == 0)
    ndigits = 20;



  if (ndigits > ((((10 +3)-4)*16)*8/27))
    ndigits = ((((10 +3)-4)*16)*8/27);

  etoasc (e, outbuf, ndigits, mode, ldp);
  s = outbuf;
  if (eisinf (e) || eisnan (e))
    {
      *decpt = 9999;
      goto stripspaces;
    }
  *decpt = ldp->outexpon + 1;




  s = outbuf;
  while (*s != '\0')
    {
      if (*s == '.')
 goto yesdecpt;
      ++s;
    }
  goto nodecpt;

yesdecpt:


  while (*s != '\0')
    {
      *s = *(s + 1);
      ++s;
    }

nodecpt:


  while (*s != 'E' && s > outbuf)
    --s;
  *s = '\0';

stripspaces:


  p = outbuf;
  while (*p == ' ' || *p == '-')
    ++p;


  s = outbuf;
  while ((*s++ = *p++) != '\0')
    ;
  --s;


  if (mode == 2)
    k = 1;
  else if (ndigits > ldp->outexpon)
    k = ndigits;
  else
    k = ldp->outexpon;

  while (*(s - 1) == '0' && ((s - outbuf) > k))
    *(--s) = '\0';



  if (mode == 3 && ((ndigits + ldp->outexpon) < 0))
    {
      s = outbuf;
      *s = '\0';
      *decpt = 0;
    }




  if (mode == 3)
    i = *decpt + orig_ndigits + 3;
  else
    i = orig_ndigits + 5 + 4;

  j = sizeof (__ULong);
  for (
      ((
      ptr
      )->_result_k) 
                               = 0;
       sizeof (_Bigint) - sizeof (__ULong) + j <= i; j <<= 1)
    
   ((
   ptr
   )->_result_k)
                           ++;
  
 ((
 ptr
 )->_result) 
                        = _Balloc (ptr, 
                                       ((
                                       ptr
                                       )->_result_k)
                                                               );


  outstr = (char *) 
                   ((
                   ptr
                   )->_result)
                                         ;
  strcpy (outstr, outbuf);

  if (rve)
    *rve = outstr + (s - outbuf);

  return outstr;
}






int
_ldcheck (long double *d)
{
  unsigned short e[(10 +3)];
  LDPARMS rnd;
  LDPARMS *ldp = &rnd;

  union uconv du;

  rnd.rlast = -1;
  rnd.rndprc = (((10 +3)-4)*16);
  du.d = *d;





  e64toe (&du.pe, e, ldp);




  if ((e[10 - 1] & 0x7fff) == 0x7fff)
    {

      if (eisnan (e))
 return (1);

      return (2);
    }
  else
    return (0);
}

static void
etoasc (short unsigned int *x, char *string, int ndigits, int outformat,
 LDPARMS * ldp)
{
  long digit;
  unsigned short y[(10 +3)], t[(10 +3)], u[(10 +3)], w[(10 +3)];
  const unsigned short *p, *r, *ten;
  unsigned short sign;
  int i, j, k, expon, rndsav, ndigs;
  char *s, *ss;
  unsigned short m;
  unsigned short *equot = ldp->equot;

  ndigs = ndigits;
  rndsav = ldp->rndprc;

  if (eisnan (x))
    {
      sprintf (string, " NaN ");
      expon = 9999;
      goto bxit;
    }

  ldp->rndprc = (((10 +3)-4)*16);
  emov (x, y);
  if (y[10 - 1] & 0x8000)
    {
      sign = 0xffff;
      y[10 - 1] &= 0x7fff;
    }
  else
    {
      sign = 0;
    }
  expon = 0;
  ten = &etens[12][0];
  emov (eone, t);

  if (y[10 - 1] == 0)
    {
      for (k = 0; k < 10 - 1; k++)
 {
   if (y[k] != 0)
     goto tnzro;
 }
      goto isone;
    }
tnzro:



  if (y[10 - 1] == 0x7fff)
    {
      if (sign)
 sprintf (string, " -Infinity ");
      else
 sprintf (string, " Infinity ");
      expon = 9999;
      goto bxit;
    }




  if ((y[10 - 1] != 0) && ((y[10 - 2] & 0x8000) == 0))
    {
      ;
      sprintf (string, "NaN");
      expon = 9999;
      goto bxit;
    }


  i = ecmp (eone, y);
  if (i == 0)
    goto isone;

  if (i < 0)
    {

      emov (y, u);
      u[10 - 1] = (0x3fff) + (((10 +3)-4)*16) - 1;

      p = &etens[12 - 4][0];
      m = 16;
      do
 {
   ediv (p, u, t, ldp);
   efloor (t, w, ldp);
   for (j = 0; j < 10 - 1; j++)
     {
       if (t[j] != w[j])
  goto noint;
     }
   emov (t, u);
   expon += (int) m;
 noint:
   p += 10;
   m >>= 1;
 }
      while (m != 0);


      u[10 - 1] += y[10 - 1] - (unsigned int) ((0x3fff) + (((10 +3)-4)*16) - 1);
      emov (u, y);

      emov (eone, t);
      m = 4096;
      p = &etens[0][0];
      while (ecmp (ten, u) <= 0)
 {
   if (ecmp (p, u) <= 0)
     {
       ediv (p, u, u, ldp);
       emul (p, t, t, ldp);
       expon += (int) m;
     }
   m >>= 1;
   if (m == 0)
     break;
   p += 10;
 }
    }
  else
    {

      if (y[10 - 1] == 0)
 {
   while ((y[10 - 2] & 0x8000) == 0)
     {
       emul (ten, y, y, ldp);
       expon -= 1;
     }
 }
      else
 {
   emovi (y, w);
   for (i = 0; i < ((((10 +3)-4)*16)*8/27) + 1; i++)
     {
       if ((w[(10 +3) - 1] & 0x7) != 0)
  break;

       emovz (w, u);
       eshdn1 (u);
       eshdn1 (u);
       eaddm (w, u);
       u[1] += 3;
       while (u[2] != 0)
  {
    eshdn1 (u);
    u[1] += 1;
  }
       if (u[(10 +3) - 1] != 0)
  break;
       if (eone[10 - 1] <= u[1])
  break;
       emovz (u, w);
       expon -= 1;
     }
   emovo (w, y, ldp);
 }
      k = -4096;
      p = &emtens[0][0];
      r = &etens[0][0];
      emov (y, w);
      emov (eone, t);
      while (ecmp (eone, w) > 0)
 {
   if (ecmp (p, w) >= 0)
     {
       emul (r, w, w, ldp);
       emul (r, t, t, ldp);
       expon += k;
     }
   k /= 2;
   if (k == 0)
     break;
   p += 10;
   r += 10;
 }
      ediv (t, eone, t, ldp);
    }
isone:

  emovi (t, w);
  emovz (w, t);
  emovi (y, w);
  emovz (w, y);
  eiremain (t, y, ldp);
  digit = equot[(10 +3) - 1];
  while ((digit == 0) && (ecmp (y, ezero) != 0))
    {
      eshup1 (y);
      emovz (y, u);
      eshup1 (u);
      eshup1 (u);
      eaddm (u, y);
      eiremain (t, y, ldp);
      digit = equot[(10 +3) - 1];
      expon -= 1;
    }
  s = string;
  if (sign)
    *s++ = '-';
  else
    *s++ = ' ';

  if (outformat == 3)
    ndigs += expon;




  if (ndigs > ((((10 +3)-4)*16)*8/27))
    ndigs = ((((10 +3)-4)*16)*8/27);
  if (digit == 10)
    {
      *s++ = '1';
      *s++ = '.';
      if (ndigs > 0)
 {
   *s++ = '0';
   ndigs -= 1;
 }
      expon += 1;
      if (ndigs < 0)
 {
   ss = s;
   goto doexp;
 }
    }
  else
    {
      *s++ = (char) digit + '0';
      *s++ = '.';
    }

  for (k = 0; k <= ndigs; k++)
    {

      eshup1 (y);
      emovz (y, u);
      eshup1 (u);
      eshup1 (u);
      eaddm (u, y);
      eiremain (t, y, ldp);
      *s++ = (char) equot[(10 +3) - 1] + '0';
    }
  digit = equot[(10 +3) - 1];
  --s;
  ss = s;

  if (digit > 4)
    {

      if (digit == 5)
 {
   emovo (y, t, ldp);
   if (ecmp (t, ezero) != 0)
     goto roun;
   if (ndigs < 0 || (*(s - 1 - (*(s - 1) == '.')) & 1) == 0)
     goto doexp;
 }

    roun:
      --s;
      k = *s & 0x7f;

      if (ndigs < 0)
 {

   *s = '1';
   expon += 1;
   goto doexp;
 }
      else if (k == '.')
 {
   --s;
   k = *s;
   k += 1;
   *s = (char) k;

   if (k > '9')
     {
       expon += 1;
       *s = '1';
     }
   goto doexp;
 }

      k += 1;
      *s = (char) k;
      if (k > '9')
 {
   *s = '0';
   goto roun;
 }
    }
doexp:






  sprintf (ss, "E%d", expon);

bxit:
  ldp->rndprc = rndsav;
  ldp->outexpon = expon;
}
static const unsigned short bmask[] = {
  0xffff,
  0xfffe,
  0xfffc,
  0xfff8,
  0xfff0,
  0xffe0,
  0xffc0,
  0xff80,
  0xff00,
  0xfe00,
  0xfc00,
  0xf800,
  0xf000,
  0xe000,
  0xc000,
  0x8000,
  0x0000,
};

static void
efloor (short unsigned int *x, short unsigned int *y, LDPARMS * ldp)
{
  register unsigned short *p;
  int e, expon, i;
  unsigned short f[10];

  emov (x, f);
  expon = (int) f[10 - 1];
  e = (expon & 0x7fff) - ((0x3fff) - 1);
  if (e <= 0)
    {
      eclear (y);
      goto isitneg;
    }

  e = (((10 +3)-4)*16) - e;
  emov (f, y);
  if (e <= 0)
    return;

  p = &y[0];
  while (e >= 16)
    {
      *p++ = 0;
      e -= 16;
    }

  *p &= bmask[e];

isitneg:

  if ((unsigned short) expon & (unsigned short) 0x8000)
    {
      for (i = 0; i < 10 - 1; i++)
 {
   if (f[i] != y[i])
     {
       esub (eone, y, y, ldp);
       break;
     }
 }
    }
}



static void
eiremain (short unsigned int *den, short unsigned int *num, LDPARMS * ldp)
{
  long ld, ln;
  unsigned short j;
  unsigned short *equot = ldp->equot;

  ld = den[1];
  ld -= enormlz (den);
  ln = num[1];
  ln -= enormlz (num);
  ecleaz (equot);
  while (ln >= ld)
    {
      if (ecmpm (den, num) <= 0)
 {
   esubm (den, num);
   j = 1;
 }
      else
 {
   j = 0;
 }
      eshup1 (equot);
      equot[(10 +3) - 1] |= j;
      eshup1 (num);
      ln -= 1;
    }
  emdnorm (num, 0, 0, ln, 0, ldp);
}
static const unsigned short nan113[8] = { 0, 0, 0, 0, 0, 0, 0x8000, 0x7fff };
static const unsigned short nan64[6] = { 0, 0, 0, 0, 0xc000, 0x7fff };
static const unsigned short nan53[4] = { 0, 0, 0, 0x7ff8 };
static const unsigned short nan24[2] = { 0, 0x7fc0 };
static void
enan (short unsigned int *nan, int size)
{
  int i, n;
  const unsigned short *p;

  switch (size)
    {

    case 113:
      n = 8;
      p = nan113;
      break;

    case 64:
      n = 6;
      p = nan64;
      break;

    case 53:
      n = 4;
      p = nan53;
      break;

    case 24:
      n = 2;
      p = nan24;
      break;

    case (((10 +3)-4)*16):

      for (i = 0; i < 10 - 2; i++)
 *nan++ = 0;
      *nan++ = 0xc000;





      *nan++ = 0x7fff;
      return;

    case (10 +3) * 16:
      *nan++ = 0;
      *nan++ = 0x7fff;
      *nan++ = 0;

      *nan++ = 0xc000;
      for (i = 4; i < (10 +3) - 1; i++)
 *nan++ = 0;





      *nan++ = 0;
      return;

    default:
      ;
      return;
    }
  for (i = 0; i < n; i++)
    *nan++ = *p++;
}
