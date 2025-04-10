
typedef long int          ptrdiff_t;
typedef long unsigned int size_t;
typedef int               wchar_t;
typedef struct
{
    long long   __max_align_ll __attribute__((__aligned__(__alignof__(long long))));
    long double __max_align_ld __attribute__((__aligned__(__alignof__(long double))));
} max_align_t;
typedef signed char             __int8_t;
typedef unsigned char           __uint8_t;
typedef short int               __int16_t;
typedef short unsigned int      __uint16_t;
typedef int                     __int32_t;
typedef unsigned int            __uint32_t;
typedef long int                __int64_t;
typedef long unsigned int       __uint64_t;
typedef signed char             __int_least8_t;
typedef unsigned char           __uint_least8_t;
typedef short int               __int_least16_t;
typedef short unsigned int      __uint_least16_t;
typedef int                     __int_least32_t;
typedef unsigned int            __uint_least32_t;
typedef long int                __int_least64_t;
typedef long unsigned int       __uint_least64_t;
typedef long int                __intmax_t;
typedef long unsigned int       __uintmax_t;
typedef long int                __intptr_t;
typedef long unsigned int       __uintptr_t;
typedef unsigned long           __blkcnt_t;
typedef unsigned long           __blksize_t;
typedef unsigned long           __fsblkcnt_t;
typedef unsigned int            __fsfilcnt_t;
typedef long                    _off_t;
typedef int                     __pid_t;
typedef int                     __dev_t;
typedef unsigned long           __uid_t;
typedef unsigned long           __gid_t;
typedef unsigned int            __id_t;
typedef unsigned long           __ino_t;
typedef unsigned int            __mode_t;
__extension__ typedef long long _off64_t;
typedef _off_t                  __off_t;
typedef _off64_t                __loff_t;
typedef long                    __key_t;
typedef long                    _fpos_t;
typedef unsigned long;
typedef signed long _ssize_t;
struct __lock;
typedef struct __lock* _flock_t;
typedef void*          _iconv_t;
typedef unsigned long  __clock_t;
typedef long           __time_t;
typedef unsigned long  __clockid_t;
typedef unsigned long  __timer_t;
typedef unsigned char  __sa_family_t;
typedef unsigned int   __socklen_t;
typedef int            __nl_item;
typedef unsigned long  __nlink_t;
typedef long           __suseconds_t;
typedef unsigned long  __useconds_t;
typedef char*          __va_list;
typedef unsigned int   wint_t;
typedef struct
{
    int __count;
    union
    {
        unsigned int  __wch;
        unsigned char __wchb[4];
    } __value;
} _mbstate_t;
typedef unsigned int __ULong;
struct _reent;
struct __locale_t;
struct _Bigint
{
    struct _Bigint* _next;
    int             _k, _maxwds, _sign, _wds;
    __ULong         _x[1];
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
struct _on_exit_args
{
    void*   _fnargs[32];
    void*   _dso_handle[32];
    __ULong _fntypes;
    __ULong _is_cxa;
};
struct _atexit
{
    struct _atexit* _next;
    int             _ind;
    void (*_fns[32])(void);
    struct _on_exit_args _on_exit_args;
};
struct __sbuf
{
    unsigned char* _base;
    int            _size;
};
struct __sFILE
{
    unsigned char* _p;
    int            _r;
    int            _w;
    short          _flags;
    short          _file;
    struct __sbuf  _bf;
    int            _lbfsize;
    void*          _cookie;
    int (*_read)(struct _reent*, void*, char*, int);
    int (*_write)(struct _reent*, void*, const char*, int);
    _fpos_t (*_seek)(struct _reent*, void*, _fpos_t, int);
    int (*_close)(struct _reent*, void*);
    struct __sbuf  _ub;
    unsigned char* _up;
    int            _ur;
    unsigned char  _ubuf[3];
    unsigned char  _nbuf[1];
    struct __sbuf  _lb;
    int            _blksize;
    _off_t         _offset;
    struct _reent* _data;
    _flock_t       _lock;
    _mbstate_t     _mbstate;
    int            _flags2;
};
typedef struct __sFILE __FILE;
struct _glue
{
    struct _glue* _next;
    int           _niobs;
    __FILE*       _iobs;
};
struct _rand48
{
    unsigned short _seed[3];
    unsigned short _mult[3];
    unsigned short _add;
};
struct _reent
{
    int                _errno;
    __FILE *           _stdin, *_stdout, *_stderr;
    int                _inc;
    char               _emergency[25];
    int                _unspecified_locale_info;
    struct __locale_t* _locale;
    int                __sdidinit;
    void (*__cleanup)(struct _reent*);
    struct _Bigint*  _result;
    int              _result_k;
    struct _Bigint*  _p5s;
    struct _Bigint** _freelist;
    int              _cvtlen;
    char*            _cvtbuf;
    union
    {
        struct
        {
            unsigned int                     _unused_rand;
            char*                            _strtok_last;
            char                             _asctime_buf[26];
            struct __tm                      _localtime_buf;
            int                              _gamma_signgam;
            __extension__ unsigned long long _rand_next;
            struct _rand48                   _r48;
            _mbstate_t                       _mblen_state;
            _mbstate_t                       _mbtowc_state;
            _mbstate_t                       _wctomb_state;
            char                             _l64a_buf[8];
            char                             _signal_buf[24];
            int                              _getdate_err;
            _mbstate_t                       _mbrlen_state;
            _mbstate_t                       _mbrtowc_state;
            _mbstate_t                       _mbsrtowcs_state;
            _mbstate_t                       _wcrtomb_state;
            _mbstate_t                       _wcsrtombs_state;
            int                              _h_errno;
        } _reent;
        struct
        {
            unsigned char* _nextf[30];
            unsigned int   _nmalloc[30];
        } _unused;
    } _new;
    struct _atexit* _atexit;
    struct _atexit  _atexit0;
    void (**(_sig_func))(int);
    struct _glue __sglue;
    __FILE       __sf[3];
};
extern struct _reent*       _impure_ptr;
extern struct _reent* const _global_impure_ptr;
void                        _reclaim_reent(struct _reent*);
extern double               atan(double);
extern double               cos(double);
extern double               sin(double);
extern double               tan(double);
extern double               tanh(double);
extern double               frexp(double, int*);
extern double               modf(double, double*);
extern double               ceil(double);
extern double               fabs(double);
extern double               floor(double);
extern double               acos(double);
extern double               asin(double);
extern double               atan2(double, double);
extern double               cosh(double);
extern double               sinh(double);
extern double               exp(double);
extern double               ldexp(double, int);
extern double               log(double);
extern double               log10(double);
extern double               pow(double, double);
extern double               sqrt(double);
extern double               fmod(double, double);
extern int                  finite(double);
extern int                  finitef(float);
extern int                  finitel(long double);
extern int                  isinff(float);
extern int                  isnanf(float);
extern int                  isinf(double);
extern int                  isnan(double);
typedef float               float_t;
typedef double              double_t;
extern int                  __isinff(float x);
extern int                  __isinfd(double x);
extern int                  __isnanf(float x);
extern int                  __isnand(double x);
extern int                  __fpclassifyf(float x);
extern int                  __fpclassifyd(double x);
extern int                  __signbitf(float x);
extern int                  __signbitd(double x);
extern double               infinity(void);
extern double               nan(const char*);
extern double               copysign(double, double);
extern double               logb(double);
extern int                  ilogb(double);
extern double               asinh(double);
extern double               cbrt(double);
extern double               nextafter(double, double);
extern double               rint(double);
extern double               scalbn(double, int);
extern double               exp2(double);
extern double               scalbln(double, long int);
extern double               tgamma(double);
extern double               nearbyint(double);
extern long int             lrint(double);
extern long long int        llrint(double);
extern double               round(double);
extern long int             lround(double);
extern long long int        llround(double);
extern double               trunc(double);
extern double               remquo(double, double, int*);
extern double               fdim(double, double);
extern double               fmax(double, double);
extern double               fmin(double, double);
extern double               fma(double, double, double);
extern double               log1p(double);
extern double               expm1(double);
extern double               acosh(double);
extern double               atanh(double);
extern double               remainder(double, double);
extern double               gamma(double);
extern double               lgamma(double);
extern double               erf(double);
extern double               erfc(double);
extern double               log2(double);
extern double               hypot(double, double);
extern float                atanf(float);
extern float                cosf(float);
extern float                sinf(float);
extern float                tanf(float);
extern float                tanhf(float);
extern float                frexpf(float, int*);
extern float                modff(float, float*);
extern float                ceilf(float);
extern float                fabsf(float);
extern float                floorf(float);
extern float                acosf(float);
extern float                asinf(float);
extern float                atan2f(float, float);
extern float                coshf(float);
extern float                sinhf(float);
extern float                expf(float);
extern float                ldexpf(float, int);
extern float                logf(float);
extern float                log10f(float);
extern float                powf(float, float);
extern float                sqrtf(float);
extern float                fmodf(float, float);
extern float                exp2f(float);
extern float                scalblnf(float, long int);
extern float                tgammaf(float);
extern float                nearbyintf(float);
extern long int             lrintf(float);
extern long long int        llrintf(float);
extern float                roundf(float);
extern long int             lroundf(float);
extern long long int        llroundf(float);
extern float                truncf(float);
extern float                remquof(float, float, int*);
extern float                fdimf(float, float);
extern float                fmaxf(float, float);
extern float                fminf(float, float);
extern float                fmaf(float, float, float);
extern float                infinityf(void);
extern float                nanf(const char*);
extern float                copysignf(float, float);
extern float                logbf(float);
extern int                  ilogbf(float);
extern float                asinhf(float);
extern float                cbrtf(float);
extern float                nextafterf(float, float);
extern float                rintf(float);
extern float                scalbnf(float, int);
extern float                log1pf(float);
extern float                expm1f(float);
extern float                acoshf(float);
extern float                atanhf(float);
extern float                remainderf(float, float);
extern float                gammaf(float);
extern float                lgammaf(float);
extern float                erff(float);
extern float                erfcf(float);
extern float                log2f(float);
extern float                hypotf(float, float);
extern long double          hypotl(long double, long double);
extern long double          sqrtl(long double);
extern double               drem(double, double);
extern float                dremf(float, float);
extern double               gamma_r(double, int*);
extern double               lgamma_r(double, int*);
extern float                gammaf_r(float, int*);
extern float                lgammaf_r(float, int*);
extern double               y0(double);
extern double               y1(double);
extern double               yn(int, double);
extern double               j0(double);
extern double               j1(double);
extern double               jn(int, double);
extern float                y0f(float);
extern float                y1f(float);
extern float                ynf(int, float);
extern float                j0f(float);
extern float                j1f(float);
extern float                jnf(int, float);
extern int*                 __signgam(void);
struct exception
{
    int    type;
    char*  name;
    double arg1;
    double arg2;
    double retval;
    int    err;
};
extern int matherr(struct exception* e);
enum __fdlibm_version
{
    __fdlibm_ieee = -1,
    __fdlibm_svid,
    __fdlibm_xopen,
    __fdlibm_posix
};
extern enum __fdlibm_version __fdlib_version;
typedef __uint8_t            u_int8_t;
typedef __uint16_t           u_int16_t;
typedef __uint32_t           u_int32_t;
typedef __uint64_t           u_int64_t;
typedef int                  register_t;
typedef __int8_t             int8_t;
typedef __uint8_t            uint8_t;
typedef __int16_t            int16_t;
typedef __uint16_t           uint16_t;
typedef __int32_t            int32_t;
typedef __uint32_t           uint32_t;
typedef __int64_t            int64_t;
typedef __uint64_t           uint64_t;
typedef __intmax_t           intmax_t;
typedef __uintmax_t          uintmax_t;
typedef __intptr_t           intptr_t;
typedef __uintptr_t          uintptr_t;
typedef unsigned long        __sigset_t;
typedef __suseconds_t        suseconds_t;
typedef long                 time_t;
struct timeval
{
    time_t      tv_sec;
    suseconds_t tv_usec;
};
struct timespec
{
    time_t tv_sec;
    long   tv_nsec;
};
struct itimerspec
{
    struct timespec it_interval;
    struct timespec it_value;
};
typedef __sigset_t    sigset_t;
typedef unsigned long fd_mask;
typedef struct _types_fd_set
{
    fd_mask fds_bits[(((64) + (((sizeof(fd_mask) * 8)) - 1)) / ((sizeof(fd_mask) * 8)))];
} _types_fd_set;
int                select(int __n, _types_fd_set* __readfds, _types_fd_set* __writefds, _types_fd_set* __exceptfds, struct timeval* __timeout);
int                pselect(int __n, _types_fd_set* __readfds, _types_fd_set* __writefds, _types_fd_set* __exceptfds, const struct timespec* __timeout,
                           const sigset_t* __set);
typedef __uint32_t in_addr_t;
typedef __uint16_t in_port_t;
typedef unsigned char  u_char;
typedef unsigned short u_short;
typedef unsigned int   u_int;
typedef unsigned long  u_long;
typedef unsigned short ushort;
typedef unsigned int   uint;
typedef unsigned long  ulong;
typedef __blkcnt_t     blkcnt_t;
typedef __blksize_t    blksize_t;
typedef unsigned long  clock_t;
typedef long           daddr_t;
typedef char*          caddr_t;
typedef __fsblkcnt_t   fsblkcnt_t;
typedef __fsfilcnt_t   fsfilcnt_t;
typedef __id_t         id_t;
typedef __ino_t        ino_t;
typedef __off_t        off_t;
typedef __dev_t        dev_t;
typedef __uid_t        uid_t;
typedef __gid_t        gid_t;
typedef __pid_t        pid_t;
typedef __key_t        key_t;
typedef _ssize_t       ssize_t;
typedef __mode_t       mode_t;
typedef __nlink_t      nlink_t;
typedef __clockid_t    clockid_t;
typedef __timer_t      timer_t;
typedef __useconds_t   useconds_t;
typedef __int64_t      sbintime_t;
struct sched_param
{
    int sched_priority;
};
typedef __uint32_t pthread_t;
typedef struct
{
    int                is_initialized;
    void*              stackaddr;
    int                stacksize;
    int                contentionscope;
    int                inheritsched;
    int                schedpolicy;
    struct sched_param schedparam;
    int                detachstate;
} pthread_attr_t;
typedef __uint32_t pthread_mutex_t;
typedef struct
{
    int is_initialized;
    int recursive;
} pthread_mutexattr_t;
typedef __uint32_t pthread_cond_t;
typedef struct
{
    int     is_initialized;
    clock_t clock;
} pthread_condattr_t;
typedef __uint32_t pthread_key_t;
typedef struct
{
    int is_initialized;
    int init_executed;
} pthread_once_t;
extern double      scalb(double, double);
extern double      significand(double);
extern long double __ieee754_hypotl(long double, long double);
extern double      __ieee754_sqrt(double);
extern double      __ieee754_acos(double);
extern double      __ieee754_acosh(double);
extern double      __ieee754_log(double);
extern double      __ieee754_atanh(double);
extern double      __ieee754_asin(double);
extern double      __ieee754_atan2(double, double);
extern double      __ieee754_exp(double);
extern double      __ieee754_cosh(double);
extern double      __ieee754_fmod(double, double);
extern double      __ieee754_pow(double, double);
extern double      __ieee754_lgamma_r(double, int*);
extern double      __ieee754_gamma_r(double, int*);
extern double      __ieee754_log10(double);
extern double      __ieee754_sinh(double);
extern double      __ieee754_hypot(double, double);
extern double      __ieee754_j0(double);
extern double      __ieee754_j1(double);
extern double      __ieee754_y0(double);
extern double      __ieee754_y1(double);
extern double      __ieee754_jn(int, double);
extern double      __ieee754_yn(int, double);
extern double      __ieee754_remainder(double, double);
extern __int32_t   __ieee754_rem_pio2(double, double*);
extern double      __ieee754_scalb(double, double);
extern double      __kernel_standard(double, double, int);
extern double      __kernel_sin(double, double, int);
extern double      __kernel_cos(double, double);
extern double      __kernel_tan(double, double, int);
extern int         __kernel_rem_pio2(double*, double*, int, int, int, const __int32_t*);
extern float       scalbf(float, float);
extern float       significandf(float);
extern float       __ieee754_sqrtf(float);
extern float       __ieee754_acosf(float);
extern float       __ieee754_acoshf(float);
extern float       __ieee754_logf(float);
extern float       __ieee754_atanhf(float);
extern float       __ieee754_asinf(float);
extern float       __ieee754_atan2f(float, float);
extern float       __ieee754_expf(float);
extern float       __ieee754_coshf(float);
extern float       __ieee754_fmodf(float, float);
extern float       __ieee754_powf(float, float);
extern float       __ieee754_lgammaf_r(float, int*);
extern float       __ieee754_gammaf_r(float, int*);
extern float       __ieee754_log10f(float);
extern float       __ieee754_sinhf(float);
extern float       __ieee754_hypotf(float, float);
extern float       __ieee754_j0f(float);
extern float       __ieee754_j1f(float);
extern float       __ieee754_y0f(float);
extern float       __ieee754_y1f(float);
extern float       __ieee754_jnf(int, float);
extern float       __ieee754_ynf(int, float);
extern float       __ieee754_remainderf(float, float);
extern __int32_t   __ieee754_rem_pio2f(float, float*);
extern float       __ieee754_scalbf(float, float);
extern float       __kernel_sinf(float, float, int);
extern float       __kernel_cosf(float, float);
extern float       __kernel_tanf(float, float, int);
extern int         __kernel_rem_pio2f(float*, float*, int, int, int, const __int32_t*);
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
    float      value;
    __uint32_t word;
} ieee_float_shape_type;
static float       pzerof(float), qzerof(float);
static const float huge = 1e30, one = 1.0, invsqrtpi = 5.6418961287e-01, tpi = 6.3661974669e-01, R02 = 1.5625000000e-02, R03 = -1.8997929874e-04,
                   R04 = 1.8295404516e-06, R05 = -4.6183270541e-09, S01 = 1.5619102865e-02, S02 = 1.1692678527e-04, S03 = 5.1354652442e-07,
                   S04  = 1.1661400734e-09;
