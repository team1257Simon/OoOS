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
            constexpr openmode() noexcept : app{0}, binary{0}, in{0}, out{0}, trunc{0}, ate{0} {}
            constexpr openmode(bool a, bool b, bool i, bool o, bool t, bool e) noexcept : app{a}, binary{b}, in{i}, out{o}, trunc{t}, ate{e} {}
            constexpr openmode(uint8_t bits) noexcept : 
                app{NZ(bits & 0x01)}, 
                binary{NZ(bits & 0x02)}, 
                in{NZ(bits & 0x04)}, 
                out{NZ(bits & 0x08)}, 
                trunc{NZ(bits & 0x10)}, 
                ate{NZ(bits & 0x20)}
            {}
            constexpr openmode(openmode const&) = default;
            constexpr operator uint8_t() const noexcept { return bit_cast<uint8_t>(*this); }
            constexpr operator bool() const noexcept { return NZ(static_cast<uint8_t>(*this)); }
            constexpr openmode& operator=(openmode const&) = default;
            constexpr openmode& operator|=(openmode const& that) noexcept
            {
                this->app |= that.app;
                this->binary |= that.binary;
                this->in |= that.in;
                this->out |= that.out;
                this->trunc |= that.trunc;
                this->ate |= that.ate;
                return *this;
            }
            constexpr openmode operator|(openmode const& that) const noexcept
            {
                openmode r{*this};
                r |= that;
                return r;
            }
            constexpr openmode& operator&=(openmode const& that) noexcept
            {
                this->app &= that.app;
                this->binary &= that.binary;
                this->in &= that.in;
                this->out &= that.out;
                this->trunc &= that.trunc;
                this->ate &= that.ate;
                return *this;
            }
            constexpr openmode operator&(openmode const& that) const noexcept
            {
                openmode r{*this};
                r &= that;
                return r;
            }
        } __pack;
        constexpr openmode app{0x01};
        constexpr openmode binary{0x02};
        constexpr openmode in{0x04};
        constexpr openmode out{0x08};
        constexpr openmode trunc{0x10};
        constexpr openmode ate{0x10};
        struct iostate
        {
            bool badbit  : 1;
            bool failbit : 1;
            uint8_t      : 5;
            bool eofbit  : 1;
            constexpr bool good() const noexcept {return !badbit && !failbit && !eofbit; }
            constexpr bool fail() const noexcept {return failbit;}
            constexpr bool bad() const noexcept {return badbit;}
            constexpr bool eof() const noexcept {return eofbit;}
            constexpr iostate() : badbit{0}, failbit{0}, eofbit{0} {}
            constexpr iostate(bool bad, bool fail, bool eof) noexcept : badbit{bad}, failbit{fail}, eofbit{eof} {}
            constexpr iostate(unsigned char c) noexcept : badbit{NZ(c & 0x01)}, failbit(NZ(c & 0x02)), eofbit(NZ(c & 0x80)) {}
            constexpr iostate(iostate const&) = default;
            constexpr iostate& operator=(iostate const&) = default;
            constexpr operator unsigned char() const { return bit_cast<unsigned char>(*this); }
            constexpr operator bool() const noexcept { return good();}
            constexpr bool operator!() const noexcept { return !good();}
            constexpr iostate& operator|=(iostate const& that) noexcept
            {
                this->badbit |= that.badbit;
                this->failbit |= that.failbit;
                this->eofbit |= that.eofbit;
                return *this;
            }
            constexpr iostate operator|(iostate const& that) const noexcept
            {
                iostate r{*this};
                r |= that;
                return r;
            }
            constexpr iostate& operator&=(iostate const& that) noexcept
            {
                this->badbit &= that.badbit;
                this->failbit &= that.failbit;
                this->eofbit &= that.eofbit;
                return *this;
            }
            constexpr iostate operator&(iostate const& that) const noexcept
            {
                iostate r{*this};
                r &= that;
                return r;
            }
        } __pack;
        constexpr iostate goodbit{};
        constexpr iostate badbit{0x01};
        constexpr iostate failbit{0x02};
        constexpr iostate eofbit{0x80};
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
            constexpr fmtflags(uint16_t bits) : 
                dec{NZ(bits & 0x0001)},
                oct{NZ(bits & 0x0002)},
                hex{NZ(bits & 0x0004)},
                left{NZ(bits & 0x008)},
                right{NZ(bits & 0x0010)},
                internal{NZ(bits & 0x0020)},
                scientific{NZ(bits & 0x0040)},
                fixed{NZ(bits & 0x0080)},
                boolalpha{ NZ(bits & 0x0100)},
                showbase{NZ(bits & 0x0200)},
                showpoint{NZ(bits & 0x0400)},
                showpos{NZ(bits & 0x0800)},
                skipws{NZ(bits & 0x1000)},
                unitbuf{NZ(bits & 0x2000)},
                uppercase{NZ(bits & 0x4000)}{}
            constexpr fmtflags() : fmtflags{0} {}
            constexpr fmtflags(fmtflags const&) = default;
            constexpr fmtflags& operator=(fmtflags const&) = default;
            constexpr operator uint16_t() const { return *reinterpret_cast<const uint16_t*>(this); }
            constexpr fmtflags operator|(fmtflags const& that) const { return { static_cast<uint16_t>(static_cast<const uint16_t>(*this) | static_cast<const uint16_t>(that)) }; }
            constexpr fmtflags& operator|=(fmtflags const& that) { return *this = (*this | that); }
            constexpr fmtflags operator&(fmtflags const& that) const { return { static_cast<uint16_t>(static_cast<const uint16_t>(*this) | static_cast<const uint16_t>(that)) }; }
            constexpr fmtflags& operator&=(fmtflags const& that) { return *this = (*this & that); }
            constexpr fmtflags basefield() const { return *this & fmtflags{0b111}; }
            constexpr fmtflags adjustfield() const { return *this & fmtflags{uint16_t(0b111000)}; }
            constexpr fmtflags floatfield() const { return *this & fmtflags{uint16_t(0b11000000)}; }
        } __pack;
        constexpr fmtflags dec{0x0001};
        constexpr fmtflags oct{0x0002};
        constexpr fmtflags hex{0x0004};
        constexpr fmtflags left{0x008};
        constexpr fmtflags right{0x0010};
        constexpr fmtflags internal{0x0020};
        constexpr fmtflags scientific{0x0040};
        constexpr fmtflags fixed{0x0080};
        constexpr fmtflags boolalpha{0x0100};
        constexpr fmtflags showbase{0x0200};
        constexpr fmtflags showpoint{0x0400};
        constexpr fmtflags showpos{0x0800};
        constexpr fmtflags skipws{0x1000};
        constexpr fmtflags unitbuf{0x2000};
        constexpr fmtflags uppercase{0x4000};
        enum seekdir
        {
            beg,
            end,
            cur
        };
    }
    typedef struct state
    {
        ios_base::iostate iost;
        ios_base::fmtflags fmtflgs;
        ios_base::openmode opnmd;
    } __pack mbstate_t;
    constexpr bool operator==(state const& s1, state const& s2)
    {
        return s1.fmtflgs.operator unsigned short() == s2.fmtflgs.operator unsigned short()
        && s1.iost.operator unsigned char() == s2.iost.operator unsigned char()
        && s1.opnmd.operator unsigned char() == s2.opnmd.operator unsigned char();
    }
    typedef size_t streamsize;
}
#endif