#ifndef __REALTIME_CLOCK
#define __REALTIME_CLOCK
#include "kernel/kernel_defs.h"
#include "kernel/isr_table.hpp"
#include "atomic"
constexpr uint16_t UNIX_YEAR_BASE = 1970u;
#ifdef __cplusplus
extern "C" {
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
#ifdef __cplusplus
}
class rtc_driver
{
    uint8_t __century_register{ 0u };
    std::atomic<rtc_time> __my_time{};
    bool __is_bcd{};
    bool __is_12h{};
    rtc_driver() = default;
    static rtc_driver __instance;
public:
    __isrcall void rtc_time_update() volatile noexcept;
    static void init_instance(uint8_t century_register = 0) noexcept;
    static rtc_driver volatile& get_instance() noexcept;
    rtc_time get_time() volatile;
    uint64_t get_timestamp() volatile;
};
#endif
struct fadt_t* find_fadt(struct xsdt_t* xsdt);
#endif