static const float zero = 0.0;
float              __ieee754_j0f(float x)
{
    float     z, s, c, ss, cc, r, u, v;
    __int32_t hx, ix;
    do {
        ieee_float_shape_type gf_u;
        gf_u.value = (x);
        (hx)       = gf_u.word;
    } while(0);
    ix = hx & 0x7fffffff;
    if(!((ix) < 0x7f800000L)) return one / (x * x);
    x = fabsf(x);
    if(ix >= 0x40000000)
    {
        s  = sinf(x);
        c  = cosf(x);
        ss = s - c;
        cc = s + c;
        if(ix <= (0x7f7fffffL - (1L << 23)))
        {
            z = -cosf(x + x);
            if((s * c) < zero)
                cc = z / ss;
            else
                ss = z / cc;
        }
        if(ix > 0x80000000)
            z = (invsqrtpi * cc) / __ieee754_sqrtf(x);
        else
        {
            u = pzerof(x);
            v = qzerof(x);
            z = invsqrtpi * (u * cc - v * ss) / __ieee754_sqrtf(x);
        }
        return z;
    }
    if(ix < 0x39000000)
    {
        if(huge + x > one)
        {
            if(ix < 0x32000000)
                return one;
            else
                return one - (float)0.25 * x * x;
        }
    }
    z = x * x;
    r = z * (R02 + z * (R03 + z * (R04 + z * R05)));
    s = one + z * (S01 + z * (S02 + z * (S03 + z * S04)));
    if(ix < 0x3F800000) { return one + z * ((float)-0.25 + (r / s)); }
    else
    {
        u = (float)0.5 * x;
        return ((one + u) * (one - u) + z * (r / s));
    }
}
static const float u00 = -7.3804296553e-02, u01 = 1.7666645348e-01, u02 = -1.3818567619e-02, u03 = 3.4745343146e-04, u04 = -3.8140706238e-06,
                   u05 = 1.9559013964e-08, u06 = -3.9820518410e-11, v01 = 1.2730483897e-02, v02 = 7.6006865129e-05, v03 = 2.5915085189e-07,
                   v04 = 4.4111031494e-10;
