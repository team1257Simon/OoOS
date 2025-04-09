
















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

typedef unsigned long __size_t;

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




struct __locale_t;
typedef struct __locale_t *locale_t;










typedef int wctype_t;




typedef int wctrans_t;


int iswalpha (wint_t);
int iswalnum (wint_t);

int iswblank (wint_t);

int iswcntrl (wint_t);
int iswctype (wint_t, wctype_t);
int iswdigit (wint_t);
int iswgraph (wint_t);
int iswlower (wint_t);
int iswprint (wint_t);
int iswpunct (wint_t);
int iswspace (wint_t);
int iswupper (wint_t);
int iswxdigit (wint_t);
wint_t towctrans (wint_t, wctrans_t);
wint_t towupper (wint_t);
wint_t towlower (wint_t);
wctrans_t wctrans (const char *);
wctype_t wctype (const char *);


extern int iswalpha_l (wint_t, locale_t);
extern int iswalnum_l (wint_t, locale_t);
extern int iswblank_l (wint_t, locale_t);
extern int iswcntrl_l (wint_t, locale_t);
extern int iswctype_l (wint_t, wctype_t, locale_t);
extern int iswdigit_l (wint_t, locale_t);
extern int iswgraph_l (wint_t, locale_t);
extern int iswlower_l (wint_t, locale_t);
extern int iswprint_l (wint_t, locale_t);
extern int iswpunct_l (wint_t, locale_t);
extern int iswspace_l (wint_t, locale_t);
extern int iswupper_l (wint_t, locale_t);
extern int iswxdigit_l (wint_t, locale_t);
extern wint_t towctrans_l (wint_t, wctrans_t, locale_t);
extern wint_t towupper_l (wint_t, locale_t);
extern wint_t towlower_l (wint_t, locale_t);
extern wctrans_t wctrans_l (const char *, locale_t);
extern wctype_t wctype_l (const char *, locale_t);



       
       
       
       
       
       
       
       
       
       
       
       
       
       
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






typedef __int_least8_t int_least8_t;
typedef __uint_least8_t uint_least8_t;




typedef __int_least16_t int_least16_t;
typedef __uint_least16_t uint_least16_t;




typedef __int_least32_t int_least32_t;
typedef __uint_least32_t uint_least32_t;




typedef __int_least64_t int_least64_t;
typedef __uint_least64_t uint_least64_t;
  typedef int int_fast8_t;
  typedef unsigned int uint_fast8_t;
  typedef int int_fast16_t;
  typedef unsigned int uint_fast16_t;
  typedef int int_fast32_t;
  typedef unsigned int uint_fast32_t;
  typedef long int int_fast64_t;
  typedef long unsigned int uint_fast64_t;






typedef long int ptrdiff_t;
typedef long unsigned int size_t;
typedef int wchar_t;
typedef struct {
  long long __max_align_ll __attribute__((__aligned__(__alignof__(long long))));
  long double __max_align_ld __attribute__((__aligned__(__alignof__(long double))));
} max_align_t;





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













int bcmp(const void *, const void *, size_t) __attribute__((__pure__));
void bcopy(const void *, void *, size_t);
void bzero(void *, size_t);


void explicit_bzero(void *, size_t);


int ffs(int) __attribute__((__const__));


int ffsl(long) __attribute__((__const__));
int ffsll(long long) __attribute__((__const__));
int fls(int) __attribute__((__const__));
int flsl(long) __attribute__((__const__));
int flsll(long long) __attribute__((__const__));


char *index(const char *, int) __attribute__((__pure__));
char *rindex(const char *, int) __attribute__((__pure__));

int strcasecmp(const char *, const char *) __attribute__((__pure__));
int strncasecmp(const char *, const char *, size_t) __attribute__((__pure__));


int strcasecmp_l (const char *, const char *, locale_t);
int strncasecmp_l (const char *, const char *, size_t, locale_t);






void * memchr (const void *, int, size_t);
int memcmp (const void *, const void *, size_t);
void * memcpy (void *restrict, const void *restrict, size_t);
void * memmove (void *, const void *, size_t);
void * memset (void *, int, size_t);
char *strcat (char *restrict, const char *restrict);
char *strchr (const char *, int);
int strcmp (const char *, const char *);
int strcoll (const char *, const char *);
char *strcpy (char *restrict, const char *restrict);
size_t strcspn (const char *, const char *);
char *strerror (int);
size_t strlen (const char *);
char *strncat (char *restrict, const char *restrict, size_t);
int strncmp (const char *, const char *, size_t);
char *strncpy (char *restrict, const char *restrict, size_t);
char *strpbrk (const char *, const char *);
char *strrchr (const char *, int);
size_t strspn (const char *, const char *);
char *strstr (const char *, const char *);

char *strtok (char *restrict, const char *restrict);

size_t strxfrm (char *restrict, const char *restrict, size_t);


int strcoll_l (const char *, const char *, locale_t);
char *strerror_l (int, locale_t);
size_t strxfrm_l (char *restrict, const char *restrict, size_t, locale_t);


char *strtok_r (char *restrict, const char *restrict, char **restrict);


int timingsafe_bcmp (const void *, const void *, size_t);
int timingsafe_memcmp (const void *, const void *, size_t);


void * memccpy (void *restrict, const void *restrict, int, size_t);
char *stpcpy (char *restrict, const char *restrict);
char *stpncpy (char *restrict, const char *restrict, size_t);






char *strdup (const char *);

char *_strdup_r (struct _reent *, const char *);

char *strndup (const char *, size_t);

char *_strndup_r (struct _reent *, const char *, size_t);
int strerror_r (int, char *, size_t)

             __asm__ ("" "__xpg_strerror_r")

  ;







char * _strerror_r (struct _reent *, int, int, int *);


size_t strlcat (char *, const char *, size_t);
size_t strlcpy (char *, const char *, size_t);


size_t strnlen (const char *, size_t);


char *strsep (char **, const char *);


char *strnstr(const char *, const char *, size_t) __attribute__((__pure__));



char *strlwr (char *);
char *strupr (char *);



char *strsignal (int __signo);













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




