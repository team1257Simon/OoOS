
typedef long unsigned int size_t;
typedef int               wchar_t;
typedef long int          ptrdiff_t;
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
int                atoi(const char* s) { return (int)strtol(s, ((void*)0), 10); }
int                _atoi_r(struct _reent* ptr, const char* s) { return (int)_strtol_r(ptr, s, ((void*)0), 10); }
