typedef long unsigned int  size_t;
struct __locale_t;
typedef struct __locale_t*      locale_t;
extern int                      tolower_l(int __c, locale_t __l);
extern const char               _ctype_[];
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
void*                       memchr(const void*, int, size_t);
int                        strcasecmp(const char* s1, const char* s2)
{
    int d = 0;
    for(;;)
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
int                        strcasecmp_l(const char* s1, const char* s2, struct __locale_t* locale)
{
    int d = 0;
    for(;;)
    {
        const int c1 = tolower_l(*s1++, locale);
        const int c2 = tolower_l(*s2++, locale);
        if(((d = c1 - c2) != 0) || (c2 == '\0')) break;
    }
    return d;
}
static size_t               critical_factorization(const unsigned char* needle, size_t needle_len, size_t* period)
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
        a = __extension__({
            __typeof__(needle[j + k]) __x = (needle[j + k]);
            (((((_ctype_) + sizeof(""[__x]))[(int)(__x)]) & (01 | 02)) == 01) ? (int)__x - 'A' + 'a' : (int)__x;
        });
        b = __extension__({
            __typeof__(needle[(size_t)(max_suffix + k)]) __x = (needle[(size_t)(max_suffix + k)]);
            (((((_ctype_) + sizeof(""[__x]))[(int)(__x)]) & (01 | 02)) == 01) ? (int)__x - 'A' + 'a' : (int)__x;
        });
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
        a = __extension__({
            __typeof__(needle[j + k]) __x = (needle[j + k]);
            (((((_ctype_) + sizeof(""[__x]))[(int)(__x)]) & (01 | 02)) == 01) ? (int)__x - 'A' + 'a' : (int)__x;
        });
        b = __extension__({
            __typeof__(needle[max_suffix_rev + k]) __x = (needle[max_suffix_rev + k]);
            (((((_ctype_) + sizeof(""[__x]))[(int)(__x)]) & (01 | 02)) == 01) ? (int)__x - 'A' + 'a' : (int)__x;
        });
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
static char* two_way_short_needle(const unsigned char* haystack, size_t haystack_len, const unsigned char* needle, size_t needle_len)
{
    size_t i;
    size_t j;
    size_t period;
    size_t suffix;
    suffix = critical_factorization(needle, needle_len, &period);
    if(strncasecmp(needle, needle + period, suffix) == 0)
    {
        size_t memory = 0;
        j             = 0;
        while((!memchr((haystack) + (haystack_len), '\0', (j) + (needle_len) - (haystack_len)) && ((haystack_len) = (j) + (needle_len))))
        {
            i = ((suffix < memory) ? (memory) : (suffix));
            while(i < needle_len && (__extension__({
                                         __typeof__(needle[i]) __x = (needle[i]);
                                         (((((_ctype_) + sizeof(""[__x]))[(int)(__x)]) & (01 | 02)) == 01) ? (int)__x - 'A' + 'a' : (int)__x;
                                     }) == __extension__({
                                         __typeof__(haystack[i + j]) __x = (haystack[i + j]);
                                         (((((_ctype_) + sizeof(""[__x]))[(int)(__x)]) & (01 | 02)) == 01) ? (int)__x - 'A' + 'a' : (int)__x;
                                     })))
                ++i;
            if(needle_len <= i)
            {
                i = suffix - 1;
                while(memory < i + 1 && (__extension__({
                                             __typeof__(needle[i]) __x = (needle[i]);
                                             (((((_ctype_) + sizeof(""[__x]))[(int)(__x)]) & (01 | 02)) == 01) ? (int)__x - 'A' + 'a' : (int)__x;
                                         }) == __extension__({
                                             __typeof__(haystack[i + j]) __x = (haystack[i + j]);
                                             (((((_ctype_) + sizeof(""[__x]))[(int)(__x)]) & (01 | 02)) == 01) ? (int)__x - 'A' + 'a' : (int)__x;
                                         })))
                    --i;
                if(i + 1 < memory + 1) return (char*)(haystack + j);
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
        while((!memchr((haystack) + (haystack_len), '\0', (j) + (needle_len) - (haystack_len)) && ((haystack_len) = (j) + (needle_len))))
        {
            i = suffix;
            while(i < needle_len && (__extension__({
                                         __typeof__(needle[i]) __x = (needle[i]);
                                         (((((_ctype_) + sizeof(""[__x]))[(int)(__x)]) & (01 | 02)) == 01) ? (int)__x - 'A' + 'a' : (int)__x;
                                     }) == __extension__({
                                         __typeof__(haystack[i + j]) __x = (haystack[i + j]);
                                         (((((_ctype_) + sizeof(""[__x]))[(int)(__x)]) & (01 | 02)) == 01) ? (int)__x - 'A' + 'a' : (int)__x;
                                     })))
                ++i;
            if(needle_len <= i)
            {
                i = suffix - 1;
                while(i != (0xffffffffffffffffUL) &&
                      (__extension__({
                           __typeof__(needle[i]) __x = (needle[i]);
                           (((((_ctype_) + sizeof(""[__x]))[(int)(__x)]) & (01 | 02)) == 01) ? (int)__x - 'A' + 'a' : (int)__x;
                       }) == __extension__({
                           __typeof__(haystack[i + j]) __x = (haystack[i + j]);
                           (((((_ctype_) + sizeof(""[__x]))[(int)(__x)]) & (01 | 02)) == 01) ? (int)__x - 'A' + 'a' : (int)__x;
                       })))
                    --i;
                if(i == (0xffffffffffffffffUL)) return (char*)(haystack + j);
                j += period;
            }
            else
                j += i - suffix + 1;
        }
    }
    return ((void*)0);
}
static char* two_way_long_needle(const unsigned char* haystack, size_t haystack_len, const unsigned char* needle, size_t needle_len)
{
    size_t i;
    size_t j;
    size_t period;
    size_t suffix;
    size_t shift_table[1U << 8];
    suffix = critical_factorization(needle, needle_len, &period);
    for(i = 0; i < 1U << 8; i++) shift_table[i] = needle_len;
    for(i = 0; i < needle_len; i++)
        shift_table[__extension__({
            __typeof__(needle[i]) __x = (needle[i]);
            (((((_ctype_) + sizeof(""[__x]))[(int)(__x)]) & (01 | 02)) == 01) ? (int)__x - 'A' + 'a' : (int)__x;
        })] = needle_len - i - 1;
    if(strncasecmp(needle, needle + period, suffix) == 0)
    {
        size_t memory = 0;
        size_t shift;
        j = 0;
        while((!memchr((haystack) + (haystack_len), '\0', (j) + (needle_len) - (haystack_len)) && ((haystack_len) = (j) + (needle_len))))
        {
            shift = shift_table[__extension__({
                __typeof__(haystack[j + needle_len - 1]) __x = (haystack[j + needle_len - 1]);
                (((((_ctype_) + sizeof(""[__x]))[(int)(__x)]) & (01 | 02)) == 01) ? (int)__x - 'A' + 'a' : (int)__x;
            })];
            if(0 < shift)
            {
                if(memory && shift < period) { shift = needle_len - period; }
                memory = 0;
                j += shift;
                continue;
            }
            i = ((suffix < memory) ? (memory) : (suffix));
            while(i < needle_len - 1 && (__extension__({
                                             __typeof__(needle[i]) __x = (needle[i]);
                                             (((((_ctype_) + sizeof(""[__x]))[(int)(__x)]) & (01 | 02)) == 01) ? (int)__x - 'A' + 'a' : (int)__x;
                                         }) == __extension__({
                                             __typeof__(haystack[i + j]) __x = (haystack[i + j]);
                                             (((((_ctype_) + sizeof(""[__x]))[(int)(__x)]) & (01 | 02)) == 01) ? (int)__x - 'A' + 'a' : (int)__x;
                                         })))
                ++i;
            if(needle_len - 1 <= i)
            {
                i = suffix - 1;
                while(memory < i + 1 && (__extension__({
                                             __typeof__(needle[i]) __x = (needle[i]);
                                             (((((_ctype_) + sizeof(""[__x]))[(int)(__x)]) & (01 | 02)) == 01) ? (int)__x - 'A' + 'a' : (int)__x;
                                         }) == __extension__({
                                             __typeof__(haystack[i + j]) __x = (haystack[i + j]);
                                             (((((_ctype_) + sizeof(""[__x]))[(int)(__x)]) & (01 | 02)) == 01) ? (int)__x - 'A' + 'a' : (int)__x;
                                         })))
                    --i;
                if(i + 1 < memory + 1) return (char*)(haystack + j);
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
        while((!memchr((haystack) + (haystack_len), '\0', (j) + (needle_len) - (haystack_len)) && ((haystack_len) = (j) + (needle_len))))
        {
            shift = shift_table[__extension__({
                __typeof__(haystack[j + needle_len - 1]) __x = (haystack[j + needle_len - 1]);
                (((((_ctype_) + sizeof(""[__x]))[(int)(__x)]) & (01 | 02)) == 01) ? (int)__x - 'A' + 'a' : (int)__x;
            })];
            if(0 < shift)
            {
                j += shift;
                continue;
            }
            i = suffix;
            while(i < needle_len - 1 && (__extension__({
                                             __typeof__(needle[i]) __x = (needle[i]);
                                             (((((_ctype_) + sizeof(""[__x]))[(int)(__x)]) & (01 | 02)) == 01) ? (int)__x - 'A' + 'a' : (int)__x;
                                         }) == __extension__({
                                             __typeof__(haystack[i + j]) __x = (haystack[i + j]);
                                             (((((_ctype_) + sizeof(""[__x]))[(int)(__x)]) & (01 | 02)) == 01) ? (int)__x - 'A' + 'a' : (int)__x;
                                         })))
                ++i;
            if(needle_len - 1 <= i)
            {
                i = suffix - 1;
                while(i != (0xffffffffffffffffUL) &&
                      (__extension__({
                           __typeof__(needle[i]) __x = (needle[i]);
                           (((((_ctype_) + sizeof(""[__x]))[(int)(__x)]) & (01 | 02)) == 01) ? (int)__x - 'A' + 'a' : (int)__x;
                       }) == __extension__({
                           __typeof__(haystack[i + j]) __x = (haystack[i + j]);
                           (((((_ctype_) + sizeof(""[__x]))[(int)(__x)]) & (01 | 02)) == 01) ? (int)__x - 'A' + 'a' : (int)__x;
                       })))
                    --i;
                if(i == (0xffffffffffffffffUL)) return (char*)(haystack + j);
                j += period;
            }
            else
                j += i - suffix + 1;
        }
    }
    return ((void*)0);
}
char* strcasestr(const char* s, const char* find)
{
    const char* haystack = s;
    const char* needle   = find;
    size_t      needle_len;
    size_t      haystack_len;
    int         ok = 1;
    while(*haystack && *needle)
        ok &= (__extension__({
                   __typeof__((unsigned char)*haystack++) __x = ((unsigned char)*haystack++);
                   (((((_ctype_) + sizeof(""[__x]))[(int)(__x)]) & (01 | 02)) == 01) ? (int)__x - 'A' + 'a' : (int)__x;
               }) == __extension__({
                   __typeof__((unsigned char)*needle++) __x = ((unsigned char)*needle++);
                   (((((_ctype_) + sizeof(""[__x]))[(int)(__x)]) & (01 | 02)) == 01) ? (int)__x - 'A' + 'a' : (int)__x;
               }));
    if(*needle) return ((void*)0);
    if(ok) return (char*)s;
    needle_len   = needle - find;
    haystack     = s + 1;
    haystack_len = needle_len - 1;
    if(needle_len < 32U) return two_way_short_needle((const unsigned char*)haystack, haystack_len, (const unsigned char*)find, needle_len);
    return two_way_long_needle((const unsigned char*)haystack, haystack_len, (const unsigned char*)find, needle_len);
}
int                        strncasecmp_l(const char* s1, const char* s2, size_t n, struct __locale_t* locale)
{
    int d = 0;
    for(; n != 0; n--)
    {
        const int c1 = tolower_l(*s1++, locale);
        const int c2 = tolower_l(*s2++, locale);
        if(((d = c1 - c2) != 0) || (c2 == '\0')) break;
    }
    return d;
}
char*                      strlwr(char* s)
{
    unsigned char* ucs = (unsigned char*)s;
    for(; *ucs != '\0'; ucs++)
    {
        *ucs = __extension__({
            __typeof__(*ucs) __x = (*ucs);
            (((((_ctype_) + sizeof(""[__x]))[(int)(__x)]) & (01 | 02)) == 01) ? (int)__x - 'A' + 'a' : (int)__x;
        });
    }
    return s;
}
char*                      strupr(char* s)
{
    unsigned char* ucs = (unsigned char*)s;
    for(; *ucs != '\0'; ucs++)
    {
        *ucs = __extension__({
            __typeof__(*ucs) __x = (*ucs);
            (((((_ctype_) + sizeof(""[__x]))[(int)(__x)]) & (01 | 02)) == 02) ? (int)__x - 'a' + 'A' : (int)__x;
        });
    }
    return s;
}
int strverscmp(const char* l0, const char* r0)
{
    const unsigned char* l = (const void*)l0;
    const unsigned char* r = (const void*)r0;
    size_t               i, dp, j;
    int                  z = 1;
    for(dp = i = 0; l[i] == r[i]; i++)
    {
        int c = l[i];
        if(!c) return 0;
        if(!((((_ctype_) + sizeof(""[c]))[(int)(c)]) & 04))
            dp = i + 1, z = 1;
        else if(c != '0')
            z = 0;
    }
    if(l[dp] != '0' && r[dp] != '0')
    {
        for(j = i; ((((_ctype_) + sizeof(""[l[j]]))[(int)(l[j])]) & 04); j++)
            if(!((((_ctype_) + sizeof(""[r[j]]))[(int)(r[j])]) & 04)) return 1;
        if(((((_ctype_) + sizeof(""[r[j]]))[(int)(r[j])]) & 04)) return -1;
    }
    else if(z && dp < i && (((((_ctype_) + sizeof(""[l[i]]))[(int)(l[i])]) & 04) || ((((_ctype_) + sizeof(""[r[i]]))[(int)(r[i])]) & 04)))
    {
        return (unsigned char)(l[i] - '0') - (unsigned char)(r[i] - '0');
    }
    return l[i] - r[i];
}
size_t                     strxfrm_l(char* restrict s1, const char* restrict s2, size_t n, struct __locale_t* locale)
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