typedef int (*__compar_fn_t) (const void *, const void *);







int __locale_mb_cur_max (void);



void abort (void) __attribute__ ((__noreturn__));
int abs (int);

__uint32_t arc4random (void);
__uint32_t arc4random_uniform (__uint32_t);
void arc4random_buf (void *, size_t);

int atexit (void (*__func)(void));
double atof (const char *__nptr);

float atoff (const char *__nptr);

int atoi (const char *__nptr);
int _atoi_r (struct _reent *, const char *__nptr);
long atol (const char *__nptr);
long _atol_r (struct _reent *, const char *__nptr);
void * bsearch (const void *__key,
         const void *__base,
         size_t __nmemb,
         size_t __size,
         __compar_fn_t _compar);
void *calloc(size_t, size_t) __attribute__((__malloc__)) __attribute__((__warn_unused_result__))
      __attribute__((__alloc_size__(1, 2))) ;
div_t div (int __numer, int __denom);
void exit (int __status) __attribute__ ((__noreturn__));
void free (void *) ;
char * getenv (const char *__string);
char * _getenv_r (struct _reent *, const char *__string);
char * _findenv (const char *, int *);
char * _findenv_r (struct _reent *, const char *, int *);

extern char *suboptarg;
int getsubopt (char **, char * const *, char **);

long labs (long);
ldiv_t ldiv (long __numer, long __denom);
void *malloc(size_t) __attribute__((__malloc__)) __attribute__((__warn_unused_result__)) __attribute__((__alloc_size__(1))) ;
int mblen (const char *, size_t);
int _mblen_r (struct _reent *, const char *, size_t, _mbstate_t *);
int mbtowc (wchar_t *restrict, const char *restrict, size_t);
int _mbtowc_r (struct _reent *, wchar_t *restrict, const char *restrict, size_t, _mbstate_t *);
int wctomb (char *, wchar_t);
int _wctomb_r (struct _reent *, char *, wchar_t, _mbstate_t *);
size_t mbstowcs (wchar_t *restrict, const char *restrict, size_t);
size_t _mbstowcs_r (struct _reent *, wchar_t *restrict, const char *restrict, size_t, _mbstate_t *);
size_t wcstombs (char *restrict, const wchar_t *restrict, size_t);
size_t _wcstombs_r (struct _reent *, char *restrict, const wchar_t *restrict, size_t, _mbstate_t *);


char * mkdtemp (char *);






int mkstemp (char *);


int mkstemps (char *, int);


char * mktemp (char *) __attribute__ ((__deprecated__("the use of `mktemp' is dangerous; use `mkstemp' instead")));


char * _mkdtemp_r (struct _reent *, char *);
int _mkostemp_r (struct _reent *, char *, int);
int _mkostemps_r (struct _reent *, char *, int, int);
int _mkstemp_r (struct _reent *, char *);
int _mkstemps_r (struct _reent *, char *, int);
char * _mktemp_r (struct _reent *, char *) __attribute__ ((__deprecated__("the use of `mktemp' is dangerous; use `mkstemp' instead")));
void qsort (void *__base, size_t __nmemb, size_t __size, __compar_fn_t _compar);
int rand (void);
void *realloc(void *, size_t) __attribute__((__warn_unused_result__)) __attribute__((__alloc_size__(2))) ;

void *reallocarray(void *, size_t, size_t) __attribute__((__warn_unused_result__)) __attribute__((__alloc_size__(2, 3)));
void *reallocf(void *, size_t) __attribute__((__warn_unused_result__)) __attribute__((__alloc_size__(2)));


char * realpath (const char *restrict path, char *restrict resolved_path);







void srand (unsigned __seed);
double strtod (const char *restrict __n, char **restrict __end_PTR);
double _strtod_r (struct _reent *,const char *restrict __n, char **restrict __end_PTR);

float strtof (const char *restrict __n, char **restrict __end_PTR);







long strtol (const char *restrict __n, char **restrict __end_PTR, int __base);
long _strtol_r (struct _reent *,const char *restrict __n, char **restrict __end_PTR, int __base);
unsigned long strtoul (const char *restrict __n, char **restrict __end_PTR, int __base);
unsigned long _strtoul_r (struct _reent *,const char *restrict __n, char **restrict __end_PTR, int __base);
int system (const char *__string);


long a64l (const char *__input);
char * l64a (long __input);
char * _l64a_r (struct _reent *,long __input);


int on_exit (void (*__func)(int, void *),void *__arg);


void _Exit (int __status) __attribute__ ((__noreturn__));


int putenv (char *__string);

int _putenv_r (struct _reent *, char *__string);
void * _reallocf_r (struct _reent *, void *, size_t);

int setenv (const char *__string, const char *__value, int __overwrite);

int _setenv_r (struct _reent *, const char *__string, const char *__value, int __overwrite);
char * __itoa (int, char *, int);
char * __utoa (unsigned, char *, int);

char * itoa (int, char *, int);
char * utoa (unsigned, char *, int);


int rand_r (unsigned *__seed);



double drand48 (void);
double _drand48_r (struct _reent *);
double erand48 (unsigned short [3]);
double _erand48_r (struct _reent *, unsigned short [3]);
long jrand48 (unsigned short [3]);
long _jrand48_r (struct _reent *, unsigned short [3]);
void lcong48 (unsigned short [7]);
void _lcong48_r (struct _reent *, unsigned short [7]);
long lrand48 (void);
long _lrand48_r (struct _reent *);
long mrand48 (void);
long _mrand48_r (struct _reent *);
long nrand48 (unsigned short [3]);
long _nrand48_r (struct _reent *, unsigned short [3]);
unsigned short *
       seed48 (unsigned short [3]);
unsigned short *
       _seed48_r (struct _reent *, unsigned short [3]);
void srand48 (long);
void _srand48_r (struct _reent *, long);


char * initstate (unsigned, char *, size_t);
long random (void);
char * setstate (char *);
void srandom (unsigned);


long long atoll (const char *__nptr);

long long _atoll_r (struct _reent *, const char *__nptr);

