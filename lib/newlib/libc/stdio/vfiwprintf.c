
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
struct stat;
struct tms;
struct timeval;
struct timezone;
extern int                _close_r(struct _reent*, int);
extern int                _execve_r(struct _reent*, const char*, char* const*, char* const*);
extern int                _fcntl_r(struct _reent*, int, int, int);
extern int                _fork_r(struct _reent*);
extern int                _fstat_r(struct _reent*, int, struct stat*);
extern int                _getpid_r(struct _reent*);
extern int                _isatty_r(struct _reent*, int);
extern int                _kill_r(struct _reent*, int, int);
extern int                _link_r(struct _reent*, const char*, const char*);
extern _off_t             _lseek_r(struct _reent*, int, _off_t, int);
extern int                _mkdir_r(struct _reent*, const char*, int);
extern int                _open_r(struct _reent*, const char*, int, int);
extern _ssize_t           _read_r(struct _reent*, int, void*, size_t);
extern int                _rename_r(struct _reent*, const char*, const char*);
extern void*              _sbrk_r(struct _reent*, ptrdiff_t);
extern int                _stat_r(struct _reent*, const char*, struct stat*);
extern unsigned long      _times_r(struct _reent*, struct tms*);
extern int                _unlink_r(struct _reent*, const char*);
extern int                _wait_r(struct _reent*, int*);
extern _ssize_t           _write_r(struct _reent*, int, const void*, size_t);
extern int                _gettimeofday_r(struct _reent*, struct timeval* __tp, void* __tzp);
typedef __builtin_va_list __gnuc_va_list;
typedef __gnuc_va_list    va_list;
typedef __uint8_t         u_int8_t;
typedef __uint16_t        u_int16_t;
typedef __uint32_t        u_int32_t;
typedef __uint64_t        u_int64_t;
typedef int               register_t;
typedef __int8_t          int8_t;
typedef __uint8_t         uint8_t;
typedef __int16_t         int16_t;
typedef __uint16_t        uint16_t;
typedef __int32_t         int32_t;
typedef __uint32_t        uint32_t;
typedef __int64_t         int64_t;
typedef __uint64_t        uint64_t;
typedef __intmax_t        intmax_t;
typedef __uintmax_t       uintmax_t;
typedef __intptr_t        intptr_t;
typedef __uintptr_t       uintptr_t;
typedef unsigned long     __sigset_t;
typedef __suseconds_t     suseconds_t;
typedef long              time_t;
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
typedef __FILE  FILE;
typedef _fpos_t fpos_t;
typedef int     _LOCK_T;
typedef int     _LOCK_RECURSIVE_T;
char*           ctermid(char*);
FILE*           tmpfile(void);
char*           tmpnam(char*);
char*           tempnam(const char*, const char*);
int             fclose(FILE*);
int             fflush(FILE*);
FILE*           freopen(const char* restrict, const char* restrict, FILE* restrict);
void            setbuf(FILE* restrict, char* restrict);
int             setvbuf(FILE* restrict, char* restrict, int, size_t);
int             fprintf(FILE* restrict, const char* restrict, ...) __attribute__((__format__(__printf__, 2, 3)));
int             fscanf(FILE* restrict, const char* restrict, ...) __attribute__((__format__(__scanf__, 2, 3)));
int             printf(const char* restrict, ...) __attribute__((__format__(__printf__, 1, 2)));
int             scanf(const char* restrict, ...) __attribute__((__format__(__scanf__, 1, 2)));
int             sscanf(const char* restrict, const char* restrict, ...) __attribute__((__format__(__scanf__, 2, 3)));
int             vfprintf(FILE* restrict, const char* restrict, __gnuc_va_list) __attribute__((__format__(__printf__, 2, 0)));
int             vprintf(const char*, __gnuc_va_list) __attribute__((__format__(__printf__, 1, 0)));
int             vsprintf(char* restrict, const char* restrict, __gnuc_va_list) __attribute__((__format__(__printf__, 2, 0)));
int             fgetc(FILE*);
char*           fgets(char* restrict, int, FILE* restrict);
int             fputc(int, FILE*);
int             fputs(const char* restrict, FILE* restrict);
int             getc(FILE*);
int             getchar(void);
char*           gets(char*);
int             putc(int, FILE*);
int             putchar(int);
int             puts(const char*);
int             ungetc(int, FILE*);
size_t          fread(void* restrict, size_t _size, size_t _n, FILE* restrict);
size_t          fwrite(const void* restrict, size_t _size, size_t _n, FILE*);
int             fgetpos(FILE* restrict, fpos_t* restrict);
int             fseek(FILE*, long, int);
int             fsetpos(FILE*, const fpos_t*);
long            ftell(FILE*);
void            rewind(FILE*);
void            clearerr(FILE*);
int             feof(FILE*);
int             ferror(FILE*);
void            perror(const char*);
FILE*           fopen(const char* restrict _name, const char* restrict _type);
int             sprintf(char* restrict, const char* restrict, ...) __attribute__((__format__(__printf__, 2, 3)));
int             remove(const char*);
int             rename(const char*, const char*);
int             fseeko(FILE*, off_t, int);
off_t           ftello(FILE*);
int             snprintf(char* restrict, size_t, const char* restrict, ...) __attribute__((__format__(__printf__, 3, 4)));
int             vsnprintf(char* restrict, size_t, const char* restrict, __gnuc_va_list) __attribute__((__format__(__printf__, 3, 0)));
int             vfscanf(FILE* restrict, const char* restrict, __gnuc_va_list) __attribute__((__format__(__scanf__, 2, 0)));
int             vscanf(const char*, __gnuc_va_list) __attribute__((__format__(__scanf__, 1, 0)));
int             vsscanf(const char* restrict, const char* restrict, __gnuc_va_list) __attribute__((__format__(__scanf__, 2, 0)));
int             asiprintf(char**, const char*, ...) __attribute__((__format__(__printf__, 2, 3)));
char*           asniprintf(char*, size_t*, const char*, ...) __attribute__((__format__(__printf__, 3, 4)));
char*           asnprintf(char* restrict, size_t* restrict, const char* restrict, ...) __attribute__((__format__(__printf__, 3, 4)));
int             diprintf(int, const char*, ...) __attribute__((__format__(__printf__, 2, 3)));
int             fiprintf(FILE*, const char*, ...) __attribute__((__format__(__printf__, 2, 3)));
int             fiscanf(FILE*, const char*, ...) __attribute__((__format__(__scanf__, 2, 3)));
int             iprintf(const char*, ...) __attribute__((__format__(__printf__, 1, 2)));
int             iscanf(const char*, ...) __attribute__((__format__(__scanf__, 1, 2)));
int             siprintf(char*, const char*, ...) __attribute__((__format__(__printf__, 2, 3)));
int             siscanf(const char*, const char*, ...) __attribute__((__format__(__scanf__, 2, 3)));
int             sniprintf(char*, size_t, const char*, ...) __attribute__((__format__(__printf__, 3, 4)));
int             vasiprintf(char**, const char*, __gnuc_va_list) __attribute__((__format__(__printf__, 2, 0)));
char*           vasniprintf(char*, size_t*, const char*, __gnuc_va_list) __attribute__((__format__(__printf__, 3, 0)));
char*           vasnprintf(char*, size_t*, const char*, __gnuc_va_list) __attribute__((__format__(__printf__, 3, 0)));
int             vdiprintf(int, const char*, __gnuc_va_list) __attribute__((__format__(__printf__, 2, 0)));
int             vfiprintf(FILE*, const char*, __gnuc_va_list) __attribute__((__format__(__printf__, 2, 0)));
int             vfiscanf(FILE*, const char*, __gnuc_va_list) __attribute__((__format__(__scanf__, 2, 0)));
int             viprintf(const char*, __gnuc_va_list) __attribute__((__format__(__printf__, 1, 0)));
int             viscanf(const char*, __gnuc_va_list) __attribute__((__format__(__scanf__, 1, 0)));
int             vsiprintf(char*, const char*, __gnuc_va_list) __attribute__((__format__(__printf__, 2, 0)));
int             vsiscanf(const char*, const char*, __gnuc_va_list) __attribute__((__format__(__scanf__, 2, 0)));
int             vsniprintf(char*, size_t, const char*, __gnuc_va_list) __attribute__((__format__(__printf__, 3, 0)));
FILE*           fdopen(int, const char*);
int             fileno(FILE*);
int             pclose(FILE*);
FILE*           popen(const char*, const char*);
void            setbuffer(FILE*, char*, int);
int             setlinebuf(FILE*);
int             getw(FILE*);
int             putw(int, FILE*);
int             getc_unlocked(FILE*);
int             getchar_unlocked(void);
void            flockfile(FILE*);
int             ftrylockfile(FILE*);
void            funlockfile(FILE*);
int             putc_unlocked(int, FILE*);
int             putchar_unlocked(int);
int             dprintf(int, const char* restrict, ...) __attribute__((__format__(__printf__, 2, 3)));
FILE*           fmemopen(void* restrict, size_t, const char* restrict);
FILE*           open_memstream(char**, size_t*);
int             vdprintf(int, const char* restrict, __gnuc_va_list) __attribute__((__format__(__printf__, 2, 0)));
int             renameat(int, const char*, int, const char*);
int             _asiprintf_r(struct _reent*, char**, const char*, ...) __attribute__((__format__(__printf__, 3, 4)));
char*           _asniprintf_r(struct _reent*, char*, size_t*, const char*, ...) __attribute__((__format__(__printf__, 4, 5)));
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
struct __locale_t;
typedef struct __locale_t* locale_t;
int                        bcmp(const void*, const void*, size_t) __attribute__((__pure__));
void                       bcopy(const void*, void*, size_t);
void                       bzero(void*, size_t);
void                       explicit_bzero(void*, size_t);
int                        ffs(int) __attribute__((__const__));
int                        ffsl(long) __attribute__((__const__));
int                        ffsll(long long) __attribute__((__const__));
int                        fls(int) __attribute__((__const__));
int                        flsl(long) __attribute__((__const__));
int                        flsll(long long) __attribute__((__const__));
char*                      index(const char*, int) __attribute__((__pure__));
char*                      rindex(const char*, int) __attribute__((__pure__));
int                        strcasecmp(const char*, const char*) __attribute__((__pure__));
int                        strncasecmp(const char*, const char*, size_t) __attribute__((__pure__));
int                        strcasecmp_l(const char*, const char*, locale_t);
int                        strncasecmp_l(const char*, const char*, size_t, locale_t);
void*                      memchr(const void*, int, size_t);
int                        memcmp(const void*, const void*, size_t);
void*                      memcpy(void* restrict, const void* restrict, size_t);
void*                      memmove(void*, const void*, size_t);
void*                      memset(void*, int, size_t);
char*                      strcat(char* restrict, const char* restrict);
char*                      strchr(const char*, int);
int                        strcmp(const char*, const char*);
int                        strcoll(const char*, const char*);
char*                      strcpy(char* restrict, const char* restrict);
size_t                     strcspn(const char*, const char*);
char*                      strerror(int);
size_t                     strlen(const char*);
char*                      strncat(char* restrict, const char* restrict, size_t);
int                        strncmp(const char*, const char*, size_t);
char*                      strncpy(char* restrict, const char* restrict, size_t);
char*                      strpbrk(const char*, const char*);
char*                      strrchr(const char*, int);
size_t                     strspn(const char*, const char*);
char*                      strstr(const char*, const char*);
char*                      strtok(char* restrict, const char* restrict);
size_t                     strxfrm(char* restrict, const char* restrict, size_t);
int                        strcoll_l(const char*, const char*, locale_t);
char*                      strerror_l(int, locale_t);
size_t                     strxfrm_l(char* restrict, const char* restrict, size_t, locale_t);
char*                      strtok_r(char* restrict, const char* restrict, char** restrict);
int                        timingsafe_bcmp(const void*, const void*, size_t);
int                        timingsafe_memcmp(const void*, const void*, size_t);
void*                      memccpy(void* restrict, const void* restrict, int, size_t);
char*                      stpcpy(char* restrict, const char* restrict);
char*                      stpncpy(char* restrict, const char* restrict, size_t);
char*                      strdup(const char*);
char*                      _strdup_r(struct _reent*, const char*);
char*                      strndup(const char*, size_t);
char*                      _strndup_r(struct _reent*, const char*, size_t);
int                        strerror_r(int, char*, size_t) __asm__(""
                                                                  "__xpg_strerror_r");
