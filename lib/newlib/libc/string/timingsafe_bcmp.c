
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
int                        timingsafe_bcmp(const void* b1, const void* b2, size_t n)
{
    const unsigned char *p1 = b1, *p2 = b2;
    int                  ret = 0;
    for(; n > 0; n--) ret |= *p1++ ^ *p2++;
    return (ret != 0);
}
