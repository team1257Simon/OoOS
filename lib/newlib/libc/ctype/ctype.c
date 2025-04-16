const char _ctype_b[384] = 
{
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 040, 040, 040, 040, 040, 040, 040,
040, 040, 050, 050, 050, 050, 050, 040, 040, 040, 040, 040, 040, 040, 040,
040, 040, 040, 040, 040, 040, 040, 040, 040, 040, 0210, 020, 020, 020, 020,
020, 020, 020, 020, 020, 020, 020, 020, 020, 020, 020, 04, 04, 04, 04,
04, 04, 04, 04, 04, 04, 020, 020, 020, 020, 020, 020, 020, 0101, 0101,
0101, 0101, 0101, 0101, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01,
01, 01, 01, 01, 01, 01, 01, 01, 01, 020, 020, 020, 020, 020, 020,
0102, 0102, 0102, 0102, 0102, 0102, 02, 02, 02, 02, 02, 02, 02, 02, 02,
02, 02, 02, 02, 02, 02, 02, 02, 02, 02, 02, 020, 020, 020, 020,
040, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0
};
const char _ctype_[257] = 
{
0, 040, 040, 040, 040, 040, 040, 040, 040, 040, 050, 050, 050, 050, 050,
040, 040, 040, 040, 040, 040, 040, 040, 040, 040, 040, 040, 040, 040, 040,
040, 040, 040, 0210, 020, 020, 020, 020, 020, 020, 020, 020, 020, 020, 020,
020, 020, 020, 020, 04, 04, 04, 04, 04, 04, 04, 04, 04, 04, 020,
020, 020, 020, 020, 020, 020, 0101, 0101, 0101, 0101, 0101, 0101, 01, 01, 01,
01, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01, 01,
01, 01, 020, 020, 020, 020, 020, 020, 0102, 0102, 0102, 0102, 0102, 0102, 02,
02, 02, 02, 02, 02, 02, 02, 02, 02, 02, 02, 02, 02, 02, 02,
02, 02, 02, 02, 020, 020, 020, 020, 040, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0
};
struct __locale_t;
typedef unsigned int wint_t;
typedef int wctype_t;
typedef int wctrans_t;
typedef int int_least32_t;
typedef unsigned int uint_least32_t;
struct _reent;
extern struct _reent* _impure_ptr;
int isalnum_l(int c, struct __locale_t* locale) { return _ctype_[c + 1] & (01 | 02 | 04); }
int isalpha_l(int c, struct __locale_t* locale) { return _ctype_[c + 1] & (01 | 02); }
int isascii_l(int c, struct __locale_t* locale) { return c >= 0 && c < 128; }
int isblank_l(int c, struct __locale_t* locale) { return (_ctype_[c + 1] & 0200) || (c == '\t'); }
int iscntrl_l(int c, struct __locale_t* locale) { return _ctype_[c + 1] & 040; }
int isdigit_l(int c, struct __locale_t* locale) { return _ctype_[c + 1] & 04; }
int islower_l(int c, struct __locale_t* locale) { return (_ctype_[c + 1] & (01 | 02)) == 02; }
int isgraph_l(int c, struct __locale_t* locale) { return _ctype_[c + 1] & (020 | 01 | 02 | 04); }
int isprint_l(int c, struct __locale_t* locale) { return _ctype_[c + 1] & (020 | 01 | 02 | 04 | 0200); }
int isupper_l(int c, struct __locale_t* locale) { return (_ctype_[c + 1] & (01 | 02)) == 01; }
int ispunct_l(int c, struct __locale_t* locale) { return _ctype_[c + 1] & 020; }
int isspace_l(int c, struct __locale_t* locale) { return _ctype_[c + 1] & 010; }
int isxdigit_l(int c, struct __locale_t* locale) { return _ctype_[c + 1] & ((0100) | (04)); }
int isalnum(int c) { return ((_ctype_)[c + 1] & (01 | 02 | 04)); }
int isalpha(int c) { return ((_ctype_)[c + 1] & (01 | 02)); }
int isascii(int c) { return c >= 0 && c < 128; }
int isblank(int c) { return (((_ctype_)[c + 1] & 0200) || (c == '\t')); }
int iscntrl(int c) { return ((_ctype_)[c + 1] & 040); }
int isdigit(int c) { return ((_ctype_)[c + 1] & 04); }
int islower(int c) { return (((_ctype_)[c + 1] & (01 | 02)) == 02); }
int isgraph(int c) { return ((_ctype_)[c + 1] & (020 | 01 | 02 | 04)); }
int isprint(int c) { return ((_ctype_)[c + 1] & (020 | 01 | 02 | 04 | 0200)); }
int isupper(int c) { return (((_ctype_)[c + 1] & (01 | 02)) == 01); }
int ispunct(int c) { return ((_ctype_)[c + 1] & 020); }
int isspace(int c) { return ((_ctype_)[c + 1] & 010); }
int isxdigit(int c) { return ((_ctype_)[c + 1] & ((0100) | (04))); }
int iswalnum_l(wint_t c, struct __locale_t* locale) { return c < (wint_t)0x100 ? ((((_ctype_) + sizeof(""[c]))[(int)(c)]) & (01 | 02 | 04)) : 0; }
int iswalpha_l(wint_t c, struct __locale_t* locale) { return c < (wint_t)0x100 ? ((((_ctype_) + sizeof(""[c]))[(int)(c)]) & (01 | 02)) : 0; }
int iswblank_l(wint_t c, struct __locale_t* locale) { return c < 0x100 ? __extension__({ __typeof__(c) __x = (c); ((((_ctype_) + sizeof(""[__x]))[(int)(__x)]) & 0200) || (int)(__x) == '\t'; }) : 0; }
int iswcntrl_l(wint_t c, struct __locale_t* locale) { return c < 0x100 ? ((((_ctype_) + sizeof(""[c]))[(int)(c)]) & 040) : 0; }
int iswdigit_l(wint_t c, struct __locale_t* locale) { return c >= (wint_t)'0' && c <= (wint_t)'9'; }
int iswprint_l(wint_t c, struct __locale_t* locale) { return c < (wint_t)0x100 ? ((((_ctype_) + sizeof(""[c]))[(int)(c)]) & (020 | 01 | 02 | 04 | 0200)) : 0; }
int iswspace_l(wint_t c, struct __locale_t* locale) { return c < 0x100 ? ((((_ctype_) + sizeof(""[c]))[(int)(c)]) & 010) : 0; }
int iswgraph_l(wint_t c, struct __locale_t* locale) { return iswprint_l(c, locale) && !iswspace_l(c, locale); }
int iswlower_l(wint_t c, struct __locale_t* locale) { return c < 0x100 ? (((((_ctype_) + sizeof(""[c]))[(int)(c)]) & (01 | 02)) == 02) : 0; }
int iswpunct_l(wint_t c, struct __locale_t* locale) { return c < (wint_t)0x100 ? ((((_ctype_) + sizeof(""[c]))[(int)(c)]) & 020) : 0; }
int iswupper_l(wint_t c, struct __locale_t* locale) { return c < 0x100 ? (((((_ctype_) + sizeof(""[c]))[(int)(c)]) & (01 | 02)) == 01) : 0; }
int iswxdigit_l(wint_t c, struct __locale_t* locale) { return ((c >= (wint_t)'0' && c <= (wint_t)'9') || (c >= (wint_t)'a' && c <= (wint_t)'f') || (c >= (wint_t)'A' && c <= (wint_t)'F')); }
int iswdigit(wint_t c) { return c >= (wint_t)'0' && c <= (wint_t)'9'; }
int iswalpha(wint_t c) { return iswalpha_l(c, 0); }
int iswblank(wint_t c) { return iswblank_l(c, 0); }
int iswcntrl(wint_t c) { return iswcntrl_l(c, 0); }
int iswalnum(wint_t c) { return iswalnum_l(c, 0); }
int iswspace(wint_t c) { return iswspace_l(c, 0); }
int iswprint(wint_t c) { return iswprint_l(c, 0); }
int iswgraph(wint_t c) { return iswgraph_l(c, 0); }
int iswlower(wint_t c) { return iswlower_l(c, 0); }
int iswpunct(wint_t c) { return iswpunct_l(c, 0); }
int iswupper(wint_t c) { return iswupper_l(c, 0); }
int iswxdigit(wint_t c) { return (c >= (wint_t)'0' && c <= (wint_t)'9') || (c >= (wint_t)'a' && c <= (wint_t)'f') || (c >= (wint_t)'A' && c <= (wint_t)'F'); }
int iswctype_l(wint_t c, wctype_t desc, struct __locale_t* locale)
{
switch(desc)
{
case 1: return iswalnum_l(c, locale);
case 2: return iswalpha_l(c, locale);
case 3: return iswblank_l(c, locale);
case 4: return iswcntrl_l(c, locale);
case 5: return iswdigit_l(c, locale);
case 6: return iswgraph_l(c, locale);
case 7: return iswlower_l(c, locale);
case 8: return iswprint_l(c, locale);
case 9: return iswpunct_l(c, locale);
case 10: return iswspace_l(c, locale);
case 11: return iswupper_l(c, locale);
case 12: return iswxdigit_l(c, locale);
default: return 0;
}
return 0;
}
int iswctype(wint_t c, wctype_t desc)
{
switch(desc)
{
case 1: return iswalnum(c);
case 2: return iswalpha(c);
case 3: return iswblank(c);
case 4: return iswcntrl(c);
case 5: return iswdigit(c);
case 6: return iswgraph(c);
case 7: return iswlower(c);
case 8: return iswprint(c);
case 9: return iswpunct(c);
case 10: return iswspace(c);
case 11: return iswupper(c);
case 12: return iswxdigit(c);
default: return 0;
}
return 0;
}
int toascii_l(int c, struct __locale_t* locale) { return c & 0177; }
int tolower_l(int c, struct __locale_t* locale) { return ((((_ctype_ + sizeof(""[c]))[(int)(c)]) & (01 | 02)) == 01) ? (c) - 'A' + 'a' : c; }
int toupper_l(int c, struct __locale_t* locale) { return ((((_ctype_ + sizeof(""[c]))[(int)(c)]) & (01 | 02)) == 02) ? c - 'a' + 'A' : c; }
int toascii(int c) { return (c) & 0177; }
int tolower(int c) { return (((((_ctype_) + sizeof(""[c]))[(int)(c)]) & (01 | 02)) == 01) ? (c) - 'A' + 'a' : c; }
int toupper(int c) { return (((((_ctype_) + sizeof(""[c]))[(int)(c)]) & (01 | 02)) == 02) ? c - 'a' + 'A' : c; }
wint_t _jp2uc_l(wint_t, struct __locale_t*);
wint_t _uc2jp_l(wint_t, struct __locale_t*);
enum
{
 TO1,
 TOLO,
 TOUP,
 TOBOTH
};
enum
{
 EVENCAP,
 ODDCAP
};
static struct caseconv_entry
{
 uint_least32_t first : 21;
 uint_least32_t diff : 8;
 uint_least32_t mode : 2;
 int_least32_t delta : 17;
} __attribute__((packed)) caseconv_table[] = 
{
    {0x0041, 25, TOLO, 32}, {0x0061, 25, TOUP, -32}, {0x00B5, 0, TOUP, 743}, {0x00C0, 22, TOLO, 32}, {0x00D8, 6, TOLO, 32},
    {0x00E0, 22, TOUP, -32}, {0x00F8, 6, TOUP, -32}, {0x00FF, 0, TOUP, 121}, {0x0100, 47, TO1, EVENCAP}, {0x0130, 0, TOLO, -199},
    {0x0131, 0, TOUP, -232}, {0x0132, 5, TO1, EVENCAP}, {0x0139, 15, TO1, ODDCAP}, {0x014A, 45, TO1, EVENCAP}, {0x0178, 0, TOLO, -121},
    {0x0179, 5, TO1, ODDCAP}, {0x017F, 0, TOUP, -300}, {0x0180, 0, TOUP, 195}, {0x0181, 0, TOLO, 210}, {0x0182, 3, TO1, EVENCAP},
    {0x0186, 0, TOLO, 206}, {0x0187, 1, TO1, ODDCAP}, {0x0189, 1, TOLO, 205}, {0x018B, 1, TO1, ODDCAP}, {0x018E, 0, TOLO, 79},
    {0x018F, 0, TOLO, 202}, {0x0190, 0, TOLO, 203}, {0x0191, 1, TO1, ODDCAP}, {0x0193, 0, TOLO, 205}, {0x0194, 0, TOLO, 207},
    {0x0195, 0, TOUP, 97}, {0x0196, 0, TOLO, 211}, {0x0197, 0, TOLO, 209}, {0x0198, 1, TO1, EVENCAP}, {0x019A, 0, TOUP, 163},
    {0x019C, 0, TOLO, 211}, {0x019D, 0, TOLO, 213}, {0x019E, 0, TOUP, 130}, {0x019F, 0, TOLO, 214}, {0x01A0, 5, TO1, EVENCAP},
    {0x01A6, 0, TOLO, 218}, {0x01A7, 1, TO1, ODDCAP}, {0x01A9, 0, TOLO, 218}, {0x01AC, 1, TO1, EVENCAP}, {0x01AE, 0, TOLO, 218},
    {0x01AF, 1, TO1, ODDCAP}, {0x01B1, 1, TOLO, 217}, {0x01B3, 3, TO1, ODDCAP}, {0x01B7, 0, TOLO, 219}, {0x01B8, 1, TO1, EVENCAP},
    {0x01BC, 1, TO1, EVENCAP}, {0x01BF, 0, TOUP, 56}, {0x01C4, 0, TOLO, 2}, {0x01C5, 0, TOBOTH, 0}, {0x01C6, 0, TOUP, -2},
    {0x01C7, 0, TOLO, 2}, {0x01C8, 0, TOBOTH, 0}, {0x01C9, 0, TOUP, -2}, {0x01CA, 0, TOLO, 2}, {0x01CB, 0, TOBOTH, 0},
    {0x01CC, 0, TOUP, -2}, {0x01CD, 15, TO1, ODDCAP}, {0x01DD, 0, TOUP, -79}, {0x01DE, 17, TO1, EVENCAP}, {0x01F1, 0, TOLO, 2},
    {0x01F2, 0, TOBOTH, 0}, {0x01F3, 0, TOUP, -2}, {0x01F4, 1, TO1, EVENCAP}, {0x01F6, 0, TOLO, -97}, {0x01F7, 0, TOLO, -56},
    {0x01F8, 39, TO1, EVENCAP}, {0x0220, 0, TOLO, -130}, {0x0222, 17, TO1, EVENCAP}, {0x023A, 0, TOLO, 10795}, {0x023B, 1, TO1, ODDCAP},
    {0x023D, 0, TOLO, -163}, {0x023E, 0, TOLO, 10792}, {0x023F, 1, TOUP, 10815}, {0x0241, 1, TO1, ODDCAP}, {0x0243, 0, TOLO, -195},
    {0x0244, 0, TOLO, 69}, {0x0245, 0, TOLO, 71}, {0x0246, 9, TO1, EVENCAP}, {0x0250, 0, TOUP, 10783}, {0x0251, 0, TOUP, 10780},
    {0x0252, 0, TOUP, 10782}, {0x0253, 0, TOUP, -210}, {0x0254, 0, TOUP, -206}, {0x0256, 1, TOUP, -205}, {0x0259, 0, TOUP, -202},
    {0x025B, 0, TOUP, -203}, {0x025C, 0, TOUP, 42319}, {0x0260, 0, TOUP, -205}, {0x0261, 0, TOUP, 42315}, {0x0263, 0, TOUP, -207},
    {0x0265, 0, TOUP, 42280}, {0x0266, 0, TOUP, 42308}, {0x0268, 0, TOUP, -209}, {0x0269, 0, TOUP, -211}, {0x026A, 0, TOUP, 42308},
    {0x026B, 0, TOUP, 10743}, {0x026C, 0, TOUP, 42305}, {0x026F, 0, TOUP, -211}, {0x0271, 0, TOUP, 10749}, {0x0272, 0, TOUP, -213},
    {0x0275, 0, TOUP, -214}, {0x027D, 0, TOUP, 10727}, {0x0280, 0, TOUP, -218}, {0x0283, 0, TOUP, -218}, {0x0287, 0, TOUP, 42282},
    {0x0288, 0, TOUP, -218}, {0x0289, 0, TOUP, -69}, {0x028A, 1, TOUP, -217}, {0x028C, 0, TOUP, -71}, {0x0292, 0, TOUP, -219},
    {0x029D, 0, TOUP, 42261}, {0x029E, 0, TOUP, 42258}, {0x0345, 0, TOUP, 84}, {0x0370, 3, TO1, EVENCAP}, {0x0376, 1, TO1, EVENCAP},
    {0x037B, 2, TOUP, 130}, {0x037F, 0, TOLO, 116}, {0x0386, 0, TOLO, 38}, {0x0388, 2, TOLO, 37}, {0x038C, 0, TOLO, 64},
    {0x038E, 1, TOLO, 63}, {0x0391, 16, TOLO, 32}, {0x03A3, 8, TOLO, 32}, {0x03AC, 0, TOUP, -38}, {0x03AD, 2, TOUP, -37},
    {0x03B1, 16, TOUP, -32}, {0x03C2, 0, TOUP, -31}, {0x03C3, 8, TOUP, -32}, {0x03CC, 0, TOUP, -64}, {0x03CD, 1, TOUP, -63},
    {0x03CF, 0, TOLO, 8}, {0x03D0, 0, TOUP, -62}, {0x03D1, 0, TOUP, -57}, {0x03D5, 0, TOUP, -47}, {0x03D6, 0, TOUP, -54},
    {0x03D7, 0, TOUP, -8}, {0x03D8, 23, TO1, EVENCAP}, {0x03F0, 0, TOUP, -86}, {0x03F1, 0, TOUP, -80}, {0x03F2, 0, TOUP, 7},
    {0x03F3, 0, TOUP, -116}, {0x03F4, 0, TOLO, -60}, {0x03F5, 0, TOUP, -96}, {0x03F7, 1, TO1, ODDCAP}, {0x03F9, 0, TOLO, -7},
    {0x03FA, 1, TO1, EVENCAP}, {0x03FD, 2, TOLO, -130}, {0x0400, 15, TOLO, 80}, {0x0410, 31, TOLO, 32}, {0x0430, 31, TOUP, -32},
    {0x0450, 15, TOUP, -80}, {0x0460, 33, TO1, EVENCAP}, {0x048A, 53, TO1, EVENCAP}, {0x04C0, 0, TOLO, 15}, {0x04C1, 13, TO1, ODDCAP},
    {0x04CF, 0, TOUP, -15}, {0x04D0, 95, TO1, EVENCAP}, {0x0531, 37, TOLO, 48}, {0x0561, 37, TOUP, -48}, {0x10A0, 37, TOLO, 7264},
    {0x10C7, 0, TOLO, 7264}, {0x10CD, 0, TOLO, 7264}, {0x13A0, 79, TOLO, 38864}, {0x13F0, 5, TOLO, 8}, {0x13F8, 5, TOUP, -8},
    {0x1C80, 0, TOUP, -6254}, {0x1C81, 0, TOUP, -6253}, {0x1C82, 0, TOUP, -6244}, {0x1C83, 1, TOUP, -6242}, {0x1C85, 0, TOUP, -6243},
    {0x1C86, 0, TOUP, -6236}, {0x1C87, 0, TOUP, -6181}, {0x1C88, 0, TOUP, 35266}, {0x1D79, 0, TOUP, 35332}, {0x1D7D, 0, TOUP, 3814},
    {0x1E00, 149, TO1, EVENCAP}, {0x1E9B, 0, TOUP, -59}, {0x1E9E, 0, TOLO, -7615}, {0x1EA0, 95, TO1, EVENCAP}, {0x1F00, 7, TOUP, 8},
    {0x1F08, 7, TOLO, -8}, {0x1F10, 5, TOUP, 8}, {0x1F18, 5, TOLO, -8}, {0x1F20, 7, TOUP, 8}, {0x1F28, 7, TOLO, -8},
    {0x1F30, 7, TOUP, 8}, {0x1F38, 7, TOLO, -8}, {0x1F40, 5, TOUP, 8}, {0x1F48, 5, TOLO, -8}, {0x1F51, 0, TOUP, 8},
    {0x1F53, 0, TOUP, 8}, {0x1F55, 0, TOUP, 8}, {0x1F57, 0, TOUP, 8}, {0x1F59, 0, TOLO, -8}, {0x1F5B, 0, TOLO, -8},
    {0x1F5D, 0, TOLO, -8}, {0x1F5F, 0, TOLO, -8}, {0x1F60, 7, TOUP, 8}, {0x1F68, 7, TOLO, -8}, {0x1F70, 1, TOUP, 74},
    {0x1F72, 3, TOUP, 86}, {0x1F76, 1, TOUP, 100}, {0x1F78, 1, TOUP, 128}, {0x1F7A, 1, TOUP, 112}, {0x1F7C, 1, TOUP, 126},
    {0x1F80, 7, TOUP, 8}, {0x1F88, 7, TOLO, -8}, {0x1F90, 7, TOUP, 8}, {0x1F98, 7, TOLO, -8}, {0x1FA0, 7, TOUP, 8},
    {0x1FA8, 7, TOLO, -8}, {0x1FB0, 1, TOUP, 8}, {0x1FB3, 0, TOUP, 9}, {0x1FB8, 1, TOLO, -8}, {0x1FBA, 1, TOLO, -74},
    {0x1FBC, 0, TOLO, -9}, {0x1FBE, 0, TOUP, -7205}, {0x1FC3, 0, TOUP, 9}, {0x1FC8, 3, TOLO, -86}, {0x1FCC, 0, TOLO, -9},
    {0x1FD0, 1, TOUP, 8}, {0x1FD8, 1, TOLO, -8}, {0x1FDA, 1, TOLO, -100}, {0x1FE0, 1, TOUP, 8}, {0x1FE5, 0, TOUP, 7},
    {0x1FE8, 1, TOLO, -8}, {0x1FEA, 1, TOLO, -112}, {0x1FEC, 0, TOLO, -7}, {0x1FF3, 0, TOUP, 9}, {0x1FF8, 1, TOLO, -128},
    {0x1FFA, 1, TOLO, -126}, {0x1FFC, 0, TOLO, -9}, {0x2126, 0, TOLO, -7517}, {0x212A, 0, TOLO, -8383}, {0x212B, 0, TOLO, -8262},
    {0x2132, 0, TOLO, 28}, {0x214E, 0, TOUP, -28}, {0x2160, 15, TOLO, 16}, {0x2170, 15, TOUP, -16}, {0x2183, 1, TO1, ODDCAP},
    {0x24B6, 25, TOLO, 26}, {0x24D0, 25, TOUP, -26}, {0x2C00, 46, TOLO, 48}, {0x2C30, 46, TOUP, -48}, {0x2C60, 1, TO1, EVENCAP},
    {0x2C62, 0, TOLO, -10743}, {0x2C63, 0, TOLO, -3814}, {0x2C64, 0, TOLO, -10727}, {0x2C65, 0, TOUP, -10795}, {0x2C66, 0, TOUP, -10792},
    {0x2C67, 5, TO1, ODDCAP}, {0x2C6D, 0, TOLO, -10780}, {0x2C6E, 0, TOLO, -10749}, {0x2C6F, 0, TOLO, -10783}, {0x2C70, 0, TOLO, -10782},
    {0x2C72, 1, TO1, EVENCAP}, {0x2C75, 1, TO1, ODDCAP}, {0x2C7E, 1, TOLO, -10815}, {0x2C80, 99, TO1, EVENCAP}, {0x2CEB, 3, TO1, ODDCAP},
    {0x2CF2, 1, TO1, EVENCAP}, {0x2D00, 37, TOUP, -7264}, {0x2D27, 0, TOUP, -7264}, {0x2D2D, 0, TOUP, -7264}, {0xA640, 45, TO1, EVENCAP},
    {0xA680, 27, TO1, EVENCAP}, {0xA722, 13, TO1, EVENCAP}, {0xA732, 61, TO1, EVENCAP}, {0xA779, 3, TO1, ODDCAP}, {0xA77D, 0, TOLO, -35332},
    {0xA77E, 9, TO1, EVENCAP}, {0xA78B, 1, TO1, ODDCAP}, {0xA78D, 0, TOLO, -42280}, {0xA790, 3, TO1, EVENCAP}, {0xA796, 19, TO1, EVENCAP},
    {0xA7AA, 0, TOLO, -42308}, {0xA7AB, 0, TOLO, -42319}, {0xA7AC, 0, TOLO, -42315}, {0xA7AD, 0, TOLO, -42305}, {0xA7AE, 0, TOLO, -42308},
    {0xA7B0, 0, TOLO, -42258}, {0xA7B1, 0, TOLO, -42282}, {0xA7B2, 0, TOLO, -42261}, {0xA7B3, 0, TOLO, 928}, {0xA7B4, 3, TO1, EVENCAP},
    {0xAB53, 0, TOUP, -928}, {0xAB70, 79, TOUP, -38864}, {0xFF21, 25, TOLO, 32}, {0xFF41, 25, TOUP, -32}, {0x10400, 39, TOLO, 40},
    {0x10428, 39, TOUP, -40}, {0x104B0, 35, TOLO, 40}, {0x104D8, 35, TOUP, -40}, {0x10C80, 50, TOLO, 64}, {0x10CC0, 50, TOUP, -64},
    {0x118A0, 31, TOLO, 32}, {0x118C0, 31, TOUP, -32}, {0x1E900, 33, TOLO, 34}, {0x1E922, 33, TOUP, -34},
};
static const struct caseconv_entry* bisearch(wint_t ucs, const struct caseconv_entry* table, int max)
{
    int min = 0;
    int mid;
    if(ucs < table[0].first || ucs > (table[max].first + table[max].diff)) return 0;
    while(max >= min)
    {
        mid = (min + max) / 2;
        if(ucs > (table[mid].first + table[mid].diff)) min = mid + 1;
        else if(ucs < table[mid].first) max = mid - 1;
        else return &table[mid];
    }
    return 0;
}
static wint_t toulower(wint_t c)
{
    const struct caseconv_entry* cce = bisearch(c, caseconv_table, sizeof(caseconv_table) / sizeof(*caseconv_table) - 1);
    if(cce) switch(cce->mode)
    {
    case TOLO: return c + cce->delta;
    case TOBOTH: return c + 1;
    case TO1:
        switch(cce->delta)
        {
        case EVENCAP:
            if(!(c & 1)) return c + 1;
            break;
        case ODDCAP:
            if(c & 1) return c + 1;
            break;
        default: break;
        }
    default: break;
    }
    return c;
}
static wint_t touupper(wint_t c)
{
    const struct caseconv_entry* cce = bisearch(c, caseconv_table, sizeof(caseconv_table) / sizeof(*caseconv_table) - 1);
    if(cce) switch(cce->mode)
    {
    case TOUP: return c + cce->delta;
    case TOBOTH: return c - 1;
    case TO1:
        switch(cce->delta)
        {
        case EVENCAP:
            if(c & 1) return c - 1;
            break;
        case ODDCAP:
            if(!(c & 1)) return c - 1;
            break;
        default: break;
        }
    default: break;
    }
    return c;
}
wint_t towctrans_l(wint_t c, wctrans_t w, struct __locale_t* locale)
{
    wint_t u = _jp2uc_l(c, locale);
    wint_t res;
    if(w == 1) res = toulower(u);
    else if(w == 2) res = touupper(u);
    else { return c; }
    if(res != u) return _uc2jp_l(res, locale);
    else return c;
}
wint_t _towctrans_r(struct _reent* r, wint_t c, wctrans_t w) { if(w == 1 || w == 2) return towctrans_l(c, w, 0); else { return c; } }
wint_t towctrans(wint_t c, wctrans_t w) { return _towctrans_r(_impure_ptr, c, w); }
wint_t towlower(wint_t c) { return c < 0x00ff ? (wint_t)(__extension__({ __typeof__((int)c) __x = ((int)c); (((((_ctype_) + sizeof(""[__x]))[(int)(__x)]) & (01 | 02)) == 01) ? (int)__x - 'A' + 'a' : (int)__x; })) : c; }
wint_t towupper(wint_t c) { return c < 0x00ff ? (wint_t)(__extension__({ __typeof__((int)c) __x = ((int)c); (((((_ctype_) + sizeof(""[__x]))[(int)(__x)]) & (01 | 02)) == 02) ? (int)__x - 'a' + 'A' : (int)__x; })) : c; }
wint_t towlower_l(wint_t c, struct __locale_t* locale) { return towlower(c); }
wint_t towupper_l(wint_t c, struct __locale_t* locale) { return towupper(c); }