long long llabs (long long);
lldiv_t lldiv (long long __numer, long long __denom);
long long strtoll (const char *restrict __n, char **restrict __end_PTR, int __base);

long long _strtoll_r (struct _reent *, const char *restrict __n, char **restrict __end_PTR, int __base);

unsigned long long strtoull (const char *restrict __n, char **restrict __end_PTR, int __base);

unsigned long long _strtoull_r (struct _reent *, const char *restrict __n, char **restrict __end_PTR, int __base);



void cfree (void *);


int unsetenv (const char *__string);

int _unsetenv_r (struct _reent *, const char *__string);



int __attribute__((__nonnull__ (1))) posix_memalign (void **, size_t, size_t);


char * _dtoa_r (struct _reent *, double, int, int, int *, int*, char**);

void * _malloc_r (struct _reent *, size_t) ;
void * _calloc_r (struct _reent *, size_t, size_t) ;
void _free_r (struct _reent *, void *) ;
void * _realloc_r (struct _reent *, void *, size_t) ;
void _mstats_r (struct _reent *, char *);

int _system_r (struct _reent *, const char *);

void __eprintf (const char *, const char *, unsigned int, const char *);
void qsort_r (void *__base, size_t __nmemb, size_t __size, void *__thunk, int (*_compar)(void *, const void *, const void *))
             __asm__ ("" "__bsd_qsort_r");
extern long double _strtold_r (struct _reent *, const char *restrict, char **restrict);

extern long double strtold (const char *restrict, char **restrict);







void * aligned_alloc(size_t, size_t) __attribute__((__malloc__)) __attribute__((__alloc_align__(1)))
     __attribute__((__alloc_size__(2)));
int at_quick_exit(void (*)(void));
_Noreturn void
 quick_exit(int);



typedef __builtin_va_list __gnuc_va_list;





typedef __gnuc_va_list va_list;





typedef __FILE FILE;






struct tm;



typedef _mbstate_t mbstate_t;


wint_t btowc (int);
int wctob (wint_t);
size_t mbrlen (const char *restrict, size_t, mbstate_t *restrict);
size_t mbrtowc (wchar_t *restrict, const char *restrict, size_t,
      mbstate_t *restrict);
size_t _mbrtowc_r (struct _reent *, wchar_t * , const char * ,
   size_t, mbstate_t *);
int mbsinit (const mbstate_t *);

size_t mbsnrtowcs (wchar_t *restrict, const char **restrict,
    size_t, size_t, mbstate_t *restrict);

size_t _mbsnrtowcs_r (struct _reent *, wchar_t * , const char ** ,
   size_t, size_t, mbstate_t *);
size_t mbsrtowcs (wchar_t *restrict, const char **restrict, size_t,
    mbstate_t *restrict);
size_t _mbsrtowcs_r (struct _reent *, wchar_t * , const char ** , size_t, mbstate_t *);
size_t wcrtomb (char *restrict, wchar_t, mbstate_t *restrict);
size_t _wcrtomb_r (struct _reent *, char * , wchar_t, mbstate_t *);

size_t wcsnrtombs (char *restrict, const wchar_t **restrict,
    size_t, size_t, mbstate_t *restrict);

size_t _wcsnrtombs_r (struct _reent *, char * , const wchar_t ** ,
   size_t, size_t, mbstate_t *);
size_t wcsrtombs (char *restrict, const wchar_t **restrict,
    size_t, mbstate_t *restrict);
size_t _wcsrtombs_r (struct _reent *, char * , const wchar_t ** ,
   size_t, mbstate_t *);

int wcscasecmp (const wchar_t *, const wchar_t *);

wchar_t *wcscat (wchar_t *restrict, const wchar_t *restrict);
wchar_t *wcschr (const wchar_t *, wchar_t);
int wcscmp (const wchar_t *, const wchar_t *);
int wcscoll (const wchar_t *, const wchar_t *);
wchar_t *wcscpy (wchar_t *restrict, const wchar_t *restrict);

wchar_t *wcpcpy (wchar_t *restrict,
     const wchar_t *restrict);
wchar_t *wcsdup (const wchar_t *);

wchar_t *_wcsdup_r (struct _reent *, const wchar_t * );
size_t wcscspn (const wchar_t *, const wchar_t *);
size_t wcsftime (wchar_t *restrict, size_t,
    const wchar_t *restrict, const struct tm *restrict);




size_t wcslcat (wchar_t *, const wchar_t *, size_t);
size_t wcslcpy (wchar_t *, const wchar_t *, size_t);
size_t wcslen (const wchar_t *);

int wcsncasecmp (const wchar_t *, const wchar_t *, size_t);

wchar_t *wcsncat (wchar_t *restrict,
     const wchar_t *restrict, size_t);
int wcsncmp (const wchar_t *, const wchar_t *, size_t);
wchar_t *wcsncpy (wchar_t *restrict,
     const wchar_t *restrict, size_t);

wchar_t *wcpncpy (wchar_t *restrict,
     const wchar_t *restrict, size_t);
size_t wcsnlen (const wchar_t *, size_t);

wchar_t *wcspbrk (const wchar_t *, const wchar_t *);
wchar_t *wcsrchr (const wchar_t *, wchar_t);
size_t wcsspn (const wchar_t *, const wchar_t *);
wchar_t *wcsstr (const wchar_t *restrict,
     const wchar_t *restrict);
wchar_t *wcstok (wchar_t *restrict, const wchar_t *restrict,
     wchar_t **restrict);
double wcstod (const wchar_t *restrict, wchar_t **restrict);
double _wcstod_r (struct _reent *, const wchar_t *, wchar_t **);

float wcstof (const wchar_t *restrict, wchar_t **restrict);

float _wcstof_r (struct _reent *, const wchar_t *, wchar_t **);



size_t wcsxfrm (wchar_t *restrict, const wchar_t *restrict,
    size_t);

extern int wcscasecmp_l (const wchar_t *, const wchar_t *, locale_t);
extern int wcsncasecmp_l (const wchar_t *, const wchar_t *, size_t, locale_t);
extern int wcscoll_l (const wchar_t *, const wchar_t *, locale_t);
extern size_t wcsxfrm_l (wchar_t *restrict, const wchar_t *restrict, size_t,
    locale_t);





