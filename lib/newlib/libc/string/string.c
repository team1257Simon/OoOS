typedef long unsigned int size_t;
typedef long int ssize_t;
struct __locale_t;
typedef long                    _fpos_t;
struct __lock;
typedef struct __lock* _flock_t;
typedef struct __locale_t* locale_t;
typedef unsigned int uint32_t;
typedef unsigned long      __sigset_t;
typedef __sigset_t    sigset_t;
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
typedef long                    _off_t;
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
char*                      strcpy(char* dst0, const char* src0)
{
    char*       dst = dst0;
    const char* src = src0;
    long*       aligned_dst;
    const long* aligned_src;
    if(!(((long)src & (sizeof(long) - 1)) | ((long)dst & (sizeof(long) - 1))))
    {
        aligned_dst = (long*)dst;
        aligned_src = (long*)src;
        while(!(((*aligned_src) - 0x0101010101010101) & ~(*aligned_src) & 0x8080808080808080)) { *aligned_dst++ = *aligned_src++; }
        dst = (char*)aligned_dst;
        src = (char*)aligned_src;
    }
    while((*dst++ = *src++))
        ;
    return dst0;
}
int                        memcmp(const void* m1, const void* m2, size_t n)
{
    unsigned char* s1 = (unsigned char*)m1;
    unsigned char* s2 = (unsigned char*)m2;
    unsigned long* a1;
    unsigned long* a2;
    if(!((n) < (sizeof(long))) && !(((long)s1 & (sizeof(long) - 1)) | ((long)s2 & (sizeof(long) - 1))))
    {
        a1 = (unsigned long*)s1;
        a2 = (unsigned long*)s2;
        while(n >= (sizeof(long)))
        {
            if(*a1 != *a2) break;
            a1++;
            a2++;
            n -= (sizeof(long));
        }
        s1 = (unsigned char*)a1;
        s2 = (unsigned char*)a2;
    }
    while(n--)
    {
        if(*s1 != *s2) return *s1 - *s2;
        s1++;
        s2++;
    }
    return 0;
}
void*                      mempcpy(void* dst0, const void* src0, size_t len0)
{
    char*       dst = dst0;
    const char* src = src0;
    long*       aligned_dst;
    const long* aligned_src;
    if(!((len0) < (sizeof(long) << 2)) && !(((long)src & (sizeof(long) - 1)) | ((long)dst & (sizeof(long) - 1))))
    {
        aligned_dst = (long*)dst;
        aligned_src = (long*)src;
        while(len0 >= (sizeof(long) << 2))
        {
            *aligned_dst++ = *aligned_src++;
            *aligned_dst++ = *aligned_src++;
            *aligned_dst++ = *aligned_src++;
            *aligned_dst++ = *aligned_src++;
            len0 -= (sizeof(long) << 2);
        }
        while(len0 >= (sizeof(long)))
        {
            *aligned_dst++ = *aligned_src++;
            len0 -= (sizeof(long));
        }
        dst = (char*)aligned_dst;
        src = (char*)aligned_src;
    }
    while(len0--) *dst++ = *src++;
    return dst;
}
char*                      stpcpy(char* restrict dst, const char* restrict src)
{
    long*       aligned_dst;
    const long* aligned_src;
    if(!(((long)src & (sizeof(long) - 1)) | ((long)dst & (sizeof(long) - 1))))
    {
        aligned_dst = (long*)dst;
        aligned_src = (long*)src;
        while(!(((*aligned_src) - 0x0101010101010101) & ~(*aligned_src) & 0x8080808080808080)) { *aligned_dst++ = *aligned_src++; }
        dst = (char*)aligned_dst;
        src = (char*)aligned_src;
    }
    while((*dst++ = *src++))
        ;
    return --dst;
}
char*                      strchr(const char* s1, int i)
{
    const unsigned char* s = (const unsigned char*)s1;
    unsigned char        c = i;
    unsigned long        mask, j;
    unsigned long*       aligned_addr;
    if(!c)
    {
        while(((long)s & (sizeof(long) - 1)))
        {
            if(!*s) return (char*)s;
            s++;
        }
        aligned_addr = (unsigned long*)s;
        while(!(((*aligned_addr) - 0x0101010101010101) & ~(*aligned_addr) & 0x8080808080808080)) aligned_addr++;
        s = (const unsigned char*)aligned_addr;
        while(*s) s++;
        return (char*)s;
    }
    while(((long)s & (sizeof(long) - 1)))
    {
        if(!*s) return ((void*)0);
        if(*s == c) return (char*)s;
        s++;
    }
    mask = c;
    for(j = 8; j < (sizeof(long)) * 8; j <<= 1) mask = (mask << j) | mask;
    aligned_addr = (unsigned long*)s;
    while(!(((*aligned_addr) - 0x0101010101010101) & ~(*aligned_addr) & 0x8080808080808080) &&
          !((((*aligned_addr ^ mask) - 0x0101010101010101) & ~(*aligned_addr ^ mask) & 0x8080808080808080)))
        aligned_addr++;
    s = (unsigned char*)aligned_addr;
    while(*s && *s != c) s++;
    if(*s == c) return (char*)s;
    return ((void*)0);
}
char*                      strrchr(const char* s, int i)
{
    const char* last = ((void*)0);
    if(i)
    {
        while((s = strchr(s, i)))
        {
            last = s;
            s++;
        }
    }
    else { last = strchr(s, i); }
    return (char*)last;
}
char*                      stpncpy(char* restrict dst, const char* restrict src, size_t count)
{
    char*       ret = ((void*)0);
    long*       aligned_dst;
    const long* aligned_src;
    if(!(((long)src & (sizeof(long) - 1)) | ((long)dst & (sizeof(long) - 1))) && !((count) < sizeof(long)))
    {
        aligned_dst = (long*)dst;
        aligned_src = (long*)src;
        while(count >= sizeof(long int) && !(((*aligned_src) - 0x0101010101010101) & ~(*aligned_src) & 0x8080808080808080))
        {
            count -= sizeof(long int);
            *aligned_dst++ = *aligned_src++;
        }
        dst = (char*)aligned_dst;
        src = (char*)aligned_src;
    }
    while(count > 0)
    {
        --count;
        if((*dst++ = *src++) == '\0')
        {
            ret = dst - 1;
            break;
        }
    }
    while(count-- > 0) *dst++ = '\0';
    return ret ? ret : dst;
}
void*                      memchr(const void* src_void, int c, size_t length)
{
    const unsigned char* src = (const unsigned char*)src_void;
    unsigned char        d   = c;
    unsigned long*       asrc;
    unsigned long        mask;
    unsigned int         i;
    while(((long)src & (sizeof(long) - 1)))
    {
        if(!length--) return ((void*)0);
        if(*src == d) return (void*)src;
        src++;
    }
    if(!((length) < (sizeof(long))))
    {
        asrc = (unsigned long*)src;
        mask = d << 8 | d;
        mask = mask << 16 | mask;
        for(i = 32; i < (sizeof(long)) * 8; i <<= 1) mask = (mask << i) | mask;
        while(length >= (sizeof(long)))
        {
            if(((((*asrc ^ mask) - 0x0101010101010101) & ~(*asrc ^ mask) & 0x8080808080808080))) break;
            length -= (sizeof(long));
            asrc++;
        }
        src = (unsigned char*)asrc;
    }
    while(length--)
    {
        if(*src == d) return (void*)src;
        src++;
    }
    return ((void*)0);
}

