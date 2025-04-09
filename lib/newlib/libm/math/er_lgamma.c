







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


static const double



two52= 4.50359962737049600000e+15,
half= 5.00000000000000000000e-01,
one = 1.00000000000000000000e+00,
pi = 3.14159265358979311600e+00,
a0 = 7.72156649015328655494e-02,
a1 = 3.22467033424113591611e-01,
a2 = 6.73523010531292681824e-02,
a3 = 2.05808084325167332806e-02,
a4 = 7.38555086081402883957e-03,
a5 = 2.89051383673415629091e-03,
a6 = 1.19270763183362067845e-03,
a7 = 5.10069792153511336608e-04,
a8 = 2.20862790713908385557e-04,
a9 = 1.08011567247583939954e-04,
a10 = 2.52144565451257326939e-05,
a11 = 4.48640949618915160150e-05,
tc = 1.46163214496836224576e+00,
tf = -1.21486290535849611461e-01,

tt = -3.63867699703950536541e-18,
t0 = 4.83836122723810047042e-01,
t1 = -1.47587722994593911752e-01,
t2 = 6.46249402391333854778e-02,
t3 = -3.27885410759859649565e-02,
t4 = 1.79706750811820387126e-02,
t5 = -1.03142241298341437450e-02,
t6 = 6.10053870246291332635e-03,
t7 = -3.68452016781138256760e-03,
t8 = 2.25964780900612472250e-03,
t9 = -1.40346469989232843813e-03,
t10 = 8.81081882437654011382e-04,
t11 = -5.38595305356740546715e-04,
t12 = 3.15632070903625950361e-04,
t13 = -3.12754168375120860518e-04,
t14 = 3.35529192635519073543e-04,
u0 = -7.72156649015328655494e-02,
u1 = 6.32827064025093366517e-01,
u2 = 1.45492250137234768737e+00,
u3 = 9.77717527963372745603e-01,
u4 = 2.28963728064692451092e-01,
u5 = 1.33810918536787660377e-02,
v1 = 2.45597793713041134822e+00,
v2 = 2.12848976379893395361e+00,
v3 = 7.69285150456672783825e-01,
v4 = 1.04222645593369134254e-01,
v5 = 3.21709242282423911810e-03,
s0 = -7.72156649015328655494e-02,
s1 = 2.14982415960608852501e-01,
s2 = 3.25778796408930981787e-01,
s3 = 1.46350472652464452805e-01,
s4 = 2.66422703033638609560e-02,
s5 = 1.84028451407337715652e-03,
s6 = 3.19475326584100867617e-05,
r1 = 1.39200533467621045958e+00,
r2 = 7.21935547567138069525e-01,
r3 = 1.71933865632803078993e-01,
r4 = 1.86459191715652901344e-02,
r5 = 7.77942496381893596434e-04,
r6 = 7.32668430744625636189e-06,
w0 = 4.18938533204672725052e-01,
w1 = 8.33333333333329678849e-02,
w2 = -2.77777777728775536470e-03,
w3 = 7.93650558643019558500e-04,
w4 = -5.95187557450339963135e-04,
w5 = 8.36339918996282139126e-04,
w6 = -1.63092934096575273989e-03;


static const double zero= 0.00000000000000000000e+00;





 static double sin_pi(double x)




{
 double y,z;
 __int32_t n,ix;

 do { ieee_double_shape_type gh_u; gh_u.value = (x); (ix) = gh_u.parts.msw; } while (0);
 ix &= 0x7fffffff;

 if(ix<0x3fd00000) return __kernel_sin(pi*x,zero,0);
 y = -x;





 z = floor(y);
 if(z!=y) {
     y *= 0.5;
     y = 2.0*(y - floor(y));
     n = (__int32_t) (y*4.0);
 } else {
            if(ix>=0x43400000) {
                y = zero; n = 0;
            } else {
                if(ix<0x43300000) z = y+two52;
  do { ieee_double_shape_type gl_u; gl_u.value = (z); (n) = gl_u.parts.lsw; } while (0);
  n &= 1;
                y = n;
                n<<= 2;
            }
        }
 switch (n) {
     case 0: y = __kernel_sin(pi*y,zero,0); break;
     case 1:
     case 2: y = __kernel_cos(pi*(0.5-y),zero); break;
     case 3:
     case 4: y = __kernel_sin(pi*(one-y),zero,0); break;
     case 5:
     case 6: y = -__kernel_cos(pi*(y-1.5),zero); break;
     default: y = __kernel_sin(pi*(y-2.0),zero,0); break;
     }
 return -y;
}



 double __ieee754_lgamma_r(double x, int *signgamp)




