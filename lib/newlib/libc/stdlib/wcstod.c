
typedef int               error_t;
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
extern int*                 __errno(void);
extern const char* const    _sys_errlist[];
extern int                  _sys_nerr;
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
typedef __builtin_va_list  __gnuc_va_list;
typedef __gnuc_va_list     va_list;
typedef __FILE             FILE;
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
typedef int        wctype_t;
typedef int        wctrans_t;
int                iswalpha(wint_t);
int                iswalnum(wint_t);
int                iswblank(wint_t);
int                iswcntrl(wint_t);
int                iswctype(wint_t, wctype_t);
int                iswdigit(wint_t);
int                iswgraph(wint_t);
int                iswlower(wint_t);
int                iswprint(wint_t);
int                iswpunct(wint_t);
int                iswspace(wint_t);
int                iswupper(wint_t);
int                iswxdigit(wint_t);
wint_t             towctrans(wint_t, wctrans_t);
wint_t             towupper(wint_t);
wint_t             towlower(wint_t);
wctrans_t          wctrans(const char*);
wctype_t           wctype(const char*);
extern int         iswalpha_l(wint_t, locale_t);
extern int         iswalnum_l(wint_t, locale_t);
extern int         iswblank_l(wint_t, locale_t);
extern int         iswcntrl_l(wint_t, locale_t);
extern int         iswctype_l(wint_t, wctype_t, locale_t);
extern int         iswdigit_l(wint_t, locale_t);
extern int         iswgraph_l(wint_t, locale_t);
extern int         iswlower_l(wint_t, locale_t);
extern int         iswprint_l(wint_t, locale_t);
extern int         iswpunct_l(wint_t, locale_t);
extern int         iswspace_l(wint_t, locale_t);
extern int         iswupper_l(wint_t, locale_t);
extern int         iswxdigit_l(wint_t, locale_t);
extern wint_t      towctrans_l(wint_t, wctrans_t, locale_t);
extern wint_t      towupper_l(wint_t, locale_t);
extern wint_t      towlower_l(wint_t, locale_t);
extern wctrans_t   wctrans_l(const char*, locale_t);
extern wctype_t    wctype_l(const char*, locale_t);
struct lconv
{
    char* decimal_point;
    char* thousands_sep;
    char* grouping;
    char* int_curr_symbol;
    char* currency_symbol;
    char* mon_decimal_point;
    char* mon_thousands_sep;
    char* mon_grouping;
    char* positive_sign;
    char* negative_sign;
    char  int_frac_digits;
    char  frac_digits;
    char  p_cs_precedes;
    char  p_sep_by_space;
    char  n_cs_precedes;
    char  n_sep_by_space;
    char  p_sign_posn;
    char  n_sign_posn;
    char  int_n_cs_precedes;
    char  int_n_sep_by_space;
    char  int_n_sign_posn;
    char  int_p_cs_precedes;
    char  int_p_sep_by_space;
    char  int_p_sign_posn;
};
struct _reent;
char*                _setlocale_r(struct _reent*, int, const char*);
struct lconv*        _localeconv_r(struct _reent*);
struct __locale_t*   _newlocale_r(struct _reent*, int, const char*, struct __locale_t*);
void                 _freelocale_r(struct _reent*, struct __locale_t*);
struct __locale_t*   _duplocale_r(struct _reent*, struct __locale_t*);
struct __locale_t*   _uselocale_r(struct _reent*, struct __locale_t*);
char*                setlocale(int, const char*);
struct lconv*        localeconv(void);
locale_t             newlocale(int, const char*, locale_t);
void                 freelocale(locale_t);
locale_t             duplocale(locale_t);
locale_t             uselocale(locale_t);
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
typedef int           fp_rnd;
fp_rnd                fpgetround(void);
fp_rnd                fpsetround(fp_rnd);
typedef int           fp_except;
fp_except             fpgetmask(void);
fp_except             fpsetmask(fp_except);
fp_except             fpgetsticky(void);
fp_except             fpsetsticky(fp_except);
typedef int           fp_rdi;
fp_rdi                fpgetroundtoi(void);
fp_rdi                fpsetroundtoi(fp_rdi);
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
struct lc_ctype_T
{
    const char* codeset;
    const char* mb_cur_max;
};
extern const struct lc_ctype_T _C_ctype_locale;
struct lc_monetary_T
{
    const char* int_curr_symbol;
    const char* currency_symbol;
    const char* mon_decimal_point;
    const char* mon_thousands_sep;
    const char* mon_grouping;
    const char* positive_sign;
    const char* negative_sign;
    const char* int_frac_digits;
    const char* frac_digits;
    const char* p_cs_precedes;
    const char* p_sep_by_space;
    const char* n_cs_precedes;
    const char* n_sep_by_space;
    const char* p_sign_posn;
    const char* n_sign_posn;
};
extern const struct lc_monetary_T _C_monetary_locale;
struct lc_numeric_T
{
    const char* decimal_point;
    const char* thousands_sep;
    const char* grouping;
};
extern const struct lc_numeric_T _C_numeric_locale;
struct lc_time_T
{
    const char* mon[12];
    const char* month[12];
    const char* wday[7];
    const char* weekday[7];
    const char* X_fmt;
    const char* x_fmt;
    const char* c_fmt;
    const char* am_pm[2];
    const char* date_fmt;
    const char* alt_month[12];
    const char* md_order;
    const char* ampm_fmt;
    const char* era;
    const char* era_d_fmt;
    const char* era_d_t_fmt;
    const char* era_t_fmt;
    const char* alt_digits;
};
extern const struct lc_time_T _C_time_locale;
struct lc_messages_T
{
    const char* yesexpr;
    const char* noexpr;
    const char* yesstr;
    const char* nostr;
};
extern const struct lc_messages_T _C_messages_locale;
struct __lc_cats
{
    const void* ptr;
    char*       buf;
};
struct __locale_t
{
    char categories[7][31 + 1];
    int (*wctomb)(struct _reent*, char*, wchar_t, mbstate_t*);
    int (*mbtowc)(struct _reent*, wchar_t*, const char*, size_t, mbstate_t*);
    int          cjk_lang;
    char*        ctype_ptr;
    struct lconv lconv;
    char         mb_cur_max[2];
    char         ctype_codeset[31 + 1];
    char         message_codeset[31 + 1];
};
extern struct lconv*                 __localeconv_l(struct __locale_t* locale);
extern size_t                        _wcsnrtombs_l(struct _reent*, char*, const wchar_t**, size_t, size_t, mbstate_t*, struct __locale_t*);
static __inline__ struct __locale_t* __get_global_locale()
{
    extern struct __locale_t __global_locale;
    return &__global_locale;
}
static __inline__ struct __locale_t*          __get_locale_r(struct _reent* r) { return __get_global_locale(); }
static __inline__ struct __locale_t*          __get_current_locale(void) { return __get_global_locale(); }
static __inline__ struct __locale_t*          __get_C_locale(void) { return __get_global_locale(); }
static __inline__ int                         __locale_mb_cur_max_l(struct __locale_t* locale) { return locale->mb_cur_max[0]; }
static __inline__ const struct lc_monetary_T* __get_monetary_locale(struct __locale_t* locale) { return &_C_monetary_locale; }
static __inline__ const struct lc_monetary_T* __get_current_monetary_locale(void) { return &_C_monetary_locale; }
static __inline__ const struct lc_numeric_T*  __get_numeric_locale(struct __locale_t* locale) { return &_C_numeric_locale; }
static __inline__ const struct lc_numeric_T*  __get_current_numeric_locale(void) { return &_C_numeric_locale; }
static __inline__ const struct lc_time_T*     __get_time_locale(struct __locale_t* locale) { return &_C_time_locale; }
static __inline__ const struct lc_time_T*     __get_current_time_locale(void) { return &_C_time_locale; }
static __inline__ const struct lc_messages_T* __get_messages_locale(struct __locale_t* locale) { return &_C_messages_locale; }
static __inline__ const struct lc_messages_T* __get_current_messages_locale(void) { return &_C_messages_locale; }
static __inline__ const char*                 __locale_charset(struct __locale_t* locale) { return locale->ctype_codeset; }
static __inline__ const char*                 __current_locale_charset(void) { return __get_current_locale()->ctype_codeset; }
static __inline__ const char*                 __locale_msgcharset(void) { return (char*)__get_current_locale()->message_codeset; }
static __inline__ int                         __locale_cjk_lang(void) { return __get_current_locale()->cjk_lang; }
int                                           __ctype_load_locale(struct __locale_t*, const char*, void*, const char*, int);
int                                           __monetary_load_locale(struct __locale_t*, const char*, void*, const char*);
int                                           __numeric_load_locale(struct __locale_t*, const char*, void*, const char*);
int                                           __time_load_locale(struct __locale_t*, const char*, void*, const char*);
int                                           __messages_load_locale(struct __locale_t*, const char*, void*, const char*);
union double_union
{
    double     d;
    __uint32_t i[2];
};
typedef __int32_t Long;
typedef union
{
    double  d;
    __ULong i[2];
} U;
typedef struct _Bigint _Bigint;
struct _reent;
struct FPI;
double                     __ulp(double x);
double                     __b2d(_Bigint* a, int* e);
_Bigint*                   _Balloc(struct _reent* p, int k);
void                       _Bfree(struct _reent* p, _Bigint* v);
_Bigint*                   __multadd(struct _reent* p, _Bigint*, int, int);
_Bigint*                   __s2b(struct _reent*, const char*, int, int, __ULong);
_Bigint*                   __i2b(struct _reent*, int);
_Bigint*                   __multiply(struct _reent*, _Bigint*, _Bigint*);
_Bigint*                   __pow5mult(struct _reent*, _Bigint*, int k);
int                        __hi0bits(__ULong);
int                        __lo0bits(__ULong*);
_Bigint*                   __d2b(struct _reent* p, double d, int* e, int* bits);
_Bigint*                   __lshift(struct _reent* p, _Bigint* b, int k);
int                        __match(const char**, char*);
_Bigint*                   __mdiff(struct _reent* p, _Bigint* a, _Bigint* b);
int                        __mcmp(_Bigint* a, _Bigint* b);
int                        __gethex(struct _reent* p, const char** sp, const struct FPI* fpi, Long* exp, _Bigint** bp, int sign, locale_t loc);
double                     __ratio(_Bigint* a, _Bigint* b);
__ULong                    __any_on(_Bigint* b, int k);
void                       __copybits(__ULong* c, int n, _Bigint* b);
double                     _strtod_l(struct _reent* ptr, const char* restrict s00, char** restrict se, locale_t loc);
int                        _strtorx_l(struct _reent*, const char*, char**, int, void*, locale_t);
int                        _strtodg_l(struct _reent* p, const char* s00, char** se, struct FPI* fpi, Long* exp, __ULong* bits, locale_t);
int                        __hexnan(const char** sp, const struct FPI* fpi, __ULong* x0);
extern const double        __mprec_tinytens[];
extern const double        __mprec_bigtens[];
extern const double        __mprec_tens[];
extern const unsigned char __hexdig[];
double                     _mprec_log10(int);
double                     _wcstod_l(struct _reent* ptr, const wchar_t* nptr, wchar_t** endptr, locale_t loc)
{
    static const mbstate_t initial;
    mbstate_t              mbs;
    double                 val;
    char *                 buf, *end;
    const wchar_t*         wcp;
    size_t                 len;
    while(iswspace_l(*nptr, loc)) nptr++;
    wcp = nptr;
    mbs = initial;
    if((len = _wcsnrtombs_l(ptr, ((void*)0), &wcp, (size_t)-1, 0, &mbs, loc)) == (size_t)-1)
    {
        if(endptr != ((void*)0)) *endptr = (wchar_t*)nptr;
        return (0.0);
    }
    if((buf = _malloc_r(ptr, len + 1)) == ((void*)0)) return (0.0);
    mbs = initial;
    _wcsnrtombs_l(ptr, buf, &wcp, (size_t)-1, len + 1, &mbs, loc);
    val = _strtod_l(ptr, buf, &end, loc);
    if(endptr != ((void*)0))
    {
        const char* decimal_point = __get_numeric_locale(loc)->decimal_point;
        len                       = strlen(decimal_point);
        if(len > 1)
        {
            char* d = strstr(buf, decimal_point);
            if(d && d < end) end -= len - 1;
        }
        *endptr = (wchar_t*)nptr + (end - buf);
    }
    _free_r(ptr, buf);
    return (val);
}
double _wcstod_r(struct _reent* ptr, const wchar_t* nptr, wchar_t** endptr) { return _wcstod_l(ptr, nptr, endptr, __get_current_locale()); }
float  _wcstof_r(struct _reent* ptr, const wchar_t* nptr, wchar_t** endptr)
{
    double retval = _wcstod_l(ptr, nptr, endptr, __get_current_locale());
    if((__builtin_isnan(retval))) return nanf("");
    return (float)retval;
}
double wcstod_l(const wchar_t* restrict nptr, wchar_t** restrict endptr, locale_t loc) { return _wcstod_l(_impure_ptr, nptr, endptr, loc); }
double wcstod(const wchar_t* restrict nptr, wchar_t** restrict endptr) { return _wcstod_l(_impure_ptr, nptr, endptr, __get_current_locale()); }
float  wcstof_l(const wchar_t* restrict nptr, wchar_t** restrict endptr, locale_t loc)
{
    double val = _wcstod_l(_impure_ptr, nptr, endptr, loc);
    if((__builtin_isnan(val))) return nanf("");
    float retval = (float)val;
    if((__builtin_isinf_sign(retval)) && !(__builtin_isinf_sign(val))) _impure_ptr->_errno = 34;
    return retval;
}
float wcstof(const wchar_t* restrict nptr, wchar_t** restrict endptr)
{
    double val = _wcstod_l(_impure_ptr, nptr, endptr, __get_current_locale());
    if((__builtin_isnan(val))) return nanf("");
    float retval = (float)val;
    if((__builtin_isinf_sign(retval)) && !(__builtin_isinf_sign(val))) _impure_ptr->_errno = 34;
    return retval;
}