static size_t              critical_factorization(const unsigned char* needle, size_t needle_len, size_t* period)
{
    size_t        max_suffix, max_suffix_rev;
    size_t        j;
    size_t        k;
    size_t        p;
    unsigned char a, b;
    max_suffix = (0xffffffffffffffffUL);
    j          = 0;
    k = p = 1;
    while(j + k < needle_len)
    {
        a = needle[j + k];
        b = needle[(size_t)(max_suffix + k)];
        if(a < b)
        {
            j += k;
            k = 1;
            p = j - max_suffix;
        }
        else if(a == b)
        {
            if(k != p)
                ++k;
            else
            {
                j += p;
                k = 1;
            }
        }
        else
        {
            max_suffix = j++;
            k = p = 1;
        }
    }
    *period        = p;
    max_suffix_rev = (0xffffffffffffffffUL);
    j              = 0;
    k = p = 1;
    while(j + k < needle_len)
    {
        a = needle[j + k];
        b = needle[max_suffix_rev + k];
        if(b < a)
        {
            j += k;
            k = 1;
            p = j - max_suffix_rev;
        }
        else if(a == b)
        {
            if(k != p)
                ++k;
            else
            {
                j += p;
                k = 1;
            }
        }
        else
        {
            max_suffix_rev = j++;
            k = p = 1;
        }
    }
    if(max_suffix_rev + 1 < max_suffix + 1) return max_suffix + 1;
    *period = p;
    return max_suffix_rev + 1;
}
static void* two_way_short_needle(const unsigned char* haystack, size_t haystack_len, const unsigned char* needle, size_t needle_len)
{
    size_t i;
    size_t j;
    size_t period;
    size_t suffix;
    suffix = critical_factorization(needle, needle_len, &period);
    if(memcmp(needle, needle + period, suffix) == 0)
    {
        size_t memory = 0;
        j             = 0;
        while(((j) <= (haystack_len) - (needle_len)))
        {
            i = ((suffix < memory) ? (memory) : (suffix));
            while(i < needle_len && (needle[i] == haystack[i + j])) ++i;
            if(needle_len <= i)
            {
                i = suffix - 1;
                while(memory < i + 1 && (needle[i] == haystack[i + j])) --i;
                if(i + 1 < memory + 1) return (void*)(haystack + j);
                j += period;
                memory = needle_len - period;
            }
            else
            {
                j += i - suffix + 1;
                memory = 0;
            }
        }
    }
    else
    {
        period = ((suffix < needle_len - suffix) ? (needle_len - suffix) : (suffix)) + 1;
        j      = 0;
        while(((j) <= (haystack_len) - (needle_len)))
        {
            i = suffix;
            while(i < needle_len && (needle[i] == haystack[i + j])) ++i;
            if(needle_len <= i)
            {
                i = suffix - 1;
                while(i != (0xffffffffffffffffUL) && (needle[i] == haystack[i + j])) --i;
                if(i == (0xffffffffffffffffUL)) return (void*)(haystack + j);
                j += period;
            }
            else
                j += i - suffix + 1;
        }
    }
    return ((void*)0);
}
static void* two_way_long_needle(const unsigned char* haystack, size_t haystack_len, const unsigned char* needle, size_t needle_len)
{
    size_t i;
    size_t j;
    size_t period;
    size_t suffix;
    size_t shift_table[1U << 8];
    suffix = critical_factorization(needle, needle_len, &period);
    for(i = 0; i < 1U << 8; i++) shift_table[i] = needle_len;
    for(i = 0; i < needle_len; i++) shift_table[needle[i]] = needle_len - i - 1;
    if(memcmp(needle, needle + period, suffix) == 0)
    {
        size_t memory = 0;
        size_t shift;
        j = 0;
        while(((j) <= (haystack_len) - (needle_len)))
        {
            shift = shift_table[haystack[j + needle_len - 1]];
            if(0 < shift)
            {
                if(memory && shift < period) { shift = needle_len - period; }
                memory = 0;
                j += shift;
                continue;
            }
            i = ((suffix < memory) ? (memory) : (suffix));
            while(i < needle_len - 1 && (needle[i] == haystack[i + j])) ++i;
            if(needle_len - 1 <= i)
            {
                i = suffix - 1;
                while(memory < i + 1 && (needle[i] == haystack[i + j])) --i;
                if(i + 1 < memory + 1) return (void*)(haystack + j);
                j += period;
                memory = needle_len - period;
            }
            else
            {
                j += i - suffix + 1;
                memory = 0;
            }
        }
    }
    else
    {
        size_t shift;
        period = ((suffix < needle_len - suffix) ? (needle_len - suffix) : (suffix)) + 1;
        j      = 0;
        while(((j) <= (haystack_len) - (needle_len)))
        {
            shift = shift_table[haystack[j + needle_len - 1]];
            if(0 < shift)
            {
                j += shift;
                continue;
            }
            i = suffix;
            while(i < needle_len - 1 && (needle[i] == haystack[i + j])) ++i;
            if(needle_len - 1 <= i)
            {
                i = suffix - 1;
                while(i != (0xffffffffffffffffUL) && (needle[i] == haystack[i + j])) --i;
                if(i == (0xffffffffffffffffUL)) return (void*)(haystack + j);
                j += period;
            }
            else
                j += i - suffix + 1;
        }
    }
    return ((void*)0);
}
static inline char* strstr2(const char* hs, const char* ne)
{
    uint32_t h1 = (ne[0] << 16) | ne[1];
    uint32_t h2 = 0;
    int      c  = hs[0];
    while(h1 != h2 && c != 0)
    {
        h2 = (h2 << 16) | c;
        c  = *++hs;
    }
    return h1 == h2 ? (char*)hs - 2 : ((void*)0);
}
static inline char* strstr3(const char* hs, const char* ne)
{
    uint32_t h1 = (ne[0] << 24) | (ne[1] << 16) | (ne[2] << 8);
    uint32_t h2 = 0;
    int      c  = hs[0];
    while(h1 != h2 && c != 0)
    {
        h2 = (h2 | c) << 8;
        c  = *++hs;
    }
    return h1 == h2 ? (char*)hs - 3 : ((void*)0);
}
static inline char* strstr4(const char* hs, const char* ne)
{
    uint32_t h1 = (ne[0] << 24) | (ne[1] << 16) | (ne[2] << 8) | ne[3];
    uint32_t h2 = 0;
    int      c  = hs[0];
    while(h1 != h2 && c != 0)
    {
        h2 = (h2 << 8) | c;
        c  = *++hs;
    }
    return h1 == h2 ? (char*)hs - 4 : ((void*)0);
}
char* strstr(const char* searchee, const char* lookfor)
{
    const char* haystack = searchee;
    const char* needle   = lookfor;
    size_t      needle_len;
    size_t      haystack_len;
    int         ok = 1;
    if(needle[0] == '\0') return (char*)haystack;
    if(needle[1] == '\0') return strchr(haystack, needle[0]);
    if(needle[2] == '\0') return strstr2(haystack, needle);
    if(needle[3] == '\0') return strstr3(haystack, needle);
    if(needle[4] == '\0') return strstr4(haystack, needle);
    while(*haystack && *needle) ok &= *haystack++ == *needle++;
    if(*needle) return ((void*)0);
    if(ok) return (char*)searchee;
    needle_len = needle - lookfor;
    haystack   = strchr(searchee + 1, *lookfor);
    if(!haystack || needle_len == 1) return (char*)haystack;
    haystack_len = (haystack > searchee + needle_len ? 1 : needle_len + searchee - haystack);
    if(needle_len < 32U) return two_way_short_needle((const unsigned char*)haystack, haystack_len, (const unsigned char*)lookfor, needle_len);
    return two_way_long_needle((const unsigned char*)haystack, haystack_len, (const unsigned char*)lookfor, needle_len);
}
void* memmem(const void* haystack_start, size_t haystack_len, const void* needle_start, size_t needle_len)
{
    const unsigned char* haystack = (const unsigned char*)haystack_start;
    const unsigned char* needle   = (const unsigned char*)needle_start;
    if(needle_len == 0) return (void*)haystack;
    if(haystack_len < needle_len) return ((void*)0);
    if(needle_len < 32U)
    {
        haystack = memchr(haystack, *needle, haystack_len);
        if(!haystack || needle_len == 1) return (void*)haystack;
        haystack_len -= haystack - (const unsigned char*)haystack_start;
        if(haystack_len < needle_len) return ((void*)0);
        return two_way_short_needle(haystack, haystack_len, needle, needle_len);
    }
    return two_way_long_needle(haystack, haystack_len, needle, needle_len);
}

