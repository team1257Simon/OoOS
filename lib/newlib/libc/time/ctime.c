
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
typedef __uint8_t           u_int8_t;
typedef __uint16_t          u_int16_t;
typedef __uint32_t          u_int32_t;
typedef __uint64_t          u_int64_t;
typedef int                 register_t;
typedef __int8_t            int8_t;
typedef __uint8_t           uint8_t;
typedef __int16_t           int16_t;
typedef __uint16_t          uint16_t;
typedef __int32_t           int32_t;
typedef __uint32_t          uint32_t;
typedef __int64_t           int64_t;
typedef __uint64_t          uint64_t;
typedef __intmax_t          intmax_t;
typedef __uintmax_t         uintmax_t;
typedef __intptr_t          intptr_t;
typedef __uintptr_t         uintptr_t;
typedef unsigned long       __sigset_t;
typedef __suseconds_t       suseconds_t;
typedef long                time_t;
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
char*          ctime(const time_t* tim_p) { return asctime(localtime(tim_p)); }