char*                      _strerror_r(struct _reent*, int, int, int*);
size_t                     strlcat(char*, const char*, size_t);
size_t                     strlcpy(char*, const char*, size_t);
size_t                     strnlen(const char*, size_t);
char*                      strsep(char**, const char*);
char*                      strnstr(const char*, const char*, size_t) __attribute__((__pure__));
char*                      strlwr(char*);
char*                      strupr(char*);
char*                      strsignal(int __signo);
typedef __int_least8_t     int_least8_t;
typedef __uint_least8_t    uint_least8_t;
typedef __int_least16_t    int_least16_t;
typedef __uint_least16_t   uint_least16_t;
typedef __int_least32_t    int_least32_t;
typedef __uint_least32_t   uint_least32_t;
typedef __int_least64_t    int_least64_t;
typedef __uint_least64_t   uint_least64_t;
typedef int                int_fast8_t;
typedef unsigned int       uint_fast8_t;
typedef int                int_fast16_t;
typedef unsigned int       uint_fast16_t;
typedef int                int_fast32_t;
typedef unsigned int       uint_fast32_t;
typedef long int           int_fast64_t;
typedef long unsigned int  uint_fast64_t;
struct tm;
typedef _mbstate_t mbstate_t;
wint_t             btowc(int);
int                wctob(wint_t);
size_t             mbrlen(const char* restrict, size_t, mbstate_t* restrict);
size_t             mbrtowc(wchar_t* restrict, const char* restrict, size_t, mbstate_t* restrict);
size_t             _mbrtowc_r(struct _reent*, wchar_t*, const char*, size_t, mbstate_t*);
int                mbsinit(const mbstate_t*);
size_t             mbsnrtowcs(wchar_t* restrict, const char** restrict, size_t, size_t, mbstate_t* restrict);
size_t             _mbsnrtowcs_r(struct _reent*, wchar_t*, const char**, size_t, size_t, mbstate_t*);
size_t             mbsrtowcs(wchar_t* restrict, const char** restrict, size_t, mbstate_t* restrict);
size_t             _mbsrtowcs_r(struct _reent*, wchar_t*, const char**, size_t, mbstate_t*);
size_t             wcrtomb(char* restrict, wchar_t, mbstate_t* restrict);
size_t             _wcrtomb_r(struct _reent*, char*, wchar_t, mbstate_t*);
size_t             wcsnrtombs(char* restrict, const wchar_t** restrict, size_t, size_t, mbstate_t* restrict);
size_t             _wcsnrtombs_r(struct _reent*, char*, const wchar_t**, size_t, size_t, mbstate_t*);
size_t             wcsrtombs(char* restrict, const wchar_t** restrict, size_t, mbstate_t* restrict);
size_t             _wcsrtombs_r(struct _reent*, char*, const wchar_t**, size_t, mbstate_t*);
int                wcscasecmp(const wchar_t*, const wchar_t*);
wchar_t*           wcscat(wchar_t* restrict, const wchar_t* restrict);
wchar_t*           wcschr(const wchar_t*, wchar_t);
int                wcscmp(const wchar_t*, const wchar_t*);
int                wcscoll(const wchar_t*, const wchar_t*);
wchar_t*           wcscpy(wchar_t* restrict, const wchar_t* restrict);
wchar_t*           wcpcpy(wchar_t* restrict, const wchar_t* restrict);
wchar_t*           wcsdup(const wchar_t*);
wchar_t*           _wcsdup_r(struct _reent*, const wchar_t*);
size_t             wcscspn(const wchar_t*, const wchar_t*);
size_t             wcsftime(wchar_t* restrict, size_t, const wchar_t* restrict, const struct tm* restrict);
size_t             wcslcat(wchar_t*, const wchar_t*, size_t);
size_t             wcslcpy(wchar_t*, const wchar_t*, size_t);
size_t             wcslen(const wchar_t*);
int                wcsncasecmp(const wchar_t*, const wchar_t*, size_t);
wchar_t*           wcsncat(wchar_t* restrict, const wchar_t* restrict, size_t);
int                wcsncmp(const wchar_t*, const wchar_t*, size_t);
wchar_t*           wcsncpy(wchar_t* restrict, const wchar_t* restrict, size_t);
wchar_t*           wcpncpy(wchar_t* restrict, const wchar_t* restrict, size_t);
size_t             wcsnlen(const wchar_t*, size_t);
wchar_t*           wcspbrk(const wchar_t*, const wchar_t*);
wchar_t*           wcsrchr(const wchar_t*, wchar_t);
size_t             wcsspn(const wchar_t*, const wchar_t*);
wchar_t*           wcsstr(const wchar_t* restrict, const wchar_t* restrict);
wchar_t*           wcstok(wchar_t* restrict, const wchar_t* restrict, wchar_t** restrict);
double             wcstod(const wchar_t* restrict, wchar_t** restrict);
double             _wcstod_r(struct _reent*, const wchar_t*, wchar_t**);
float              wcstof(const wchar_t* restrict, wchar_t** restrict);
float              _wcstof_r(struct _reent*, const wchar_t*, wchar_t**);
size_t             wcsxfrm(wchar_t* restrict, const wchar_t* restrict, size_t);
extern int         wcscasecmp_l(const wchar_t*, const wchar_t*, locale_t);
extern int         wcsncasecmp_l(const wchar_t*, const wchar_t*, size_t, locale_t);
extern int         wcscoll_l(const wchar_t*, const wchar_t*, locale_t);
extern size_t      wcsxfrm_l(wchar_t* restrict, const wchar_t* restrict, size_t, locale_t);
wchar_t*           wmemchr(const wchar_t*, wchar_t, size_t);
int                wmemcmp(const wchar_t*, const wchar_t*, size_t);
wchar_t*           wmemcpy(wchar_t* restrict, const wchar_t* restrict, size_t);
wchar_t*           wmemmove(wchar_t*, const wchar_t*, size_t);
wchar_t*           wmemset(wchar_t*, wchar_t, size_t);
long               wcstol(const wchar_t* restrict, wchar_t** restrict, int);
long long          wcstoll(const wchar_t* restrict, wchar_t** restrict, int);
unsigned long      wcstoul(const wchar_t* restrict, wchar_t** restrict, int);
unsigned long long wcstoull(const wchar_t* restrict, wchar_t** restrict, int);
long               _wcstol_r(struct _reent*, const wchar_t*, wchar_t**, int);
long long          _wcstoll_r(struct _reent*, const wchar_t*, wchar_t**, int);
unsigned long      _wcstoul_r(struct _reent*, const wchar_t*, wchar_t**, int);
unsigned long long _wcstoull_r(struct _reent*, const wchar_t*, wchar_t**, int);
long double        wcstold(const wchar_t*, wchar_t**);
wint_t             fgetwc(__FILE*);
wchar_t*           fgetws(wchar_t* restrict, int, __FILE* restrict);
wint_t             fputwc(wchar_t, __FILE*);
int                fputws(const wchar_t* restrict, __FILE* restrict);
int                fwide(__FILE*, int);
wint_t             getwc(__FILE*);
wint_t             getwchar(void);
wint_t             putwc(wchar_t, __FILE*);
wint_t             putwchar(wchar_t);
wint_t             ungetwc(wint_t wc, __FILE*);
wint_t             _fgetwc_r(struct _reent*, __FILE*);
wint_t             _fgetwc_unlocked_r(struct _reent*, __FILE*);
wchar_t*           _fgetws_r(struct _reent*, wchar_t*, int, __FILE*);
wchar_t*           _fgetws_unlocked_r(struct _reent*, wchar_t*, int, __FILE*);
wint_t             _fputwc_r(struct _reent*, wchar_t, __FILE*);
wint_t             _fputwc_unlocked_r(struct _reent*, wchar_t, __FILE*);
int                _fputws_r(struct _reent*, const wchar_t*, __FILE*);
int                _fputws_unlocked_r(struct _reent*, const wchar_t*, __FILE*);
int                _fwide_r(struct _reent*, __FILE*, int);
wint_t             _getwc_r(struct _reent*, __FILE*);
wint_t             _getwc_unlocked_r(struct _reent*, __FILE*);
wint_t             _getwchar_r(struct _reent* ptr);
wint_t             _getwchar_unlocked_r(struct _reent* ptr);
wint_t             _putwc_r(struct _reent*, wchar_t, __FILE*);
wint_t             _putwc_unlocked_r(struct _reent*, wchar_t, __FILE*);
wint_t             _putwchar_r(struct _reent*, wchar_t);
wint_t             _putwchar_unlocked_r(struct _reent*, wchar_t);
wint_t             _ungetwc_r(struct _reent*, wint_t wc, __FILE*);
__FILE*            open_wmemstream(wchar_t**, size_t*);
__FILE*            _open_wmemstream_r(struct _reent*, wchar_t**, size_t*);
int                fwprintf(__FILE* restrict, const wchar_t* restrict, ...);
int                swprintf(wchar_t* restrict, size_t, const wchar_t* restrict, ...);
int                vfwprintf(__FILE* restrict, const wchar_t* restrict, __gnuc_va_list);
int                vswprintf(wchar_t* restrict, size_t, const wchar_t* restrict, __gnuc_va_list);
int                vwprintf(const wchar_t* restrict, __gnuc_va_list);
int                wprintf(const wchar_t* restrict, ...);
int                _fwprintf_r(struct _reent*, __FILE*, const wchar_t*, ...);
int                _swprintf_r(struct _reent*, wchar_t*, size_t, const wchar_t*, ...);
int                _vfwprintf_r(struct _reent*, __FILE*, const wchar_t*, __gnuc_va_list);
int                _vswprintf_r(struct _reent*, wchar_t*, size_t, const wchar_t*, __gnuc_va_list);
int                _vwprintf_r(struct _reent*, const wchar_t*, __gnuc_va_list);
int                _wprintf_r(struct _reent*, const wchar_t*, ...);
int                fwscanf(__FILE* restrict, const wchar_t* restrict, ...);
int                swscanf(const wchar_t* restrict, const wchar_t* restrict, ...);
int                vfwscanf(__FILE* restrict, const wchar_t* restrict, __gnuc_va_list);
int                vswscanf(const wchar_t* restrict, const wchar_t* restrict, __gnuc_va_list);
int                vwscanf(const wchar_t* restrict, __gnuc_va_list);
int                wscanf(const wchar_t* restrict, ...);
int                _fwscanf_r(struct _reent*, __FILE*, const wchar_t*, ...);
int                _swscanf_r(struct _reent*, const wchar_t*, const wchar_t*, ...);
int                _vfwscanf_r(struct _reent*, __FILE*, const wchar_t*, __gnuc_va_list);
int                _vswscanf_r(struct _reent*, const wchar_t*, const wchar_t*, __gnuc_va_list);
int                _vwscanf_r(struct _reent*, const wchar_t*, __gnuc_va_list);
int                _wscanf_r(struct _reent*, const wchar_t*, ...);
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
extern wint_t      __fgetwc(struct _reent*, FILE*);
extern wint_t      __fputwc(struct _reent*, wchar_t, FILE*);
extern u_char*     __sccl(char*, u_char* fmt);
extern int         __svfscanf_r(struct _reent*, FILE*, const char*, va_list);
extern int         __ssvfscanf_r(struct _reent*, FILE*, const char*, va_list);
extern int         __svfiscanf_r(struct _reent*, FILE*, const char*, va_list);
extern int         __ssvfiscanf_r(struct _reent*, FILE*, const char*, va_list);
extern int         __svfwscanf_r(struct _reent*, FILE*, const wchar_t*, va_list);
extern int         __ssvfwscanf_r(struct _reent*, FILE*, const wchar_t*, va_list);
extern int         __svfiwscanf_r(struct _reent*, FILE*, const wchar_t*, va_list);
extern int         __ssvfiwscanf_r(struct _reent*, FILE*, const wchar_t*, va_list);
int                _svfprintf_r(struct _reent*, FILE*, const char*, va_list) __attribute__((__format__(__printf__, 3, 0)));
int                _svfiprintf_r(struct _reent*, FILE*, const char*, va_list) __attribute__((__format__(__printf__, 3, 0)));
int                _svfwprintf_r(struct _reent*, FILE*, const wchar_t*, va_list);
int                _svfiwprintf_r(struct _reent*, FILE*, const wchar_t*, va_list);
extern FILE*       __sfp(struct _reent*);
extern int         __sflags(struct _reent*, const char*, int*);
extern int         __sflush_r(struct _reent*, FILE*);
extern int         __srefill_r(struct _reent*, FILE*);
extern int         __sread(struct _reent*, void*, char*, int);
extern int         __seofread(struct _reent*, void*, char*, int);
extern int         __swrite(struct _reent*, void*, const char*, int);
extern _fpos_t     __sseek(struct _reent*, void*, _fpos_t, int);
extern int         __sclose(struct _reent*, void*);
extern int         __stextmode(int);
extern void        __sinit(struct _reent*);
extern void        _cleanup_r(struct _reent*);
extern void        __smakebuf_r(struct _reent*, FILE*);
extern int         __swhatbuf_r(struct _reent*, FILE*, size_t*, int*);
extern int         _fwalk(struct _reent*, int (*)(FILE*));
extern int         _fwalk_reent(struct _reent*, int (*)(struct _reent*, FILE*));
struct _glue*      __sfmoreglue(struct _reent*, int n);
extern int         __submore(struct _reent*, FILE*);
char*              _dcvt(struct _reent*, char*, double, int, int, char, int);
char*              _sicvt(char*, short, char);
char*              _icvt(char*, int, char);
char*              _licvt(char*, long, char);
char*              _llicvt(char*, long long, char);
void               __sfp_lock_acquire(void);
void               __sfp_lock_release(void);
void               __sinit_lock_acquire(void);
void               __sinit_lock_release(void);
typedef enum __attribute__((__packed__))
{
    ZERO,
    DIGIT,
    DOLLAR,
    MODFR,
    SPEC,
    DOT,
    STAR,
    FLAG,
    OTHER,
    MAX_CH_CLASS
} __CH_CLASS;
typedef enum __attribute__((__packed__))
{
    START,
    SFLAG,
    WDIG,
    WIDTH,
    SMOD,
    SDOT,
    VARW,
    VARP,
    PREC,
    VWDIG,
    VPDIG,
    DONE,
    MAX_STATE,
} __STATE;
typedef enum __attribute__((__packed__))
{
    NOOP,
    NUMBER,
    SKIPNUM,
    GETMOD,
    GETARG,
    GETPW,
    GETPWB,
    GETPOS,
    PWPOS,
} __ACTION;
extern const __CH_CLASS __chclass[256];
extern const __STATE    __state_table[MAX_STATE][MAX_CH_CLASS];
extern const __ACTION   __action_table[MAX_STATE][MAX_CH_CLASS];
struct __siov
{
    const void* iov_base;
    size_t      iov_len;
};
struct __suio
{
    struct __siov* uio_iov;
    int            uio_iovcnt;
    size_t         uio_resid;
};
extern int __sfvwrite_r(struct _reent*, FILE*, struct __suio*);
extern int __swsetup_r(struct _reent*, FILE*);
typedef union
{
    double value;
    struct
    {
        unsigned int fraction1 : 32;
        unsigned int fraction0 : 20;
        unsigned int exponent : 11;
        unsigned int sign : 1;
    } number;
    struct
    {
        unsigned int function1 : 32;
        unsigned int function0 : 19;
        unsigned int quiet : 1;
        unsigned int exponent : 11;
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
        unsigned int fraction0 : 7;
        unsigned int fraction1 : 16;
        unsigned int exponent : 8;
        unsigned int sign : 1;
    } number;
    struct
    {
        unsigned int function1 : 16;
        unsigned int function0 : 6;
        unsigned int quiet : 1;
        unsigned int exponent : 8;
        unsigned int sign : 1;
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
    long double     extu_ld;
    struct ieee_ext extu_ext;
} ieee_ext_u;
typedef int          fp_rnd;
fp_rnd               fpgetround(void);
fp_rnd               fpsetround(fp_rnd);
typedef int          fp_except;
fp_except            fpgetmask(void);
fp_except            fpsetmask(fp_except);
fp_except            fpgetsticky(void);
fp_except            fpsetsticky(fp_except);
typedef int          fp_rdi;
fp_rdi               fpgetroundtoi(void);
fp_rdi               fpsetroundtoi(fp_rdi);
extern double        atan(double);
extern double        cos(double);
extern double        sin(double);
extern double        tan(double);
extern double        tanh(double);
extern double        frexp(double, int*);
extern double        modf(double, double*);
extern double        ceil(double);
extern double        fabs(double);
extern double        floor(double);
extern double        acos(double);
extern double        asin(double);
extern double        atan2(double, double);
extern double        cosh(double);
extern double        sinh(double);
extern double        exp(double);
extern double        ldexp(double, int);
extern double        log(double);
extern double        log10(double);
extern double        pow(double, double);
extern double        sqrt(double);
extern double        fmod(double, double);
extern int           finite(double);
extern int           finitef(float);
extern int           finitel(long double);
extern int           isinff(float);
extern int           isnanf(float);
extern int           isinf(double);
extern int           isnan(double);
typedef float        float_t;
typedef double       double_t;
extern int           __isinff(float x);
extern int           __isinfd(double x);
extern int           __isnanf(float x);
extern int           __isnand(double x);
extern int           __fpclassifyf(float x);
extern int           __fpclassifyd(double x);
extern int           __signbitf(float x);
extern int           __signbitd(double x);
extern double        infinity(void);
extern double        nan(const char*);
extern double        copysign(double, double);
extern double        logb(double);
extern int           ilogb(double);
extern double        asinh(double);
extern double        cbrt(double);
extern double        nextafter(double, double);
extern double        rint(double);
extern double        scalbn(double, int);
extern double        exp2(double);
extern double        scalbln(double, long int);
extern double        tgamma(double);
extern double        nearbyint(double);
extern long int      lrint(double);
extern long long int llrint(double);
extern double        round(double);
extern long int      lround(double);
extern long long int llround(double);
extern double        trunc(double);
extern double        remquo(double, double, int*);
extern double        fdim(double, double);
extern double        fmax(double, double);
extern double        fmin(double, double);
extern double        fma(double, double, double);
extern double        log1p(double);
extern double        expm1(double);
extern double        acosh(double);
extern double        atanh(double);
extern double        remainder(double, double);
extern double        gamma(double);
extern double        lgamma(double);
extern double        erf(double);
extern double        erfc(double);
extern double        log2(double);
extern double        hypot(double, double);
extern float         atanf(float);
extern float         cosf(float);
extern float         sinf(float);
extern float         tanf(float);
extern float         tanhf(float);
extern float         frexpf(float, int*);
extern float         modff(float, float*);
extern float         ceilf(float);
extern float         fabsf(float);
extern float         floorf(float);
extern float         acosf(float);
extern float         asinf(float);
extern float         atan2f(float, float);
extern float         coshf(float);
extern float         sinhf(float);
extern float         expf(float);
extern float         ldexpf(float, int);
extern float         logf(float);
extern float         log10f(float);
extern float         powf(float, float);
extern float         sqrtf(float);
extern float         fmodf(float, float);
extern float         exp2f(float);
extern float         scalblnf(float, long int);
extern float         tgammaf(float);
extern float         nearbyintf(float);
extern long int      lrintf(float);
extern long long int llrintf(float);
extern float         roundf(float);
extern long int      lroundf(float);
extern long long int llroundf(float);
extern float         truncf(float);
extern float         remquof(float, float, int*);
extern float         fdimf(float, float);
extern float         fmaxf(float, float);
extern float         fminf(float, float);
extern float         fmaf(float, float, float);
extern float         infinityf(void);
extern float         nanf(const char*);
extern float         copysignf(float, float);
extern float         logbf(float);
extern int           ilogbf(float);
extern float         asinhf(float);
extern float         cbrtf(float);
extern float         nextafterf(float, float);
extern float         rintf(float);
extern float         scalbnf(float, int);
extern float         log1pf(float);
extern float         expm1f(float);
extern float         acoshf(float);
extern float         atanhf(float);
extern float         remainderf(float, float);
extern float         gammaf(float);
extern float         lgammaf(float);
extern float         erff(float);
extern float         erfcf(float);
extern float         log2f(float);
extern float         hypotf(float, float);
extern long double   hypotl(long double, long double);
extern long double   sqrtl(long double);
extern double        drem(double, double);
extern float         dremf(float, float);
extern double        gamma_r(double, int*);
extern double        lgamma_r(double, int*);
extern float         gammaf_r(float, int*);
extern float         lgammaf_r(float, int*);
extern double        y0(double);
extern double        y1(double);
extern double        yn(int, double);
extern double        j0(double);
extern double        j1(double);
extern double        jn(int, double);
extern float         y0f(float);
extern float         y1f(float);
extern float         ynf(int, float);
extern float         j0f(float);
extern float         j1f(float);
extern float         jnf(int, float);
extern int*          __signgam(void);
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
typedef int                  error_t;
extern int*                  __errno(void);
extern const char* const     _sys_errlist[];
extern int                   _sys_nerr;
union double_union
{
    double     d;
    __uint32_t i[2];
};
int        _vfiwprintf_r(struct _reent*, FILE*, const wchar_t*, va_list);
int        __sprint_r(struct _reent*, FILE*, register struct __suio*);
static int __sbwprintf(struct _reent* rptr, register FILE* fp, const wchar_t* fmt, va_list ap)
{
    int           ret;
    FILE          fake;
    unsigned char buf[1024];
    fake._flags    = fp->_flags & ~0x0002;
    fake._flags2   = fp->_flags2;
    fake._file     = fp->_file;
    fake._cookie   = fp->_cookie;
    fake._write    = fp->_write;
    fake._bf._base = fake._p = buf;
    fake._bf._size = fake._w = sizeof(buf);
    fake._lbfsize            = 0;
    ((void)0);
    ret = _vfiwprintf_r(rptr, &fake, fmt, ap);
    if(ret >= 0 && _fflush_r(rptr, &fake)) ret = (-1);
    if(fake._flags & 0x0040) fp->_flags |= 0x0040;
    ((void)0);
    return (ret);
}
typedef long*    quad_ptr_t;
typedef void*    void_ptr_t;
typedef char*    char_ptr_t;
typedef wchar_t* wchar_ptr_t;
typedef long*    long_ptr_t;
typedef int*     int_ptr_t;
typedef short*   short_ptr_t;
int              vfiwprintf(FILE* restrict fp, const wchar_t* restrict fmt0, va_list ap)
{
    int result;
    result = _vfiwprintf_r(_impure_ptr, fp, fmt0, ap);
    return result;
}
int _vfiwprintf_r(struct _reent* data, FILE* fp, const wchar_t* fmt0, va_list ap)
{
    register wchar_t* fmt;
    register wint_t   ch;
    register int      n, m;
    register wchar_t* cp;
    register int      flags;
    wchar_t*          fmt_anchor;
    int               ret;
    int               width;
    int               prec;
    wchar_t           sign;
    unsigned long     _uquad;
    enum
    {
        OCT,
        DEC,
        HEX
    } base;
    int                     dprec;
    int                     realsz;
    int                     size  = 0;
    wchar_t*                xdigs = ((void*)0);
    struct __suio           uio;
    struct __siov           iov[8];
    register struct __siov* iovp;
    wchar_t                 buf[40];
    wchar_t                 ox[2];
    wchar_t*                malloc_buf = ((void*)0);
    static const wchar_t    blanks[16] = {L' ', L' ', L' ', L' ', L' ', L' ', L' ', L' ', L' ', L' ', L' ', L' ', L' ', L' ', L' ', L' '};
    static const wchar_t    zeroes[16] = {L'0', L'0', L'0', L'0', L'0', L'0', L'0', L'0', L'0', L'0', L'0', L'0', L'0', L'0', L'0', L'0'};
    do {
        struct _reent* _check_init_ptr = (data);
        if((_check_init_ptr) && !(_check_init_ptr)->__sdidinit) __sinit(_check_init_ptr);
    } while(0);
    {
        if(!(fp->_flags2 & 0x0001)) (((fp)->_flags & 0x0200) ? 0 : ((void)0));
        do {
            if(!((fp)->_flags & 0x2000))
            {
                (fp)->_flags |= 0x2000;
                if(1 > 0)
                    (fp)->_flags2 |= 0x2000;
                else
                    (fp)->_flags2 &= ~0x2000;
            }
        } while(0);
        if(((((fp)->_flags & 0x0008) == 0 || (fp)->_bf._base == ((void*)0)) && __swsetup_r(data, fp)))
        {
            if(!(fp->_flags2 & 0x0001)) (((fp)->_flags & 0x0200) ? 0 : ((void)0));
            ;
            return ((-1));
        }
        if((fp->_flags & (0x0002 | 0x0008 | 0x0010)) == (0x0002 | 0x0008) && fp->_file >= 0)
        {
            if(!(fp->_flags2 & 0x0001)) (((fp)->_flags & 0x0200) ? 0 : ((void)0));
            ;
            return (__sbwprintf(data, fp, fmt0, ap));
        }
        fmt         = (wchar_t*)fmt0;
        uio.uio_iov = iovp = iov;
        uio.uio_resid      = 0;
        uio.uio_iovcnt     = 0;
        ret                = 0;
        for(;;)
        {
            cp = fmt;
            while(*fmt != L'\0' && *fmt != L'%') ++fmt;
            if((m = fmt - cp) != 0)
            {
                {
                    iovp->iov_base = (char*)(cp);
                    iovp->iov_len  = (m) * sizeof(wchar_t);
                    uio.uio_resid += (m) * sizeof(wchar_t);
                    iovp++;
                    if(++uio.uio_iovcnt >= 8)
                    {
                        if(__sprint_r(data, fp, &uio)) goto error;
                        iovp = iov;
                    }
                };
                ret += m;
            }
            if(*fmt == L'\0') goto done;
            fmt_anchor = fmt;
            fmt++;
            flags = 0;
            dprec = 0;
            width = 0;
            prec  = -1;
            sign  = L'\0';
        rflag:
            ch = *fmt++;
        reswitch:
            switch(ch)
            {
            case L' ':
                if(!sign) sign = L' ';
                goto rflag;
            case L'#': flags |= 0x001; goto rflag;
            case L'*':
                width = (__builtin_va_arg(ap, int));
                if(width >= 0) goto rflag;
                width = -width;
            case L'-': flags |= 0x004; goto rflag;
            case L'+': sign = L'+'; goto rflag;
            case L'.':
                if((ch = *fmt++) == L'*')
                {
                    prec = (__builtin_va_arg(ap, int));
                    if(prec < 0) prec = -1;
                    goto rflag;
                }
                n = 0;
                while(((unsigned)((ch) - L'0') <= 9))
                {
                    n  = 10 * n + ((ch) - L'0');
                    ch = *fmt++;
                }
                prec = n < 0 ? -1 : n;
                goto reswitch;
            case L'0': flags |= 0x080; goto rflag;
            case L'1':
            case L'2':
            case L'3':
            case L'4':
            case L'5':
            case L'6':
            case L'7':
            case L'8':
            case L'9':
                n = 0;
                do {
                    n  = 10 * n + ((ch) - L'0');
                    ch = *fmt++;
                } while(((unsigned)((ch) - L'0') <= 9));
                width = n;
                goto reswitch;
            case L'h': flags |= 0x040; goto rflag;
            case L'l': flags |= 0x010; goto rflag;
            case L'q': flags |= 0x010; goto rflag;
            case L'c':
                cp = buf;
                if(ch == L'c' && !(flags & 0x010))
                {
                    wint_t wc = btowc((int)(__builtin_va_arg(ap, int)));
                    if(wc == ((wint_t)-1))
                    {
                        fp->_flags |= 0x0040;
                        goto error;
                    }
                    cp[0] = (wchar_t)wc;
                }
                else { cp[0] = (__builtin_va_arg(ap, int)); }
                cp[1] = L'\0';
                size  = 1;
                sign  = L'\0';
                break;
            case L'd':
            case L'i':
                _uquad = (flags & 0x010 ? (__builtin_va_arg(ap, long)) :
                          flags & 0x040 ? (long)(short)(__builtin_va_arg(ap, int)) :
                          flags & 0     ? (long)(signed char)(__builtin_va_arg(ap, int)) :
                                          (long)(__builtin_va_arg(ap, int)));
                if((long)_uquad < 0)
                {
                    _uquad = -_uquad;
                    sign   = L'-';
                }
                base = DEC;
                goto number;
            case L'n':
                if(flags & 0x010)
                    *(__builtin_va_arg(ap, long_ptr_t)) = ret;
                else if(flags & 0x040)
                    *(__builtin_va_arg(ap, short_ptr_t)) = ret;
                else
                    *(__builtin_va_arg(ap, int_ptr_t)) = ret;
                continue;
            case L'o':
                _uquad = (flags & 0x010 ? (__builtin_va_arg(ap, u_long)) :
                          flags & 0x040 ? (u_long)(u_short)(__builtin_va_arg(ap, int)) :
                          flags & 0     ? (u_long)(unsigned char)(__builtin_va_arg(ap, int)) :
                                          (u_long)(__builtin_va_arg(ap, u_int)));
                base   = OCT;
                goto nosign;
            case L'p':
                _uquad = (uintptr_t)(__builtin_va_arg(ap, void_ptr_t));
                base   = HEX;
                xdigs  = L"0123456789abcdef";
                flags |= 0x002;
                ox[0] = L'0';
                ox[1] = ch = L'x';
                goto nosign;
            case L's':
                cp   = (__builtin_va_arg(ap, wchar_ptr_t));
                sign = '\0';
                if(cp == ((void*)0))
                {
                    cp   = L"(null)";
                    size = ((unsigned)prec > 6U) ? 6 : prec;
                }
                else if(ch != L'S' && !(flags & 0x010))
                {
                    char*  arg    = (char*)cp;
                    size_t insize = 0;
                    if(prec >= 0)
                    {
                        char* p = memchr(arg, '\0', prec);
                        insize  = p ? p - arg : prec;
                    }
                    else
                        insize = strlen(arg);
                    if(insize >= 40)
                    {
                        if((malloc_buf = (wchar_t*)_malloc_r(data, (insize + 1) * sizeof(wchar_t))) == ((void*)0))
                        {
                            fp->_flags |= 0x0040;
                            goto error;
                        }
                        cp = malloc_buf;
                    }
                    else
                        cp = buf;
                    for(size = 0; size < insize; ++size) cp[size] = arg[size];
                    cp[size] = L'\0';
                }
                else if(prec >= 0)
                {
                    wchar_t* p = wmemchr(cp, L'\0', prec);
                    if(p != ((void*)0))
                    {
                        size = p - cp;
                        if(size > prec) size = prec;
                    }
                    else
                        size = prec;
                }
                else
                    size = wcslen(cp);
                break;
            case L'u':
                _uquad = (flags & 0x010 ? (__builtin_va_arg(ap, u_long)) :
                          flags & 0x040 ? (u_long)(u_short)(__builtin_va_arg(ap, int)) :
                          flags & 0     ? (u_long)(unsigned char)(__builtin_va_arg(ap, int)) :
                                          (u_long)(__builtin_va_arg(ap, u_int)));
                base   = DEC;
                goto nosign;
            case L'X': xdigs = L"0123456789ABCDEF"; goto hex;
            case L'x':
                xdigs = L"0123456789abcdef";
            hex:
                _uquad = (flags & 0x010 ? (__builtin_va_arg(ap, u_long)) :
                          flags & 0x040 ? (u_long)(u_short)(__builtin_va_arg(ap, int)) :
                          flags & 0     ? (u_long)(unsigned char)(__builtin_va_arg(ap, int)) :
                                          (u_long)(__builtin_va_arg(ap, u_int)));
                base   = HEX;
                if(flags & 0x001 && _uquad != 0)
                {
                    ox[0] = L'0';
                    ox[1] = ch;
                    flags |= 0x002;
                }
            nosign:
                sign = L'\0';
            number:
                if((dprec = prec) >= 0) flags &= ~0x080;
                cp = buf + 40;
                if(_uquad != 0 || prec != 0)
                {
                    switch(base)
                    {
                    case OCT:
                        do {
                            *--cp = ((_uquad & 7) + L'0');
                            _uquad >>= 3;
                        } while(_uquad);
                        if(flags & 0x001 && *cp != L'0') *--cp = L'0';
                        break;
                    case DEC:
                        if(_uquad < 10)
                        {
                            *--cp = ((_uquad) + L'0');
                            break;
                        }
                        do {
                            *--cp = ((_uquad % 10) + L'0');
                            _uquad /= 10;
                        } while(_uquad != 0);
                        break;
                    case HEX:
                        do {
                            *--cp = xdigs[_uquad & 15];
                            _uquad >>= 4;
                        } while(_uquad);
                        break;
                    default:
                        cp   = L"bug in vfprintf: bad base";
                        size = wcslen(cp);
                        goto skipsize;
                    }
                }
                else if(base == OCT && (flags & 0x001))
                    *--cp = L'0';
                size = buf + 40 - cp;
            skipsize:
                break;
            default:
                if(ch == L'\0') goto done;
                cp   = buf;
                *cp  = ch;
                size = 1;
                sign = L'\0';
                break;
            }
            realsz = dprec > size ? dprec : size;
            if(sign) realsz++;
            if(flags & 0x002) realsz += 2;
            if((flags & (0x004 | 0x080)) == 0)
            {
                if((n = (width - realsz)) > 0)
                {
                    while(n > 16)
                    {
                        {
                            iovp->iov_base = (char*)(blanks);
                            iovp->iov_len  = (16) * sizeof(wchar_t);
                            uio.uio_resid += (16) * sizeof(wchar_t);
                            iovp++;
                            if(++uio.uio_iovcnt >= 8)
                            {
                                if(__sprint_r(data, fp, &uio)) goto error;
                                iovp = iov;
                            }
                        };
                        n -= 16;
                    }
                    {
                        iovp->iov_base = (char*)(blanks);
                        iovp->iov_len  = (n) * sizeof(wchar_t);
                        uio.uio_resid += (n) * sizeof(wchar_t);
                        iovp++;
                        if(++uio.uio_iovcnt >= 8)
                        {
                            if(__sprint_r(data, fp, &uio)) goto error;
                            iovp = iov;
                        }
                    };
                }
            };
            if(sign)
            {
                iovp->iov_base = (char*)(&sign);
                iovp->iov_len  = (1) * sizeof(wchar_t);
                uio.uio_resid += (1) * sizeof(wchar_t);
                iovp++;
                if(++uio.uio_iovcnt >= 8)
                {
                    if(__sprint_r(data, fp, &uio)) goto error;
                    iovp = iov;
                }
            };
            if(flags & 0x002)
            {
                iovp->iov_base = (char*)(ox);
                iovp->iov_len  = (2) * sizeof(wchar_t);
                uio.uio_resid += (2) * sizeof(wchar_t);
                iovp++;
                if(++uio.uio_iovcnt >= 8)
                {
                    if(__sprint_r(data, fp, &uio)) goto error;
                    iovp = iov;
                }
            };
            if((flags & (0x004 | 0x080)) == 0x080)
            {
                if((n = (width - realsz)) > 0)
                {
                    while(n > 16)
                    {
                        {
                            iovp->iov_base = (char*)(zeroes);
                            iovp->iov_len  = (16) * sizeof(wchar_t);
                            uio.uio_resid += (16) * sizeof(wchar_t);
                            iovp++;
                            if(++uio.uio_iovcnt >= 8)
                            {
                                if(__sprint_r(data, fp, &uio)) goto error;
                                iovp = iov;
                            }
                        };
                        n -= 16;
                    }
                    {
                        iovp->iov_base = (char*)(zeroes);
                        iovp->iov_len  = (n) * sizeof(wchar_t);
                        uio.uio_resid += (n) * sizeof(wchar_t);
                        iovp++;
                        if(++uio.uio_iovcnt >= 8)
                        {
                            if(__sprint_r(data, fp, &uio)) goto error;
                            iovp = iov;
                        }
                    };
                }
            };
            {
                if((n = (dprec - size)) > 0)
                {
                    while(n > 16)
                    {
                        {
                            iovp->iov_base = (char*)(zeroes);
                            iovp->iov_len  = (16) * sizeof(wchar_t);
                            uio.uio_resid += (16) * sizeof(wchar_t);
                            iovp++;
                            if(++uio.uio_iovcnt >= 8)
                            {
                                if(__sprint_r(data, fp, &uio)) goto error;
                                iovp = iov;
                            }
                        };
                        n -= 16;
                    }
                    {
                        iovp->iov_base = (char*)(zeroes);
                        iovp->iov_len  = (n) * sizeof(wchar_t);
                        uio.uio_resid += (n) * sizeof(wchar_t);
                        iovp++;
                        if(++uio.uio_iovcnt >= 8)
                        {
                            if(__sprint_r(data, fp, &uio)) goto error;
                            iovp = iov;
                        }
                    };
                }
            };
            {
                iovp->iov_base = (char*)(cp);
                iovp->iov_len  = (size) * sizeof(wchar_t);
                uio.uio_resid += (size) * sizeof(wchar_t);
                iovp++;
                if(++uio.uio_iovcnt >= 8)
                {
                    if(__sprint_r(data, fp, &uio)) goto error;
                    iovp = iov;
                }
            };
            if(flags & 0x004)
            {
                if((n = (width - realsz)) > 0)
                {
                    while(n > 16)
                    {
                        {
                            iovp->iov_base = (char*)(blanks);
                            iovp->iov_len  = (16) * sizeof(wchar_t);
                            uio.uio_resid += (16) * sizeof(wchar_t);
                            iovp++;
                            if(++uio.uio_iovcnt >= 8)
                            {
                                if(__sprint_r(data, fp, &uio)) goto error;
                                iovp = iov;
                            }
                        };
                        n -= 16;
                    }
                    {
                        iovp->iov_base = (char*)(blanks);
                        iovp->iov_len  = (n) * sizeof(wchar_t);
                        uio.uio_resid += (n) * sizeof(wchar_t);
                        iovp++;
                        if(++uio.uio_iovcnt >= 8)
                        {
                            if(__sprint_r(data, fp, &uio)) goto error;
                            iovp = iov;
                        }
                    };
                }
            };
            ret += width > realsz ? width : realsz;
            {
                if(uio.uio_resid && __sprint_r(data, fp, &uio)) goto error;
                uio.uio_iovcnt = 0;
                iovp           = iov;
            };
            if(malloc_buf != ((void*)0))
            {
                _free_r(data, malloc_buf);
                malloc_buf = ((void*)0);
            }
        }
    done:
    {
        if(uio.uio_resid && __sprint_r(data, fp, &uio)) goto error;
        uio.uio_iovcnt = 0;
        iovp           = iov;
    };
    error:
        if(malloc_buf != ((void*)0)) _free_r(data, malloc_buf);
        if(!(fp->_flags2 & 0x0001)) (((fp)->_flags & 0x0200) ? 0 : ((void)0));
    };
    return (((int)(((fp)->_flags & 0x0040) != 0)) ? (-1) : ret);
}
