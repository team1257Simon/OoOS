#ifndef __REALTIME_CLOCK
#define __REALTIME_CLOCK
#include "sys/time.h"
#include "kernel/isr_table.hpp"
#include "atomic"
#ifdef __cplusplus
extern "C"
{
#endif
struct rtc_time
{
    uint8_t sec;
    uint8_t min;
    uint8_t hr;
    uint8_t wkday;
    uint8_t day;
    uint8_t month;
    uint16_t year;
} __pack;
int syscall_gettimeofday(struct timeval* restrict tm, void* restrict tz);
#ifdef __cplusplus
}
class rtc
{
    uint8_t __century_register{ 0ui8 };
    std::atomic<rtc_time> __my_time{};
    bool __is_bcd{};
    bool __is_12h{};
    rtc() = default;
    static rtc __instance;
public:
    __isrcall void rtc_time_update() volatile noexcept;
    static void init_instance(uint8_t century_register = 0) noexcept;
    static rtc volatile& get_instance() noexcept;
    rtc_time get_time() volatile;
    uint64_t get_timestamp() volatile;
};
constexpr timeval timestamp_to_timeval(uint64_t ts) { return { ts, 0U }; }
#endif
struct fadt_t* find_fadt();
#endif