wchar_t *wmemchr (const wchar_t *, wchar_t, size_t);
int wmemcmp (const wchar_t *, const wchar_t *, size_t);
wchar_t *wmemcpy (wchar_t *restrict, const wchar_t *restrict,
     size_t);
wchar_t *wmemmove (wchar_t *, const wchar_t *, size_t);




wchar_t *wmemset (wchar_t *, wchar_t, size_t);

long wcstol (const wchar_t *restrict, wchar_t **restrict, int);

long long wcstoll (const wchar_t *restrict, wchar_t **restrict,
      int);

unsigned long wcstoul (const wchar_t *restrict, wchar_t **restrict,
       int);

unsigned long long wcstoull (const wchar_t *restrict,
         wchar_t **restrict, int);

long _wcstol_r (struct _reent *, const wchar_t *, wchar_t **, int);
long long _wcstoll_r (struct _reent *, const wchar_t *, wchar_t **, int);
unsigned long _wcstoul_r (struct _reent *, const wchar_t *, wchar_t **, int);
unsigned long long _wcstoull_r (struct _reent *, const wchar_t *, wchar_t **, int);

long double wcstold (const wchar_t *, wchar_t **);
wint_t fgetwc (__FILE *);
wchar_t *fgetws (wchar_t *restrict, int, __FILE *restrict);
wint_t fputwc (wchar_t, __FILE *);
int fputws (const wchar_t *restrict, __FILE *restrict);

int fwide (__FILE *, int);

wint_t getwc (__FILE *);
wint_t getwchar (void);
wint_t putwc (wchar_t, __FILE *);
wint_t putwchar (wchar_t);
wint_t ungetwc (wint_t wc, __FILE *);

wint_t _fgetwc_r (struct _reent *, __FILE *);
wint_t _fgetwc_unlocked_r (struct _reent *, __FILE *);
wchar_t *_fgetws_r (struct _reent *, wchar_t *, int, __FILE *);
wchar_t *_fgetws_unlocked_r (struct _reent *, wchar_t *, int, __FILE *);
wint_t _fputwc_r (struct _reent *, wchar_t, __FILE *);
wint_t _fputwc_unlocked_r (struct _reent *, wchar_t, __FILE *);
int _fputws_r (struct _reent *, const wchar_t *, __FILE *);
int _fputws_unlocked_r (struct _reent *, const wchar_t *, __FILE *);
int _fwide_r (struct _reent *, __FILE *, int);
wint_t _getwc_r (struct _reent *, __FILE *);
wint_t _getwc_unlocked_r (struct _reent *, __FILE *);
wint_t _getwchar_r (struct _reent *ptr);
wint_t _getwchar_unlocked_r (struct _reent *ptr);
wint_t _putwc_r (struct _reent *, wchar_t, __FILE *);
wint_t _putwc_unlocked_r (struct _reent *, wchar_t, __FILE *);
wint_t _putwchar_r (struct _reent *, wchar_t);
wint_t _putwchar_unlocked_r (struct _reent *, wchar_t);
wint_t _ungetwc_r (struct _reent *, wint_t wc, __FILE *);
__FILE *open_wmemstream (wchar_t **, size_t *);

__FILE *_open_wmemstream_r (struct _reent *, wchar_t **, size_t *);
int fwprintf (__FILE *restrict, const wchar_t *restrict, ...);
int swprintf (wchar_t *restrict, size_t,
   const wchar_t *restrict, ...);
int vfwprintf (__FILE *restrict, const wchar_t *restrict,
   __gnuc_va_list);
int vswprintf (wchar_t *restrict, size_t,
   const wchar_t *restrict, __gnuc_va_list);
int vwprintf (const wchar_t *restrict, __gnuc_va_list);
int wprintf (const wchar_t *restrict, ...);


int _fwprintf_r (struct _reent *, __FILE *, const wchar_t *, ...);
int _swprintf_r (struct _reent *, wchar_t *, size_t, const wchar_t *, ...);
int _vfwprintf_r (struct _reent *, __FILE *, const wchar_t *, __gnuc_va_list);
int _vswprintf_r (struct _reent *, wchar_t *, size_t, const wchar_t *, __gnuc_va_list);
int _vwprintf_r (struct _reent *, const wchar_t *, __gnuc_va_list);
int _wprintf_r (struct _reent *, const wchar_t *, ...);


int fwscanf (__FILE *restrict, const wchar_t *restrict, ...);
int swscanf (const wchar_t *restrict,
   const wchar_t *restrict, ...);
int vfwscanf (__FILE *restrict, const wchar_t *restrict,
   __gnuc_va_list);
int vswscanf (const wchar_t *restrict, const wchar_t *restrict,
   __gnuc_va_list);
int vwscanf (const wchar_t *restrict, __gnuc_va_list);
int wscanf (const wchar_t *restrict, ...);


int _fwscanf_r (struct _reent *, __FILE *, const wchar_t *, ...);
int _swscanf_r (struct _reent *, const wchar_t *, const wchar_t *, ...);
int _vfwscanf_r (struct _reent *, __FILE *, const wchar_t *, __gnuc_va_list);
int _vswscanf_r (struct _reent *, const wchar_t *, const wchar_t *, __gnuc_va_list);
int _vwscanf_r (struct _reent *, const wchar_t *, __gnuc_va_list);
int _wscanf_r (struct _reent *, const wchar_t *, ...);



struct lconv
{
  char *decimal_point;
  char *thousands_sep;
  char *grouping;
  char *int_curr_symbol;
  char *currency_symbol;
  char *mon_decimal_point;
  char *mon_thousands_sep;
  char *mon_grouping;
  char *positive_sign;
  char *negative_sign;
  char int_frac_digits;
  char frac_digits;
  char p_cs_precedes;
  char p_sep_by_space;
  char n_cs_precedes;
  char n_sep_by_space;
  char p_sign_posn;
  char n_sign_posn;
  char int_n_cs_precedes;
  char int_n_sep_by_space;
  char int_n_sign_posn;
  char int_p_cs_precedes;
  char int_p_sep_by_space;
  char int_p_sign_posn;
};

