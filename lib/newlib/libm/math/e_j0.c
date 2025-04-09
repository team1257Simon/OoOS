







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

extern double scalb (double, double);

extern double significand (double);

extern long double __ieee754_hypotl (long double, long double);


extern double __ieee754_sqrt (double);
extern double __ieee754_acos (double);
extern double __ieee754_acosh (double);
extern double __ieee754_log (double);
extern double __ieee754_atanh (double);
extern double __ieee754_asin (double);
extern double __ieee754_atan2 (double,double);
extern double __ieee754_exp (double);
extern double __ieee754_cosh (double);
extern double __ieee754_fmod (double,double);
extern double __ieee754_pow (double,double);
extern double __ieee754_lgamma_r (double,int *);
extern double __ieee754_gamma_r (double,int *);
extern double __ieee754_log10 (double);
extern double __ieee754_sinh (double);
extern double __ieee754_hypot (double,double);
extern double __ieee754_j0 (double);
extern double __ieee754_j1 (double);
extern double __ieee754_y0 (double);
extern double __ieee754_y1 (double);
extern double __ieee754_jn (int,double);
extern double __ieee754_yn (int,double);
extern double __ieee754_remainder (double,double);
extern __int32_t __ieee754_rem_pio2 (double,double*);



extern double __ieee754_scalb (double,double);



extern double __kernel_standard (double,double,int);
extern double __kernel_sin (double,double,int);
extern double __kernel_cos (double,double);
extern double __kernel_tan (double,double,int);
extern int __kernel_rem_pio2 (double*,double*,int,int,int,const __int32_t*);





extern float scalbf (float, float);

extern float significandf (float);


extern float __ieee754_sqrtf (float);
extern float __ieee754_acosf (float);
extern float __ieee754_acoshf (float);
extern float __ieee754_logf (float);
extern float __ieee754_atanhf (float);
extern float __ieee754_asinf (float);
extern float __ieee754_atan2f (float,float);
extern float __ieee754_expf (float);
extern float __ieee754_coshf (float);
extern float __ieee754_fmodf (float,float);
extern float __ieee754_powf (float,float);
extern float __ieee754_lgammaf_r (float,int *);
extern float __ieee754_gammaf_r (float,int *);
extern float __ieee754_log10f (float);
extern float __ieee754_sinhf (float);
extern float __ieee754_hypotf (float,float);
extern float __ieee754_j0f (float);
extern float __ieee754_j1f (float);
extern float __ieee754_y0f (float);
extern float __ieee754_y1f (float);
extern float __ieee754_jnf (int,float);
extern float __ieee754_ynf (int,float);
extern float __ieee754_remainderf (float,float);
extern __int32_t __ieee754_rem_pio2f (float,float*);



extern float __ieee754_scalbf (float,float);
extern float __kernel_sinf (float,float,int);
extern float __kernel_cosf (float,float);
extern float __kernel_tanf (float,float,int);
extern int __kernel_rem_pio2f (float*,float*,int,int,int,const __int32_t*);
typedef union
{
  double value;
  struct
  {
    __uint32_t lsw;
    __uint32_t msw;
  } parts;
} ieee_double_shape_type;
typedef union
{
  float value;
  __uint32_t word;
} ieee_float_shape_type;




static double pzero(double), qzero(double);





static const double



huge = 1e300,
one = 1.0,
invsqrtpi= 5.64189583547756279280e-01,
tpi = 6.36619772367581382433e-01,

R02 = 1.56249999999999947958e-02,
R03 = -1.89979294238854721751e-04,
R04 = 1.82954049532700665670e-06,
R05 = -4.61832688532103189199e-09,
S01 = 1.56191029464890010492e-02,
S02 = 1.16926784663337450260e-04,
S03 = 5.13546550207318111446e-07,
S04 = 1.16614003333790000205e-09;


static const double zero = 0.0;





 double __ieee754_j0(double x)




