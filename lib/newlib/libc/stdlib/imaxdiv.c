
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
struct __hack;
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
struct __locale_t;
typedef struct __locale_t* locale_t;
typedef struct
{
    intmax_t quot;
    intmax_t rem;
} imaxdiv_t;
struct _reent;
extern intmax_t  imaxabs(intmax_t j);
extern imaxdiv_t imaxdiv(intmax_t numer, intmax_t denomer);
extern intmax_t  strtoimax(const char* restrict, char** restrict, int);
extern intmax_t  _strtoimax_r(struct _reent*, const char* restrict, char** restrict, int);
extern uintmax_t strtoumax(const char* restrict, char** restrict, int);
extern uintmax_t _strtoumax_r(struct _reent*, const char* restrict, char** restrict, int);
extern intmax_t  wcstoimax(const wchar_t* restrict, wchar_t** restrict, int);
extern intmax_t  _wcstoimax_r(struct _reent*, const wchar_t* restrict, wchar_t** restrict, int);
extern uintmax_t wcstoumax(const wchar_t* restrict, wchar_t** restrict, int);
extern uintmax_t _wcstoumax_r(struct _reent*, const wchar_t* restrict, wchar_t** restrict, int);
extern intmax_t  strtoimax_l(const char* restrict, char** _restrict, int, locale_t);
extern uintmax_t strtoumax_l(const char* restrict, char** _restrict, int, locale_t);
extern intmax_t  wcstoimax_l(const wchar_t* restrict, wchar_t** _restrict, int, locale_t);
extern uintmax_t wcstoumax_l(const wchar_t* restrict, wchar_t** _restrict, int, locale_t);
imaxdiv_t        imaxdiv(intmax_t numer, intmax_t denom)
{
    imaxdiv_t retval;
    retval.quot = numer / denom;
    retval.rem  = numer % denom;
    return (retval);
}
