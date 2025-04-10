
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
typedef __uint8_t     u_int8_t;
typedef __uint16_t    u_int16_t;
typedef __uint32_t    u_int32_t;
typedef __uint64_t    u_int64_t;
typedef int           register_t;
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
typedef int          error_t;
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
extern int*                 __errno(void);
extern const char* const    _sys_errlist[];
extern int                  _sys_nerr;
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
typedef __builtin_va_list __gnuc_va_list;
typedef __gnuc_va_list    va_list;
typedef __FILE            FILE;
typedef _fpos_t           fpos_t;
typedef int               _LOCK_T;
typedef int               _LOCK_RECURSIVE_T;
char*                     ctermid(char*);
FILE*                     tmpfile(void);
char*                     tmpnam(char*);
char*                     tempnam(const char*, const char*);
int                       fclose(FILE*);
int                       fflush(FILE*);
FILE*                     freopen(const char* restrict, const char* restrict, FILE* restrict);
void                      setbuf(FILE* restrict, char* restrict);
int                       setvbuf(FILE* restrict, char* restrict, int, size_t);
int                       fprintf(FILE* restrict, const char* restrict, ...) __attribute__((__format__(__printf__, 2, 3)));
int                       fscanf(FILE* restrict, const char* restrict, ...) __attribute__((__format__(__scanf__, 2, 3)));
int                       printf(const char* restrict, ...) __attribute__((__format__(__printf__, 1, 2)));
int                       scanf(const char* restrict, ...) __attribute__((__format__(__scanf__, 1, 2)));
int                       sscanf(const char* restrict, const char* restrict, ...) __attribute__((__format__(__scanf__, 2, 3)));
int                       vfprintf(FILE* restrict, const char* restrict, __gnuc_va_list) __attribute__((__format__(__printf__, 2, 0)));
int                       vprintf(const char*, __gnuc_va_list) __attribute__((__format__(__printf__, 1, 0)));
int                       vsprintf(char* restrict, const char* restrict, __gnuc_va_list) __attribute__((__format__(__printf__, 2, 0)));
int                       fgetc(FILE*);
char*                     fgets(char* restrict, int, FILE* restrict);
int                       fputc(int, FILE*);
int                       fputs(const char* restrict, FILE* restrict);
int                       getc(FILE*);
int                       getchar(void);
char*                     gets(char*);
int                       putc(int, FILE*);
int                       putchar(int);
int                       puts(const char*);
int                       ungetc(int, FILE*);
size_t                    fread(void* restrict, size_t _size, size_t _n, FILE* restrict);
size_t                    fwrite(const void* restrict, size_t _size, size_t _n, FILE*);
int                       fgetpos(FILE* restrict, fpos_t* restrict);
int                       fseek(FILE*, long, int);
int                       fsetpos(FILE*, const fpos_t*);
long                      ftell(FILE*);
void                      rewind(FILE*);
void                      clearerr(FILE*);
int                       feof(FILE*);
int                       ferror(FILE*);
void                      perror(const char*);
FILE*                     fopen(const char* restrict _name, const char* restrict _type);
int                       sprintf(char* restrict, const char* restrict, ...) __attribute__((__format__(__printf__, 2, 3)));
int                       remove(const char*);
int                       rename(const char*, const char*);
int                       fseeko(FILE*, off_t, int);
off_t                     ftello(FILE*);
int                       snprintf(char* restrict, size_t, const char* restrict, ...) __attribute__((__format__(__printf__, 3, 4)));
int                       vsnprintf(char* restrict, size_t, const char* restrict, __gnuc_va_list) __attribute__((__format__(__printf__, 3, 0)));
int                       vfscanf(FILE* restrict, const char* restrict, __gnuc_va_list) __attribute__((__format__(__scanf__, 2, 0)));
int                       vscanf(const char*, __gnuc_va_list) __attribute__((__format__(__scanf__, 1, 0)));
int                       vsscanf(const char* restrict, const char* restrict, __gnuc_va_list) __attribute__((__format__(__scanf__, 2, 0)));
int                       asiprintf(char**, const char*, ...) __attribute__((__format__(__printf__, 2, 3)));
char*                     asniprintf(char*, size_t*, const char*, ...) __attribute__((__format__(__printf__, 3, 4)));
char*                     asnprintf(char* restrict, size_t* restrict, const char* restrict, ...) __attribute__((__format__(__printf__, 3, 4)));
int                       diprintf(int, const char*, ...) __attribute__((__format__(__printf__, 2, 3)));
int                       fiprintf(FILE*, const char*, ...) __attribute__((__format__(__printf__, 2, 3)));
int                       fiscanf(FILE*, const char*, ...) __attribute__((__format__(__scanf__, 2, 3)));
int                       iprintf(const char*, ...) __attribute__((__format__(__printf__, 1, 2)));
int                       iscanf(const char*, ...) __attribute__((__format__(__scanf__, 1, 2)));
int                       siprintf(char*, const char*, ...) __attribute__((__format__(__printf__, 2, 3)));
int                       siscanf(const char*, const char*, ...) __attribute__((__format__(__scanf__, 2, 3)));
int                       sniprintf(char*, size_t, const char*, ...) __attribute__((__format__(__printf__, 3, 4)));
int                       vasiprintf(char**, const char*, __gnuc_va_list) __attribute__((__format__(__printf__, 2, 0)));
char*                     vasniprintf(char*, size_t*, const char*, __gnuc_va_list) __attribute__((__format__(__printf__, 3, 0)));
char*                     vasnprintf(char*, size_t*, const char*, __gnuc_va_list) __attribute__((__format__(__printf__, 3, 0)));
int                       vdiprintf(int, const char*, __gnuc_va_list) __attribute__((__format__(__printf__, 2, 0)));
int                       vfiprintf(FILE*, const char*, __gnuc_va_list) __attribute__((__format__(__printf__, 2, 0)));
int                       vfiscanf(FILE*, const char*, __gnuc_va_list) __attribute__((__format__(__scanf__, 2, 0)));
int                       viprintf(const char*, __gnuc_va_list) __attribute__((__format__(__printf__, 1, 0)));
int                       viscanf(const char*, __gnuc_va_list) __attribute__((__format__(__scanf__, 1, 0)));
int                       vsiprintf(char*, const char*, __gnuc_va_list) __attribute__((__format__(__printf__, 2, 0)));
int                       vsiscanf(const char*, const char*, __gnuc_va_list) __attribute__((__format__(__scanf__, 2, 0)));
int                       vsniprintf(char*, size_t, const char*, __gnuc_va_list) __attribute__((__format__(__printf__, 3, 0)));
FILE*                     fdopen(int, const char*);
int                       fileno(FILE*);
int                       pclose(FILE*);
FILE*                     popen(const char*, const char*);
void                      setbuffer(FILE*, char*, int);
int                       setlinebuf(FILE*);
int                       getw(FILE*);
int                       putw(int, FILE*);
int                       getc_unlocked(FILE*);
int                       getchar_unlocked(void);
void                      flockfile(FILE*);
int                       ftrylockfile(FILE*);
void                      funlockfile(FILE*);
int                       putc_unlocked(int, FILE*);
int                       putchar_unlocked(int);
int                       dprintf(int, const char* restrict, ...) __attribute__((__format__(__printf__, 2, 3)));
FILE*                     fmemopen(void* restrict, size_t, const char* restrict);
FILE*                     open_memstream(char**, size_t*);
int                       vdprintf(int, const char* restrict, __gnuc_va_list) __attribute__((__format__(__printf__, 2, 0)));
int                       renameat(int, const char*, int, const char*);
int                       _asiprintf_r(struct _reent*, char**, const char*, ...) __attribute__((__format__(__printf__, 3, 4)));
char*                     _asniprintf_r(struct _reent*, char*, size_t*, const char*, ...) __attribute__((__format__(__printf__, 4, 5)));
char*   _asnprintf_r(struct _reent*, char* restrict, size_t* restrict, const char* restrict, ...) __attribute__((__format__(__printf__, 4, 5)));
int     _asprintf_r(struct _reent*, char** restrict, const char* restrict, ...) __attribute__((__format__(__printf__, 3, 4)));
int     _diprintf_r(struct _reent*, int, const char*, ...) __attribute__((__format__(__printf__, 3, 4)));
int     _dprintf_r(struct _reent*, int, const char* restrict, ...) __attribute__((__format__(__printf__, 3, 4)));
int     _fclose_r(struct _reent*, FILE*);
int     _fcloseall_r(struct _reent*);
FILE*   _fdopen_r(struct _reent*, int, const char*);
int     _fflush_r(struct _reent*, FILE*);
int     _fgetc_r(struct _reent*, FILE*);
int     _fgetc_unlocked_r(struct _reent*, FILE*);
char*   _fgets_r(struct _reent*, char* restrict, int, FILE* restrict);
char*   _fgets_unlocked_r(struct _reent*, char* restrict, int, FILE* restrict);
int     _fgetpos_r(struct _reent*, FILE*, fpos_t*);
int     _fsetpos_r(struct _reent*, FILE*, const fpos_t*);
int     _fiprintf_r(struct _reent*, FILE*, const char*, ...) __attribute__((__format__(__printf__, 3, 4)));
int     _fiscanf_r(struct _reent*, FILE*, const char*, ...) __attribute__((__format__(__scanf__, 3, 4)));
FILE*   _fmemopen_r(struct _reent*, void* restrict, size_t, const char* restrict);
FILE*   _fopen_r(struct _reent*, const char* restrict, const char* restrict);
FILE*   _freopen_r(struct _reent*, const char* restrict, const char* restrict, FILE* restrict);
int     _fprintf_r(struct _reent*, FILE* restrict, const char* restrict, ...) __attribute__((__format__(__printf__, 3, 4)));
int     _fpurge_r(struct _reent*, FILE*);
int     _fputc_r(struct _reent*, int, FILE*);
int     _fputc_unlocked_r(struct _reent*, int, FILE*);
int     _fputs_r(struct _reent*, const char* restrict, FILE* restrict);
int     _fputs_unlocked_r(struct _reent*, const char* restrict, FILE* restrict);
size_t  _fread_r(struct _reent*, void* restrict, size_t _size, size_t _n, FILE* restrict);
size_t  _fread_unlocked_r(struct _reent*, void* restrict, size_t _size, size_t _n, FILE* restrict);
int     _fscanf_r(struct _reent*, FILE* restrict, const char* restrict, ...) __attribute__((__format__(__scanf__, 3, 4)));
int     _fseek_r(struct _reent*, FILE*, long, int);
int     _fseeko_r(struct _reent*, FILE*, _off_t, int);
long    _ftell_r(struct _reent*, FILE*);
_off_t  _ftello_r(struct _reent*, FILE*);
void    _rewind_r(struct _reent*, FILE*);
size_t  _fwrite_r(struct _reent*, const void* restrict, size_t _size, size_t _n, FILE* restrict);
size_t  _fwrite_unlocked_r(struct _reent*, const void* restrict, size_t _size, size_t _n, FILE* restrict);
int     _getc_r(struct _reent*, FILE*);
int     _getc_unlocked_r(struct _reent*, FILE*);
int     _getchar_r(struct _reent*);
int     _getchar_unlocked_r(struct _reent*);
char*   _gets_r(struct _reent*, char*);
int     _iprintf_r(struct _reent*, const char*, ...) __attribute__((__format__(__printf__, 2, 3)));
int     _iscanf_r(struct _reent*, const char*, ...) __attribute__((__format__(__scanf__, 2, 3)));
FILE*   _open_memstream_r(struct _reent*, char**, size_t*);
void    _perror_r(struct _reent*, const char*);
int     _printf_r(struct _reent*, const char* restrict, ...) __attribute__((__format__(__printf__, 2, 3)));
int     _putc_r(struct _reent*, int, FILE*);
int     _putc_unlocked_r(struct _reent*, int, FILE*);
int     _putchar_unlocked_r(struct _reent*, int);
int     _putchar_r(struct _reent*, int);
int     _puts_r(struct _reent*, const char*);
int     _remove_r(struct _reent*, const char*);
int     _rename_r(struct _reent*, const char* _old, const char* _new);
int     _scanf_r(struct _reent*, const char* restrict, ...) __attribute__((__format__(__scanf__, 2, 3)));
int     _siprintf_r(struct _reent*, char*, const char*, ...) __attribute__((__format__(__printf__, 3, 4)));
int     _siscanf_r(struct _reent*, const char*, const char*, ...) __attribute__((__format__(__scanf__, 3, 4)));
int     _sniprintf_r(struct _reent*, char*, size_t, const char*, ...) __attribute__((__format__(__printf__, 4, 5)));
int     _snprintf_r(struct _reent*, char* restrict, size_t, const char* restrict, ...) __attribute__((__format__(__printf__, 4, 5)));
int     _sprintf_r(struct _reent*, char* restrict, const char* restrict, ...) __attribute__((__format__(__printf__, 3, 4)));
int     _sscanf_r(struct _reent*, const char* restrict, const char* restrict, ...) __attribute__((__format__(__scanf__, 3, 4)));
char*   _tempnam_r(struct _reent*, const char*, const char*);
FILE*   _tmpfile_r(struct _reent*);
char*   _tmpnam_r(struct _reent*, char*);
int     _ungetc_r(struct _reent*, int, FILE*);
int     _vasiprintf_r(struct _reent*, char**, const char*, __gnuc_va_list) __attribute__((__format__(__printf__, 3, 0)));
char*   _vasniprintf_r(struct _reent*, char*, size_t*, const char*, __gnuc_va_list) __attribute__((__format__(__printf__, 4, 0)));
char*   _vasnprintf_r(struct _reent*, char*, size_t*, const char*, __gnuc_va_list) __attribute__((__format__(__printf__, 4, 0)));
int     _vasprintf_r(struct _reent*, char**, const char*, __gnuc_va_list) __attribute__((__format__(__printf__, 3, 0)));
int     _vdiprintf_r(struct _reent*, int, const char*, __gnuc_va_list) __attribute__((__format__(__printf__, 3, 0)));
int     _vdprintf_r(struct _reent*, int, const char* restrict, __gnuc_va_list) __attribute__((__format__(__printf__, 3, 0)));
int     _vfiprintf_r(struct _reent*, FILE*, const char*, __gnuc_va_list) __attribute__((__format__(__printf__, 3, 0)));
int     _vfiscanf_r(struct _reent*, FILE*, const char*, __gnuc_va_list) __attribute__((__format__(__scanf__, 3, 0)));
int     _vfprintf_r(struct _reent*, FILE* restrict, const char* restrict, __gnuc_va_list) __attribute__((__format__(__printf__, 3, 0)));
int     _vfscanf_r(struct _reent*, FILE* restrict, const char* restrict, __gnuc_va_list) __attribute__((__format__(__scanf__, 3, 0)));
int     _viprintf_r(struct _reent*, const char*, __gnuc_va_list) __attribute__((__format__(__printf__, 2, 0)));
int     _viscanf_r(struct _reent*, const char*, __gnuc_va_list) __attribute__((__format__(__scanf__, 2, 0)));
int     _vprintf_r(struct _reent*, const char* restrict, __gnuc_va_list) __attribute__((__format__(__printf__, 2, 0)));
int     _vscanf_r(struct _reent*, const char* restrict, __gnuc_va_list) __attribute__((__format__(__scanf__, 2, 0)));
int     _vsiprintf_r(struct _reent*, char*, const char*, __gnuc_va_list) __attribute__((__format__(__printf__, 3, 0)));
int     _vsiscanf_r(struct _reent*, const char*, const char*, __gnuc_va_list) __attribute__((__format__(__scanf__, 3, 0)));
int     _vsniprintf_r(struct _reent*, char*, size_t, const char*, __gnuc_va_list) __attribute__((__format__(__printf__, 4, 0)));
int     _vsnprintf_r(struct _reent*, char* restrict, size_t, const char* restrict, __gnuc_va_list) __attribute__((__format__(__printf__, 4, 0)));
int     _vsprintf_r(struct _reent*, char* restrict, const char* restrict, __gnuc_va_list) __attribute__((__format__(__printf__, 3, 0)));
int     _vsscanf_r(struct _reent*, const char* restrict, const char* restrict, __gnuc_va_list) __attribute__((__format__(__scanf__, 3, 0)));
int     fpurge(FILE*);
ssize_t __getdelim(char**, size_t*, int, FILE*);
ssize_t __getline(char**, size_t*, FILE*);
void    clearerr_unlocked(FILE*);
int     feof_unlocked(FILE*);
int     ferror_unlocked(FILE*);
int     fileno_unlocked(FILE*);
int     fflush_unlocked(FILE*);
int     fgetc_unlocked(FILE*);
int     fputc_unlocked(int, FILE*);
size_t  fread_unlocked(void* restrict, size_t _size, size_t _n, FILE* restrict);
size_t  fwrite_unlocked(const void* restrict, size_t _size, size_t _n, FILE*);
int     __srget_r(struct _reent*, FILE*);
int     __swbuf_r(struct _reent*, int, FILE*);
FILE*   funopen(const void* __cookie,
                int (*__readfn)(void* __cookie, char* __buf, int __n),
                int (*__writefn)(void* __cookie, const char* __buf, int __n),
                fpos_t (*__seekfn)(void* __cookie, fpos_t __off, int __whence),
                int (*__closefn)(void* __cookie));
