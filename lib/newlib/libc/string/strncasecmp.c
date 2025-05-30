
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
int                        isalnum(int __c);
int                        isalpha(int __c);
int                        iscntrl(int __c);
int                        isdigit(int __c);
int                        isgraph(int __c);
int                        islower(int __c);
int                        isprint(int __c);
int                        ispunct(int __c);
int                        isspace(int __c);
int                        isupper(int __c);
int                        isxdigit(int __c);
int                        tolower(int __c);
int                        toupper(int __c);
int                        isblank(int __c);
int                        isascii(int __c);
int                        toascii(int __c);
extern int                 isalnum_l(int __c, locale_t __l);
extern int                 isalpha_l(int __c, locale_t __l);
extern int                 isblank_l(int __c, locale_t __l);
extern int                 iscntrl_l(int __c, locale_t __l);
extern int                 isdigit_l(int __c, locale_t __l);
extern int                 isgraph_l(int __c, locale_t __l);
extern int                 islower_l(int __c, locale_t __l);
extern int                 isprint_l(int __c, locale_t __l);
extern int                 ispunct_l(int __c, locale_t __l);
extern int                 isspace_l(int __c, locale_t __l);
extern int                 isupper_l(int __c, locale_t __l);
extern int                 isxdigit_l(int __c, locale_t __l);
extern int                 tolower_l(int __c, locale_t __l);
extern int                 toupper_l(int __c, locale_t __l);
extern int                 isascii_l(int __c, locale_t __l);
extern int                 toascii_l(int __c, locale_t __l);
extern const char          _ctype_[];
int                        strncasecmp(const char* s1, const char* s2, size_t n)
{
    int d = 0;
    for(; n != 0; n--)
    {
        const int c1 = __extension__({
            __typeof__(*s1++) __x = (*s1++);
            (((((_ctype_) + sizeof(""[__x]))[(int)(__x)]) & (01 | 02)) == 01) ? (int)__x - 'A' + 'a' : (int)__x;
        });
        const int c2 = __extension__({
            __typeof__(*s2++) __x = (*s2++);
            (((((_ctype_) + sizeof(""[__x]))[(int)(__x)]) & (01 | 02)) == 01) ? (int)__x - 'A' + 'a' : (int)__x;
        });
        if(((d = c1 - c2) != 0) || (c2 == '\0')) break;
    }
    return d;
}
