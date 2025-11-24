#ifndef __TIMES
#define __TIMES
#include <sys/types.h>
struct tms 
{
	clock_t	tms_utime;		/* user time */
	clock_t	tms_stime;		/* system time */
	clock_t	tms_cutime;		/* user time, children */
	clock_t	tms_cstime;		/* system time, children */
};
#ifdef __cplusplus
constexpr tms operator+(tms const& __this, tms const& __that) noexcept 
{ 
	return tms
	{ 
		.tms_utime	= __this.tms_utime,
		.tms_stime	= __this.tms_stime,
		.tms_cutime	= __this.tms_cutime + __that.tms_cutime,
		.tms_cstime	= __this.tms_cstime + __that.tms_cstime
	}; 
}
constexpr tms& operator+=(tms& __this, tms const& __that) noexcept { return (__this = __this + __that); }
#endif
#endif