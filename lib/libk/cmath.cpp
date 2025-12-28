/**
 * Some implementations of math functions on an as-needed basis.
 */
#include <float.h>
#include <libk_decls.h>
#include <errno.h>
extern "C" double modf(double x, double* iptr);
constexpr uint32_t minus_sign	= 0x80000000U;
constexpr double one			= 1.0;
constexpr static qword extract(double d) { return std::bit_cast<qword>(d); }
constexpr static double revert(qword q) { return std::bit_cast<double>(q); }
constexpr static double __domain_error() { if(!std::is_constant_evaluated()) errno = EDOM; return __builtin_nan("1"); }
constexpr static double __pole_error(bool sign) { if(!std::is_constant_evaluated()) errno = ERANGE; return (sign ? -1 : 1) * __builtin_huge_val(); }
constexpr static double __range_error(bool sign) { if(!std::is_constant_evaluated()) errno = ERANGE; return (sign ? -1 : 1) * __builtin_inf(); }
static bool __even_integer(double d)
{
	double ival{};
	if(modf(d, std::addressof(ival)) == 0.0)
	{
		double half	= d / 2.;
		if(modf(half, std::addressof(ival)) == 0.0)
			return true;
	}
	return false;
}
static inline double __norm_fpow(double x, double y)
{
	double result;
	asm volatile("fyl2x\n fld %%st\n frndint\n fsub %%st, %%st(1)\n fxch\n fchs\n f2xm1\n fld1\n faddp\n fxch\n fld1\n fscale\n fstp %%st(1)\n fmulp" : "=t"(result) : "0"(x), "u"(y) : "%st(1)");
	return result;
}
extern "C"
{
	double ldexp(double d, int e)
	{
		if(__unlikely(__builtin_isnan(d))) errno = EDOM;
		else if(__unlikely(__builtin_isinf(d))) errno = ERANGE;
		else if(d != 0.0)
		{
			qword bits			= extract(d);
			uint32_t hidword	= bits.hi;
			int exponent		= ((hidword & 0x7FF00000) >> 20) + e;
			int exponent_adj	= exponent - 1023;
			if(__unlikely(exponent_adj > DBL_MAX_EXP)) return __range_error(false);
			else if(__unlikely(exponent_adj < DBL_MIN_EXP)) return __range_error(true);
			else
			{
				hidword	&= 0x800FFFFF;
				hidword	|= exponent << 20;
				bits.hi	= hidword;
			}
			return revert(bits);
		}
		return d;
	}
	double modf(double x, double* iptr)
	{
		qword bits						= extract(x);
		uint32_t exponent				= ((bits.hi >> 20) & 0x7FF) - 0x3FF;
		bool sign						= x < 0.0;
		if(exponent < 0) {
			*iptr	= revert(qword(0U, sign ? minus_sign : 0U));
			return x;
		}
		if(exponent < 20)
		{
			uint32_t integral_mask	= (0x000FFFFF) >> exponent;
			if(!((bits.hi & integral_mask) | bits.lo)) {
				*iptr	= x;
				return revert(qword(0U, sign ? minus_sign : 0U));
			}
			else
			{
				double iresult	= revert(qword(0U, bits.hi & ~integral_mask));
				*iptr			= iresult;
				return x - iresult;
			}
		}
		else if(exponent > 51) {
			*iptr	= x * one;
			return revert(qword(0U, sign ? minus_sign : 0U));
		}
		else
		{
			uint32_t integral_mask	= (0xFFFFFFFFU) >> (exponent - 20);
			if(!(bits.lo & integral_mask)) {
				*iptr	= x;
				return revert(qword(0U, sign ? minus_sign : 0U));
			}
			else
			{
				double iresult	= revert(qword(0U, bits.hi & ~integral_mask));
				*iptr			= iresult;
				return x - iresult;
			}
		}
	}
	double pow(double x, double y)
	{
		if(__unlikely(__builtin_isnan(x) || __builtin_isnan(y)))
			return __domain_error();
		else if(__unlikely(__builtin_isinf(x)))
		{
			if(y <= 0.0)
				return __domain_error();
			else return __range_error(x < 0.0);
		}
		else if(__unlikely(__builtin_isinf(y)))
		{
			if(x == 0.0)
				return __domain_error();
			else return __range_error(x < 0.0);
		}
		else if(x == 0.0)
		{
			if(__unlikely(y <= 0.0))
				return __domain_error();
			return 0.0;
		}
		qword xbits		= extract(x);
		double yint{};
		double yfrac	= modf(y, std::addressof(yint));
		bool exp_even	= __even_integer(y);
		if(xbits.hi & minus_sign) {
			if(yfrac) {
				errno	= EDOM;
				return 0.0;
			}
			qword plus(xbits.lo, xbits.hi & ~minus_sign);
			qword result	= extract(pow(revert(plus), y));
			if(!exp_even)
				result.hi	|= minus_sign;
			return revert(result);
		}
		return __norm_fpow(x, y);
	}
}