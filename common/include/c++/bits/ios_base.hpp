#ifndef __IOS_BASE_H
#define __IOS_BASE_H
#define NZ(x) ((x) != 0)
#include "kernel/libk_decls.h"
namespace std
{
     namespace ios_base
    {
        struct openmode
        {
            bool app       : 1;
            bool binary    : 1;
            bool in        : 1;
            bool out       : 1;
            bool trunc     : 1;
            bool ate       : 1;
            bool           : 2;
            constexpr openmode() noexcept : 
                app     { 0 }, 
                binary  { 0 }, 
                in      { 0 }, 
                out     { 0 }, 
                trunc   { 0 }, 
                ate     { 0 } 
                        {}
            constexpr openmode(bool a, bool b, bool i, bool o, bool t, bool e) noexcept : 
                app     { a }, 
                binary  { b }, 
                in      { i }, 
                out     { o }, 
                trunc   { t }, 
                ate     { e } 
                        {}
            constexpr openmode(uint8_t bits) noexcept : 
                app     { NZ(bits & 0x01) }, 
                binary  { NZ(bits & 0x02) }, 
                in      { NZ(bits & 0x04) }, 
                out     { NZ(bits & 0x08) }, 
                trunc   { NZ(bits & 0x10) }, 
                ate     { NZ(bits & 0x20) }
            {}
            constexpr openmode(openmode const&) = default;
            constexpr operator uint8_t() const noexcept { return bit_cast<uint8_t>(*this); }
            constexpr operator bool() const noexcept { return NZ(uint8_t(*this)); }
            constexpr openmode& operator=(openmode const&) = default;
            constexpr openmode operator|(openmode const& that) const noexcept { return openmode{ uint8_t(*this) | uint8_t(that) }; }
            constexpr openmode operator&(openmode const& that) const noexcept { return openmode{ uint8_t(*this) & uint8_t(that) }; }
            constexpr openmode& operator|=(openmode const& that) noexcept { return (*this = (*this | that)); }
            constexpr openmode& operator&=(openmode const& that) noexcept { return (*this = (*this & that)); }
        } __pack;
        constexpr openmode app      { 0x01 };
        constexpr openmode binary   { 0x02 };
        constexpr openmode in       { 0x04 };
        constexpr openmode out      { 0x08 };
        constexpr openmode trunc    { 0x10 };
        constexpr openmode ate      { 0x10 };
        struct iostate
        {
            bool badbit  : 1;
            bool failbit : 1;
            uint8_t      : 5;
            bool eofbit  : 1;
            constexpr bool good() const noexcept { return !badbit && !failbit && !eofbit; }
            constexpr bool fail() const noexcept { return failbit; }
            constexpr bool bad() const noexcept { return badbit; }
            constexpr bool eof() const noexcept { return eofbit; }
            constexpr iostate() : 
                badbit  { 0 }, 
                failbit { 0 }, 
                eofbit  { 0 } 
                        {}
            constexpr iostate(bool bad, bool fail, bool eof) noexcept : 
                badbit  {  bad  }, 
                failbit {  fail }, 
                eofbit  {  eof  } 
                        {}
            constexpr iostate(uint8_t c) noexcept : 
                badbit  { NZ(c & 0x01) }, 
                failbit { NZ(c & 0x02) },
                eofbit  { NZ(c & 0x80) } 
                        {}
            constexpr iostate(iostate const&) noexcept = default;
            constexpr iostate& operator=(iostate const&) noexcept = default;
            constexpr operator uint8_t() const noexcept { return bit_cast<uint8_t>(*this); }
            constexpr operator bool() const noexcept { return good();}
            constexpr bool operator!() const noexcept { return !good();}
            constexpr iostate operator|(iostate const& that) const noexcept { return iostate{ uint8_t(*this) | uint8_t(that) }; }
            constexpr iostate operator&(iostate const& that) const noexcept { return iostate{ uint8_t(*this) & uint8_t(that) }; }
            constexpr iostate& operator|=(iostate const& that) noexcept { return (*this = (*this | that)); }
            constexpr iostate& operator&=(iostate const& that) noexcept { return (*this = (*this & that)); }
        } __pack;
        constexpr iostate goodbit   {      };
        constexpr iostate badbit    { 0x01 };
        constexpr iostate failbit   { 0x02 };
        constexpr iostate eofbit    { 0x80 };
        struct fmtflags
        {
            bool dec        : 1;
            bool oct        : 1;
            bool hex        : 1;
            bool left       : 1;
            bool right      : 1;
            bool internal   : 1;
            bool scientific : 1;
            bool fixed      : 1;
            bool boolalpha  : 1;
            bool showbase   : 1;
            bool showpoint  : 1;
            bool showpos	: 1;
            bool skipws  	: 1;
            bool unitbuf  	: 1;
            bool uppercase  : 1;
            bool 			: 1;
            constexpr fmtflags(uint16_t bits) noexcept : 
                dec         { NZ(bits & 0x0001) },
                oct         { NZ(bits & 0x0002) },
                hex         { NZ(bits & 0x0004) },
                left        { NZ(bits & 0x0008) },
                right       { NZ(bits & 0x0010) },
                internal    { NZ(bits & 0x0020) },
                scientific  { NZ(bits & 0x0040) },
                fixed       { NZ(bits & 0x0080) },
                boolalpha   { NZ(bits & 0x0100) },
                showbase    { NZ(bits & 0x0200) },
                showpoint   { NZ(bits & 0x0400) },
                showpos     { NZ(bits & 0x0800) },
                skipws      { NZ(bits & 0x1000) },
                unitbuf     { NZ(bits & 0x2000) },
                uppercase   { NZ(bits & 0x4000) }
                            {}
            constexpr fmtflags() noexcept : fmtflags{0} {}
            constexpr fmtflags(fmtflags const&) noexcept = default;
            constexpr fmtflags& operator=(fmtflags const&) noexcept = default;
            constexpr operator uint16_t() const noexcept { return bit_cast<uint16_t>(*this); }
            constexpr fmtflags operator|(fmtflags const& that) const noexcept { return fmtflags{ uint16_t(*this) | uint16_t(that) }; }
            constexpr fmtflags& operator|=(fmtflags const& that) noexcept { return *this = (*this | that); }
            constexpr fmtflags operator&(fmtflags const& that) const noexcept { return fmtflags{ uint16_t(*this) & uint16_t(that) }; }
            constexpr fmtflags& operator&=(fmtflags const& that) noexcept { return *this = (*this & that); }
            constexpr fmtflags basefield() const noexcept { return *this & fmtflags{ uint16_t(0b00000111) }; }
            constexpr fmtflags adjustfield() const noexcept { return *this & fmtflags{ uint16_t(0b00111000) }; }
            constexpr fmtflags floatfield() const noexcept { return *this & fmtflags{ uint16_t(0b11000000) }; }
        } __pack;
        constexpr fmtflags dec          { 0x0001 };
        constexpr fmtflags oct          { 0x0002 };
        constexpr fmtflags hex          { 0x0004 };
        constexpr fmtflags left         { 0x0008 };
        constexpr fmtflags right        { 0x0010 };
        constexpr fmtflags internal     { 0x0020 };
        constexpr fmtflags scientific   { 0x0040 };
        constexpr fmtflags fixed        { 0x0080 };
        constexpr fmtflags boolalpha    { 0x0100 };
        constexpr fmtflags showbase     { 0x0200 };
        constexpr fmtflags showpoint    { 0x0400 };
        constexpr fmtflags showpos      { 0x0800 };
        constexpr fmtflags skipws       { 0x1000 };
        constexpr fmtflags unitbuf      { 0x2000 };
        constexpr fmtflags uppercase    { 0x4000 };
        enum seekdir : int { beg = -1, end = 1, cur = 0 };
    }
    typedef struct state
    {
        ios_base::iostate iost;
        ios_base::fmtflags fmtflgs;
        ios_base::openmode opnmd;
    } __pack mbstate_t;
    constexpr bool operator==(state const& s1, state const& s2) noexcept { return uint16_t(s1.fmtflgs) == uint16_t(s2.fmtflgs) && uint8_t(s1.iost) == uint8_t(s2.iost) && uint8_t(s1.opnmd) == uint8_t(s2.opnmd); }
    typedef size_t streamsize;
}
#endif