struct _reent;
char *_setlocale_r (struct _reent *, int, const char *);
struct lconv *_localeconv_r (struct _reent *);

struct __locale_t *_newlocale_r (struct _reent *, int, const char *,
     struct __locale_t *);
void _freelocale_r (struct _reent *, struct __locale_t *);
struct __locale_t *_duplocale_r (struct _reent *, struct __locale_t *);
struct __locale_t *_uselocale_r (struct _reent *, struct __locale_t *);



char *setlocale (int, const char *);
struct lconv *localeconv (void);


locale_t newlocale (int, const char *, locale_t);
void freelocale (locale_t);
locale_t duplocale (locale_t);
locale_t uselocale (locale_t);








struct lc_ctype_T
{
  const char *codeset;
  const char *mb_cur_max;




};
extern const struct lc_ctype_T _C_ctype_locale;

struct lc_monetary_T
{
  const char *int_curr_symbol;
  const char *currency_symbol;
  const char *mon_decimal_point;
  const char *mon_thousands_sep;
  const char *mon_grouping;
  const char *positive_sign;
  const char *negative_sign;
  const char *int_frac_digits;
  const char *frac_digits;
  const char *p_cs_precedes;
  const char *p_sep_by_space;
  const char *n_cs_precedes;
  const char *n_sep_by_space;
  const char *p_sign_posn;
  const char *n_sign_posn;
};
extern const struct lc_monetary_T _C_monetary_locale;

struct lc_numeric_T
{
  const char *decimal_point;
  const char *thousands_sep;
  const char *grouping;





};
extern const struct lc_numeric_T _C_numeric_locale;

struct lc_time_T
{
  const char *mon[12];
  const char *month[12];
  const char *wday[7];
  const char *weekday[7];
  const char *X_fmt;
  const char *x_fmt;
  const char *c_fmt;
  const char *am_pm[2];
  const char *date_fmt;
  const char *alt_month[12];
  const char *md_order;
  const char *ampm_fmt;
  const char *era;
  const char *era_d_fmt;
  const char *era_d_t_fmt;
  const char *era_t_fmt;
  const char *alt_digits;
};
extern const struct lc_time_T _C_time_locale;

struct lc_messages_T
{
  const char *yesexpr;
  const char *noexpr;
  const char *yesstr;
  const char *nostr;







};
extern const struct lc_messages_T _C_messages_locale;

struct __lc_cats
{
  const void *ptr;
  char *buf;
};

struct __locale_t
{
  char categories[7][31 + 1];
  int (*wctomb) (struct _reent *, char *, wchar_t,
       mbstate_t *);
  int (*mbtowc) (struct _reent *, wchar_t *,
       const char *, size_t, mbstate_t *);
  int cjk_lang;
  char *ctype_ptr;
  struct lconv lconv;

  char mb_cur_max[2];
  char ctype_codeset[31 + 1];
  char message_codeset[31 + 1];



};






extern struct lconv *__localeconv_l (struct __locale_t *locale);

extern size_t _wcsnrtombs_l (struct _reent *, char *, const wchar_t **,
        size_t, size_t, mbstate_t *, struct __locale_t *);




static __inline__ 
                struct __locale_t *
__get_global_locale ()
{
  extern struct __locale_t __global_locale;
  return &__global_locale;
}



static __inline__ 
                struct __locale_t *
__get_locale_r (struct _reent *r)
{



  return __get_global_locale();

}






static __inline__ 
                struct __locale_t *
__get_current_locale (void)
{



  return __get_global_locale();

}




static __inline__ 
                struct __locale_t *
__get_C_locale (void)
{

  return __get_global_locale ();




}

static __inline__ 
                int
__locale_mb_cur_max_l (struct __locale_t *locale)
{



  return locale->mb_cur_max[0];

}

static __inline__ 
                const struct lc_monetary_T *
__get_monetary_locale (struct __locale_t *locale)
{
  return &_C_monetary_locale;
}


static __inline__ 
                const struct lc_monetary_T *
__get_current_monetary_locale (void)
{
  return &_C_monetary_locale;
}


static __inline__ 
                const struct lc_numeric_T *
__get_numeric_locale (struct __locale_t *locale)
{
  return &_C_numeric_locale;
}


static __inline__ 
                const struct lc_numeric_T *
__get_current_numeric_locale (void)
{
  return &_C_numeric_locale;
}


static __inline__ 
                const struct lc_time_T *
__get_time_locale (struct __locale_t *locale)
{
  return &_C_time_locale;
}


static __inline__ 
                const struct lc_time_T *
__get_current_time_locale (void)
{
  return &_C_time_locale;
}


static __inline__ 
                const struct lc_messages_T *
__get_messages_locale (struct __locale_t *locale)
{
  return &_C_messages_locale;
}


static __inline__ 
                const struct lc_messages_T *
__get_current_messages_locale (void)
{
  return &_C_messages_locale;
}



static __inline__ 
                const char *
__locale_charset (struct __locale_t *locale)
{



  return locale->ctype_codeset;

}


static __inline__ 
                const char *
__current_locale_charset (void)
{



  return __get_current_locale ()->ctype_codeset;

}


static __inline__ 
                const char *
__locale_msgcharset (void)
{



  return (char *) __get_current_locale ()->message_codeset;

}


static __inline__ 
                int
__locale_cjk_lang (void)
{
  return __get_current_locale ()->cjk_lang;
}

int __ctype_load_locale (struct __locale_t *, const char *, void *,
    const char *, int);
int __monetary_load_locale (struct __locale_t *, const char *, void *,
       const char *);
int __numeric_load_locale (struct __locale_t *, const char *, void *,
      const char *);
int __time_load_locale (struct __locale_t *, const char *, void *,
   const char *);
int __messages_load_locale (struct __locale_t *, const char *, void *,
       const char *);

