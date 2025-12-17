#include <arch/cpu_time.hpp>
#include <arch/arch_amd64.h>
static std::pair<time_t, time_t> compute_cpu_tsc_ratio();
static time_t diff_tsc(time_t from);
static void read_tsc(addr_t out) { asm volatile("rdtsc" : "=a"(out.deref<uint32_t>()), "=d"(out.plus(4Z).deref<uint32_t>()) :: "memory"); }
const cpu_timer_info cpu_timer_info::instance(compute_cpu_tsc_ratio());
cpu_timer_info::cpu_timer_info(std::pair<time_t, time_t> ratio) : tsc_ratio_numerator(ratio.first), tsc_ratio_denominator(ratio.second) {}
suseconds_t cpu_timer_info::tsc_to_us(time_t tsc) const { return (tsc * tsc_ratio_denominator) / tsc_ratio_numerator; }
time_t cpu_timer_info::us_to_tsc(suseconds_t us) const { return (us * tsc_ratio_numerator) / tsc_ratio_denominator; }
cpu_timer_stopwatch::cpu_timer_stopwatch() = default;
cpu_timer_stopwatch::cpu_timer_stopwatch(started_t) : __is_started(true) { start(); }
time_t cpu_timer_stopwatch::get() const { return diff_tsc(__initial); }
time_t cpu_timer_stopwatch::get(tsplit_t) const { return diff_tsc(__split); }
void cpu_timer_stopwatch::reset()
{
	__initial		= 0UL;
	__split			= 0UL;
	__is_started	= false;
}
void cpu_timer_stopwatch::start()
{
	read_tsc(std::addressof(__initial));
	__split			= __initial;
	__is_started	= true;
}
time_t cpu_timer_stopwatch::split()
{
	time_t old		= __split;
	read_tsc(std::addressof(__split));
	__split			-= old;
	return __split;
}
static time_t diff_tsc(time_t from)
{
	uint32_t h, l;
	asm volatile("rdtsc" : "=a"(l), "=d"(h) :: "memory");
	return static_cast<time_t>(qword(l, h) - from);
}
static std::pair<time_t, time_t> compute_cpu_tsc_ratio()
{
    cpuid_leaf leaf_0x15	= cpuid(0x15U, 0U);
    if(leaf_0x15.ebx && leaf_0x15.eax) return (leaf_0x15.ecx ? std::pair<time_t, time_t>(leaf_0x15.ecx * leaf_0x15.ebx, leaf_0x15.eax) : std::pair<time_t, time_t>(leaf_0x15.ebx, leaf_0x15.eax));
    return std::pair<time_t, time_t>(cpuid(0x16U, 0U).ecx, 1UL);
}
void cpu_timer_stopwatch::repeat_on_interval(suseconds_t interval, std::function<bool()> const& fn)
{
    time_t tsc_interval	= cpu_timer_info::instance.us_to_tsc(interval);
    bool result			= false;
    if(!__is_started) start();
    do {
        split();
        while(get(tsplit) < tsc_interval) pause();
        result			= fn();
    } while(!result);
}
bool cpu_timer_stopwatch::repeat_on_interval(suseconds_t interval, std::function<bool()> const& fn, size_t max_reps)
{
    time_t tsc_interval	= cpu_timer_info::instance.us_to_tsc(interval);
    size_t reps			= 0UZ;
    bool result			= false;
    if(!__is_started) start();
    do {
        split();
        while(get(tsplit) < tsc_interval) pause();
        result			= fn();
    } while(!result && ++reps < max_reps);
    return result;
}