float __ieee754_y0f(float x)
{
    float     z, s, c, ss, cc, u, v;
    __int32_t hx, ix;
    do {
        ieee_float_shape_type gf_u;
        gf_u.value = (x);
        (hx)       = gf_u.word;
    } while(0);
    ix = 0x7fffffff & hx;
    if(!((ix) < 0x7f800000L)) return one / (x + x * x);
    if(((ix) == 0)) return -one / zero;
    if(hx < 0) return zero / zero;
    if(ix >= 0x40000000)
    {
        s  = sinf(x);
        c  = cosf(x);
        ss = s - c;
        cc = s + c;
        if(ix <= (0x7f7fffffL - (1L << 23)))
        {
            z = -cosf(x + x);
            if((s * c) < zero)
                cc = z / ss;
            else
                ss = z / cc;
        }
        if(ix > 0x80000000)
            z = (invsqrtpi * ss) / __ieee754_sqrtf(x);
        else
        {
            u = pzerof(x);
            v = qzerof(x);
            z = invsqrtpi * (u * ss + v * cc) / __ieee754_sqrtf(x);
        }
        return z;
    }
    if(ix <= 0x32000000) { return (u00 + tpi * __ieee754_logf(x)); }
    z = x * x;
    u = u00 + z * (u01 + z * (u02 + z * (u03 + z * (u04 + z * (u05 + z * u06)))));
    v = one + z * (v01 + z * (v02 + z * (v03 + z * v04)));
    return (u / v + tpi * (__ieee754_j0f(x) * __ieee754_logf(x)));
}
static const float pR8[6] = {
    0.0000000000e+00, -7.0312500000e-02, -8.0816707611e+00, -2.5706311035e+02, -2.4852163086e+03, -5.2530439453e+03,
};
static const float pS8[5] = {
    1.1653436279e+02, 3.8337448730e+03, 4.0597855469e+04, 1.1675296875e+05, 4.7627726562e+04,
};
static const float pR5[6] = {
    -1.1412546255e-11, -7.0312492549e-02, -4.1596107483e+00, -6.7674766541e+01, -3.3123129272e+02, -3.4643338013e+02,
};
static const float pS5[5] = {
    6.0753936768e+01, 1.0512523193e+03, 5.9789707031e+03, 9.6254453125e+03, 2.4060581055e+03,
};
static const float pR3[6] = {
    -2.5470459075e-09, -7.0311963558e-02, -2.4090321064e+00, -2.1965976715e+01, -5.8079170227e+01, -3.1447946548e+01,
};
static const float pS3[5] = {
    3.5856033325e+01, 3.6151397705e+02, 1.1936077881e+03, 1.1279968262e+03, 1.7358093262e+02,
};
static const float pR2[6] = {
    -8.8753431271e-08, -7.0303097367e-02, -1.4507384300e+00, -7.6356959343e+00, -1.1193166733e+01, -3.2336456776e+00,
};
static const float pS2[5] = {
    2.2220300674e+01, 1.3620678711e+02, 2.7047027588e+02, 1.5387539673e+02, 1.4657617569e+01,
};
static float pzerof(float x)
{
    const float *p, *q;
    float        z, r, s;
    __int32_t    ix;
    do {
        ieee_float_shape_type gf_u;
        gf_u.value = (x);
        (ix)       = gf_u.word;
    } while(0);
    ix &= 0x7fffffff;
    if(ix >= 0x41000000)
    {
        p = pR8;
        q = pS8;
    }
    else if(ix >= 0x40f71c58)
    {
        p = pR5;
        q = pS5;
    }
    else if(ix >= 0x4036db68)
    {
        p = pR3;
        q = pS3;
    }
    else
    {
        p = pR2;
        q = pS2;
    }
    z = one / (x * x);
    r = p[0] + z * (p[1] + z * (p[2] + z * (p[3] + z * (p[4] + z * p[5]))));
    s = one + z * (q[0] + z * (q[1] + z * (q[2] + z * (q[3] + z * q[4]))));
    return one + r / s;
}
static const float qR8[6] = {
    0.0000000000e+00, 7.3242187500e-02, 1.1768206596e+01, 5.5767340088e+02, 8.8591972656e+03, 3.7014625000e+04,
};
static const float qS8[6] = {
    1.6377603149e+02, 8.0983447266e+03, 1.4253829688e+05, 8.0330925000e+05, 8.4050156250e+05, -3.4389928125e+05,
};
static const float qR5[6] = {
    1.8408595828e-11, 7.3242180049e-02, 5.8356351852e+00, 1.3511157227e+02, 1.0272437744e+03, 1.9899779053e+03,
};
static const float qS5[6] = {
    8.2776611328e+01, 2.0778142090e+03, 1.8847289062e+04, 5.6751113281e+04, 3.5976753906e+04, -5.3543427734e+03,
};
static const float qR3[6] = {
    4.3774099900e-09, 7.3241114616e-02, 3.3442313671e+00, 4.2621845245e+01, 1.7080809021e+02, 1.6673394775e+02,
};
static const float qS3[6] = {
    4.8758872986e+01, 7.0968920898e+02, 3.7041481934e+03, 6.4604252930e+03, 2.5163337402e+03, -1.4924745178e+02,
};
static const float qR2[6] = {
    1.5044444979e-07, 7.3223426938e-02, 1.9981917143e+00, 1.4495602608e+01, 3.1666231155e+01, 1.6252708435e+01,
};
static const float qS2[6] = {
    3.0365585327e+01, 2.6934811401e+02, 8.4478375244e+02, 8.8293585205e+02, 2.1266638184e+02, -5.3109550476e+00,
};
static float qzerof(float x)
{
    const float *p, *q;
    float        s, r, z;
    __int32_t    ix;
    do {
        ieee_float_shape_type gf_u;
        gf_u.value = (x);
        (ix)       = gf_u.word;
    } while(0);
    ix &= 0x7fffffff;
    if(ix >= 0x41000000)
    {
        p = qR8;
        q = qS8;
    }
    else if(ix >= 0x40f71c58)
    {
        p = qR5;
        q = qS5;
    }
    else if(ix >= 0x4036db68)
    {
        p = qR3;
        q = qS3;
    }
    else
    {
        p = qR2;
        q = qS2;
    }
    z = one / (x * x);
    r = p[0] + z * (p[1] + z * (p[2] + z * (p[3] + z * (p[4] + z * p[5]))));
    s = one + z * (q[0] + z * (q[1] + z * (q[2] + z * (q[3] + z * (q[4] + z * q[5])))));
    return (-(float).125 + r / s) / x;
}