FILE*   _funopen_r(struct _reent*, const void* __cookie, int (*__readfn)(void* __cookie, char* __buf, int __n),
                   int (*__writefn)(void* __cookie, const char* __buf, int __n), fpos_t (*__seekfn)(void* __cookie, fpos_t __off, int __whence),
                   int (*__closefn)(void* __cookie));
static __inline__ int __sputc_r(struct _reent* _ptr, int _c, FILE* _p)
{
    if(--_p->_w >= 0 || (_p->_w >= _p->_lbfsize && (char)_c != '\n'))
        return (*_p->_p++ = _c);
    else
        return (__swbuf_r(_ptr, _c, _p));
}
static __inline int _getchar_unlocked(void)
{
    struct _reent* _ptr;
    _ptr = _impure_ptr;
    return ((--(((_ptr)->_stdin))->_r < 0 ? __srget_r(_ptr, ((_ptr)->_stdin)) : (int)(*(((_ptr)->_stdin))->_p++)));
}
static __inline int _putchar_unlocked(int _c)
{
    struct _reent* _ptr;
    _ptr = _impure_ptr;
    return (__sputc_r(_ptr, _c, ((_ptr)->_stdout)));
}
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
typedef __uint32_t pgno_t;
typedef __uint16_t indx_t;
typedef __uint32_t recno_t;
typedef struct
{
    void*  data;
    size_t size;
} DBT;
typedef enum
{
    DB_BTREE,
    DB_HASH,
    DB_RECNO
} DBTYPE;
typedef struct __db
{
    DBTYPE type;
    int (*close)(struct __db*);
    int (*del)(const struct __db*, const DBT*, u_int);
    int (*get)(const struct __db*, const DBT*, DBT*, u_int);
    int (*put)(const struct __db*, DBT*, const DBT*, u_int);
    int (*seq)(const struct __db*, DBT*, DBT*, u_int);
    int (*sync)(const struct __db*, u_int);
    void* internal;
    int (*fd)(const struct __db*);
} DB;
typedef struct
{
    u_long flags;
    u_int  cachesize;
    int    maxkeypage;
    int    minkeypage;
    u_int  psize;
    int (*compare)(const DBT*, const DBT*);
    size_t (*prefix)(const DBT*, const DBT*);
    int lorder;
} BTREEINFO;
typedef struct
{
    u_int bsize;
    u_int ffactor;
    u_int nelem;
    u_int cachesize;
    __uint32_t (*hash)(const void*, size_t);
    int lorder;
} HASHINFO;
typedef struct
{
    u_long flags;
    u_int  cachesize;
    u_int  psize;
    int    lorder;
    size_t reclen;
    u_char bval;
    char*  bfname;
} RECNOINFO;
DB*                       dbopen(const char*, int, int, DBTYPE, const void*);
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
typedef enum
{
    HASH_GET,
    HASH_PUT,
    HASH_PUTNEW,
    HASH_DELETE,
    HASH_FIRST,
    HASH_NEXT
} ACTION;
typedef struct _bufhead BUFHEAD;
struct _bufhead
{
    BUFHEAD*   prev;
    BUFHEAD*   next;
    BUFHEAD*   ovfl;
    __uint32_t addr;
    char*      page;
    char       flags;
};
typedef BUFHEAD** SEGMENT;
typedef struct hashhdr
{
    int32_t    magic;
    int32_t    version;
    __uint32_t lorder;
    int32_t    bsize;
    int32_t    bshift;
    int32_t    dsize;
    int32_t    ssize;
    int32_t    sshift;
    int32_t    ovfl_point;
    int32_t    last_freed;
    int32_t    max_bucket;
    int32_t    high_mask;
    int32_t    low_mask;
    int32_t    ffactor;
    int32_t    nkeys;
    int32_t    hdrpages;
    int32_t    h_charkey;
    int32_t    spares[32];
    __uint16_t bitmaps[32];
} HASHHDR;
typedef struct htab
{
    HASHHDR hdr;
    int     nsegs;
    int     exsegs;
    __uint32_t (*hash)(const void*, size_t);
    int         flags;
    int         fp;
    char*       tmp_buf;
    char*       tmp_key;
    BUFHEAD*    cpage;
    int32_t     cbucket;
    int         cndx;
    int         error;
    int         new_file;
    int         save_file;
    __uint32_t* mapp[32];
    int         nmaps;
    int         nbufs;
    BUFHEAD     bufhead;
    SEGMENT*    dir;
} HTAB;
typedef struct
{
    BUFHEAD*   newp;
    BUFHEAD*   oldp;
    BUFHEAD*   nextp;
    __uint16_t next_addr;
} SPLIT_RETURN;
BUFHEAD*   __add_ovflpage(HTAB*, BUFHEAD*);
int        __addel(HTAB*, BUFHEAD*, const DBT*, const DBT*);
int        __big_delete(HTAB*, BUFHEAD*);
int        __big_insert(HTAB*, BUFHEAD*, const DBT*, const DBT*);
int        __big_keydata(HTAB*, BUFHEAD*, DBT*, DBT*, int);
int        __big_return(HTAB*, BUFHEAD*, int, DBT*, int);
int        __big_split(HTAB*, BUFHEAD*, BUFHEAD*, BUFHEAD*, int, __uint32_t, SPLIT_RETURN*);
int        __buf_free(HTAB*, int, int);
void       __buf_init(HTAB*, int);
__uint32_t __call_hash(HTAB*, char*, int);
int        __delpair(HTAB*, BUFHEAD*, int);
int        __expand_table(HTAB*);
int        __find_bigpair(HTAB*, BUFHEAD*, int, char*, int);
__uint16_t __find_last_page(HTAB*, BUFHEAD**);
void       __free_ovflpage(HTAB*, BUFHEAD*);
BUFHEAD*   __get_buf(HTAB*, __uint32_t, BUFHEAD*, int);
int        __get_page(HTAB*, char*, __uint32_t, int, int, int);
int        __ibitmap(HTAB*, int, int, int);
__uint32_t __log2(__uint32_t);
int        __put_page(HTAB*, char*, __uint32_t, int, int);
void       __reclaim_buf(HTAB*, BUFHEAD*);
int        __split_page(HTAB*, __uint32_t, __uint32_t);
extern __uint32_t (*__default_hash)(const void*, size_t);
static __uint32_t* fetch_bitmap(HTAB*, int);
static __uint32_t  first_free(__uint32_t);
static int         open_temp(HTAB*);
static __uint16_t  overflow_page(HTAB*);
static void        putpair(char*, const DBT*, const DBT*);
static void        squeeze_key(__uint16_t*, const DBT*, const DBT*);
static int         ugly_split(HTAB*, __uint32_t, BUFHEAD*, BUFHEAD*, int, int);
static void        putpair(p, key, val) char* p;
const DBT *        key, *val;
{
    __uint16_t *bp, n, off;
    bp  = (__uint16_t*)p;
    n   = bp[0];
    off = ((bp)[(bp)[0] + 2]) - key->size;
    memmove(p + off, key->data, key->size);
    bp[++n] = off;
    off -= val->size;
    memmove(p + off, val->data, val->size);
    bp[++n]   = off;
    bp[0]     = n;
    bp[n + 1] = off - ((n + 3) * sizeof(__uint16_t));
    bp[n + 2] = off;
}
extern int __delpair(hashp, bufp, ndx)
HTAB*      hashp;
BUFHEAD*   bufp;
int        ndx;
{
    __uint16_t *bp, newoff;
    int         n;
    __uint16_t  pairlen;
    bp = (__uint16_t*)bufp->page;
    n  = bp[0];
    if(bp[ndx + 1] < 4) return (__big_delete(hashp, bufp));
    if(ndx != 1)
        newoff = bp[ndx - 1];
    else
        newoff = hashp->hdr.bsize;
    pairlen = newoff - bp[ndx + 1];
    if(ndx != (n - 1))
    {
        int   i;
        char* src = bufp->page + (int)((bp)[(bp)[0] + 2]);
        char* dst = src + (int)pairlen;
        memmove(dst, src, bp[ndx + 1] - ((bp)[(bp)[0] + 2]));
        for(i = ndx + 2; i <= n; i += 2)
        {
            if(bp[i + 1] == 0)
            {
                bp[i - 2] = bp[i];
                bp[i - 1] = bp[i + 1];
            }
            else
            {
                bp[i - 2] = bp[i] + pairlen;
                bp[i - 1] = bp[i + 1] + pairlen;
            }
        }
    }
    bp[n]     = ((bp)[(bp)[0] + 2]) + pairlen;
    bp[n - 1] = bp[n + 1] + pairlen + 2 * sizeof(__uint16_t);
    bp[0]     = n - 2;
    hashp->hdr.nkeys--;
    bufp->flags |= 0x0001;
    return (0);
}
extern int __split_page(hashp, obucket, nbucket)
HTAB*      hashp;
__uint32_t obucket, nbucket;
{
    BUFHEAD *   new_bufp, *old_bufp;
    __uint16_t* ino;
    char*       np;
    DBT         key, val;
    int         n, ndx, retval;
    __uint16_t  copyto, diff, off, moved;
    char*       op;
    copyto   = (__uint16_t)hashp->hdr.bsize;
    off      = (__uint16_t)hashp->hdr.bsize;
    old_bufp = __get_buf(hashp, obucket, ((void*)0), 0);
    if(old_bufp == ((void*)0)) return (-1);
    new_bufp = __get_buf(hashp, nbucket, ((void*)0), 0);
    if(new_bufp == ((void*)0)) return (-1);
    old_bufp->flags |= (0x0001 | 0x0008);
    new_bufp->flags |= (0x0001 | 0x0008);
    ino   = (__uint16_t*)(op = old_bufp->page);
    np    = new_bufp->page;
    moved = 0;
    for(n = 1, ndx = 1; n < ino[0]; n += 2)
    {
        if(ino[n + 1] < 4)
        {
            retval = ugly_split(hashp, obucket, old_bufp, new_bufp, (int)copyto, (int)moved);
            old_bufp->flags &= ~0x0008;
            new_bufp->flags &= ~0x0008;
            return (retval);
        }
        key.data = (u_char*)op + ino[n];
        key.size = off - ino[n];
        if(__call_hash(hashp, key.data, key.size) == obucket)
        {
            diff = copyto - off;
            if(diff)
            {
                copyto = ino[n + 1] + diff;
                memmove(op + copyto, op + ino[n + 1], off - ino[n + 1]);
                ino[ndx]     = copyto + ino[n] - ino[n + 1];
                ino[ndx + 1] = copyto;
            }
            else
                copyto = ino[n + 1];
            ndx += 2;
        }
        else
        {
            val.data = (u_char*)op + ino[n + 1];
            val.size = ino[n] - ino[n + 1];
            putpair(np, &key, &val);
            moved += 2;
        }
        off = ino[n + 1];
    }
    ino[0] -= moved;
    ((ino)[(ino)[0] + 1]) = copyto - sizeof(__uint16_t) * (ino[0] + 3);
    ((ino)[(ino)[0] + 2]) = copyto;
    old_bufp->flags &= ~0x0008;
    new_bufp->flags &= ~0x0008;
    return (0);
}
static int ugly_split(hashp, obucket, old_bufp, new_bufp, copyto, moved)
HTAB*      hashp;
__uint32_t obucket;
BUFHEAD *  old_bufp, *new_bufp;
int        copyto;
int        moved;
{
    BUFHEAD*     bufp;
    __uint16_t*  ino;
    __uint16_t*  np;
    __uint16_t*  op;
    BUFHEAD*     last_bfp;
    DBT          key, val;
    SPLIT_RETURN ret;
    __uint16_t   n, off, ov_addr, scopyto;
    char*        cino;
    bufp     = old_bufp;
    ino      = (__uint16_t*)old_bufp->page;
    np       = (__uint16_t*)new_bufp->page;
    op       = (__uint16_t*)old_bufp->page;
    last_bfp = ((void*)0);
    scopyto  = (__uint16_t)copyto;
    n        = ino[0] - 1;
    while(n < ino[0])
    {
        if(ino[2] < 4 && ino[2] != 0)
        {
            if(__big_split(hashp, old_bufp, new_bufp, bufp, bufp->addr, obucket, &ret)) return (-1);
            old_bufp = ret.oldp;
            if(!old_bufp) return (-1);
            op       = (__uint16_t*)old_bufp->page;
            new_bufp = ret.newp;
            if(!new_bufp) return (-1);
            np   = (__uint16_t*)new_bufp->page;
            bufp = ret.nextp;
            if(!bufp) return (0);
            cino     = (char*)bufp->page;
            ino      = (__uint16_t*)cino;
            last_bfp = ret.nextp;
        }
        else if(ino[n + 1] == 0)
        {
            ov_addr = ino[n];
            ino[0] -= (moved + 2);
            ((ino)[(ino)[0] + 1]) = scopyto - sizeof(__uint16_t) * (ino[0] + 3);
            ((ino)[(ino)[0] + 2]) = scopyto;
            bufp                  = __get_buf(hashp, ov_addr, bufp, 0);
            if(!bufp) return (-1);
            ino     = (__uint16_t*)bufp->page;
            n       = 1;
            scopyto = hashp->hdr.bsize;
            moved   = 0;
            if(last_bfp) __free_ovflpage(hashp, last_bfp);
            last_bfp = bufp;
        }
        off = hashp->hdr.bsize;
        for(n = 1; (n < ino[0]) && (ino[n + 1] >= 4); n += 2)
        {
            cino     = (char*)ino;
            key.data = (u_char*)cino + ino[n];
            key.size = off - ino[n];
            val.data = (u_char*)cino + ino[n + 1];
            val.size = ino[n] - ino[n + 1];
            off      = ino[n + 1];
            if(__call_hash(hashp, key.data, key.size) == obucket)
            {
                if((((op)[2] >= 4) &&
                    ((2 * sizeof(__uint16_t) + (((&key)))->size + (((&val)))->size) + (2 * sizeof(__uint16_t))) <= (((op))[((op))[0] + 1])))
                    putpair((char*)op, &key, &val);
                else
                {
                    old_bufp = __add_ovflpage(hashp, old_bufp);
                    if(!old_bufp) return (-1);
                    op = (__uint16_t*)old_bufp->page;
                    putpair((char*)op, &key, &val);
                }
                old_bufp->flags |= 0x0001;
            }
            else
            {
                if((((np)[2] >= 4) &&
                    ((2 * sizeof(__uint16_t) + (((&key)))->size + (((&val)))->size) + (2 * sizeof(__uint16_t))) <= (((np))[((np))[0] + 1])))
                    putpair((char*)np, &key, &val);
                else
                {
                    new_bufp = __add_ovflpage(hashp, new_bufp);
                    if(!new_bufp) return (-1);
                    np = (__uint16_t*)new_bufp->page;
                    putpair((char*)np, &key, &val);
                }
                new_bufp->flags |= 0x0001;
            }
        }
    }
    if(last_bfp) __free_ovflpage(hashp, last_bfp);
    return (0);
}
extern int __addel(hashp, bufp, key, val)
HTAB*      hashp;
BUFHEAD*   bufp;
const DBT *key, *val;
{
    __uint16_t *bp, *sop;
    int         do_expand;
    bp        = (__uint16_t*)bufp->page;
    do_expand = 0;
    while(bp[0] && (bp[2] < 4 || bp[bp[0]] < 4))
        if(bp[2] == 3 && bp[0] == 2)
            break;
        else if(bp[2] < 4 && bp[bp[0]] != 0)
        {
            bufp = __get_buf(hashp, bp[bp[0] - 1], bufp, 0);
            if(!bufp) return (-1);
            bp = (__uint16_t*)bufp->page;
        }
        else if(((bp)[(bp)[0] + 1]) > (2 * sizeof(__uint16_t) + (key)->size + (val)->size))
        {
            squeeze_key(bp, key, val);
            return (0);
        }
        else
        {
            bufp = __get_buf(hashp, bp[bp[0] - 1], bufp, 0);
            if(!bufp) return (-1);
            bp = (__uint16_t*)bufp->page;
        }
    if((((bp)[2] >= 4) && ((2 * sizeof(__uint16_t) + ((key))->size + ((val))->size) + (2 * sizeof(__uint16_t))) <= (((bp))[((bp))[0] + 1])))
        putpair(bufp->page, key, val);
    else
    {
        do_expand = 1;
        bufp      = __add_ovflpage(hashp, bufp);
        if(!bufp) return (-1);
        sop = (__uint16_t*)bufp->page;
        if((((sop)[2] >= 4) && ((2 * sizeof(__uint16_t) + ((key))->size + ((val))->size) + (2 * sizeof(__uint16_t))) <= (((sop))[((sop))[0] + 1])))
            putpair((char*)sop, key, val);
        else if(__big_insert(hashp, bufp, key, val))
            return (-1);
    }
    bufp->flags |= 0x0001;
    hashp->hdr.nkeys++;
    if(do_expand || (hashp->hdr.nkeys / (hashp->hdr.max_bucket + 1) > hashp->hdr.ffactor)) return (__expand_table(hashp));
    return (0);
}
extern BUFHEAD* __add_ovflpage(hashp, bufp)
HTAB*           hashp;
BUFHEAD*        bufp;
{
    __uint16_t* sp;
    __uint16_t  ndx, ovfl_num;
    sp = (__uint16_t*)bufp->page;
    if(hashp->hdr.ffactor == 65536)
    {
        hashp->hdr.ffactor = sp[0] >> 1;
        if(hashp->hdr.ffactor < 4) hashp->hdr.ffactor = 4;
    }
    bufp->flags |= 0x0001;
    ovfl_num = overflow_page(hashp);
    if(!ovfl_num || !(bufp->ovfl = __get_buf(hashp, ovfl_num, bufp, 1))) return (((void*)0));
    bufp->ovfl->flags |= 0x0001;
    ndx         = sp[0];
    sp[ndx + 4] = ((sp)[(sp)[0] + 2]);
    sp[ndx + 3] = ((sp)[(sp)[0] + 1]) - (2 * sizeof(__uint16_t));
    sp[ndx + 1] = ovfl_num;
    sp[ndx + 2] = 0;
    sp[0]       = ndx + 2;
    return (bufp->ovfl);
}
extern int __get_page(hashp, p, bucket, is_bucket, is_disk, is_bitmap)
HTAB*      hashp;
char*      p;
__uint32_t bucket;
int        is_bucket, is_disk, is_bitmap;
{
    int         fd, page, size;
    int         rsize;
    __uint16_t* bp;
    fd   = hashp->fp;
    size = hashp->hdr.bsize;
    if((fd == -1) || !is_disk)
    {
        {
            ((__uint16_t*)(p))[0] = 0;
            ((__uint16_t*)(p))[1] = hashp->hdr.bsize - 3 * sizeof(__uint16_t);
            ((__uint16_t*)(p))[2] = hashp->hdr.bsize;
        };
        return (0);
    }
    if(is_bucket)
        page = (bucket) + hashp->hdr.hdrpages + ((bucket) ? hashp->hdr.spares[__log2((bucket) + 1) - 1] : 0);
    else
        page =
            ((1 << (((__uint32_t)((bucket))) >> 11)) - 1) + hashp->hdr.hdrpages +
            (((1 << (((__uint32_t)((bucket))) >> 11)) - 1) ? hashp->hdr.spares[__log2(((1 << (((__uint32_t)((bucket))) >> 11)) - 1) + 1) - 1] : 0) +
            (((bucket)) & 0x7FF);
    ;
    if((lseek(fd, (off_t)page << hashp->hdr.bshift, 0) == -1) || ((rsize = read(fd, p, size)) == -1)) return (-1);
    bp = (__uint16_t*)p;
    if(!rsize)
        bp[0] = 0;
    else if(rsize != size)
    {
        (*__errno()) = 79;
        return (-1);
    }
    if(!is_bitmap && !bp[0])
    {
        {
            ((__uint16_t*)(p))[0] = 0;
            ((__uint16_t*)(p))[1] = hashp->hdr.bsize - 3 * sizeof(__uint16_t);
            ((__uint16_t*)(p))[2] = hashp->hdr.bsize;
        };
    }
    else if(hashp->hdr.lorder != 1234)
    {
        int i, max;
        if(is_bitmap)
        {
            max = hashp->hdr.bsize >> 2;
            for(i = 0; i < max; i++)
            {
                __uint32_t _tmp           = ((int*)p)[i];
                ((char*)&((int*)p)[i])[0] = ((char*)&_tmp)[3];
                ((char*)&((int*)p)[i])[1] = ((char*)&_tmp)[2];
                ((char*)&((int*)p)[i])[2] = ((char*)&_tmp)[1];
                ((char*)&((int*)p)[i])[3] = ((char*)&_tmp)[0];
            };
        }
        else
        {
            {
                __uint16_t _tmp    = bp[0];
                ((char*)&bp[0])[0] = ((char*)&_tmp)[1];
                ((char*)&bp[0])[1] = ((char*)&_tmp)[0];
            };
            max = bp[0] + 2;
            for(i = 1; i <= max; i++)
            {
                __uint16_t _tmp    = bp[i];
                ((char*)&bp[i])[0] = ((char*)&_tmp)[1];
                ((char*)&bp[i])[1] = ((char*)&_tmp)[0];
            };
        }
    }
    return (0);
}
extern int __put_page(hashp, p, bucket, is_bucket, is_bitmap)
HTAB*      hashp;
char*      p;
__uint32_t bucket;
int        is_bucket, is_bitmap;
{
    int fd, page, size;
    int wsize;
    size = hashp->hdr.bsize;
    if((hashp->fp == -1) && open_temp(hashp)) return (-1);
    fd = hashp->fp;
    if(hashp->hdr.lorder != 1234)
    {
        int i;
        int max;
        if(is_bitmap)
        {
            max = hashp->hdr.bsize >> 2;
            for(i = 0; i < max; i++)
            {
                __uint32_t _tmp           = ((int*)p)[i];
                ((char*)&((int*)p)[i])[0] = ((char*)&_tmp)[3];
                ((char*)&((int*)p)[i])[1] = ((char*)&_tmp)[2];
                ((char*)&((int*)p)[i])[2] = ((char*)&_tmp)[1];
                ((char*)&((int*)p)[i])[3] = ((char*)&_tmp)[0];
            };
        }
        else
        {
            max = ((__uint16_t*)p)[0] + 2;
            for(i = 0; i <= max; i++)
            {
                __uint16_t _tmp                  = ((__uint16_t*)p)[i];
                ((char*)&((__uint16_t*)p)[i])[0] = ((char*)&_tmp)[1];
                ((char*)&((__uint16_t*)p)[i])[1] = ((char*)&_tmp)[0];
            };
        }
    }
    if(is_bucket)
        page = (bucket) + hashp->hdr.hdrpages + ((bucket) ? hashp->hdr.spares[__log2((bucket) + 1) - 1] : 0);
    else
        page =
            ((1 << (((__uint32_t)((bucket))) >> 11)) - 1) + hashp->hdr.hdrpages +
            (((1 << (((__uint32_t)((bucket))) >> 11)) - 1) ? hashp->hdr.spares[__log2(((1 << (((__uint32_t)((bucket))) >> 11)) - 1) + 1) - 1] : 0) +
            (((bucket)) & 0x7FF);
    ;
    if((lseek(fd, (off_t)page << hashp->hdr.bshift, 0) == -1) || ((wsize = write(fd, p, size)) == -1)) return (-1);
    if(wsize != size)
    {
        (*__errno()) = 79;
        return (-1);
    }
    return (0);
}
extern int __ibitmap(hashp, pnum, nbits, ndx)
HTAB*      hashp;
int        pnum, nbits, ndx;
{
    __uint32_t* ip;
    int         clearbytes, clearints;
    if((ip = (__uint32_t*)malloc(hashp->hdr.bsize)) == ((void*)0)) return (1);
    hashp->nmaps++;
    clearints  = ((nbits - 1) >> 5) + 1;
    clearbytes = clearints << 2;
    (void)memset((char*)ip, 0, clearbytes);
    (void)memset(((char*)ip) + clearbytes, 0xFF, hashp->hdr.bsize - clearbytes);
    ip[clearints - 1] = ((__uint32_t)0xFFFFFFFF) << (nbits & ((1 << 5) - 1));
    ((ip)[(0) / 32] |= (1 << ((0) % 32)));
    hashp->hdr.bitmaps[ndx] = (__uint16_t)pnum;
    hashp->mapp[ndx]        = ip;
    return (0);
}
static __uint32_t first_free(map)
__uint32_t        map;
{
    __uint32_t i, mask;
    mask = 0x1;
    for(i = 0; i < 32; i++)
    {
        if(!(mask & map)) return (i);
        mask = mask << 1;
    }
    return (i);
}
static __uint16_t overflow_page(hashp)
HTAB*             hashp;
{
    __uint32_t* freep = ((void*)0);
    int         max_free, offset, splitnum;
    __uint16_t  addr;
    int         bit, first_page, free_bit, free_page, i, in_use_bits, j;
    splitnum   = hashp->hdr.ovfl_point;
    max_free   = hashp->hdr.spares[splitnum];
    free_page  = (max_free - 1) >> (hashp->hdr.bshift + 3);
    free_bit   = (max_free - 1) & ((hashp->hdr.bsize << 3) - 1);
    first_page = hashp->hdr.last_freed >> (hashp->hdr.bshift + 3);
    for(i = first_page; i <= free_page; i++)
    {
        if(!(freep = (__uint32_t*)hashp->mapp[i]) && !(freep = fetch_bitmap(hashp, i))) return (0);
        if(i == free_page)
            in_use_bits = free_bit;
        else
            in_use_bits = (hashp->hdr.bsize << 3) - 1;
        if(i == first_page)
        {
            bit = hashp->hdr.last_freed & ((hashp->hdr.bsize << 3) - 1);
            j   = bit / 32;
            bit = bit & ~(32 - 1);
        }
        else
        {
            bit = 0;
            j   = 0;
        }
        for(; bit <= in_use_bits; j++, bit += 32)
            if(freep[j] != ((__uint32_t)0xFFFFFFFF)) goto found;
    }
    hashp->hdr.last_freed = hashp->hdr.spares[splitnum];
    hashp->hdr.spares[splitnum]++;
    offset = hashp->hdr.spares[splitnum] - (splitnum ? hashp->hdr.spares[splitnum - 1] : 0);
    if(offset > 0x7FF)
    {
        if(++splitnum >= 32)
        {
            (void)write(2, "HASH: Out of overflow pages.  Increase page size\n", sizeof("HASH: Out of overflow pages.  Increase page size\n") - 1);
            return (0);
        }
        hashp->hdr.ovfl_point       = splitnum;
        hashp->hdr.spares[splitnum] = hashp->hdr.spares[splitnum - 1];
        hashp->hdr.spares[splitnum - 1]--;
        offset = 1;
    }
    if(free_bit == (hashp->hdr.bsize << 3) - 1)
    {
        free_page++;
        if(free_page >= 32)
        {
            (void)write(2, "HASH: Out of overflow pages.  Increase page size\n", sizeof("HASH: Out of overflow pages.  Increase page size\n") - 1);
            return (0);
        }
        if(__ibitmap(hashp, (int)((__uint32_t)((__uint32_t)(splitnum) << 11) + (offset)), 1, free_page)) return (0);
        hashp->hdr.spares[splitnum]++;
        offset++;
        if(offset > 0x7FF)
        {
            if(++splitnum >= 32)
            {
                (void)write(2, "HASH: Out of overflow pages.  Increase page size\n",
                            sizeof("HASH: Out of overflow pages.  Increase page size\n") - 1);
                return (0);
            }
            hashp->hdr.ovfl_point       = splitnum;
            hashp->hdr.spares[splitnum] = hashp->hdr.spares[splitnum - 1];
            hashp->hdr.spares[splitnum - 1]--;
            offset = 0;
        }
    }
    else
    {
        free_bit++;
        ((freep)[(free_bit) / 32] |= (1 << ((free_bit) % 32)));
    }
    addr = ((__uint32_t)((__uint32_t)(splitnum) << 11) + (offset));
    return (addr);
found:
    bit = bit + first_free(freep[j]);
    ((freep)[(bit) / 32] |= (1 << ((bit) % 32)));
    bit = 1 + bit + (i * (hashp->hdr.bsize << 3));
    if(bit >= hashp->hdr.last_freed) hashp->hdr.last_freed = bit - 1;
    for(i = 0; (i < splitnum) && (bit > hashp->hdr.spares[i]); i++)
        ;
    offset = (i ? bit - hashp->hdr.spares[i - 1] : bit);
    if(offset >= 0x7FF) return (0);
    addr = ((__uint32_t)((__uint32_t)(i) << 11) + (offset));
    return (addr);
}
extern void __free_ovflpage(hashp, obufp) HTAB* hashp;
BUFHEAD*    obufp;
{
    __uint16_t  addr;
    __uint32_t* freep;
    int         bit_address, free_page, free_bit;
    __uint16_t  ndx;
    addr        = obufp->addr;
    ndx         = (((__uint16_t)addr) >> 11);
    bit_address = (ndx ? hashp->hdr.spares[ndx - 1] : 0) + (addr & 0x7FF) - 1;
    if(bit_address < hashp->hdr.last_freed) hashp->hdr.last_freed = bit_address;
    free_page = (bit_address >> (hashp->hdr.bshift + 3));
    free_bit  = bit_address & ((hashp->hdr.bsize << 3) - 1);
    if(!(freep = hashp->mapp[free_page])) freep = fetch_bitmap(hashp, free_page);
    ((freep)[(free_bit) / 32] &= ~(1 << ((free_bit) % 32)));
    __reclaim_buf(hashp, obufp);
}
static int open_temp(hashp)
HTAB*      hashp;
{
    sigset_t    set, oset;
    static char namestr[] = "_hashXXXXXX";
    (void)(*(&set) = ~(0), 0);
    (void)sigprocmask(1, &set, &oset);
    if((hashp->fp = mkstemp(namestr)) != -1) { (void)unlink(namestr); }
    (void)sigprocmask(0, &oset, (sigset_t*)((void*)0));
    return (hashp->fp != -1 ? 0 : -1);
}
static void squeeze_key(sp, key, val) __uint16_t* sp;
const DBT * key, *val;
{
    char*      p;
    __uint16_t free_space, n, off, pageno;
    p          = (char*)sp;
    n          = sp[0];
    free_space = ((sp)[(sp)[0] + 1]);
    off        = ((sp)[(sp)[0] + 2]);
    pageno     = sp[n - 1];
    off -= key->size;
    sp[n - 1] = off;
    memmove(p + off, key->data, key->size);
    off -= val->size;
    sp[n] = off;
    memmove(p + off, val->data, val->size);
    sp[0]               = n + 2;
    sp[n + 1]           = pageno;
    sp[n + 2]           = 0;
    ((sp)[(sp)[0] + 1]) = free_space - (2 * sizeof(__uint16_t) + (key)->size + (val)->size);
    ((sp)[(sp)[0] + 2]) = off;
}
static __uint32_t* fetch_bitmap(hashp, ndx)
HTAB*              hashp;
int                ndx;
{
    if(ndx >= hashp->nmaps) return (((void*)0));
    if((hashp->mapp[ndx] = (__uint32_t*)malloc(hashp->hdr.bsize)) == ((void*)0)) return (((void*)0));
    if(__get_page(hashp, (char*)hashp->mapp[ndx], hashp->hdr.bitmaps[ndx], 0, 1, 1))
    {
        free(hashp->mapp[ndx]);
        return (((void*)0));
    }
    return (hashp->mapp[ndx]);
}
