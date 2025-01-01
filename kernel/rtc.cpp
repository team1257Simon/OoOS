#include "rtc.h"
#include "arch/arch_amd64.h"
#include "isr_table.hpp"
rtc_driver rtc_driver::__instance{}; 
constexpr static uint32_t years_to_days(uint16_t yr, uint16_t from) { return ((yr - from) * 365 + (yr - from) / 4 + (((yr % 4 == 0) || (from % 4 == 0)) ? 1 : 0)) - 1; }
constexpr static uint16_t bcd_conv(uint16_t bcd) { return ((bcd & 0xF0) >> 1) + ((bcd & 0xF0) >> 3) + (bcd & 0x0F); }
constexpr static byte read_rtc_register_dyn(byte r) { byte prev = inbw(command_rtc); outbw(command_rtc, (prev & 0x80) | r); return inb(data_rtc); }
constexpr static uint8_t days_in_month(uint8_t month, bool leap) { if(month == 2) return leap ? 29 : 28; if(month == 1 || month == 3 || month == 5 || month == 7 || month == 10 || month == 12) return 31; return 30; }
constexpr static uint16_t day_of_year(uint8_t month, uint16_t day, bool leap) { uint16_t result = day - 1; for(uint8_t i = 1; i < month; i++) result += days_in_month(i, leap); return result; }
constexpr static uint8_t day_of_week(uint16_t year, uint8_t month, uint16_t day) { uint32_t dy = day_of_year(month, day, (year % 4 == 0)); dy += years_to_days(year, 1800); return ((dy + 3) % 7) + 1; /* 1800 started on a Wednesday */ }
constexpr static uint64_t to_unix_timestamp(rtc_time const& t) { uint64_t result = t.sec; result += static_cast<uint64_t>(t.min) * 60uL; result += static_cast<uint64_t>(t.hr) * 3600uL; result += static_cast<uint64_t>(day_of_year(t.month, t.day, (t.year % 4 == 0)) + years_to_days(t.year, 1970)) * 86400uL; return result * 1000uL; }
void rtc_driver::init_instance(uint8_t century_register) noexcept { __instance.__century_register = century_register; __instance.__is_12h = !(read_rtc_register<0x0B>() & 0x02); __instance.__is_bcd = !(read_rtc_register<0x0B>() & 0x04); interrupt_table::add_irq_handler(0, IRQ_LAMBDA() { rtc_driver::get_instance().rtc_time_update(); }); irq_clear_mask<0>(); }
rtc_driver volatile &rtc_driver::get_instance() noexcept { return __instance; }
rtc_time rtc_driver::get_time() volatile { return __my_time; }
uint64_t rtc_driver::get_timestamp() volatile { return to_unix_timestamp(get_time()); }
fadt_t *find_fadt(xsdt_t *xsdt) { return reinterpret_cast<fadt_t*>(find_system_table(xsdt, "FACP")); }
extern "C" uint64_t syscall_time(uint64_t* tm_target) { uint64_t t = rtc_driver::get_instance().get_timestamp(); if(tm_target) __atomic_store_n(tm_target, t, __ATOMIC_SEQ_CST); return t; }
__isr_registers void rtc_driver::rtc_time_update() volatile noexcept
{
    while(is_cmos_update_in_progress());
    uint16_t century = (__century_register > 0 ? read_rtc_register_dyn(__century_register) : 20);
    rtc_time nt
    {
        read_rtc_register<0x00>(),
        read_rtc_register<0x02>(),
        read_rtc_register<0x04>(),
        0,
        read_rtc_register<0x07>(),
        read_rtc_register<0x08>(),
        read_rtc_register<0x09>()
    };
    if(__is_12h)
    {
        bool pm = (nt.hr & 0x80) != 0;
        nt.hr &= ~(0x80);
        nt.hr %= 12;
        if(pm) nt.hr += 12;
    }
    if(__is_bcd)
    {
        if(__century_register > 0) century = bcd_conv(century);
        nt.sec = bcd_conv(nt.sec);
        nt.min = bcd_conv(nt.min);
        nt.hr = bcd_conv(nt.hr);
        nt.day = bcd_conv(nt.day);
        nt.month = bcd_conv(nt.month);
        nt.year = bcd_conv(nt.year);
    }
    nt.year += century * 100;
    nt.wkday = day_of_week(nt.year, nt.month, nt.day);
    __my_time.store(nt);
}