{
 double t,y,z,nadj = 0.0,p,p1,p2,p3,q,r,w;
 __int32_t i,hx,lx,ix;

 do { ieee_double_shape_type ew_u; ew_u.value = (x); (hx) = ew_u.parts.msw; (lx) = ew_u.parts.lsw; } while (0);


 *signgamp = 1;
 ix = hx&0x7fffffff;
 if(ix>=0x7ff00000) return x*x;
 if((ix|lx)==0) return one/zero;
 if(ix<0x3b900000) {
     if(hx<0) {
         *signgamp = -1;
         return -__ieee754_log(-x);
     } else return -__ieee754_log(x);
 }
 if(hx<0) {
     if(ix>=0x43300000)
  return one/zero;
     t = sin_pi(x);
     if(t==zero) return one/zero;
     nadj = __ieee754_log(pi/fabs(t*x));
     if(t<zero) *signgamp = -1;
     x = -x;
 }


 if((((ix-0x3ff00000)|lx)==0)||(((ix-0x40000000)|lx)==0)) r = 0;

 else if(ix<0x40000000) {
     if(ix<=0x3feccccc) {
  r = -__ieee754_log(x);
  if(ix>=0x3FE76944) {y = one-x; i= 0;}
  else if(ix>=0x3FCDA661) {y= x-(tc-one); i=1;}
    else {y = x; i=2;}
     } else {
    r = zero;
         if(ix>=0x3FFBB4C3) {y=2.0-x;i=0;}
         else if(ix>=0x3FF3B4C4) {y=x-tc;i=1;}
  else {y=x-one;i=2;}
     }
     switch(i) {
       case 0:
  z = y*y;
  p1 = a0+z*(a2+z*(a4+z*(a6+z*(a8+z*a10))));
  p2 = z*(a1+z*(a3+z*(a5+z*(a7+z*(a9+z*a11)))));
  p = y*p1+p2;
  r += (p-0.5*y); break;
       case 1:
  z = y*y;
  w = z*y;
  p1 = t0+w*(t3+w*(t6+w*(t9 +w*t12)));
  p2 = t1+w*(t4+w*(t7+w*(t10+w*t13)));
  p3 = t2+w*(t5+w*(t8+w*(t11+w*t14)));
  p = z*p1-(tt-w*(p2+y*p3));
  r += (tf + p); break;
       case 2:
  p1 = y*(u0+y*(u1+y*(u2+y*(u3+y*(u4+y*u5)))));
  p2 = one+y*(v1+y*(v2+y*(v3+y*(v4+y*v5))));
  r += (-0.5*y + p1/p2);
     }
 }
 else if(ix<0x40200000) {
     i = (__int32_t)x;
     t = zero;
     y = x-(double)i;
     p = y*(s0+y*(s1+y*(s2+y*(s3+y*(s4+y*(s5+y*s6))))));
     q = one+y*(r1+y*(r2+y*(r3+y*(r4+y*(r5+y*r6)))));
     r = half*y+p/q;
     z = one;
     switch(i) {
     case 7: z *= (y+6.0);
     case 6: z *= (y+5.0);
     case 5: z *= (y+4.0);
     case 4: z *= (y+3.0);
     case 3: z *= (y+2.0);
      r += __ieee754_log(z); break;
     }

 } else if (ix < 0x43900000) {
     t = __ieee754_log(x);
     z = one/x;
     y = z*z;
     w = w0+z*(w1+y*(w2+y*(w3+y*(w4+y*(w5+y*w6)))));
     r = (x-half)*(t-one)+w;
 } else

     r = x*(__ieee754_log(x)-one);
 if(hx<0) r = nadj - r;
 return r;
}
