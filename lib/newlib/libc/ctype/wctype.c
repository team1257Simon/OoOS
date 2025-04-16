typedef struct
{
    int __count;
    union
    {
        unsigned int  __wch;
        unsigned char __wchb[4];
    } __value;
} _mbstate_t;
struct _reent;
struct __locale_t;
struct _Bigint
{
    struct _Bigint* _next;
    int             _k, _maxwds, _sign, _wds;
    unsigned int    _x[1];
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
    unsigned int _fntypes;
    unsigned int _is_cxa;
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
    long (*_seek)(struct _reent*, void*, long, int);
    int (*_close)(struct _reent*, void*);
    struct __sbuf  _ub;
    unsigned char* _up;
    int            _ur;
    unsigned char  _ubuf[3];
    unsigned char  _nbuf[1];
    struct __sbuf  _lb;
    int            _blksize;
    long           _offset;
    struct _reent* _data;
    void*          _lock;
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
extern struct _reent* _impure_ptr;
int strcmp(const char*, const char*);
typedef _mbstate_t mbstate_t;
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
struct __locale_t
{
    char categories[7][31 + 1];
    int (*wctomb)(struct _reent*, char*, int, mbstate_t*);
    int (*mbtowc)(struct _reent*, int*, const char*, unsigned long, mbstate_t*);
    int          cjk_lang;
    char*        ctype_ptr;
    struct lconv lconv;
    char         mb_cur_max[2];
    char         ctype_codeset[31 + 1];
    char         message_codeset[31 + 1];
};
int _wctrans_r(struct _reent* r, const char* c)
{
    if(!strcmp(c, "tolower"))
        return 1;
    else if(!strcmp(c, "toupper"))
        return 2;
    else
    {
        r->_errno = 22;
        return 0;
    }
}
int wctrans(const char* c) { return _wctrans_r(_impure_ptr, c); }
int wctrans_l(const char* c, struct __locale_t* locale) { return wctrans(c); }
int _wctype_r(struct _reent* r, const char* c)
{
    switch(*c)
    {
    case 'a':
        if(!strcmp(c, "alnum"))
            return 1;
        else if(!strcmp(c, "alpha"))
            return 2;
        break;
    case 'b':
        if(!strcmp(c, "blank")) return 3;
        break;
    case 'c':
        if(!strcmp(c, "cntrl")) return 4;
        break;
    case 'd':
        if(!strcmp(c, "digit")) return 5;
        break;
    case 'g':
        if(!strcmp(c, "graph")) return 6;
        break;
    case 'l':
        if(!strcmp(c, "lower")) return 7;
        break;
    case 'p':
        if(!strcmp(c, "print"))
            return 8;
        else if(!strcmp(c, "punct"))
            return 9;
        break;
    case 's':
        if(!strcmp(c, "space")) return 10;
        break;
    case 'u':
        if(!strcmp(c, "upper")) return 11;
        break;
    case 'x':
        if(!strcmp(c, "xdigit")) return 12;
        break;
    }
    r->_errno = 22;
    return 0;
}
int wctype(const char* c) { return _wctype_r(_impure_ptr, c); }
int wctype_l(const char* c, struct __locale_t* locale) { return wctype(c); }
