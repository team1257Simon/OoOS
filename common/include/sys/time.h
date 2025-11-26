#ifndef __TIME
#define __TIME
#include <sys/types.h>
struct timeval { time_t tv_sec; suseconds_t tv_usec; };
struct timespec { time_t tv_sec; long tv_nsec; };
struct tm
{
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;    // Years since 1900
    int tm_wday;    // Sunday = 0
    int tm_yday;    // 0-365 (365 used in leap years)
    int tm_isdst;   // Daylight Savings flag
};
#endif