{
 double z, s,c,ss,cc,r,u,v;
 __int32_t hx,ix;

 do { ieee_double_shape_type gh_u; gh_u.value = (x); (hx) = gh_u.parts.msw; } while (0);
 ix = hx&0x7fffffff;
 if(ix>=0x7ff00000) return one/(x*x);
 x = fabs(x);
 if(ix >= 0x40000000) {
  s = sin(x);
  c = cos(x);
  ss = s-c;
  cc = s+c;
  if(ix<0x7fe00000) {
      z = -cos(x+x);
      if ((s*c)<zero) cc = z/ss;
      else ss = z/cc;
  }




  if(ix>0x48000000) z = (invsqrtpi*cc)/__ieee754_sqrt(x);
  else {
      u = pzero(x); v = qzero(x);
      z = invsqrtpi*(u*cc-v*ss)/__ieee754_sqrt(x);
  }
  return z;
 }
 if(ix<0x3f200000) {
     if(huge+x>one) {
         if(ix<0x3e400000) return one;
         else return one - 0.25*x*x;
     }
 }
 z = x*x;
 r = z*(R02+z*(R03+z*(R04+z*R05)));
 s = one+z*(S01+z*(S02+z*(S03+z*S04)));
 if(ix < 0x3FF00000) {
     return one + z*(-0.25+(r/s));
 } else {
     u = 0.5*x;
     return((one+u)*(one-u)+z*(r/s));
 }
}


static const double



u00 = -7.38042951086872317523e-02,
u01 = 1.76666452509181115538e-01,
u02 = -1.38185671945596898896e-02,
u03 = 3.47453432093683650238e-04,
u04 = -3.81407053724364161125e-06,
u05 = 1.95590137035022920206e-08,
u06 = -3.98205194132103398453e-11,
v01 = 1.27304834834123699328e-02,
v02 = 7.60068627350353253702e-05,
v03 = 2.59150851840457805467e-07,
v04 = 4.41110311332675467403e-10;


 double __ieee754_y0(double x)




{
 double z, s,c,ss,cc,u,v;
 __int32_t hx,ix,lx;

 do { ieee_double_shape_type ew_u; ew_u.value = (x); (hx) = ew_u.parts.msw; (lx) = ew_u.parts.lsw; } while (0);
        ix = 0x7fffffff&hx;

 if(ix>=0x7ff00000) return one/(x+x*x);
        if((ix|lx)==0) return -one/zero;
        if(hx<0) return zero/zero;
        if(ix >= 0x40000000) {
                s = sin(x);
                c = cos(x);
                ss = s-c;
                cc = s+c;




                if(ix<0x7fe00000) {
                    z = -cos(x+x);
                    if ((s*c)<zero) cc = z/ss;
                    else ss = z/cc;
                }
                if(ix>0x48000000) z = (invsqrtpi*ss)/__ieee754_sqrt(x);
                else {
                    u = pzero(x); v = qzero(x);
                    z = invsqrtpi*(u*ss+v*cc)/__ieee754_sqrt(x);
                }
                return z;
 }
 if(ix<=0x3e400000) {
     return(u00 + tpi*__ieee754_log(x));
 }
 z = x*x;
 u = u00+z*(u01+z*(u02+z*(u03+z*(u04+z*(u05+z*u06)))));
 v = one+z*(v01+z*(v02+z*(v03+z*v04)));
 return(u/v + tpi*(__ieee754_j0(x)*__ieee754_log(x)));
}
static const double pR8[6] = {



  0.00000000000000000000e+00,
 -7.03124999999900357484e-02,
 -8.08167041275349795626e+00,
 -2.57063105679704847262e+02,
 -2.48521641009428822144e+03,
 -5.25304380490729545272e+03,
};

static const double pS8[5] = {



  1.16534364619668181717e+02,
  3.83374475364121826715e+03,
  4.05978572648472545552e+04,
  1.16752972564375915681e+05,
  4.76277284146730962675e+04,
};


static const double pR5[6] = {



 -1.14125464691894502584e-11,
 -7.03124940873599280078e-02,
 -4.15961064470587782438e+00,
 -6.76747652265167261021e+01,
 -3.31231299649172967747e+02,
 -3.46433388365604912451e+02,
};

static const double pS5[5] = {



  6.07539382692300335975e+01,
  1.05125230595704579173e+03,
  5.97897094333855784498e+03,
  9.62544514357774460223e+03,
  2.40605815922939109441e+03,
};


static const double pR3[6] = {



 -2.54704601771951915620e-09,
 -7.03119616381481654654e-02,
 -2.40903221549529611423e+00,
 -2.19659774734883086467e+01,
 -5.80791704701737572236e+01,
 -3.14479470594888503854e+01,
};

static const double pS3[5] = {



  3.58560338055209726349e+01,
  3.61513983050303863820e+02,
  1.19360783792111533330e+03,
  1.12799679856907414432e+03,
  1.73580930813335754692e+02,
};