void* __attribute__((__optimize__("-fno-tree-loop-distribute-patterns"), weak)) memcpy(void* restrict dst0, const void* restrict src0, size_t len0)
{
    char*       dst = dst0;
    const char* src = src0;
    long*       aligned_dst;
    const long* aligned_src;
    if(!((len0) < (sizeof(long) << 2)) && !(((long)src & (sizeof(long) - 1)) | ((long)dst & (sizeof(long) - 1))))
    {
        aligned_dst = (long*)dst;
        aligned_src = (long*)src;
        while(len0 >= (sizeof(long) << 2))
        {
            *aligned_dst++ = *aligned_src++;
            *aligned_dst++ = *aligned_src++;
            *aligned_dst++ = *aligned_src++;
            *aligned_dst++ = *aligned_src++;
            len0 -= (sizeof(long) << 2);
        }
        while(len0 >= (sizeof(long)))
        {
            *aligned_dst++ = *aligned_src++;
            len0 -= (sizeof(long));
        }
        dst = (char*)aligned_dst;
        src = (char*)aligned_src;
    }
    while(len0--) *dst++ = *src++;
    return dst0;
}

void*                      memccpy(void* restrict dst0, const void* restrict src0, int endchar0, size_t len0)
{
    void*       ptr = ((void*)0);
    char*       dst = dst0;
    const char* src = src0;
    long*       aligned_dst;
    const long* aligned_src;
    char        endchar = endchar0 & 0xff;
    if(!((len0) < (sizeof(long))) && !(((long)src & (sizeof(long) - 1)) | ((long)dst & (sizeof(long) - 1))))
    {
        unsigned int  i;
        unsigned long mask = 0;
        aligned_dst        = (long*)dst;
        aligned_src        = (long*)src;
        for(i = 0; i < (sizeof(long)); i++) mask = (mask << 8) + endchar;
        while(len0 >= (sizeof(long)))
        {
            unsigned long buffer = (unsigned long)(*aligned_src);
            buffer ^= mask;
            if((((buffer)-0x0101010101010101) & ~(buffer)&0x8080808080808080)) break;
            *aligned_dst++ = *aligned_src++;
            len0 -= (sizeof(long));
        }
        dst = (char*)aligned_dst;
        src = (char*)aligned_src;
    }
    while(len0--)
    {
        if((*dst++ = *src++) == endchar)
        {
            ptr = dst;
            break;
        }
    }
    return ptr;
}
void* __attribute__((__optimize__("-fno-tree-loop-distribute-patterns"))) memmove(void* dst_void, const void* src_void, size_t length)
{
    char*       dst = dst_void;
    const char* src = src_void;
    long*       aligned_dst;
    const long* aligned_src;
    if(src < dst && dst < src + length)
    {
        src += length;
        dst += length;
        while(length--) { *--dst = *--src; }
    }
    else
    {
        if(!((length) < (sizeof(long) << 2)) && !(((long)src & (sizeof(long) - 1)) | ((long)dst & (sizeof(long) - 1))))
        {
            aligned_dst = (long*)dst;
            aligned_src = (long*)src;
            while(length >= (sizeof(long) << 2))
            {
                *aligned_dst++ = *aligned_src++;
                *aligned_dst++ = *aligned_src++;
                *aligned_dst++ = *aligned_src++;
                *aligned_dst++ = *aligned_src++;
                length -= (sizeof(long) << 2);
            }
            while(length >= (sizeof(long)))
            {
                *aligned_dst++ = *aligned_src++;
                length -= (sizeof(long));
            }
            dst = (char*)aligned_dst;
            src = (char*)aligned_src;
        }
        while(length--) { *dst++ = *src++; }
    }
    return dst_void;
}
void* __attribute__((__optimize__("-fno-tree-loop-distribute-patterns"), weak)) memset(void* m, int c, size_t n)
{
    char*          s = (char*)m;
    unsigned int   i;
    unsigned long  buffer;
    unsigned long* aligned_addr;
    unsigned int   d = c & 0xff;
    while(((long)s & ((sizeof(long)) - 1)))
    {
        if(n--)
            *s++ = (char)c;
        else
            return m;
    }
    if(!((n) < (sizeof(long))))
    {
        aligned_addr = (unsigned long*)s;
        buffer       = (d << 8) | d;
        buffer |= (buffer << 16);
        for(i = 32; i < (sizeof(long)) * 8; i <<= 1) buffer = (buffer << i) | buffer;
        while(n >= (sizeof(long)) * 4)
        {
            *aligned_addr++ = buffer;
            *aligned_addr++ = buffer;
            *aligned_addr++ = buffer;
            *aligned_addr++ = buffer;
            n -= 4 * (sizeof(long));
        }
        while(n >= (sizeof(long)))
        {
            *aligned_addr++ = buffer;
            n -= (sizeof(long));
        }
        s = (char*)aligned_addr;
    }
    while(n--) *s++ = (char)c;
    return m;
}

