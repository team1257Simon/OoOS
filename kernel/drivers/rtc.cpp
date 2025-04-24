#include "rtc.h"
#include "arch/arch_amd64.h"
#include "isr_table.hpp"
rtc rtc::__instance{}; 
constexpr static uint16_t bcd_conv(uint16_t bcd) { return ((bcd & 0xF0U) >> 1) + ((bcd & 0xF0U) >> 3) + (bcd & 0x0FU); }
constexpr static byte read_rtc_register_dyn(byte r) { byte prev = inbw(command_rtc); outbw(command_rtc, (prev & 0x80ui8) | r); return inb(data_rtc); }
constexpr static uint8_t day_of_week(uint16_t year, uint8_t month, uint16_t day) { uint32_t dy = day_of_year(month, day, (year % 4U == 0U)); dy += years_to_days(year, 1800U); return ((dy + 3U) % 7U) + 1U; /* 1800 started on a Wednesday */ }
constexpr static uint64_t to_unix_timestamp(rtc_time const& t) { return static_cast<uint64_t>(t.sec + static_cast<uint64_t>(t.min) * 60UL + static_cast<uint64_t>(t.hr) * 3600UL + static_cast<uint64_t>(day_of_year(t.month, t.day, (t.year % 4U == 0U)) + years_to_days(t.year, unix_year_base)) * 86400UL); }
void rtc::init_instance(uint8_t century_register) noexcept { __instance.__century_register = century_register; __instance.__is_12h = !(read_rtc_register<0x0Bui8>().b1); __instance.__is_bcd = !(read_rtc_register<0x0Bui8>().b2); if(interrupt_table::add_irq_handler(0ui8, std::move(LAMBDA_ISR() { __instance.rtc_time_update(); }))) irq_clear_mask<0ui8>(); }
rtc volatile& rtc::get_instance() noexcept { return __instance; }
rtc_time rtc::get_time() volatile { return __my_time; }
uint64_t rtc::get_timestamp() volatile { return to_unix_timestamp(get_time()); }
fadt_t* find_fadt() { return static_cast<fadt_t*>(find_system_table("FACP")); }
extern "C" { uint64_t sys_time(uint64_t* tm_target) { uint64_t t = rtc::get_instance().get_timestamp(); if(tm_target) __atomic_store_n(tm_target, t, __ATOMIC_SEQ_CST); return t; } }
__isrcall void rtc::rtc_time_update() volatile noexcept
{
    while(is_cmos_update_in_progress());
    uint16_t century = (__century_register > 0U ? read_rtc_register_dyn(__century_register) : 20ui8);
    rtc_time nt
    {
        .sec = read_rtc_register<0x00ui8>(),
        .min = read_rtc_register<0x02ui8>(),
        .hr = read_rtc_register<0x04ui8>(),
        .wkday = 0U,
        .day = read_rtc_register<0x07ui8>(),
        .month = read_rtc_register<0x08ui8>(),
        .year = read_rtc_register<0x09ui8>()
    };
    if(__is_12h)
    {
        bool pm = ((nt.hr & 0x80U) != 0);
        nt.hr &= ~0x80;
        nt.hr %= 12;
        if(pm) nt.hr += 12U;
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
    nt.year += century * 100U;
    nt.wkday = day_of_week(nt.year, nt.month, nt.day);
    __my_time.store(nt);
}