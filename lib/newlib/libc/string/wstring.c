typedef long unsigned int size_t;
typedef int               wchar_t;
typedef long                    _off_t;
__extension__ typedef long long _off64_t;
typedef _off_t                  __off_t;
typedef long                    _fpos_t;
typedef unsigned long;
typedef signed long _ssize_t;
struct __lock;
typedef struct __lock* _flock_t;
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
struct __locale_t;
typedef struct __locale_t* locale_t;
typedef _mbstate_t mbstate_t;
void*              			_malloc_r(struct _reent*, size_t);
wchar_t*           wmemset(wchar_t* s, wchar_t c, size_t n)
{
    size_t   i;
    wchar_t* p;
    p = (wchar_t*)s;
    for(i = 0; i < n; i++)
    {
        *p = c;
        p++;
    }
    return s;
}
void*                      memmove(void*, const void*, size_t);
void*                      memcpy(void* restrict, const void* restrict, size_t);
void*                      mempcpy(void*, const void*, size_t);
wchar_t*           wmemmove(wchar_t* d, const wchar_t* s, size_t n) { return (wchar_t*)memmove(d, s, n * sizeof(wchar_t)); }
wchar_t*           wmemcpy(wchar_t* restrict d, const wchar_t* restrict s, size_t n) { return (wchar_t*)memcpy(d, s, n * sizeof(wchar_t)); }
wchar_t*           wmempcpy(wchar_t* restrict d, const wchar_t* restrict s, size_t n) { return (wchar_t*)mempcpy(d, s, n * sizeof(wchar_t)); }
size_t             wcslen(const wchar_t* s)
{
    const wchar_t* p;
    p = s;
    while(*p) p++;
    return p - s;
}
wchar_t*           wcschr(const wchar_t* s, wchar_t c)
{
    const wchar_t* p;
    p = s;
    do {
        if(*p == c) { return (wchar_t*)p; }
    } while(*p++);
    return ((void*)0);
}
wchar_t*           wcscat(wchar_t* restrict s1, const wchar_t* restrict s2)
{
    wchar_t*       p;
    wchar_t*       q;
    const wchar_t* r;
    p = s1;
    while(*p) p++;
    q = p;
    r = s2;
    while(*r) *q++ = *r++;
    *q = '\0';
    return s1;
}
int                wcscmp(const wchar_t* s1, const wchar_t* s2)
{
    while(*s1 == *s2++)
        if(*s1++ == 0) return (0);
    return (*s1 - *--s2);
}
int                wcscoll(const wchar_t* a, const wchar_t* b) { return wcscmp(a, b); }
int                wcscoll_l(const wchar_t* a, const wchar_t* b, struct __locale_t* locale) { return wcscmp(a, b); }
wchar_t*           wcscpy(wchar_t* restrict s1, const wchar_t* restrict s2)
{
    wchar_t*       p;
    const wchar_t* q;
    *s1 = '\0';
    p   = s1;
    q   = s2;
    while(*q) *p++ = *q++;
    *p = '\0';
    return s1;
}
wchar_t*           _wcsdup_r(struct _reent* p, const wchar_t* str)
{
    size_t   len  = wcslen(str) + 1;
    wchar_t* copy = _malloc_r(p, len * sizeof(wchar_t));
    if(copy) wmemcpy(copy, str, len);
    return copy;
}
wchar_t* wcsdup(const wchar_t* str) { return _wcsdup_r(_impure_ptr, str); }
size_t             wcscspn(const wchar_t* s, const wchar_t* set)
{
    const wchar_t* p;
    const wchar_t* q;
    p = s;
    while(*p)
    {
        q = set;
        while(*q)
        {
            if(*p == *q) goto done;
            q++;
        }
        p++;
    }
done:
    return (p - s);
}
size_t             wcslcat(wchar_t* dst, const wchar_t* src, size_t siz)
{
    wchar_t*       d = dst;
    const wchar_t* s = src;
    size_t         n = siz;
    size_t         dlen;
    while(*d != '\0' && n-- != 0) d++;
    dlen = d - dst;
    n    = siz - dlen;
    if(n == 0) return (dlen + wcslen(s));
    while(*s != '\0')
    {
        if(n != 1)
        {
            *d++ = *s;
            n--;
        }
        s++;
    }
    *d = '\0';
    return (dlen + (s - src));
}
size_t             wcslcpy(wchar_t* dst, const wchar_t* src, size_t siz)
{
    wchar_t*       d = dst;
    const wchar_t* s = src;
    size_t         n = siz;
    if(n != 0 && --n != 0)
    {
        do {
            if((*d++ = *s++) == 0) break;
        } while(--n != 0);
    }
    if(n == 0)
    {
        if(siz != 0) *d = '\0';
        while(*s++)
            ;
    }
    return (s - src - 1);
}
wchar_t*           wcpncpy(wchar_t* restrict dst, const wchar_t* restrict src, size_t count)
{
    wchar_t* ret = ((void*)0);
    while(count > 0)
    {
        --count;
        if((*dst++ = *src++) == L'\0')
        {
            ret = dst - 1;
            break;
        }
    }
    while(count-- > 0) *dst++ = L'\0';
    return ret ? ret : dst;
}
wint_t             towlower(wint_t);
extern wint_t      towlower_l(wint_t, locale_t);
int                wcscasecmp(const wchar_t* s1, const wchar_t* s2)
{
    int d = 0;
    for(;;)
    {
        const int c1 = towlower(*s1++);
        const int c2 = towlower(*s2++);
        if(((d = c1 - c2) != 0) || (c2 == '\0')) break;
    }
    return d;
}
int                wcscasecmp_l(const wchar_t* s1, const wchar_t* s2, struct __locale_t* locale)
{
    int d = 0;
    for(;;)
    {
        const int c1 = towlower_l(*s1++, locale);
        const int c2 = towlower_l(*s2++, locale);
        if(((d = c1 - c2) != 0) || (c2 == '\0')) break;
    }
    return d;
}
int                wcsncasecmp_l(const wchar_t* s1, const wchar_t* s2, size_t n, struct __locale_t* locale)
{
    int d = 0;
    for(; n != 0; n--)
    {
        const int c1 = towlower_l(*s1++, locale);
        const int c2 = towlower_l(*s2++, locale);
        if(((d = c1 - c2) != 0) || (c2 == '\0')) break;
    }
    return d;
}
int                wcsncasecmp(const wchar_t* s1, const wchar_t* s2, size_t n)
{
    int d = 0;
    for(; n != 0; n--)
    {
        const int c1 = towlower(*s1++);
        const int c2 = towlower(*s2++);
        if(((d = c1 - c2) != 0) || (c2 == '\0')) break;
    }
    return d;
}
wchar_t*           wcsncat(wchar_t* restrict s1, const wchar_t* restrict s2, size_t n)
{
    wchar_t*       p;
    wchar_t*       q;
    const wchar_t* r;
    p = s1;
    while(*p) p++;
    q = p;
    r = s2;
    while(*r && n)
    {
        *q++ = *r++;
        n--;
    }
    *q = '\0';
    return s1;
}
int                wcsncmp(const wchar_t* s1, const wchar_t* s2, size_t n)
{
    if(n == 0) return (0);
    do {
        if(*s1 != *s2++) { return (*s1 - *--s2); }
        if(*s1++ == 0) break;
    } while(--n != 0);
    return (0);
}
wchar_t*           wcsncpy(wchar_t* restrict s1, const wchar_t* restrict s2, size_t n)
{
    wchar_t* dscan = s1;
    while(n > 0)
    {
        --n;
        if((*dscan++ = *s2++) == L'\0') break;
    }
    while(n-- > 0) *dscan++ = L'\0';
    return s1;
}
size_t             wcsnlen(const wchar_t* s, size_t maxlen)
{
    const wchar_t* p;
    p = s;
    while(*p && maxlen-- > 0) p++;
    return (size_t)(p - s);
}
wchar_t*           wcspbrk(const wchar_t* s, const wchar_t* set)
{
    const wchar_t* p;
    const wchar_t* q;
    p = s;
    while(*p)
    {
        q = set;
        while(*q)
        {
            if(*p == *q) { return (wchar_t*)p; }
            q++;
        }
        p++;
    }
    return ((void*)0);
}
wchar_t*           wcsrchr(const wchar_t* s, wchar_t c)
{
    const wchar_t* p;
    p = s;
    while(*p) p++;
    while(s <= p)
    {
        if(*p == c) { return (wchar_t*)p; }
        p--;
    }
    return ((void*)0);
}
size_t             wcsspn(const wchar_t* s, const wchar_t* set)
{
    const wchar_t* p;
    const wchar_t* q;
    p = s;
    while(*p)
    {
        q = set;
        while(*q)
        {
            if(*p == *q) break;
            q++;
        }
        if(!*q) goto done;
        p++;
    }
done:
    return (p - s);
}
wchar_t*           wcsstr(const wchar_t* restrict big, const wchar_t* restrict little)
{
    const wchar_t* p;
    const wchar_t* q;
    const wchar_t* r;
    if(!*little) { return (wchar_t*)big; }
    if(wcslen(big) < wcslen(little)) return ((void*)0);
    p = big;
    q = little;
    while(*p)
    {
        q = little;
        r = p;
        while(*q)
        {
            if(*r != *q) break;
            q++;
            r++;
        }
        if(!*q) { return (wchar_t*)p; }
        p++;
    }
    return ((void*)0);
}
wchar_t*           wcstok(register wchar_t* restrict s, register const wchar_t* restrict delim, wchar_t** restrict lasts)
{
    register const wchar_t* spanp;
    register int            c, sc;
    wchar_t*                tok;
    if(s == ((void*)0) && (s = *lasts) == ((void*)0)) return (((void*)0));
cont:
    c = *s++;
    for(spanp = delim; (sc = *spanp++) != L'\0';)
    {
        if(c == sc) goto cont;
    }
    if(c == L'\0')
    {
        *lasts = ((void*)0);
        return (((void*)0));
    }
    tok = s - 1;
    for(;;)
    {
        c     = *s++;
        spanp = delim;
        do {
            if((sc = *spanp++) == c)
            {
                if(c == L'\0')
                    s = ((void*)0);
                else
                    s[-1] = L'\0';
                *lasts = s;
                return (tok);
            }
        } while(sc != L'\0');
    }
}
int                iswprint(wint_t);
int                iswcntrl(wint_t);
int                                           __wcwidth(const wint_t ucs)
{
    if(iswprint(ucs)) return 1;
    if(iswcntrl(ucs) || ucs == L'\0') return 0;
    return -1;
}
int wcwidth(const wint_t wc)
{
    wint_t wi = wc;
    return __wcwidth(wi);
}
int                                           wcswidth(const wchar_t* pwcs, size_t n)
{
    int w, len = 0;
    if(!pwcs || n == 0) return 0;
    do {
        wint_t wi = *pwcs;
        if((w = __wcwidth(wi)) < 0) return -1;
        len += w;
    } while(*pwcs++ && --n > 0);
    return len;
}
size_t             wcsxfrm(wchar_t* restrict a, const wchar_t* restrict b, size_t n) { return wcslcpy(a, b, n); }
wchar_t*           wmemchr(const wchar_t* s, wchar_t c, size_t n)
{
    size_t i;
    for(i = 0; i < n; i++)
    {
        if(*s == c) { return (wchar_t*)s; }
        s++;
    }
    return ((void*)0);
}
int                wmemcmp(const wchar_t* s1, const wchar_t* s2, size_t n)
{
    size_t i;
    for(i = 0; i < n; i++)
    {
        if(*s1 != *s2) { return *s1 > *s2 ? 1 : -1; }
        s1++;
        s2++;
    }
    return 0;
}
size_t             wcsxfrm_l(wchar_t* restrict a, const wchar_t* restrict b, size_t n, struct __locale_t* locale) { return wcslcpy(a, b, n); }
wchar_t*           wcpcpy(wchar_t* restrict s1, const wchar_t* restrict s2)
{
    while((*s1++ = *s2++))
        ;
    return --s1;
}