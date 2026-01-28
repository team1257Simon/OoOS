#include <rtc.h>
#include <arch/arch_amd64.h>
rtc rtc::__instance{}; 
constexpr static uint16_t bcd_conv(uint16_t bcd) { return ((bcd & 0xF0US) >> 1) + ((bcd & 0xF0US) >> 3) + (bcd & 0x0FUS); }
constexpr static uint8_t read_rtc_register_dyn(u8 r) { u8 prev = inbw(command_rtc); outbw(command_rtc, (prev & 0x80UC) | r); return inb(data_rtc); }
constexpr static uint8_t day_of_week(uint16_t year, uint8_t month, uint16_t day) { uint32_t dy = day_of_year(month, day, (year % 4US == 0US)); dy += years_to_days(year, 1800US); return ((dy + 3US) % 7US) + 1US; /* 1800 started on a Wednesday */ }
constexpr static uint64_t to_unix_timestamp(rtc_time const& t) { return static_cast<uint64_t>(t.sec + static_cast<uint64_t>(t.min) * 60UL + static_cast<uint64_t>(t.hr) * 3600UL + static_cast<uint64_t>(day_of_year(t.month, t.day, (t.year % 4U == 0U)) + years_to_days(t.year, unix_year_base)) * 86400UL); }
void rtc::init_instance(uint8_t century_register) noexcept { __instance.__century_register = century_register; __instance.__is_12h = !(read_rtc_register<0x0BUC>()[1]); __instance.__is_bcd = !(read_rtc_register<0x0BUC>()[2]); interrupt_table::add_irq_handler(0UC, std::bind(&rtc::rtc_time_update, std::addressof(__instance))); }
rtc volatile& rtc::get_instance() noexcept { return __instance; }
rtc_time rtc::get_time() volatile { return __my_time; }
uint64_t rtc::get_timestamp() volatile { return to_unix_timestamp(get_time()); }
fadt_t* find_fadt() { return static_cast<fadt_t*>(find_system_table("FACP")); }
extern "C" { uint64_t sys_time(uint64_t* tm_target) { uint64_t t = rtc::get_instance().get_timestamp(); if(tm_target) __atomic_store_n(tm_target, t, __ATOMIC_SEQ_CST); return t; } }
void rtc::rtc_time_update() volatile noexcept
{
	while(is_cmos_update_in_progress()) pause();
	uint16_t century	= (__century_register > 0U ? read_rtc_register_dyn(__century_register) : 20UC);
	rtc_time nt
	{
		.sec	= read_rtc_register<0x00UC>(),
		.min	= read_rtc_register<0x02UC>(),
		.hr		= read_rtc_register<0x04UC>(),
		.wkday	= 0UC,
		.day	= read_rtc_register<0x07UC>(),
		.month	= read_rtc_register<0x08UC>(),
		.year	= read_rtc_register<0x09UC>()
	};
	if(__is_12h)
	{
		bool pm		= ((nt.hr & 0x80UC) != 0);
		nt.hr		&= ~0x80UC;
		nt.hr		%= 12UC;
		if(pm)
			nt.hr	+= 12UC;
	}
	if(__is_bcd)
	{
		if(__century_register > 0)
			century	= bcd_conv(century);
		nt.sec		= bcd_conv(nt.sec);
		nt.min		= bcd_conv(nt.min);
		nt.hr		= bcd_conv(nt.hr);
		nt.day		= bcd_conv(nt.day);
		nt.month	= bcd_conv(nt.month);
		nt.year		= bcd_conv(nt.year);
	}
	nt.year			+= century * 100US;
	nt.wkday		= day_of_week(nt.year, nt.month, nt.day);
	__my_time.store(nt);
}