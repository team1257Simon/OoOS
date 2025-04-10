
typedef signed char        __int8_t;
typedef unsigned char      __uint8_t;
typedef short int          __int16_t;
typedef short unsigned int __uint16_t;
typedef int                __int32_t;
typedef unsigned int       __uint32_t;
typedef long int           __int64_t;
typedef long unsigned int  __uint64_t;
typedef signed char        __int_least8_t;
typedef unsigned char      __uint_least8_t;
typedef short int          __int_least16_t;
typedef short unsigned int __uint_least16_t;
typedef int                __int_least32_t;
typedef unsigned int       __uint_least32_t;
typedef long int           __int_least64_t;
typedef long unsigned int  __uint_least64_t;
typedef long int           __intmax_t;
typedef long unsigned int  __uintmax_t;
typedef long int           __intptr_t;
typedef long unsigned int  __uintptr_t;
typedef long int           ptrdiff_t;
typedef long unsigned int  size_t;
typedef int                wchar_t;
typedef struct
{
    long long   __max_align_ll __attribute__((__aligned__(__alignof__(long long))));
    long double __max_align_ld __attribute__((__aligned__(__alignof__(long double))));
} max_align_t;
struct flock
{
    short l_type;
    short l_whence;
    long  l_start;
    long  l_len;
    short l_pid;
    short l_xxx;
};
struct eflock
{
    short l_type;
    short l_whence;
    long  l_start;
    long  l_len;
    short l_pid;
    short l_xxx;
    long  l_rpid;
    long  l_rsys;
};
typedef __uint8_t               u_int8_t;
typedef __uint16_t              u_int16_t;
typedef __uint32_t              u_int32_t;
typedef __uint64_t              u_int64_t;
typedef int                     register_t;
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
typedef struct __lock*    _flock_t;
typedef void*             _iconv_t;
typedef unsigned long     __clock_t;
typedef long              __time_t;
typedef unsigned long     __clockid_t;
typedef unsigned long     __timer_t;
typedef unsigned char     __sa_family_t;
typedef unsigned int      __socklen_t;
typedef int               __nl_item;
typedef unsigned long     __nlink_t;
typedef long              __suseconds_t;
typedef unsigned long     __useconds_t;
typedef __builtin_va_list __va_list;
typedef unsigned int      wint_t;
typedef struct
{
    int __count;
    union
    {
        unsigned int  __wch;
        unsigned char __wchb[4];
    } __value;
} _mbstate_t;
typedef __int8_t      int8_t;
typedef __uint8_t     uint8_t;
typedef __int16_t     int16_t;
typedef __uint16_t    uint16_t;
typedef __int32_t     int32_t;
typedef __uint32_t    uint32_t;
typedef __int64_t     int64_t;
typedef __uint64_t    uint64_t;
typedef __intmax_t    intmax_t;
typedef __uintmax_t   uintmax_t;
typedef __intptr_t    intptr_t;
typedef __uintptr_t   uintptr_t;
typedef unsigned long __sigset_t;
typedef __suseconds_t suseconds_t;
typedef long          time_t;
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
struct __locale_t;
typedef struct __locale_t* locale_t;
struct tm
{
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
};
clock_t       clock(void);
double        difftime(time_t _time2, time_t _time1);
time_t        mktime(struct tm* _timeptr);
time_t        time(time_t* _timer);
char*         asctime(const struct tm* _tblock);
char*         ctime(const time_t* _time);
struct tm*    gmtime(const time_t* _timer);
struct tm*    localtime(const time_t* _timer);
size_t        strftime(char* restrict _s, size_t _maxsize, const char* restrict _fmt, const struct tm* restrict _t);
extern size_t strftime_l(char* restrict _s, size_t _maxsize, const char* restrict _fmt, const struct tm* restrict _t, locale_t _l);
char*         asctime_r(const struct tm* restrict, char* restrict);
char*         ctime_r(const time_t*, char*);
struct tm*    gmtime_r(const time_t* restrict, struct tm* restrict);
struct tm*    localtime_r(const time_t* restrict, struct tm* restrict);
void          tzset(void);
void          _tzset_r(struct _reent*);
typedef struct __tzrule_struct
{
    char   ch;
    int    m;
    int    n;
    int    d;
    int    s;
    time_t change;
    long   offset;
} __tzrule_type;
typedef struct __tzinfo_struct
{
    int           __tznorth;
    int           __tzyear;
    __tzrule_type __tzrule[2];
} __tzinfo_type;
__tzinfo_type* __gettzinfo(void);
extern long    _timezone;
extern int     _daylight;
extern char*   _tzname[2];
struct stat
{
    dev_t     st_dev;
    ino_t     st_ino;
    mode_t    st_mode;
    nlink_t   st_nlink;
    uid_t     st_uid;
    gid_t     st_gid;
    dev_t     st_rdev;
    off_t     st_size;
    time_t    st_atime;
    long      st_spare1;
    time_t    st_mtime;
    long      st_spare2;
    time_t    st_ctime;
    long      st_spare3;
    blksize_t st_blksize;
    blkcnt_t  st_blocks;
    long      st_spare4[2];
};
int        chmod(const char* __path, mode_t __mode);
int        fchmod(int __fd, mode_t __mode);
int        fstat(int __fd, struct stat* __sbuf);
int        mkdir(const char* _path, mode_t __mode);
int        mkfifo(const char* __path, mode_t __mode);
int        stat(const char* restrict __path, struct stat* restrict __sbuf);
mode_t     umask(mode_t __mask);
int        fchmodat(int, const char*, mode_t, int);
int        fstatat(int, const char* restrict, struct stat* restrict, int);
int        mkdirat(int, const char*, mode_t);
int        mkfifoat(int, const char*, mode_t);
int        mknodat(int, const char*, mode_t, dev_t);
int        utimensat(int, const char*, const struct timespec*, int);
int        futimens(int, const struct timespec*);
extern int open(const char*, int, ...);
extern int openat(int, const char*, int, ...);
extern int creat(const char*, mode_t);
extern int fcntl(int, int, ...);
extern int flock(int, int);
union sigval
{
    int   sival_int;
    void* sival_ptr;
};
struct sigevent
{
    int          sigev_notify;
    int          sigev_signo;
    union sigval sigev_value;
};
typedef struct
{
    int          si_signo;
    int          si_code;
    union sigval si_value;
} siginfo_t;
typedef void (*_sig_func_ptr)(int);
struct sigaction
{
    _sig_func_ptr sa_handler;
    sigset_t      sa_mask;
    int           sa_flags;
};
typedef struct sigaltstack
{
    void*  ss_sp;
    int    ss_flags;
    size_t ss_size;
} stack_t;
int                   sigprocmask(int, const sigset_t*, sigset_t*);
int                   pthread_sigmask(int, const sigset_t*, sigset_t*);
int                   kill(pid_t, int);
int                   killpg(pid_t, int);
int                   sigaction(int, const struct sigaction*, struct sigaction*);
int                   sigaddset(sigset_t*, const int);
int                   sigdelset(sigset_t*, const int);
int                   sigismember(const sigset_t*, int);
int                   sigfillset(sigset_t*);
int                   sigemptyset(sigset_t*);
int                   sigpending(sigset_t*);
int                   sigsuspend(const sigset_t*);
int                   sigwait(const sigset_t*, int*);
int                   sigpause(int);
int                   sigaltstack(const stack_t* restrict, stack_t* restrict);
int                   pthread_kill(pthread_t, int);
int                   sigwaitinfo(const sigset_t*, siginfo_t*);
int                   sigtimedwait(const sigset_t*, siginfo_t*, const struct timespec*);
int                   sigqueue(pid_t, int, const union sigval);
typedef int           sig_atomic_t;
typedef _sig_func_ptr sig_t;
struct _reent;
_sig_func_ptr             _signal_r(struct _reent*, int, _sig_func_ptr);
int                       _raise_r(struct _reent*, int);
_sig_func_ptr             signal(int, _sig_func_ptr);
int                       raise(int);
void                      psignal(int, const char*);
typedef __int_least8_t    int_least8_t;
typedef __uint_least8_t   uint_least8_t;
typedef __int_least16_t   int_least16_t;
typedef __uint_least16_t  uint_least16_t;
typedef __int_least32_t   int_least32_t;
typedef __uint_least32_t  uint_least32_t;
typedef __int_least64_t   int_least64_t;
typedef __uint_least64_t  uint_least64_t;
typedef int               int_fast8_t;
typedef unsigned int      uint_fast8_t;
typedef int               int_fast16_t;
typedef unsigned int      uint_fast16_t;
typedef int               int_fast32_t;
typedef unsigned int      uint_fast32_t;
typedef long int          int_fast64_t;
typedef long unsigned int uint_fast64_t;
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
typedef int (*__compar_fn_t)(const void*, const void*);
int                __locale_mb_cur_max(void);
void               abort(void) __attribute__((__noreturn__));
int                abs(int);
__uint32_t         arc4random(void);
__uint32_t         arc4random_uniform(__uint32_t);
void               arc4random_buf(void*, size_t);
int                atexit(void (*__func)(void));
double             atof(const char* __nptr);
float              atoff(const char* __nptr);
int                atoi(const char* __nptr);
int                _atoi_r(struct _reent*, const char* __nptr);
long               atol(const char* __nptr);
long               _atol_r(struct _reent*, const char* __nptr);
void*              bsearch(const void* __key, const void* __base, size_t __nmemb, size_t __size, __compar_fn_t _compar);
void*              calloc(size_t, size_t) __attribute__((__malloc__)) __attribute__((__warn_unused_result__)) __attribute__((__alloc_size__(1, 2)));
div_t              div(int __numer, int __denom);
void               exit(int __status) __attribute__((__noreturn__));
void               free(void*);
char*              getenv(const char* __string);
char*              _getenv_r(struct _reent*, const char* __string);
char*              _findenv(const char*, int*);
char*              _findenv_r(struct _reent*, const char*, int*);
extern char*       suboptarg;
int                getsubopt(char**, char* const*, char**);
long               labs(long);
ldiv_t             ldiv(long __numer, long __denom);
void*              malloc(size_t) __attribute__((__malloc__)) __attribute__((__warn_unused_result__)) __attribute__((__alloc_size__(1)));
int                mblen(const char*, size_t);
int                _mblen_r(struct _reent*, const char*, size_t, _mbstate_t*);
int                mbtowc(wchar_t* restrict, const char* restrict, size_t);
int                _mbtowc_r(struct _reent*, wchar_t* restrict, const char* restrict, size_t, _mbstate_t*);
int                wctomb(char*, wchar_t);
int                _wctomb_r(struct _reent*, char*, wchar_t, _mbstate_t*);
size_t             mbstowcs(wchar_t* restrict, const char* restrict, size_t);
size_t             _mbstowcs_r(struct _reent*, wchar_t* restrict, const char* restrict, size_t, _mbstate_t*);
size_t             wcstombs(char* restrict, const wchar_t* restrict, size_t);
size_t             _wcstombs_r(struct _reent*, char* restrict, const wchar_t* restrict, size_t, _mbstate_t*);
char*              mkdtemp(char*);
int                mkstemp(char*);
int                mkstemps(char*, int);
char*              mktemp(char*) __attribute__((__deprecated__("the use of `mktemp' is dangerous; use `mkstemp' instead")));
char*              _mkdtemp_r(struct _reent*, char*);
int                _mkostemp_r(struct _reent*, char*, int);
int                _mkostemps_r(struct _reent*, char*, int, int);
int                _mkstemp_r(struct _reent*, char*);
int                _mkstemps_r(struct _reent*, char*, int);
char*              _mktemp_r(struct _reent*, char*) __attribute__((__deprecated__("the use of `mktemp' is dangerous; use `mkstemp' instead")));
void               qsort(void* __base, size_t __nmemb, size_t __size, __compar_fn_t _compar);
int                rand(void);
void*              realloc(void*, size_t) __attribute__((__warn_unused_result__)) __attribute__((__alloc_size__(2)));
void*              reallocarray(void*, size_t, size_t) __attribute__((__warn_unused_result__)) __attribute__((__alloc_size__(2, 3)));
void*              reallocf(void*, size_t) __attribute__((__warn_unused_result__)) __attribute__((__alloc_size__(2)));
char*              realpath(const char* restrict path, char* restrict resolved_path);
void               srand(unsigned __seed);
double             strtod(const char* restrict __n, char** restrict __end_PTR);
double             _strtod_r(struct _reent*, const char* restrict __n, char** restrict __end_PTR);
float              strtof(const char* restrict __n, char** restrict __end_PTR);
long               strtol(const char* restrict __n, char** restrict __end_PTR, int __base);
long               _strtol_r(struct _reent*, const char* restrict __n, char** restrict __end_PTR, int __base);
unsigned long      strtoul(const char* restrict __n, char** restrict __end_PTR, int __base);
unsigned long      _strtoul_r(struct _reent*, const char* restrict __n, char** restrict __end_PTR, int __base);
int                system(const char* __string);
long               a64l(const char* __input);
char*              l64a(long __input);
char*              _l64a_r(struct _reent*, long __input);
int                on_exit(void (*__func)(int, void*), void* __arg);
void               _Exit(int __status) __attribute__((__noreturn__));
int                putenv(char* __string);
int                _putenv_r(struct _reent*, char* __string);
void*              _reallocf_r(struct _reent*, void*, size_t);
int                setenv(const char* __string, const char* __value, int __overwrite);
int                _setenv_r(struct _reent*, const char* __string, const char* __value, int __overwrite);
char*              __itoa(int, char*, int);
char*              __utoa(unsigned, char*, int);
char*              itoa(int, char*, int);
char*              utoa(unsigned, char*, int);
int                rand_r(unsigned* __seed);
double             drand48(void);
double             _drand48_r(struct _reent*);
double             erand48(unsigned short[3]);
double             _erand48_r(struct _reent*, unsigned short[3]);
long               jrand48(unsigned short[3]);
long               _jrand48_r(struct _reent*, unsigned short[3]);
void               lcong48(unsigned short[7]);
void               _lcong48_r(struct _reent*, unsigned short[7]);
long               lrand48(void);
long               _lrand48_r(struct _reent*);
long               mrand48(void);
long               _mrand48_r(struct _reent*);
long               nrand48(unsigned short[3]);
long               _nrand48_r(struct _reent*, unsigned short[3]);
unsigned short*    seed48(unsigned short[3]);
unsigned short*    _seed48_r(struct _reent*, unsigned short[3]);
void               srand48(long);
void               _srand48_r(struct _reent*, long);
char*              initstate(unsigned, char*, size_t);
long               random(void);
char*              setstate(char*);
void               srandom(unsigned);
long long          atoll(const char* __nptr);
long long          _atoll_r(struct _reent*, const char* __nptr);
long long          llabs(long long);
lldiv_t            lldiv(long long __numer, long long __denom);
long long          strtoll(const char* restrict __n, char** restrict __end_PTR, int __base);
long long          _strtoll_r(struct _reent*, const char* restrict __n, char** restrict __end_PTR, int __base);
unsigned long long strtoull(const char* restrict __n, char** restrict __end_PTR, int __base);
unsigned long long _strtoull_r(struct _reent*, const char* restrict __n, char** restrict __end_PTR, int __base);
void               cfree(void*);
int                unsetenv(const char* __string);
int                _unsetenv_r(struct _reent*, const char* __string);
int __attribute__((__nonnull__(1))) posix_memalign(void**, size_t, size_t);
char*              _dtoa_r(struct _reent*, double, int, int, int*, int*, char**);
void*              _malloc_r(struct _reent*, size_t);
void*              _calloc_r(struct _reent*, size_t, size_t);
void               _free_r(struct _reent*, void*);
void*              _realloc_r(struct _reent*, void*, size_t);
void               _mstats_r(struct _reent*, char*);
int                _system_r(struct _reent*, const char*);
void               __eprintf(const char*, const char*, unsigned int, const char*);
void               qsort_r(void* __base, size_t __nmemb, size_t __size, void* __thunk, int (*_compar)(void*, const void*, const void*)) __asm__(""
                                                                                                                                                "__bsd_qsort_r");