wint_t _jp2uc (wint_t);
wint_t _jp2uc_l (wint_t, struct __locale_t *);
wint_t _uc2jp_l (wint_t, struct __locale_t *);
enum {TO1, TOLO, TOUP, TOBOTH};
enum {EVENCAP, ODDCAP};
static struct caseconv_entry {
  uint_least32_t first: 21;
  uint_least32_t diff: 8;
  uint_least32_t mode: 2;
  int_least32_t delta: 17;
} __attribute__ ((packed))
caseconv_table [] = {
  {0x0041, 25, TOLO, 32},
  {0x0061, 25, TOUP, -32},
  {0x00B5, 0, TOUP, 743},
  {0x00C0, 22, TOLO, 32},
  {0x00D8, 6, TOLO, 32},
  {0x00E0, 22, TOUP, -32},
  {0x00F8, 6, TOUP, -32},
  {0x00FF, 0, TOUP, 121},
  {0x0100, 47, TO1, EVENCAP},
  {0x0130, 0, TOLO, -199},
  {0x0131, 0, TOUP, -232},
  {0x0132, 5, TO1, EVENCAP},
  {0x0139, 15, TO1, ODDCAP},
  {0x014A, 45, TO1, EVENCAP},
  {0x0178, 0, TOLO, -121},
  {0x0179, 5, TO1, ODDCAP},
  {0x017F, 0, TOUP, -300},
  {0x0180, 0, TOUP, 195},
  {0x0181, 0, TOLO, 210},
  {0x0182, 3, TO1, EVENCAP},
  {0x0186, 0, TOLO, 206},
  {0x0187, 1, TO1, ODDCAP},
  {0x0189, 1, TOLO, 205},
  {0x018B, 1, TO1, ODDCAP},
  {0x018E, 0, TOLO, 79},
  {0x018F, 0, TOLO, 202},
  {0x0190, 0, TOLO, 203},
  {0x0191, 1, TO1, ODDCAP},
  {0x0193, 0, TOLO, 205},
  {0x0194, 0, TOLO, 207},
  {0x0195, 0, TOUP, 97},
  {0x0196, 0, TOLO, 211},
  {0x0197, 0, TOLO, 209},
  {0x0198, 1, TO1, EVENCAP},
  {0x019A, 0, TOUP, 163},
  {0x019C, 0, TOLO, 211},
  {0x019D, 0, TOLO, 213},
  {0x019E, 0, TOUP, 130},
  {0x019F, 0, TOLO, 214},
  {0x01A0, 5, TO1, EVENCAP},
  {0x01A6, 0, TOLO, 218},
  {0x01A7, 1, TO1, ODDCAP},
  {0x01A9, 0, TOLO, 218},
  {0x01AC, 1, TO1, EVENCAP},
  {0x01AE, 0, TOLO, 218},
  {0x01AF, 1, TO1, ODDCAP},
  {0x01B1, 1, TOLO, 217},
  {0x01B3, 3, TO1, ODDCAP},
  {0x01B7, 0, TOLO, 219},
  {0x01B8, 1, TO1, EVENCAP},
  {0x01BC, 1, TO1, EVENCAP},
  {0x01BF, 0, TOUP, 56},
  {0x01C4, 0, TOLO, 2},
  {0x01C5, 0, TOBOTH, 0},
  {0x01C6, 0, TOUP, -2},
  {0x01C7, 0, TOLO, 2},
  {0x01C8, 0, TOBOTH, 0},
  {0x01C9, 0, TOUP, -2},
  {0x01CA, 0, TOLO, 2},
  {0x01CB, 0, TOBOTH, 0},
  {0x01CC, 0, TOUP, -2},
  {0x01CD, 15, TO1, ODDCAP},
  {0x01DD, 0, TOUP, -79},
  {0x01DE, 17, TO1, EVENCAP},
  {0x01F1, 0, TOLO, 2},
  {0x01F2, 0, TOBOTH, 0},
  {0x01F3, 0, TOUP, -2},
  {0x01F4, 1, TO1, EVENCAP},
  {0x01F6, 0, TOLO, -97},
  {0x01F7, 0, TOLO, -56},
  {0x01F8, 39, TO1, EVENCAP},
  {0x0220, 0, TOLO, -130},
  {0x0222, 17, TO1, EVENCAP},
  {0x023A, 0, TOLO, 10795},
  {0x023B, 1, TO1, ODDCAP},
  {0x023D, 0, TOLO, -163},
  {0x023E, 0, TOLO, 10792},
  {0x023F, 1, TOUP, 10815},
  {0x0241, 1, TO1, ODDCAP},
  {0x0243, 0, TOLO, -195},
  {0x0244, 0, TOLO, 69},
  {0x0245, 0, TOLO, 71},
  {0x0246, 9, TO1, EVENCAP},
  {0x0250, 0, TOUP, 10783},
  {0x0251, 0, TOUP, 10780},
  {0x0252, 0, TOUP, 10782},
  {0x0253, 0, TOUP, -210},
  {0x0254, 0, TOUP, -206},
  {0x0256, 1, TOUP, -205},
  {0x0259, 0, TOUP, -202},
  {0x025B, 0, TOUP, -203},
  {0x025C, 0, TOUP, 42319},
  {0x0260, 0, TOUP, -205},
  {0x0261, 0, TOUP, 42315},
  {0x0263, 0, TOUP, -207},
  {0x0265, 0, TOUP, 42280},
  {0x0266, 0, TOUP, 42308},
  {0x0268, 0, TOUP, -209},
  {0x0269, 0, TOUP, -211},
  {0x026A, 0, TOUP, 42308},
  {0x026B, 0, TOUP, 10743},
  {0x026C, 0, TOUP, 42305},
  {0x026F, 0, TOUP, -211},
  {0x0271, 0, TOUP, 10749},
  {0x0272, 0, TOUP, -213},
  {0x0275, 0, TOUP, -214},
  {0x027D, 0, TOUP, 10727},
  {0x0280, 0, TOUP, -218},
  {0x0283, 0, TOUP, -218},
  {0x0287, 0, TOUP, 42282},
  {0x0288, 0, TOUP, -218},
  {0x0289, 0, TOUP, -69},
  {0x028A, 1, TOUP, -217},
  {0x028C, 0, TOUP, -71},
  {0x0292, 0, TOUP, -219},
  {0x029D, 0, TOUP, 42261},
  {0x029E, 0, TOUP, 42258},
  {0x0345, 0, TOUP, 84},
  {0x0370, 3, TO1, EVENCAP},
  {0x0376, 1, TO1, EVENCAP},
  {0x037B, 2, TOUP, 130},
  {0x037F, 0, TOLO, 116},
  {0x0386, 0, TOLO, 38},
  {0x0388, 2, TOLO, 37},
  {0x038C, 0, TOLO, 64},
  {0x038E, 1, TOLO, 63},
  {0x0391, 16, TOLO, 32},
  {0x03A3, 8, TOLO, 32},
  {0x03AC, 0, TOUP, -38},
  {0x03AD, 2, TOUP, -37},
  {0x03B1, 16, TOUP, -32},
  {0x03C2, 0, TOUP, -31},
  {0x03C3, 8, TOUP, -32},
  {0x03CC, 0, TOUP, -64},
  {0x03CD, 1, TOUP, -63},
  {0x03CF, 0, TOLO, 8},
  {0x03D0, 0, TOUP, -62},
  {0x03D1, 0, TOUP, -57},
  {0x03D5, 0, TOUP, -47},
  {0x03D6, 0, TOUP, -54},
  {0x03D7, 0, TOUP, -8},
  {0x03D8, 23, TO1, EVENCAP},
  {0x03F0, 0, TOUP, -86},
  {0x03F1, 0, TOUP, -80},
  {0x03F2, 0, TOUP, 7},
  {0x03F3, 0, TOUP, -116},
  {0x03F4, 0, TOLO, -60},
  {0x03F5, 0, TOUP, -96},
  {0x03F7, 1, TO1, ODDCAP},
  {0x03F9, 0, TOLO, -7},
  {0x03FA, 1, TO1, EVENCAP},
  {0x03FD, 2, TOLO, -130},
  {0x0400, 15, TOLO, 80},
  {0x0410, 31, TOLO, 32},
  {0x0430, 31, TOUP, -32},
  {0x0450, 15, TOUP, -80},
  {0x0460, 33, TO1, EVENCAP},
  {0x048A, 53, TO1, EVENCAP},
  {0x04C0, 0, TOLO, 15},
  {0x04C1, 13, TO1, ODDCAP},
  {0x04CF, 0, TOUP, -15},
  {0x04D0, 95, TO1, EVENCAP},
  {0x0531, 37, TOLO, 48},
  {0x0561, 37, TOUP, -48},
  {0x10A0, 37, TOLO, 7264},
  {0x10C7, 0, TOLO, 7264},
  {0x10CD, 0, TOLO, 7264},
  {0x13A0, 79, TOLO, 38864},
  {0x13F0, 5, TOLO, 8},
  {0x13F8, 5, TOUP, -8},
  {0x1C80, 0, TOUP, -6254},
  {0x1C81, 0, TOUP, -6253},
  {0x1C82, 0, TOUP, -6244},
  {0x1C83, 1, TOUP, -6242},
  {0x1C85, 0, TOUP, -6243},
  {0x1C86, 0, TOUP, -6236},
  {0x1C87, 0, TOUP, -6181},
  {0x1C88, 0, TOUP, 35266},
  {0x1D79, 0, TOUP, 35332},
  {0x1D7D, 0, TOUP, 3814},
  {0x1E00, 149, TO1, EVENCAP},
  {0x1E9B, 0, TOUP, -59},
  {0x1E9E, 0, TOLO, -7615},
  {0x1EA0, 95, TO1, EVENCAP},
  {0x1F00, 7, TOUP, 8},
  {0x1F08, 7, TOLO, -8},
  {0x1F10, 5, TOUP, 8},
  {0x1F18, 5, TOLO, -8},
  {0x1F20, 7, TOUP, 8},
  {0x1F28, 7, TOLO, -8},
  {0x1F30, 7, TOUP, 8},
  {0x1F38, 7, TOLO, -8},
  {0x1F40, 5, TOUP, 8},
  {0x1F48, 5, TOLO, -8},
  {0x1F51, 0, TOUP, 8},
  {0x1F53, 0, TOUP, 8},
  {0x1F55, 0, TOUP, 8},
  {0x1F57, 0, TOUP, 8},
  {0x1F59, 0, TOLO, -8},
  {0x1F5B, 0, TOLO, -8},
  {0x1F5D, 0, TOLO, -8},
  {0x1F5F, 0, TOLO, -8},
  {0x1F60, 7, TOUP, 8},
  {0x1F68, 7, TOLO, -8},
  {0x1F70, 1, TOUP, 74},
  {0x1F72, 3, TOUP, 86},
  {0x1F76, 1, TOUP, 100},
  {0x1F78, 1, TOUP, 128},
  {0x1F7A, 1, TOUP, 112},
  {0x1F7C, 1, TOUP, 126},
  {0x1F80, 7, TOUP, 8},
  {0x1F88, 7, TOLO, -8},
  {0x1F90, 7, TOUP, 8},
  {0x1F98, 7, TOLO, -8},
  {0x1FA0, 7, TOUP, 8},
  {0x1FA8, 7, TOLO, -8},
  {0x1FB0, 1, TOUP, 8},
  {0x1FB3, 0, TOUP, 9},
  {0x1FB8, 1, TOLO, -8},
  {0x1FBA, 1, TOLO, -74},
  {0x1FBC, 0, TOLO, -9},
  {0x1FBE, 0, TOUP, -7205},
  {0x1FC3, 0, TOUP, 9},
  {0x1FC8, 3, TOLO, -86},
  {0x1FCC, 0, TOLO, -9},
  {0x1FD0, 1, TOUP, 8},
  {0x1FD8, 1, TOLO, -8},
  {0x1FDA, 1, TOLO, -100},
  {0x1FE0, 1, TOUP, 8},
  {0x1FE5, 0, TOUP, 7},
  {0x1FE8, 1, TOLO, -8},
  {0x1FEA, 1, TOLO, -112},
  {0x1FEC, 0, TOLO, -7},
  {0x1FF3, 0, TOUP, 9},
  {0x1FF8, 1, TOLO, -128},
  {0x1FFA, 1, TOLO, -126},
  {0x1FFC, 0, TOLO, -9},
  {0x2126, 0, TOLO, -7517},
  {0x212A, 0, TOLO, -8383},
  {0x212B, 0, TOLO, -8262},
  {0x2132, 0, TOLO, 28},
  {0x214E, 0, TOUP, -28},
  {0x2160, 15, TOLO, 16},
  {0x2170, 15, TOUP, -16},
  {0x2183, 1, TO1, ODDCAP},
  {0x24B6, 25, TOLO, 26},
  {0x24D0, 25, TOUP, -26},
  {0x2C00, 46, TOLO, 48},
  {0x2C30, 46, TOUP, -48},
  {0x2C60, 1, TO1, EVENCAP},
  {0x2C62, 0, TOLO, -10743},
  {0x2C63, 0, TOLO, -3814},
  {0x2C64, 0, TOLO, -10727},
  {0x2C65, 0, TOUP, -10795},
  {0x2C66, 0, TOUP, -10792},
  {0x2C67, 5, TO1, ODDCAP},
  {0x2C6D, 0, TOLO, -10780},
  {0x2C6E, 0, TOLO, -10749},
  {0x2C6F, 0, TOLO, -10783},
  {0x2C70, 0, TOLO, -10782},
  {0x2C72, 1, TO1, EVENCAP},
  {0x2C75, 1, TO1, ODDCAP},
  {0x2C7E, 1, TOLO, -10815},
  {0x2C80, 99, TO1, EVENCAP},
  {0x2CEB, 3, TO1, ODDCAP},
  {0x2CF2, 1, TO1, EVENCAP},
  {0x2D00, 37, TOUP, -7264},
  {0x2D27, 0, TOUP, -7264},
  {0x2D2D, 0, TOUP, -7264},
  {0xA640, 45, TO1, EVENCAP},
  {0xA680, 27, TO1, EVENCAP},
  {0xA722, 13, TO1, EVENCAP},
  {0xA732, 61, TO1, EVENCAP},
  {0xA779, 3, TO1, ODDCAP},
  {0xA77D, 0, TOLO, -35332},
  {0xA77E, 9, TO1, EVENCAP},
  {0xA78B, 1, TO1, ODDCAP},
  {0xA78D, 0, TOLO, -42280},
  {0xA790, 3, TO1, EVENCAP},
  {0xA796, 19, TO1, EVENCAP},
  {0xA7AA, 0, TOLO, -42308},
  {0xA7AB, 0, TOLO, -42319},
  {0xA7AC, 0, TOLO, -42315},
  {0xA7AD, 0, TOLO, -42305},
  {0xA7AE, 0, TOLO, -42308},
  {0xA7B0, 0, TOLO, -42258},
  {0xA7B1, 0, TOLO, -42282},
  {0xA7B2, 0, TOLO, -42261},
  {0xA7B3, 0, TOLO, 928},
  {0xA7B4, 3, TO1, EVENCAP},
  {0xAB53, 0, TOUP, -928},
  {0xAB70, 79, TOUP, -38864},
  {0xFF21, 25, TOLO, 32},
  {0xFF41, 25, TOUP, -32},
  {0x10400, 39, TOLO, 40},
  {0x10428, 39, TOUP, -40},
  {0x104B0, 35, TOLO, 40},
  {0x104D8, 35, TOUP, -40},
  {0x10C80, 50, TOLO, 64},
  {0x10CC0, 50, TOUP, -64},
  {0x118A0, 31, TOLO, 32},
  {0x118C0, 31, TOUP, -32},
  {0x1E900, 33, TOLO, 34},
  {0x1E922, 33, TOUP, -34},
};




