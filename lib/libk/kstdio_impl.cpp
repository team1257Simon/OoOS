#include "ext/dynamic_streambuf.hpp"
#include "string"
#include "stdarg.h"
constexpr const char errstr[] = "[SPECIFIER ERROR]";
static int stderr_fd_placeholder = 1;
static int stdout_stdin_placeholder = 0;
extern std::basic_streambuf<char>* get_kstio_stream();
size_t __arg_insert_ptr(void* ptr, std::basic_streambuf<char>* stream) { std::string str = std::to_string(ptr); return stream->sputn(str.c_str(), str.size()); }
template<std::floating_point FT> std::string fcvtg(FT ft, size_t ndigit);
template<std::floating_point FT> std::string fcvtg(FT ft, size_t ndigit, std::ext::hex_t);
template<> std::string fcvtg(float f, size_t ndigit) { return std::ext::fcvt(f, ndigit); }
template<> std::string fcvtg(double d, size_t ndigit) { return std::ext::fcvtd(d, ndigit); }
template<> std::string fcvtg(long double ld, size_t ndigit) { return std::ext::fcvtl(ld, ndigit); }
template<> std::string fcvtg(float f, size_t ndigit, std::ext::hex_t) { return std::ext::fcvth(f, ndigit); }
template<> std::string fcvtg(double d, size_t ndigit, std::ext::hex_t) { return std::ext::fcvthd(d, ndigit); }
template<> std::string fcvtg(long double ld, size_t ndigit, std::ext::hex_t) { return std::ext::fcvthl(ld, ndigit); }
template<std::integral IT>
size_t __arg_insert_dec(IT i, std::basic_streambuf<char>* stream, size_t minwid, bool zeropad, bool left, bool sign)
{
    std::string str = std::to_string(i);
    if(i > 0 && sign) str.insert(str.begin(), '+');
    if(str.size() < minwid) 
    { 
        size_t diff = std::min(SIZE_MAX / 4, static_cast<size_t>(minwid - str.size()));
        if(left) str.append(std::string(diff, ' ')); 
        else str.insert(str.begin() + (i < 0 || sign ? 1 : 0), std::string(diff, zeropad ? '0' : ' '));
    }
    return stream->sputn(str.c_str(), str.size());
}
template<std::integral IT>
size_t __arg_insert_hex(IT i, std::basic_streambuf<char>* stream, size_t minwid, bool zeropad, bool left, bool caps, bool pref)
{
    std::string str = std::to_string(i, std::ext::hex);
    if(!pref) str.erase(str.begin(), str.begin() + 2);
    if(str.size() < minwid) 
    { 
        size_t diff = std::min(SIZE_MAX / 4, static_cast<size_t>(minwid - str.size()));
        if(left) str.append(std::string(diff, ' ')); 
        else str.insert(str.begin() + (pref && zeropad ? 2 : 0), std::string(diff, zeropad ? '0' : ' '));
    }
    if(caps) return stream->sputn(str.c_str(), str.size());
    std::string lower = std::ext::to_lower(str);
    return stream->sputn(lower.c_str(), lower.size());
}
template<std::floating_point FT>
size_t __arg_insert_fp(FT f, std::basic_streambuf<char>* stream, size_t minwid, unsigned int precision, bool zeropad, bool left, bool sign)
{
    std::string str = fcvtg(f, precision);
    if(f > 0 && sign) str.insert(str.begin(), '+');
     if(str.size() < minwid) 
    { 
        size_t diff = std::min(SIZE_MAX / 4, static_cast<size_t>(minwid - str.size()));
        if(left) str.append(std::string(diff, ' ')); 
        else str.insert(str.begin() + (f < 0 || sign ? 1 : 0), std::string(diff, zeropad ? '0' : ' '));
    }
    return stream->sputn(str.c_str(), str.size());
}
template<std::floating_point FT>
size_t __arg_insert_fpx(FT f, std::basic_streambuf<char>* stream, size_t minwid, unsigned int precision, bool zeropad, bool left, bool caps, bool pref)
{
    std::string str = fcvtg(f, precision, std::ext::hex);
    if(pref) str.insert(str.begin(), std::move(std::string("0x")));
    if(str.size() < minwid) 
    { 
        size_t diff = std::min(SIZE_MAX / 4, static_cast<size_t>(minwid - str.size()));
        if(left) str.append(std::string(diff, ' ')); 
        else str.insert(str.begin() + (pref && zeropad ? 2 : 0), std::string(diff, zeropad ? '0' : ' '));
    }
    if(caps) return stream->sputn(str.c_str(), str.size());
    std::string lower = std::ext::to_lower(str);
    return stream->sputn(lower.c_str(), lower.size());
}
size_t __kvfprintf_impl(std::basic_streambuf<char>* stream, const char* fmt, va_list args)
{
    size_t n = std::strlen(fmt);
    typedef const char* cstr;
    size_t cnt = 0;
    cstr c, d, end;
    for(c = fmt, d = std::find(fmt, n, '%'), end = fmt + n; d && d < end; c = d + 1, d = std::find(c, n, '%'))
    {
        cnt += stream->sputn(c, static_cast<std::streamsize>(d - c));
        char spec               = d[1];
        bool zeropad            = false;
        bool left               = false;
        bool alt                = false;
        bool sign               = false;
        bool caps               = false;
        bool dot                = false;
        size_t minwid           = 0;
        int lenarg              = 0;
        unsigned int precision  = 1;
        bool have_prec          = false;
        int widarg              = 0;
        bool finish             = false;
        char* tmpptr            = nullptr;
        int tmpint              = 0;
        for(cstr e = d + 1; e < end && !finish; e++) // <ObligatoryReference>E</ObligatoryReference>
        {
            switch(spec)
            {
            case '%':
                stream->sputc('%');
                cnt++;
                finish = true;
                break;;
            case '.':
                dot = true;
                break;
            case '+':
                sign = true;
                break;
            case '#':
                alt = true;
                break;
            case '-':
                left = true;
                break;
            case '*':
                widarg = va_arg(args, int);
            case '0':
                if(!dot && !zeropad) { zeropad = true; break; }
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                widarg = spec - '0';
                if(dot)
                {
                    precision   = widarg > 0 ? widarg : precision;
                    have_prec   = true;
                    dot         = false;
                }
                else { if(widarg < 0) { left = true; widarg *= -1; } minwid = widarg; }
                break;
            case 'c':
                stream->sputc(va_arg(args, int));
                cnt++;
                finish = true;
                break;
            case 's':
                tmpptr          = va_arg(args, char*);
                if(!have_prec)
                    precision   = std::strlen(tmpptr);
                cnt             += stream->sputn(tmpptr, precision);
                finish          = true;
                break;
            case 'h':
                lenarg--;
                break;
            case 'L':
                lenarg++;
            case 't':
            case 'z':
            case 'j':
            case 'l':
                lenarg++;
                break;
            case 'i':
            case 'd':
                if(lenarg > 1) cnt          += __arg_insert_dec(va_arg(args, long long), stream, minwid, zeropad, left, sign);
                else if(lenarg == 1) cnt    += __arg_insert_dec(va_arg(args, long), stream, minwid, zeropad, left, sign);
                else if(lenarg == 0) cnt    += __arg_insert_dec(va_arg(args, int), stream, minwid, zeropad, left, sign);
                else
                {
                    tmpint                  = va_arg(args, int);
                    if(lenarg == -1) cnt    += __arg_insert_dec(static_cast<short>(tmpint), stream, minwid, zeropad, left, sign);
                    else cnt                += __arg_insert_dec(static_cast<signed char>(tmpint), stream, minwid, zeropad, left, sign);
                }
                finish = true;
                break;
            case 'u':
                if(lenarg > 1) cnt          += __arg_insert_dec(va_arg(args, unsigned long long), stream, minwid, zeropad, left, sign);
                else if(lenarg == 1) cnt    += __arg_insert_dec(va_arg(args, unsigned long), stream, minwid, zeropad, left, sign);
                else if(lenarg == 0) cnt    += __arg_insert_dec(va_arg(args, unsigned int), stream, minwid, zeropad, left, sign);
                else
                {
                    tmpint                  = va_arg(args, int);
                    if(lenarg == -1) cnt    += __arg_insert_dec(static_cast<unsigned short>(tmpint), stream, minwid, zeropad, left, sign);
                    else cnt                += __arg_insert_dec(static_cast<unsigned char>(tmpint), stream, minwid, zeropad, left, sign);
                }
                finish = true;
                break;
            case 'X': 
                caps = true;
            case 'x':
                if(lenarg > 1) cnt          += __arg_insert_hex(va_arg(args, unsigned long long), stream, minwid, zeropad, left, caps, alt);
                else if(lenarg == 1) cnt    += __arg_insert_hex(va_arg(args, unsigned long), stream, minwid, zeropad, left, caps, alt);
                else if(lenarg == 0) cnt    += __arg_insert_hex(va_arg(args, unsigned int), stream, minwid, zeropad, left, caps, alt);
                else 
                {
                    tmpint                  = va_arg(args, int);
                    if(lenarg == -1) cnt    += __arg_insert_hex(static_cast<unsigned short>(tmpint), stream, minwid, zeropad, left, sign, alt);
                    else cnt                += __arg_insert_hex(static_cast<unsigned char>(tmpint), stream, minwid, zeropad, left, sign, alt);
                }
                finish = true;
                break;
            case 'f':
            case 'F':
                if(lenarg > 1) cnt  += __arg_insert_fp(va_arg(args, long double), stream, minwid, have_prec ? precision : 6, zeropad, left, sign);
                else cnt            += __arg_insert_fp(va_arg(args, double), stream, minwid, have_prec ? precision : 6, zeropad, left, sign);
                finish = true;
                break;
            case 'A':
                caps = true;
            case 'a':
                if(lenarg > 1) cnt  += __arg_insert_fpx(va_arg(args, long double), stream, minwid, have_prec ? precision : 6, zeropad, left, caps, !alt);
                else cnt            += __arg_insert_fpx(va_arg(args, double), stream, minwid, have_prec ? precision : 6, zeropad, left, caps, !alt);
                finish = true;
                break;
            case 'n':
                *(va_arg(args, size_t*)) = cnt;
                finish = true;
                break;
            case 'p':
                cnt     += __arg_insert_ptr(va_arg(args, void*), stream);
                finish  = true;
                break;
            default:
                cnt     += stream->sputn(errstr, sizeof(errstr));
                finish  = true;
                break;
            }
        }
    }
    stream->pubsync();
    return cnt;
}
typedef int FILE;
extern "C"
{
    attribute(weak) FILE* stdin     = std::addressof(stdout_stdin_placeholder);
    attribute(weak) FILE* stdout    = std::addressof(stdout_stdin_placeholder);
    attribute(weak) FILE* stderr    = std::addressof(stderr_fd_placeholder);
    size_t kvfprintf(FILE* fd, const char* fmt, va_list args)
    {
        std::basic_streambuf<char>* stream = get_kstio_stream();
        if(*fd) stream->sputn("[!]", 3);
        return __kvfprintf_impl(stream, fmt, args);
    }
    size_t kvsnprintf(char* restrict buffer, size_t n, const char* restrict fmt, va_list args)
    {
        std::ext::dynamic_streambuf<char> db(n);
        size_t result = __kvfprintf_impl(std::addressof(db), fmt, args);
        size_t actual = std::min(n, result);
        array_copy(buffer, db.data(), actual);
        return actual;
    }
    size_t kvsprintf(char* restrict buffer, const char* restrict fmt, va_list args)
    {
        std::ext::dynamic_streambuf<char> db;
        size_t result = __kvfprintf_impl(std::addressof(db), fmt, args);
        array_copy(buffer, db.data(), result);
        return result;
    }
    size_t kvasprintf(char** restrict strp, const char* restrict fmt, va_list args)
    {
        std::ext::dynamic_streambuf<char> db;
        size_t result   = __kvfprintf_impl(std::addressof(db), fmt, args);
        *strp           = std::allocator<char>().allocate(result);
        array_copy(*strp, db.data(), result);
        return result;
    }
}