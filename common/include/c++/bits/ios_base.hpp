#ifndef __IOS_BASE_H
#define __IOS_BASE_H
#define NZ(x) ((x) != 0)
#include <kernel/libk_decls.h>
namespace std
{
	struct __openmode
	{
		bool app		: 1;
		bool binary		: 1;
		bool in			: 1;
		bool out		: 1;
		bool trunc		: 1;
		bool ate		: 1;
		bool			: 2;
		constexpr __openmode() noexcept :
			app		{ 0 },
			binary	{ 0 },
			in		{ 0 },
			out		{ 0 },
			trunc	{ 0 },
			ate		{ 0 }
					{}
		constexpr __openmode(bool a, bool b, bool i, bool o, bool t, bool e) noexcept :
			app		{ a },
			binary	{ b },
			in		{ i },
			out		{ o },
			trunc	{ t },
			ate		{ e }
					{}
		constexpr __openmode(uint8_t bits) noexcept :
			app		{ NZ(bits & 0x01) },
			binary	{ NZ(bits & 0x02) },
			in		{ NZ(bits & 0x04) },
			out		{ NZ(bits & 0x08) },
			trunc	{ NZ(bits & 0x10) },
			ate		{ NZ(bits & 0x20) }
		{}
		constexpr __openmode(__openmode const&) = default;
		constexpr operator uint8_t() const noexcept { return bit_cast<uint8_t>(*this); }
		constexpr operator bool() const noexcept { return NZ(uint8_t(*this)); }
		constexpr __openmode& operator=(__openmode const&) = default;
		constexpr __openmode operator|(__openmode const& that) const noexcept { return __openmode{ uint8_t(uint8_t(*this) | uint8_t(that)) }; }
		constexpr __openmode operator&(__openmode const& that) const noexcept { return __openmode{ uint8_t(uint8_t(*this) & uint8_t(that)) }; }
		constexpr __openmode& operator|=(__openmode const& that) noexcept { return (*this = (*this | that)); }
		constexpr __openmode& operator&=(__openmode const& that) noexcept { return (*this = (*this & that)); }
	} __pack;
	struct __iostate
	{
		bool badbit		: 1;
		bool failbit	: 1;
		uint8_t			: 5;
		bool eofbit		: 1;
		constexpr bool good() const noexcept { return !badbit && !failbit && !eofbit; }
		constexpr bool fail() const noexcept { return failbit; }
		constexpr bool bad() const noexcept { return badbit; }
		constexpr bool eof() const noexcept { return eofbit; }
		constexpr __iostate() :
			badbit	{ 0 },
			failbit { 0 },
			eofbit	{ 0 }
					{}
		constexpr __iostate(bool bad, bool fail, bool eof) noexcept :
			badbit	{ bad },
			failbit { fail },
			eofbit	{ eof }
					{}
		constexpr __iostate(uint8_t c) noexcept :
			badbit	{ NZ(c & 0x01) },
			failbit { NZ(c & 0x02) },
			eofbit	{ NZ(c & 0x80) }
					{}
		constexpr __iostate(__iostate const&) noexcept = default;
		constexpr __iostate& operator=(__iostate const&) noexcept = default;
		constexpr operator uint8_t() const noexcept { return bit_cast<uint8_t>(*this); }
		constexpr operator bool() const noexcept { return good();}
		constexpr bool operator!() const noexcept { return !good();}
		constexpr __iostate operator|(__iostate const& that) const noexcept { return __iostate{ uint8_t(uint8_t(*this) | uint8_t(that)) }; }
		constexpr __iostate operator&(__iostate const& that) const noexcept { return __iostate{ uint8_t(uint8_t(*this) & uint8_t(that)) }; }
		constexpr __iostate& operator|=(__iostate const& that) noexcept { return (*this = (*this | that)); }
		constexpr __iostate& operator&=(__iostate const& that) noexcept { return (*this = (*this & that)); }
	} __pack;
	struct __fmtflags
	{
		bool dec		: 1;
		bool oct		: 1;
		bool hex		: 1;
		bool left		: 1;
		bool right		: 1;
		bool internal	: 1;
		bool scientific : 1;
		bool fixed		: 1;
		bool boolalpha	: 1;
		bool showbase	: 1;
		bool showpoint	: 1;
		bool showpos	: 1;
		bool skipws		: 1;
		bool unitbuf	: 1;
		bool uppercase	: 1;
		bool 			: 1;
		constexpr __fmtflags(uint16_t bits) noexcept :
			dec			{ NZ(bits & 0x0001) },
			oct			{ NZ(bits & 0x0002) },
			hex			{ NZ(bits & 0x0004) },
			left		{ NZ(bits & 0x0008) },
			right		{ NZ(bits & 0x0010) },
			internal	{ NZ(bits & 0x0020) },
			scientific	{ NZ(bits & 0x0040) },
			fixed		{ NZ(bits & 0x0080) },
			boolalpha	{ NZ(bits & 0x0100) },
			showbase	{ NZ(bits & 0x0200) },
			showpoint	{ NZ(bits & 0x0400) },
			showpos		{ NZ(bits & 0x0800) },
			skipws		{ NZ(bits & 0x1000) },
			unitbuf		{ NZ(bits & 0x2000) },
			uppercase	{ NZ(bits & 0x4000) }
						{}
		constexpr __fmtflags() noexcept : __fmtflags{0} {}
		constexpr __fmtflags(__fmtflags const&) noexcept = default;
		constexpr __fmtflags& operator=(__fmtflags const&) noexcept = default;
		constexpr operator uint16_t() const noexcept { return bit_cast<uint16_t>(*this); }
		constexpr __fmtflags operator|(__fmtflags const& that) const noexcept { return __fmtflags(uint16_t(uint16_t(*this) | uint16_t(that))); }
		constexpr __fmtflags& operator|=(__fmtflags const& that) noexcept { return *this = (*this | that); }
		constexpr __fmtflags operator&(__fmtflags const& that) const noexcept { return __fmtflags(uint16_t(uint16_t(*this) & uint16_t(that))); }
		constexpr __fmtflags& operator&=(__fmtflags const& that) noexcept { return *this = (*this & that); }
		constexpr __fmtflags basefield() const noexcept { return *this & __fmtflags(0b00000111US); }
		constexpr __fmtflags adjustfield() const noexcept { return *this & __fmtflags(0b00111000US); }
		constexpr __fmtflags floatfield() const noexcept { return *this & __fmtflags(0b11000000US); }
	} __pack;
	enum __seekdir : int { __beg = -1, __end = 1, __cur = 0 };
	class ios_base
	{
	public:
		typedef __openmode openmode;
		constexpr static openmode app			{ 0x01 };
		constexpr static openmode binary		{ 0x02 };
		constexpr static openmode in			{ 0x04 };
		constexpr static openmode out			{ 0x08 };
		constexpr static openmode trunc			{ 0x10 };
		constexpr static openmode ate			{ 0x10 };
		typedef __iostate iostate;
		constexpr static iostate goodbit		{		};
		constexpr static iostate badbit			{ 0x01  };
		constexpr static iostate failbit		{ 0x02  };
		constexpr static iostate eofbit			{ 0x80  };
		typedef __fmtflags fmtflags;
		constexpr static fmtflags dec			{ 0x0001 };
		constexpr static fmtflags oct			{ 0x0002 };
		constexpr static fmtflags hex			{ 0x0004 };
		constexpr static fmtflags left			{ 0x0008 };
		constexpr static fmtflags right			{ 0x0010 };
		constexpr static fmtflags internal		{ 0x0020 };
		constexpr static fmtflags scientific	{ 0x0040 };
		constexpr static fmtflags fixed			{ 0x0080 };
		constexpr static fmtflags boolalpha		{ 0x0100 };
		constexpr static fmtflags showbase		{ 0x0200 };
		constexpr static fmtflags showpoint		{ 0x0400 };
		constexpr static fmtflags showpos		{ 0x0800 };
		constexpr static fmtflags skipws		{ 0x1000 };
		constexpr static fmtflags unitbuf		{ 0x2000 };
		constexpr static fmtflags uppercase		{ 0x4000 };
		typedef __seekdir seekdir;
		constexpr static seekdir beg			{ __beg	};
		constexpr static seekdir end			{ __end	};
		constexpr static seekdir cur			{ __cur	};
	};
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