static const struct caseconv_entry *
bisearch (wint_t ucs, const struct caseconv_entry *table, int max)
{
  int min = 0;
  int mid;

  if (ucs < table[0].first || ucs > (table[max].first + table[max].diff))
    return 0;
  while (max >= min)
    {
      mid = (min + max) / 2;
      if (ucs > (table[mid].first + table[mid].diff))
 min = mid + 1;
      else if (ucs < table[mid].first)
 max = mid - 1;
      else
 return &table[mid];
    }
  return 0;
}

static wint_t
toulower (wint_t c)
{
  const struct caseconv_entry * cce =
    bisearch(c, caseconv_table,
             sizeof(caseconv_table) / sizeof(*caseconv_table) - 1);

  if (cce)
    switch (cce->mode)
      {
      case TOLO:
 return c + cce->delta;
      case TOBOTH:
 return c + 1;
      case TO1:
 switch (cce->delta)
   {
   case EVENCAP:
     if (!(c & 1))
       return c + 1;
     break;
   case ODDCAP:
     if (c & 1)
       return c + 1;
     break;
   default:
     break;
   }
 default:
   break;
      }

  return c;
}

static wint_t
touupper (wint_t c)
{
  const struct caseconv_entry * cce =
    bisearch(c, caseconv_table,
             sizeof(caseconv_table) / sizeof(*caseconv_table) - 1);

  if (cce)
    switch (cce->mode)
      {
      case TOUP:
 return c + cce->delta;
      case TOBOTH:
 return c - 1;
      case TO1:
 switch (cce->delta)
   {
   case EVENCAP:
     if (c & 1)
       return c - 1;
     break;
   case ODDCAP:
     if (!(c & 1))
       return c - 1;
     break;
   default:
     break;
   }
      default:
 break;
      }

  return c;
}

wint_t
towctrans_l (wint_t c, wctrans_t w, struct __locale_t *locale)
{
  wint_t u = _jp2uc_l (c, locale);
  wint_t res;
  if (w == 1)
    res = toulower (u);
  else if (w == 2)
    res = touupper (u);
  else
    {



      return c;
    }
  if (res != u)
    return _uc2jp_l (res, locale);
  else
    return c;
}