extern long double _strtold_r(struct _reent*, const char* restrict, char** restrict);
extern long double strtold(const char* restrict, char** restrict);
void*              aligned_alloc(size_t, size_t) __attribute__((__malloc__)) __attribute__((__alloc_align__(1))) __attribute__((__alloc_size__(2)));
int                at_quick_exit(void (*)(void));
_Noreturn void     quick_exit(int);
int                bcmp(const void*, const void*, size_t) __attribute__((__pure__));
void               bcopy(const void*, void*, size_t);
void               bzero(void*, size_t);
void               explicit_bzero(void*, size_t);
int                ffs(int) __attribute__((__const__));
int                ffsl(long) __attribute__((__const__));
int                ffsll(long long) __attribute__((__const__));
int                fls(int) __attribute__((__const__));
int                flsl(long) __attribute__((__const__));
int                flsll(long long) __attribute__((__const__));
char*              index(const char*, int) __attribute__((__pure__));
char*              rindex(const char*, int) __attribute__((__pure__));
int                strcasecmp(const char*, const char*) __attribute__((__pure__));
int                strncasecmp(const char*, const char*, size_t) __attribute__((__pure__));
int                strcasecmp_l(const char*, const char*, locale_t);
int                strncasecmp_l(const char*, const char*, size_t, locale_t);
void*              memchr(const void*, int, size_t);
int                memcmp(const void*, const void*, size_t);
void*              memcpy(void* restrict, const void* restrict, size_t);
void*              memmove(void*, const void*, size_t);
void*              memset(void*, int, size_t);
char*              strcat(char* restrict, const char* restrict);
char*              strchr(const char*, int);
int                strcmp(const char*, const char*);
int                strcoll(const char*, const char*);
char*              strcpy(char* restrict, const char* restrict);
size_t             strcspn(const char*, const char*);
char*              strerror(int);
size_t             strlen(const char*);
char*              strncat(char* restrict, const char* restrict, size_t);
int                strncmp(const char*, const char*, size_t);
char*              strncpy(char* restrict, const char* restrict, size_t);
char*              strpbrk(const char*, const char*);
char*              strrchr(const char*, int);
size_t             strspn(const char*, const char*);
char*              strstr(const char*, const char*);
char*              strtok(char* restrict, const char* restrict);
size_t             strxfrm(char* restrict, const char* restrict, size_t);
int                strcoll_l(const char*, const char*, locale_t);
char*              strerror_l(int, locale_t);
size_t             strxfrm_l(char* restrict, const char* restrict, size_t, locale_t);
char*              strtok_r(char* restrict, const char* restrict, char** restrict);
int                timingsafe_bcmp(const void*, const void*, size_t);
int                timingsafe_memcmp(const void*, const void*, size_t);
void*              memccpy(void* restrict, const void* restrict, int, size_t);
char*              stpcpy(char* restrict, const char* restrict);
char*              stpncpy(char* restrict, const char* restrict, size_t);
char*              strdup(const char*);
char*              _strdup_r(struct _reent*, const char*);
char*              strndup(const char*, size_t);
char*              _strndup_r(struct _reent*, const char*, size_t);
int                strerror_r(int, char*, size_t) __asm__(""
                                                          "__xpg_strerror_r");