void*                      memrchr(const void* src_void, int c, size_t length)
{
    const unsigned char* src = (const unsigned char*)src_void + length - 1;
    unsigned char        d   = c;
    unsigned long*       asrc;
    unsigned long        mask;
    unsigned int         i;
    while(((long)(src + 1) & (sizeof(long) - 1)))
    {
        if(!length--) return ((void*)0);
        if(*src == d) return (void*)src;
        src--;
    }
    if(!((length) < (sizeof(long))))
    {
        asrc = (unsigned long*)(src - (sizeof(long)) + 1);
        mask = d << 8 | d;
        mask = mask << 16 | mask;
        for(i = 32; i < (sizeof(long)) * 8; i <<= 1) mask = (mask << i) | mask;
        while(length >= (sizeof(long)))
        {
            if(((((*asrc ^ mask) - 0x0101010101010101) & ~(*asrc ^ mask) & 0x8080808080808080))) break;
            length -= (sizeof(long));
            asrc--;
        }
        src = (unsigned char*)asrc + (sizeof(long)) - 1;
    }
    while(length--)
    {
        if(*src == d) return (void*)src;
        src--;
    }
    return ((void*)0);
}
void*                      rawmemchr(const void* src_void, int c)
{
    const unsigned char* src = (const unsigned char*)src_void;
    unsigned char        d   = c;
    unsigned long*       asrc;
    unsigned long        mask;
    unsigned int         i;
    while(((long)src & (sizeof(long) - 1)))
    {
        if(*src == d) return (void*)src;
        src++;
    }
    asrc = (unsigned long*)src;
    mask = d << 8 | d;
    mask = mask << 16 | mask;
    for(i = 32; i < (sizeof(long)) * 8; i <<= 1) mask = (mask << i) | mask;
    while(1)
    {
        if(((((*asrc ^ mask) - 0x0101010101010101) & ~(*asrc ^ mask) & 0x8080808080808080))) break;
        asrc++;
    }
    src = (unsigned char*)asrc;
    while(1)
    {
        if(*src == d) return (void*)src;
        src++;
    }
}
char*                      __gnu_basename(const char* path)
{
    char* p;
    if((p = strrchr(path, '/'))) return p + 1;
    return (char*)path;
}
char*                      strcat(char* restrict s1, const char* restrict s2)
{
    char* s = s1;
    if((((long)s1 & (sizeof(long) - 1)) == 0))
    {
        unsigned long* aligned_s1 = (unsigned long*)s1;
        while(!(((*aligned_s1) - 0x0101010101010101) & ~(*aligned_s1) & 0x8080808080808080)) aligned_s1++;
        s1 = (char*)aligned_s1;
    }
    while(*s1) s1++;
    strcpy(s1, s2);
    return s;
}
size_t                     strlen(const char* str)
{
    const char*    start = str;
    unsigned long* aligned_addr;
    while(((long)str & ((sizeof(long)) - 1)))
    {
        if(!*str) return str - start;
        str++;
    }
    aligned_addr = (unsigned long*)str;
    while(!(((*aligned_addr) - 0x0101010101010101) & ~(*aligned_addr) & 0x8080808080808080)) aligned_addr++;
    str = (char*)aligned_addr;
    while(*str) str++;
    return str - start;
}
char*                      strchrnul(const char* s1, int i)
{
    char* s = strchr(s1, i);
    return s ? s : (char*)s1 + strlen(s1);
}
int                        strcmp(const char* s1, const char* s2)
{
    unsigned long* a1;
    unsigned long* a2;
    if(!(((long)s1 & (sizeof(long) - 1)) | ((long)s2 & (sizeof(long) - 1))))
    {
        a1 = (unsigned long*)s1;
        a2 = (unsigned long*)s2;
        while(*a1 == *a2)
        {
            if((((*a1) - 0x0101010101010101) & ~(*a1) & 0x8080808080808080)) return 0;
            a1++;
            a2++;
        }
        s1 = (char*)a1;
        s2 = (char*)a2;
    }
    while(*s1 != '\0' && *s1 == *s2)
    {
        s1++;
        s2++;
    }
    return (*(unsigned char*)s1) - (*(unsigned char*)s2);
}
size_t                     strcspn(const char* s1, const char* s2)
{
    const char* s = s1;
    const char* c;
    while(*s1)
    {
        for(c = s2; *c; c++)
        {
            if(*s1 == *c) break;
        }
        if(*c) break;
        s1++;
    }
    return s1 - s;
}
size_t                     strspn(const char* s1, const char* s2)
{
    const char* s = s1;
    const char* c;
    while(*s1)
    {
        for(c = s2; *c; c++)
        {
            if(*s1 == *c) break;
        }
        if(*c == '\0') break;
        s1++;
    }
    return s1 - s;
}
void*              			_malloc_r(struct _reent*, size_t);
char*                      _strdup_r(struct _reent* reent_ptr, const char* str)
{
    size_t len  = strlen(str) + 1;
    char*  copy = _malloc_r(reent_ptr, len);
    if(copy) { memcpy(copy, str, len); }
    return copy;
}
size_t                     strlcat(char* dst, const char* src, size_t siz)
{
    register char*       d = dst;
    register const char* s = src;
    register size_t      n = siz;
    size_t               dlen;
    while(n-- != 0 && *d != '\0') d++;
    dlen = d - dst;
    n    = siz - dlen;
    if(n == 0) return (dlen + strlen(s));
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
size_t                     strlcpy(char* dst, const char* src, size_t siz)
{
    register char*       d = dst;
    register const char* s = src;
    register size_t      n = siz;
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
int                       siprintf(char*, const char*, ...) __attribute__((__format__(__printf__, 2, 3)));
char*                strsignal(int signal)
{
    char*          buffer;
    struct _reent* ptr;
    ptr = _impure_ptr;
    ;
    buffer = ((ptr)->_new._reent._signal_buf);
    switch(signal)
    {
    case 1: buffer = "Hangup"; break;
    case 2: buffer = "Interrupt"; break;
    case 3: buffer = "Quit"; break;
    case 4: buffer = "Illegal instruction"; break;
    case 5: buffer = "Trace/breakpoint trap"; break;
    case 6: buffer = "IOT trap"; break;
    case 7: buffer = "EMT trap"; break;
    case 8: buffer = "Floating point exception"; break;
    case 9: buffer = "Killed"; break;
    case 10: buffer = "Bus error"; break;
    case 11: buffer = "Segmentation fault"; break;
    case 12: buffer = "Bad system call"; break;
    case 13: buffer = "Broken pipe"; break;
    case 14: buffer = "Alarm clock"; break;
    case 15: buffer = "Terminated"; break;
    case 16: buffer = "Urgent I/O condition"; break;
    case 17: buffer = "Stopped (signal)"; break;
    case 18: buffer = "Stopped"; break;
    case 19: buffer = "Continued"; break;
    case 20: buffer = "Child exited"; break;
    case 21: buffer = "Stopped (tty input)"; break;
    case 23: buffer = "I/O possible"; break;
    case 28: buffer = "Window changed"; break;
    case 30: buffer = "User defined signal 1"; break;
    case 31: buffer = "User defined signal 2"; break;
    case 24: buffer = "CPU time limit exceeded"; break;
    case 25: buffer = "File size limit exceeded"; break;
    case 26: buffer = "Virtual timer expired"; break;
    case 27: buffer = "Profiling timer expired"; break;
    case 29: buffer = "Resource lost"; break;
    default: siprintf(buffer, "Unknown signal %d", signal); break;
    }
    return buffer;
}
__attribute__((weak)) int sigprocmask(int, const sigset_t*, sigset_t*)
{
    _impure_ptr->_errno = 88;
    return -1;
}
char*                      _strerror_r(struct _reent* ptr, int errnum, int internal, int* errptr)
{
    char*        error;
    extern char* _user_strerror(int, int, int*);
    switch(errnum)
    {
    case 0: error = "Success"; break;
    case 1: error = "Not owner"; break;
    case 2: error = "No such file or directory"; break;
    case 3: error = "No such process"; break;
    case 4: error = "Interrupted system call"; break;
    case 5: error = "I/O error"; break;
    case 6: error = "No such device or address"; break;
    case 7: error = "Arg list too long"; break;
    case 8: error = "Exec format error"; break;
    case 120: error = "Socket already connected"; break;
    case 9: error = "Bad file number"; break;
    case 10: error = "No children"; break;
    case 121: error = "Destination address required"; break;
    case 11: error = "No more processes"; break;
    case 12: error = "Not enough space"; break;
    case 13: error = "Permission denied"; break;
    case 14: error = "Bad address"; break;
    case 16: error = "Device or resource busy"; break;
    case 17: error = "File exists"; break;
    case 18: error = "Cross-device link"; break;
    case 19: error = "No such device"; break;
    case 20: error = "Not a directory"; break;
    case 117: error = "Host is down"; break;
    case 119: error = "Connection already in progress"; break;
    case 21: error = "Is a directory"; break;
    case 22: error = "Invalid argument"; break;
    case 115: error = "Network interface is not configured"; break;
    case 126: error = "Connection aborted by network"; break;
    case 23: error = "Too many open files in system"; break;
    case 24: error = "File descriptor value too large"; break;
    case 25: error = "Not a character device"; break;
    case 26: error = "Text file busy"; break;
    case 27: error = "File too large"; break;
    case 118: error = "Host is unreachable"; break;
    case 28: error = "No space left on device"; break;
    case 134: error = "Not supported"; break;
    case 29: error = "Illegal seek"; break;
    case 30: error = "Read-only file system"; break;
    case 31: error = "Too many links"; break;
    case 32: error = "Broken pipe"; break;
    case 33: error = "Mathematics argument out of domain of function"; break;
    case 34: error = "Result too large"; break;
    case 35: error = "No message of desired type"; break;
    case 36: error = "Identifier removed"; break;
    case 138: error = "Illegal byte sequence"; break;
    case 45: error = "Deadlock"; break;
    case 114: error = "Network is unreachable"; break;
    case 46: error = "No lock"; break;
    case 60: error = "Not a stream"; break;
    case 62: error = "Stream ioctl timeout"; break;
    case 63: error = "No stream resources"; break;
    case 67: error = "Virtual circuit is gone"; break;
    case 71: error = "Protocol error"; break;
    case 123: error = "Unknown protocol"; break;
    case 74: error = "Multihop attempted"; break;
    case 77: error = "Bad message"; break;
    case 88: error = "Function not implemented"; break;
    case 90: error = "Directory not empty"; break;
    case 91: error = "File or path name too long"; break;
    case 92: error = "Too many symbolic links"; break;
    case 105: error = "No buffer space available"; break;
    case 61: error = "No data"; break;
    case 106: error = "Address family not supported by protocol family"; break;
    case 107: error = "Protocol wrong type for socket"; break;
    case 108: error = "Socket operation on non-socket"; break;
    case 109: error = "Protocol not available"; break;
    case 111: error = "Connection refused"; break;
    case 104: error = "Connection reset by peer"; break;
    case 112: error = "Address already in use"; break;
    case 125: error = "Address not available"; break;
    case 113: error = "Software caused connection abort"; break;
    case 128: error = "Socket is not connected"; break;
    case 127: error = "Socket is already connected"; break;
    case 140: error = "Operation canceled"; break;
    case 141: error = "State not recoverable"; break;
    case 142: error = "Previous owner died"; break;
    case 95: error = "Operation not supported on socket"; break;
    case 139: error = "Value too large for defined data type"; break;
    case 122: error = "Message too long"; break;
    case 116: error = "Connection timed out"; break;
    default:
        if(!errptr) errptr = &ptr->_errno;
        if((error = _user_strerror(errnum, internal, errptr)) == 0) error = "";
        break;
    }
    return error;
}
char* strerror_r(int errnum, char* buffer, size_t n)
{
    char* error = _strerror_r(_impure_ptr, errnum, 1, ((void*)0));
    if(strlen(error) >= n) return error;
    return strcpy(buffer, error);
}
char*                      strncat(char* restrict s1, const char* restrict s2, size_t n)
{
    char* s = s1;
    if((((long)s1 & (sizeof(long) - 1)) == 0))
    {
        unsigned long* aligned_s1 = (unsigned long*)s1;
        while(!(((*aligned_s1) - 0x0101010101010101) & ~(*aligned_s1) & 0x8080808080808080)) aligned_s1++;
        s1 = (char*)aligned_s1;
    }
    while(*s1) s1++;
    while(n-- != 0 && (*s1++ = *s2++))
    {
        if(n == 0) *s1 = '\0';
    }
    return s;
}
char*                      strncpy(char* restrict dst0, const char* restrict src0, size_t count)
{
    char*       dst = dst0;
    const char* src = src0;
    long*       aligned_dst;
    const long* aligned_src;
    if(!(((long)src & (sizeof(long) - 1)) | ((long)dst & (sizeof(long) - 1))) && !((count) < sizeof(long)))
    {
        aligned_dst = (long*)dst;
        aligned_src = (long*)src;
        while(count >= sizeof(long int) && !(((*aligned_src) - 0x0101010101010101) & ~(*aligned_src) & 0x8080808080808080))
        {
            count -= sizeof(long int);
            *aligned_dst++ = *aligned_src++;
        }
        dst = (char*)aligned_dst;
        src = (char*)aligned_src;
    }
    while(count > 0)
    {
        --count;
        if((*dst++ = *src++) == '\0') break;
    }
    while(count-- > 0) *dst++ = '\0';
    return dst0;
}
int                        strncmp(const char* s1, const char* s2, size_t n)
{
    unsigned long* a1;
    unsigned long* a2;
    if(n == 0) return 0;
    if(!(((long)s1 & (sizeof(long) - 1)) | ((long)s2 & (sizeof(long) - 1))))
    {
        a1 = (unsigned long*)s1;
        a2 = (unsigned long*)s2;
        while(n >= sizeof(long) && *a1 == *a2)
        {
            n -= sizeof(long);
            if(n == 0 || (((*a1) - 0x0101010101010101) & ~(*a1) & 0x8080808080808080)) return 0;
            a1++;
            a2++;
        }
        s1 = (char*)a1;
        s2 = (char*)a2;
    }
    while(n-- > 0 && *s1 == *s2)
    {
        if(n == 0 || *s1 == '\0') return 0;
        s1++;
        s2++;
    }
    return (*(unsigned char*)s1) - (*(unsigned char*)s2);
}
char*                      _strndup_r(struct _reent* reent_ptr, const char* str, size_t n)
{
    const char* ptr = str;
    size_t      len;
    char*       copy;
    while(n-- > 0 && *ptr) ptr++;
    len  = ptr - str;
    copy = _malloc_r(reent_ptr, len + 1);
    if(copy)
    {
        memcpy(copy, str, len);
        copy[len] = '\0';
    }
    return copy;
}
size_t                     strnlen(const char* str, size_t n)
{
    const char* start = str;
    while(n-- > 0 && *str) str++;
    return str - start;
}
char* strnstr(const char* haystack, const char* needle, size_t haystack_len)
{
    size_t needle_len = strnlen(needle, haystack_len);
    if(needle_len < haystack_len || !needle[needle_len])
    {
        char* x = memmem(haystack, haystack_len, needle, needle_len);
        if(x && !memchr(haystack, 0, x - haystack)) return x;
    }
    return ((void*)0);
}
char*                      strpbrk(const char* s1, const char* s2)
{
    const char* c = s2;
    if(!*s1) return (char*)((void*)0);
    while(*s1)
    {
        for(c = s2; *c; c++)
        {
            if(*s1 == *c) break;
        }
        if(*c) break;
        s1++;
    }
    if(*c == '\0') s1 = ((void*)0);
    return (char*)s1;
}
char*                      __strtok_r(register char* s, register const char* delim, char** lasts, int skip_leading_delim)
{
    register char* spanp;
    register int   c, sc;
    char*          tok;
    if(s == ((void*)0) && (s = *lasts) == ((void*)0)) return (((void*)0));
cont:
    c = *s++;
    for(spanp = (char*)delim; (sc = *spanp++) != 0;)
    {
        if(c == sc)
        {
            if(skip_leading_delim) { goto cont; }
            else
            {
                *lasts = s;
                s[-1]  = 0;
                return (s - 1);
            }
        }
    }
    if(c == 0)
    {
        *lasts = ((void*)0);
        return (((void*)0));
    }
    tok = s - 1;
    for(;;)
    {
        c     = *s++;
        spanp = (char*)delim;
        do {
            if((sc = *spanp++) == c)
            {
                if(c == 0)
                    s = ((void*)0);
                else
                    s[-1] = 0;
                *lasts = s;
                return (tok);
            }
        } while(sc != 0);
    }
}
size_t                     strxfrm(char* restrict s1, const char* restrict s2, size_t n)
{
    size_t res;
    res = 0;
    while(n-- > 0)
    {
        if((*s1++ = *s2++) != '\0')
            ++res;
        else
            return res;
    }
    while(*s2)
    {
        ++s2;
        ++res;
    }
    return res;
}
char* strtok_r(register char* restrict s, register const char* restrict delim, char** restrict lasts) { return __strtok_r(s, delim, lasts, 1); }
char*                strtok(register char* restrict s, register const char* restrict delim)
{
    struct _reent* reent = _impure_ptr;
    ;
    return __strtok_r(s, delim, &(((reent)->_new._reent._strtok_last)), 1);
}
void          swab(const void* b1, void* b2, ssize_t length)
{
    const char* from = b1;
    char*       to   = b2;
    ssize_t     ptr;
    for(ptr = 1; ptr < length; ptr += 2)
    {
        char p      = from[ptr];
        char q      = from[ptr - 1];
        to[ptr - 1] = p;
        to[ptr]     = q;
    }
    if(ptr == length) to[ptr - 1] = 0;
}
int                        timingsafe_bcmp(const void* b1, const void* b2, size_t n)
{
    const unsigned char *p1 = b1, *p2 = b2;
    int                  ret = 0;
    for(; n > 0; n--) ret |= *p1++ ^ *p2++;
    return (ret != 0);
}
int                        timingsafe_memcmp(const void* b1, const void* b2, size_t len)
{
    const unsigned char *p1 = b1, *p2 = b2;
    size_t               i;
    int                  res = 0, done = 0;
    for(i = 0; i < len; i++)
    {
        int lt  = (p1[i] - p2[i]) >> 8;
        int gt  = (p2[i] - p1[i]) >> 8;
        int cmp = lt - gt;
        res |= cmp & ~done;
        done |= lt | gt;
    }
    return (res);
}
char*                strsep(register char** source_ptr, register const char* delim) { return __strtok_r(*source_ptr, delim, source_ptr, 0); }
char*                      strndup(const char* str, size_t n) { return _strndup_r(_impure_ptr, str, n); }
char* strerror(int errnum) { return _strerror_r(_impure_ptr, errnum, 0, ((void*)0)); }
char* strerror_l(int errnum, locale_t locale) { return _strerror_r(_impure_ptr, errnum, 0, ((void*)0)); }
char*                      strdup(const char* str) { return _strdup_r(_impure_ptr, str); }
int                        strcoll(const char* a, const char* b) { return strcmp(a, b); }
int                        strcoll_l(const char* a, const char* b, struct __locale_t* locale) { return strcmp(a, b); }
char*                      rindex(const char* s, int c) { return strrchr(s, c); }
char*                      index(const char* s, int c) { return strchr(s, c); }
void                       bzero(void* b, size_t length) { memset(b, 0, length); }
void                       explicit_bzero(void* p, size_t n) { bzero(p, n); }
void                       bcopy(const void* b1, void* b2, size_t length) { memmove(b2, b1, length); }
int                        bcmp(const void* m1, const void* m2, size_t n) { return memcmp(m1, m2, n); }
int                        __xpg_strerror_r(int errnum, char* buffer, size_t n)
{
    char* error;
    int   result = 0;
    if(!n) return 34;
    error = _strerror_r(_impure_ptr, errnum, 1, &result);
    if(strlen(error) >= n)
    {
        memcpy(buffer, error, n - 1);
        buffer[n - 1] = '\0';
        return 34;
    }
    strcpy(buffer, error);
    return (result || *error) ? result : 22;
}