static const double pR2[6] = {



 -8.87534333032526411254e-08,
 -7.03030995483624743247e-02,
 -1.45073846780952986357e+00,
 -7.63569613823527770791e+00,
 -1.11931668860356747786e+01,
 -3.23364579351335335033e+00,
};

static const double pS2[5] = {



  2.22202997532088808441e+01,
  1.36206794218215208048e+02,
  2.70470278658083486789e+02,
  1.53875394208320329881e+02,
  1.46576176948256193810e+01,
};


 static double pzero(double x)




{

 const double *p,*q;



 double z,r,s;
 __int32_t ix;
 do { ieee_double_shape_type gh_u; gh_u.value = (x); (ix) = gh_u.parts.msw; } while (0);
 ix &= 0x7fffffff;
 if(ix>=0x40200000) {p = pR8; q= pS8;}
 else if(ix>=0x40122E8B){p = pR5; q= pS5;}
 else if(ix>=0x4006DB6D){p = pR3; q= pS3;}
      else {p = pR2; q= pS2;}
 z = one/(x*x);
 r = p[0]+z*(p[1]+z*(p[2]+z*(p[3]+z*(p[4]+z*p[5]))));
 s = one+z*(q[0]+z*(q[1]+z*(q[2]+z*(q[3]+z*q[4]))));
 return one+ r/s;
}
static const double qR8[6] = {



  0.00000000000000000000e+00,
  7.32421874999935051953e-02,
  1.17682064682252693899e+01,
  5.57673380256401856059e+02,
  8.85919720756468632317e+03,
  3.70146267776887834771e+04,
};

static const double qS8[6] = {



  1.63776026895689824414e+02,
  8.09834494656449805916e+03,
  1.42538291419120476348e+05,
  8.03309257119514397345e+05,
  8.40501579819060512818e+05,
 -3.43899293537866615225e+05,
};


static const double qR5[6] = {



  1.84085963594515531381e-11,
  7.32421766612684765896e-02,
  5.83563508962056953777e+00,
  1.35111577286449829671e+02,
  1.02724376596164097464e+03,
  1.98997785864605384631e+03,
};

static const double qS5[6] = {



  8.27766102236537761883e+01,
  2.07781416421392987104e+03,
  1.88472887785718085070e+04,
  5.67511122894947329769e+04,
  3.59767538425114471465e+04,
 -5.35434275601944773371e+03,
};


static const double qR3[6] = {



  4.37741014089738620906e-09,
  7.32411180042911447163e-02,
  3.34423137516170720929e+00,
  4.26218440745412650017e+01,
  1.70808091340565596283e+02,
  1.66733948696651168575e+02,
};

static const double qS3[6] = {



  4.87588729724587182091e+01,
  7.09689221056606015736e+02,
  3.70414822620111362994e+03,
  6.46042516752568917582e+03,
  2.51633368920368957333e+03,
 -1.49247451836156386662e+02,
};


static const double qR2[6] = {



  1.50444444886983272379e-07,
  7.32234265963079278272e-02,
  1.99819174093815998816e+00,
  1.44956029347885735348e+01,
  3.16662317504781540833e+01,
  1.62527075710929267416e+01,
};

static const double qS2[6] = {



  3.03655848355219184498e+01,
  2.69348118608049844624e+02,
  8.44783757595320139444e+02,
  8.82935845112488550512e+02,
  2.12666388511798828631e+02,
 -5.31095493882666946917e+00,
};


 static double qzero(double x)




{

 const double *p,*q;



 double s,r,z;
 __int32_t ix;
 do { ieee_double_shape_type gh_u; gh_u.value = (x); (ix) = gh_u.parts.msw; } while (0);
 ix &= 0x7fffffff;
 if(ix>=0x40200000) {p = qR8; q= qS8;}
 else if(ix>=0x40122E8B){p = qR5; q= qS5;}
 else if(ix>=0x4006DB6D){p = qR3; q= qS3;}
      else {p = qR2; q= qS2;}
 z = one/(x*x);
 r = p[0]+z*(p[1]+z*(p[2]+z*(p[3]+z*(p[4]+z*p[5]))));
 s = one+z*(q[0]+z*(q[1]+z*(q[2]+z*(q[3]+z*(q[4]+z*q[5])))));
 return (-.125 + r/s)/x;
}