char*              _strerror_r(struct _reent*, int, int, int*);
size_t             strlcat(char*, const char*, size_t);
size_t             strlcpy(char*, const char*, size_t);
size_t             strnlen(const char*, size_t);
char*              strsep(char**, const char*);
char*              strnstr(const char*, const char*, size_t) __attribute__((__pure__));
char*              strlwr(char*);
char*              strupr(char*);
char*              strsignal(int __signo);
extern char**      environ;
void               _exit(int __status) __attribute__((__noreturn__));
int                access(const char* __path, int __amode);
unsigned           alarm(unsigned __secs);
int                chdir(const char* __path);
int                chmod(const char* __path, mode_t __mode);
int                chown(const char* __path, uid_t __owner, gid_t __group);
int                chroot(const char* __path);
int                close(int __fildes);
size_t             confstr(int __name, char* __buf, size_t __len);
int                daemon(int nochdir, int noclose);
int                dup(int __fildes);
int                dup2(int __fildes, int __fildes2);
void               endusershell(void);
int                execl(const char* __path, const char*, ...);
int                execle(const char* __path, const char*, ...);
int                execlp(const char* __file, const char*, ...);
int                execlpe(const char* __file, const char*, ...);
int                execv(const char* __path, char* const __argv[]);
int                execve(const char* __path, char* const __argv[], char* const __envp[]);
int                execvp(const char* __file, char* const __argv[]);
int                faccessat(int __dirfd, const char* __path, int __mode, int __flags);
int                fchdir(int __fildes);
int                fchmod(int __fildes, mode_t __mode);
int                fchown(int __fildes, uid_t __owner, gid_t __group);
int                fchownat(int __dirfd, const char* __path, uid_t __owner, gid_t __group, int __flags);
int                fexecve(int __fd, char* const __argv[], char* const __envp[]);
pid_t              fork(void);
long               fpathconf(int __fd, int __name);
int                fsync(int __fd);
int                fdatasync(int __fd);
char*              getcwd(char* __buf, size_t __size);
int                getdomainname(char* __name, size_t __len);
int                getentropy(void*, size_t);
gid_t              getegid(void);
uid_t              geteuid(void);
gid_t              getgid(void);
int                getgroups(int __gidsetsize, gid_t __grouplist[]);
long               gethostid(void);
char*              getlogin(void);
char*              getpass(const char* __prompt);
int                getpagesize(void);
int                getpeereid(int, uid_t*, gid_t*);
pid_t              getpgid(pid_t);
pid_t              getpgrp(void);
pid_t              getpid(void);
pid_t              getppid(void);
pid_t              getsid(pid_t);
uid_t              getuid(void);
char*              getusershell(void);
char*              getwd(char* __buf);
int                iruserok(unsigned long raddr, int superuser, const char* ruser, const char* luser);
int                isatty(int __fildes);
int                issetugid(void);
int                lchown(const char* __path, uid_t __owner, gid_t __group);
int                link(const char* __path1, const char* __path2);
int                linkat(int __dirfd1, const char* __path1, int __dirfd2, const char* __path2, int __flags);
int                nice(int __nice_value);
off_t              lseek(int __fildes, off_t __offset, int __whence);
int                lockf(int __fd, int __cmd, off_t __len);
long               pathconf(const char* __path, int __name);
int                pause(void);
int                pthread_atfork(void (*)(void), void (*)(void), void (*)(void));
int                pipe(int __fildes[2]);
ssize_t            pread(int __fd, void* __buf, size_t __nbytes, off_t __offset);
ssize_t            pwrite(int __fd, const void* __buf, size_t __nbytes, off_t __offset);
int                read(int __fd, void* __buf, size_t __nbyte);
int                rresvport(int* __alport);
int                revoke(char* __path);
int                rmdir(const char* __path);
int                ruserok(const char* rhost, int superuser, const char* ruser, const char* luser);
void*              sbrk(ptrdiff_t __incr);
int                setegid(gid_t __gid);
int                seteuid(uid_t __uid);
int                setgid(gid_t __gid);
int                setgroups(int ngroups, const gid_t* grouplist);
int                sethostname(const char*, size_t);
int                setpgid(pid_t __pid, pid_t __pgid);
int                setpgrp(void);
int                setregid(gid_t __rgid, gid_t __egid);
int                setreuid(uid_t __ruid, uid_t __euid);
pid_t              setsid(void);
int                setuid(uid_t __uid);
void               setusershell(void);
unsigned           sleep(unsigned int __seconds);
long               sysconf(int __name);
pid_t              tcgetpgrp(int __fildes);
int                tcsetpgrp(int __fildes, pid_t __pgrp_id);
char*              ttyname(int __fildes);
int                ttyname_r(int, char*, size_t);
int                unlink(const char* __path);
int                usleep(useconds_t __useconds);
int                vhangup(void);
int                write(int __fd, const void* __buf, size_t __nbyte);
extern char*       optarg;
extern int         optind, opterr, optopt;
int                getopt(int, char* const[], const char*);
extern int         optreset;
pid_t              vfork(void);
int                ftruncate(int __fd, off_t __length);
int                truncate(const char*, off_t __length);
int                getdtablesize(void);
useconds_t         ualarm(useconds_t __useconds, useconds_t __interval);
int                gethostname(char* __name, size_t __len);
int                setdtablesize(int);
void               sync(void);
ssize_t            readlink(const char* restrict __path, char* restrict __buf, size_t __buflen);
int                symlink(const char* __name1, const char* __name2);
ssize_t            readlinkat(int __dirfd1, const char* restrict __path, char* restrict __buf, size_t __buflen);
int                symlinkat(const char*, int, const char*);
int                unlinkat(int, const char*, int);
struct timezone
{
    int tz_minuteswest;
    int tz_dsttime;
};
struct bintime
{
    time_t   sec;
    uint64_t frac;
};
static __inline void bintime_addx(struct bintime* _bt, uint64_t _x)
{
    uint64_t _u;
    _u = _bt->frac;
    _bt->frac += _x;
    if(_u > _bt->frac) _bt->sec++;
}
static __inline void bintime_add(struct bintime* _bt, const struct bintime* _bt2)
{
    uint64_t _u;
    _u = _bt->frac;
    _bt->frac += _bt2->frac;
    if(_u > _bt->frac) _bt->sec++;
    _bt->sec += _bt2->sec;
}
static __inline void bintime_sub(struct bintime* _bt, const struct bintime* _bt2)
{
    uint64_t _u;
    _u = _bt->frac;
    _bt->frac -= _bt2->frac;
    if(_u < _bt->frac) _bt->sec--;
    _bt->sec -= _bt2->sec;
}
static __inline void bintime_mul(struct bintime* _bt, u_int _x)
{
    uint64_t _p1, _p2;
    _p1 = (_bt->frac & 0xffffffffull) * _x;
    _p2 = (_bt->frac >> 32) * _x + (_p1 >> 32);
    _bt->sec *= _x;
    _bt->sec += (_p2 >> 32);
    _bt->frac = (_p2 << 32) | (_p1 & 0xffffffffull);
}
static __inline void bintime_shift(struct bintime* _bt, int _exp)
{
    if(_exp > 0)
    {
        _bt->sec <<= _exp;
        _bt->sec |= _bt->frac >> (64 - _exp);
        _bt->frac <<= _exp;
    }
    else if(_exp < 0)
    {
        _bt->frac >>= -_exp;
        _bt->frac |= (uint64_t)_bt->sec << (64 + _exp);
        _bt->sec >>= -_exp;
    }
}
static __inline int            sbintime_getsec(sbintime_t _sbt) { return (_sbt >> 32); }
static __inline sbintime_t     bttosbt(const struct bintime _bt) { return (((sbintime_t)_bt.sec << 32) + (_bt.frac >> 32)); }
static __inline struct bintime sbttobt(sbintime_t _sbt)
{
    struct bintime _bt;
    _bt.sec  = _sbt >> 32;
    _bt.frac = _sbt << 32;
    return (_bt);
}
static __inline void bintime2timespec(const struct bintime* _bt, struct timespec* _ts)
{
    _ts->tv_sec  = _bt->sec;
    _ts->tv_nsec = ((uint64_t)1000000000 * (uint32_t)(_bt->frac >> 32)) >> 32;
}
static __inline void timespec2bintime(const struct timespec* _ts, struct bintime* _bt)
{
    _bt->sec  = _ts->tv_sec;
    _bt->frac = _ts->tv_nsec * (uint64_t)18446744073LL;
}
static __inline void bintime2timeval(const struct bintime* _bt, struct timeval* _tv)
{
    _tv->tv_sec  = _bt->sec;
    _tv->tv_usec = ((uint64_t)1000000 * (uint32_t)(_bt->frac >> 32)) >> 32;
}
static __inline void timeval2bintime(const struct timeval* _tv, struct bintime* _bt)
{
    _bt->sec  = _tv->tv_sec;
    _bt->frac = _tv->tv_usec * (uint64_t)18446744073709LL;
}
static __inline struct timespec sbttots(sbintime_t _sbt)
{
    struct timespec _ts;
    _ts.tv_sec  = _sbt >> 32;
    _ts.tv_nsec = ((uint64_t)1000000000 * (uint32_t)_sbt) >> 32;
    return (_ts);
}
static __inline sbintime_t tstosbt(struct timespec _ts)
{
    return (((sbintime_t)_ts.tv_sec << 32) + (_ts.tv_nsec * (((uint64_t)1 << 63) / 500000000) >> 32));
}
static __inline struct timeval sbttotv(sbintime_t _sbt)
{
    struct timeval _tv;
    _tv.tv_sec  = _sbt >> 32;
    _tv.tv_usec = ((uint64_t)1000000 * (uint32_t)_sbt) >> 32;
    return (_tv);
}
static __inline sbintime_t tvtosbt(struct timeval _tv)
{
    return (((sbintime_t)_tv.tv_sec << 32) + (_tv.tv_usec * (((uint64_t)1 << 63) / 500000) >> 32));
}
struct itimerval
{
    struct timeval it_interval;
    struct timeval it_value;
};
int              utimes(const char* __path, const struct timeval* __tvp);
int              adjtime(const struct timeval*, struct timeval*);
int              futimes(int, const struct timeval*);
int              lutimes(const char*, const struct timeval*);
int              settimeofday(const struct timeval*, const struct timezone*);
int              getitimer(int __which, struct itimerval* __value);
int              setitimer(int __which, const struct itimerval* restrict __value, struct itimerval* restrict __ovalue);
int              gettimeofday(struct timeval* restrict __p, void* restrict __tz);
typedef uint8_t  u8;
typedef uint32_t u32;
typedef struct
{
    u32 input[16];
} chacha_ctx;
static const char sigma[16] = "expand 32-byte k";
static const char tau[16]   = "expand 16-byte k";
static void       chacha_keysetup(chacha_ctx* x, const u8* k, u32 kbits, u32 ivbits)
{
    const char* constants;
    x->input[4] = (((u32)((k + 0)[0])) | ((u32)((k + 0)[1]) << 8) | ((u32)((k + 0)[2]) << 16) | ((u32)((k + 0)[3]) << 24));
    x->input[5] = (((u32)((k + 4)[0])) | ((u32)((k + 4)[1]) << 8) | ((u32)((k + 4)[2]) << 16) | ((u32)((k + 4)[3]) << 24));
    x->input[6] = (((u32)((k + 8)[0])) | ((u32)((k + 8)[1]) << 8) | ((u32)((k + 8)[2]) << 16) | ((u32)((k + 8)[3]) << 24));
    x->input[7] = (((u32)((k + 12)[0])) | ((u32)((k + 12)[1]) << 8) | ((u32)((k + 12)[2]) << 16) | ((u32)((k + 12)[3]) << 24));
    if(kbits == 256)
    {
        k += 16;
        constants = sigma;
    }
    else { constants = tau; }
    x->input[8]  = (((u32)((k + 0)[0])) | ((u32)((k + 0)[1]) << 8) | ((u32)((k + 0)[2]) << 16) | ((u32)((k + 0)[3]) << 24));
    x->input[9]  = (((u32)((k + 4)[0])) | ((u32)((k + 4)[1]) << 8) | ((u32)((k + 4)[2]) << 16) | ((u32)((k + 4)[3]) << 24));
    x->input[10] = (((u32)((k + 8)[0])) | ((u32)((k + 8)[1]) << 8) | ((u32)((k + 8)[2]) << 16) | ((u32)((k + 8)[3]) << 24));
    x->input[11] = (((u32)((k + 12)[0])) | ((u32)((k + 12)[1]) << 8) | ((u32)((k + 12)[2]) << 16) | ((u32)((k + 12)[3]) << 24));
    x->input[0] =
        (((u32)((constants + 0)[0])) | ((u32)((constants + 0)[1]) << 8) | ((u32)((constants + 0)[2]) << 16) | ((u32)((constants + 0)[3]) << 24));
    x->input[1] =
        (((u32)((constants + 4)[0])) | ((u32)((constants + 4)[1]) << 8) | ((u32)((constants + 4)[2]) << 16) | ((u32)((constants + 4)[3]) << 24));
    x->input[2] =
        (((u32)((constants + 8)[0])) | ((u32)((constants + 8)[1]) << 8) | ((u32)((constants + 8)[2]) << 16) | ((u32)((constants + 8)[3]) << 24));
    x->input[3] =
        (((u32)((constants + 12)[0])) | ((u32)((constants + 12)[1]) << 8) | ((u32)((constants + 12)[2]) << 16) | ((u32)((constants + 12)[3]) << 24));
}
static void chacha_ivsetup(chacha_ctx* x, const u8* iv)
{
    x->input[12] = 0;
    x->input[13] = 0;
    x->input[14] = (((u32)((iv + 0)[0])) | ((u32)((iv + 0)[1]) << 8) | ((u32)((iv + 0)[2]) << 16) | ((u32)((iv + 0)[3]) << 24));
    x->input[15] = (((u32)((iv + 4)[0])) | ((u32)((iv + 4)[1]) << 8) | ((u32)((iv + 4)[2]) << 16) | ((u32)((iv + 4)[3]) << 24));
}
static void chacha_encrypt_bytes(chacha_ctx* x, const u8* m, u8* c, u32 bytes)
{
    u32   x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15;
    u32   j0, j1, j2, j3, j4, j5, j6, j7, j8, j9, j10, j11, j12, j13, j14, j15;
    u8*   ctarget = ((void*)0);
    u8    tmp[64];
    u_int i;
    if(!bytes) return;
    j0  = x->input[0];
    j1  = x->input[1];
    j2  = x->input[2];
    j3  = x->input[3];
    j4  = x->input[4];
    j5  = x->input[5];
    j6  = x->input[6];
    j7  = x->input[7];
    j8  = x->input[8];
    j9  = x->input[9];
    j10 = x->input[10];
    j11 = x->input[11];
    j12 = x->input[12];
    j13 = x->input[13];
    j14 = x->input[14];
    j15 = x->input[15];
    for(;;)
    {
        if(bytes < 64)
        {
            for(i = 0; i < bytes; ++i) tmp[i] = m[i];
            m       = tmp;
            ctarget = c;
            c       = tmp;
        }
        x0  = j0;
        x1  = j1;
        x2  = j2;
        x3  = j3;
        x4  = j4;
        x5  = j5;
        x6  = j6;
        x7  = j7;
        x8  = j8;
        x9  = j9;
        x10 = j10;
        x11 = j11;
        x12 = j12;
        x13 = j13;
        x14 = j14;
        x15 = j15;
        for(i = 20; i > 0; i -= 2)
        {
            x0  = (((u32)((x0) + (x4))));
            x12 = ((((u32)((((x12) ^ (x0))) << (16))) | ((((x12) ^ (x0))) >> (32 - (16)))));
            x8  = (((u32)((x8) + (x12))));
            x4  = ((((u32)((((x4) ^ (x8))) << (12))) | ((((x4) ^ (x8))) >> (32 - (12)))));
            x0  = (((u32)((x0) + (x4))));
            x12 = ((((u32)((((x12) ^ (x0))) << (8))) | ((((x12) ^ (x0))) >> (32 - (8)))));
            x8  = (((u32)((x8) + (x12))));
            x4  = ((((u32)((((x4) ^ (x8))) << (7))) | ((((x4) ^ (x8))) >> (32 - (7)))));
            x1  = (((u32)((x1) + (x5))));
            x13 = ((((u32)((((x13) ^ (x1))) << (16))) | ((((x13) ^ (x1))) >> (32 - (16)))));
            x9  = (((u32)((x9) + (x13))));
            x5  = ((((u32)((((x5) ^ (x9))) << (12))) | ((((x5) ^ (x9))) >> (32 - (12)))));
            x1  = (((u32)((x1) + (x5))));
            x13 = ((((u32)((((x13) ^ (x1))) << (8))) | ((((x13) ^ (x1))) >> (32 - (8)))));
            x9  = (((u32)((x9) + (x13))));
            x5  = ((((u32)((((x5) ^ (x9))) << (7))) | ((((x5) ^ (x9))) >> (32 - (7)))));
            x2  = (((u32)((x2) + (x6))));
            x14 = ((((u32)((((x14) ^ (x2))) << (16))) | ((((x14) ^ (x2))) >> (32 - (16)))));
            x10 = (((u32)((x10) + (x14))));
            x6  = ((((u32)((((x6) ^ (x10))) << (12))) | ((((x6) ^ (x10))) >> (32 - (12)))));
            x2  = (((u32)((x2) + (x6))));
            x14 = ((((u32)((((x14) ^ (x2))) << (8))) | ((((x14) ^ (x2))) >> (32 - (8)))));
            x10 = (((u32)((x10) + (x14))));
            x6  = ((((u32)((((x6) ^ (x10))) << (7))) | ((((x6) ^ (x10))) >> (32 - (7)))));
            x3  = (((u32)((x3) + (x7))));
            x15 = ((((u32)((((x15) ^ (x3))) << (16))) | ((((x15) ^ (x3))) >> (32 - (16)))));
            x11 = (((u32)((x11) + (x15))));
            x7  = ((((u32)((((x7) ^ (x11))) << (12))) | ((((x7) ^ (x11))) >> (32 - (12)))));
            x3  = (((u32)((x3) + (x7))));
            x15 = ((((u32)((((x15) ^ (x3))) << (8))) | ((((x15) ^ (x3))) >> (32 - (8)))));
            x11 = (((u32)((x11) + (x15))));
            x7  = ((((u32)((((x7) ^ (x11))) << (7))) | ((((x7) ^ (x11))) >> (32 - (7)))));
            x0  = (((u32)((x0) + (x5))));
            x15 = ((((u32)((((x15) ^ (x0))) << (16))) | ((((x15) ^ (x0))) >> (32 - (16)))));
            x10 = (((u32)((x10) + (x15))));
            x5  = ((((u32)((((x5) ^ (x10))) << (12))) | ((((x5) ^ (x10))) >> (32 - (12)))));
            x0  = (((u32)((x0) + (x5))));
            x15 = ((((u32)((((x15) ^ (x0))) << (8))) | ((((x15) ^ (x0))) >> (32 - (8)))));
            x10 = (((u32)((x10) + (x15))));
            x5  = ((((u32)((((x5) ^ (x10))) << (7))) | ((((x5) ^ (x10))) >> (32 - (7)))));
            x1  = (((u32)((x1) + (x6))));
            x12 = ((((u32)((((x12) ^ (x1))) << (16))) | ((((x12) ^ (x1))) >> (32 - (16)))));
            x11 = (((u32)((x11) + (x12))));
            x6  = ((((u32)((((x6) ^ (x11))) << (12))) | ((((x6) ^ (x11))) >> (32 - (12)))));
            x1  = (((u32)((x1) + (x6))));
            x12 = ((((u32)((((x12) ^ (x1))) << (8))) | ((((x12) ^ (x1))) >> (32 - (8)))));
            x11 = (((u32)((x11) + (x12))));
            x6  = ((((u32)((((x6) ^ (x11))) << (7))) | ((((x6) ^ (x11))) >> (32 - (7)))));
            x2  = (((u32)((x2) + (x7))));
            x13 = ((((u32)((((x13) ^ (x2))) << (16))) | ((((x13) ^ (x2))) >> (32 - (16)))));
            x8  = (((u32)((x8) + (x13))));
            x7  = ((((u32)((((x7) ^ (x8))) << (12))) | ((((x7) ^ (x8))) >> (32 - (12)))));
            x2  = (((u32)((x2) + (x7))));
            x13 = ((((u32)((((x13) ^ (x2))) << (8))) | ((((x13) ^ (x2))) >> (32 - (8)))));
            x8  = (((u32)((x8) + (x13))));
            x7  = ((((u32)((((x7) ^ (x8))) << (7))) | ((((x7) ^ (x8))) >> (32 - (7)))));
            x3  = (((u32)((x3) + (x4))));
            x14 = ((((u32)((((x14) ^ (x3))) << (16))) | ((((x14) ^ (x3))) >> (32 - (16)))));
            x9  = (((u32)((x9) + (x14))));
            x4  = ((((u32)((((x4) ^ (x9))) << (12))) | ((((x4) ^ (x9))) >> (32 - (12)))));
            x3  = (((u32)((x3) + (x4))));
            x14 = ((((u32)((((x14) ^ (x3))) << (8))) | ((((x14) ^ (x3))) >> (32 - (8)))));
            x9  = (((u32)((x9) + (x14))));
            x4  = ((((u32)((((x4) ^ (x9))) << (7))) | ((((x4) ^ (x9))) >> (32 - (7)))));
        }
        x0  = (((u32)((x0) + (j0))));
        x1  = (((u32)((x1) + (j1))));
        x2  = (((u32)((x2) + (j2))));
        x3  = (((u32)((x3) + (j3))));
        x4  = (((u32)((x4) + (j4))));
        x5  = (((u32)((x5) + (j5))));
        x6  = (((u32)((x6) + (j6))));
        x7  = (((u32)((x7) + (j7))));
        x8  = (((u32)((x8) + (j8))));
        x9  = (((u32)((x9) + (j9))));
        x10 = (((u32)((x10) + (j10))));
        x11 = (((u32)((x11) + (j11))));
        x12 = (((u32)((x12) + (j12))));
        x13 = (((u32)((x13) + (j13))));
        x14 = (((u32)((x14) + (j14))));
        x15 = (((u32)((x15) + (j15))));
        j12 = ((((u32)(((j12)) + (1)))));
        if(!j12) { j13 = ((((u32)(((j13)) + (1))))); }
        do {
            (c + 0)[0] = ((u8)((x0)));
            (c + 0)[1] = ((u8)((x0) >> 8));
            (c + 0)[2] = ((u8)((x0) >> 16));
            (c + 0)[3] = ((u8)((x0) >> 24));
        } while(0);
        do {
            (c + 4)[0] = ((u8)((x1)));
            (c + 4)[1] = ((u8)((x1) >> 8));
            (c + 4)[2] = ((u8)((x1) >> 16));
            (c + 4)[3] = ((u8)((x1) >> 24));
        } while(0);
        do {
            (c + 8)[0] = ((u8)((x2)));
            (c + 8)[1] = ((u8)((x2) >> 8));
            (c + 8)[2] = ((u8)((x2) >> 16));
            (c + 8)[3] = ((u8)((x2) >> 24));
        } while(0);
        do {
            (c + 12)[0] = ((u8)((x3)));
            (c + 12)[1] = ((u8)((x3) >> 8));
            (c + 12)[2] = ((u8)((x3) >> 16));
            (c + 12)[3] = ((u8)((x3) >> 24));
        } while(0);
        do {
            (c + 16)[0] = ((u8)((x4)));
            (c + 16)[1] = ((u8)((x4) >> 8));
            (c + 16)[2] = ((u8)((x4) >> 16));
            (c + 16)[3] = ((u8)((x4) >> 24));
        } while(0);
        do {
            (c + 20)[0] = ((u8)((x5)));
            (c + 20)[1] = ((u8)((x5) >> 8));
            (c + 20)[2] = ((u8)((x5) >> 16));
            (c + 20)[3] = ((u8)((x5) >> 24));
        } while(0);
        do {
            (c + 24)[0] = ((u8)((x6)));
            (c + 24)[1] = ((u8)((x6) >> 8));
            (c + 24)[2] = ((u8)((x6) >> 16));
            (c + 24)[3] = ((u8)((x6) >> 24));
        } while(0);
        do {
            (c + 28)[0] = ((u8)((x7)));
            (c + 28)[1] = ((u8)((x7) >> 8));
            (c + 28)[2] = ((u8)((x7) >> 16));
            (c + 28)[3] = ((u8)((x7) >> 24));
        } while(0);
        do {
            (c + 32)[0] = ((u8)((x8)));
            (c + 32)[1] = ((u8)((x8) >> 8));
            (c + 32)[2] = ((u8)((x8) >> 16));
            (c + 32)[3] = ((u8)((x8) >> 24));
        } while(0);
        do {
            (c + 36)[0] = ((u8)((x9)));
            (c + 36)[1] = ((u8)((x9) >> 8));
            (c + 36)[2] = ((u8)((x9) >> 16));
            (c + 36)[3] = ((u8)((x9) >> 24));
        } while(0);
        do {
            (c + 40)[0] = ((u8)((x10)));
            (c + 40)[1] = ((u8)((x10) >> 8));
            (c + 40)[2] = ((u8)((x10) >> 16));
            (c + 40)[3] = ((u8)((x10) >> 24));
        } while(0);
        do {
            (c + 44)[0] = ((u8)((x11)));
            (c + 44)[1] = ((u8)((x11) >> 8));
            (c + 44)[2] = ((u8)((x11) >> 16));
            (c + 44)[3] = ((u8)((x11) >> 24));
        } while(0);
        do {
            (c + 48)[0] = ((u8)((x12)));
            (c + 48)[1] = ((u8)((x12) >> 8));
            (c + 48)[2] = ((u8)((x12) >> 16));
            (c + 48)[3] = ((u8)((x12) >> 24));
        } while(0);
        do {
            (c + 52)[0] = ((u8)((x13)));
            (c + 52)[1] = ((u8)((x13) >> 8));
            (c + 52)[2] = ((u8)((x13) >> 16));
            (c + 52)[3] = ((u8)((x13) >> 24));
        } while(0);
        do {
            (c + 56)[0] = ((u8)((x14)));
            (c + 56)[1] = ((u8)((x14) >> 8));
            (c + 56)[2] = ((u8)((x14) >> 16));
            (c + 56)[3] = ((u8)((x14) >> 24));
        } while(0);
        do {
            (c + 60)[0] = ((u8)((x15)));
            (c + 60)[1] = ((u8)((x15) >> 8));
            (c + 60)[2] = ((u8)((x15) >> 16));
            (c + 60)[3] = ((u8)((x15) >> 24));
        } while(0);
        if(bytes <= 64)
        {
            if(bytes < 64)
            {
                for(i = 0; i < bytes; ++i) ctarget[i] = c[i];
            }
            x->input[12] = j12;
            x->input[13] = j13;
            return;
        }
        bytes -= 64;
        c += 64;
    }
}
static struct _rs
{
    size_t rs_have;
    size_t rs_count;
} * rs;
static struct _rsx
{
    chacha_ctx rs_chacha;
    u_char     rs_buf[(16 * 64)];
} * rsx;
static __inline int  _rs_allocate(struct _rs**, struct _rsx**);
static __inline void _rs_forkdetect(void);
typedef int          _LOCK_T;
typedef int          _LOCK_RECURSIVE_T;
static int           __arc4random_mutex = 0;
;
static struct
{
    struct _rs  rs;
    struct _rsx rsx;
} _arc4random_data;
static __inline void _getentropy_fail(void) { raise(9); }
static __inline int  _rs_allocate(struct _rs** rsp, struct _rsx** rsxp)
{
    *rsp  = &_arc4random_data.rs;
    *rsxp = &_arc4random_data.rsx;
    return (0);
}
static __inline void _rs_forkdetect(void) { }
static __inline void _rs_rekey(u_char* dat, size_t datlen);
static __inline void _rs_init(u_char* buf, size_t n)
{
    if(n < 32 + 8) return;
    if(rs == ((void*)0))
    {
        if(_rs_allocate(&rs, &rsx) == -1) abort();
    }
    chacha_keysetup(&rsx->rs_chacha, buf, 32 * 8, 0);
    chacha_ivsetup(&rsx->rs_chacha, buf + 32);
}
static void _rs_stir(void)
{
    u_char rnd[32 + 8];
    if(getentropy(rnd, sizeof rnd) == -1) _getentropy_fail();
    if(!rs)
        _rs_init(rnd, sizeof(rnd));
    else
        _rs_rekey(rnd, sizeof(rnd));
    explicit_bzero(rnd, sizeof(rnd));
    rs->rs_have = 0;
    memset(rsx->rs_buf, 0, sizeof(rsx->rs_buf));
    rs->rs_count = ((0xffffffffffffffffUL) <= 65535) ? 65000 : 1600000;
}
static __inline void _rs_stir_if_needed(size_t len)
{
    _rs_forkdetect();
    if(!rs || rs->rs_count <= len) _rs_stir();
    if(rs->rs_count <= len)
        rs->rs_count = 0;
    else
        rs->rs_count -= len;
}
static __inline void _rs_rekey(u_char* dat, size_t datlen)
{
    chacha_encrypt_bytes(&rsx->rs_chacha, rsx->rs_buf, rsx->rs_buf, sizeof(rsx->rs_buf));
    if(dat)
    {
        size_t i, m;
        m = ((datlen) < (32 + 8) ? (datlen) : (32 + 8));
        for(i = 0; i < m; i++) rsx->rs_buf[i] ^= dat[i];
    }
    _rs_init(rsx->rs_buf, 32 + 8);
    memset(rsx->rs_buf, 0, 32 + 8);
    rs->rs_have = sizeof(rsx->rs_buf) - 32 - 8;
}
static __inline void _rs_random_buf(void* _buf, size_t n)
{
    u_char* buf = (u_char*)_buf;
    u_char* keystream;
    size_t  m;
    _rs_stir_if_needed(n);
    while(n > 0)
    {
        if(rs->rs_have > 0)
        {
            m         = ((n) < (rs->rs_have) ? (n) : (rs->rs_have));
            keystream = rsx->rs_buf + sizeof(rsx->rs_buf) - rs->rs_have;
            memcpy(buf, keystream, m);
            memset(keystream, 0, m);
            buf += m;
            n -= m;
            rs->rs_have -= m;
        }
        if(rs->rs_have == 0) _rs_rekey(((void*)0), 0);
    }
}
static __inline void _rs_random_u32(uint32_t* val)
{
    u_char* keystream;
    _rs_stir_if_needed(sizeof(*val));
    if(rs->rs_have < sizeof(*val)) _rs_rekey(((void*)0), 0);
    keystream = rsx->rs_buf + sizeof(rsx->rs_buf) - rs->rs_have;
    memcpy(val, keystream, sizeof(*val));
    memset(keystream, 0, sizeof(*val));
    rs->rs_have -= sizeof(*val);
}
uint32_t arc4random(void)
{
    uint32_t val;
    ((void)0);
    _rs_random_u32(&val);
    ((void)0);
    return val;
}
void arc4random_buf(void* buf, size_t n)
{
    ((void)0);
    _rs_random_buf(buf, n);
    ((void)0);
}
