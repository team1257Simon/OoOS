#ifndef __CPU_TSC
#define __CPU_TSC
#include "sys/types.h"
#include "tuple"
#include "functional"
struct cpu_timer_info
{
	time_t tsc_ratio_numerator;
	time_t tsc_ratio_denominator;
	static const cpu_timer_info instance;
	cpu_timer_info(std::pair<time_t, time_t> ratio);
	suseconds_t tsc_to_us(time_t tsc) const;
	time_t us_to_tsc(suseconds_t us) const;
};
class cpu_timer_stopwatch
{
	time_t __initial;
	time_t __split;
	bool __is_started;
public:
	constexpr static struct started_t { constexpr explicit started_t() noexcept = default; } started{};
	constexpr static struct tsplit_t { constexpr explicit tsplit_t() noexcept = default; } tsplit{};
	constexpr operator bool() const noexcept { return __is_started; }
	cpu_timer_stopwatch();
	cpu_timer_stopwatch(started_t);
	void start();
	void reset();
	time_t split();
	time_t get() const;
	time_t get(tsplit_t) const;
	/**
	 * repeat_on_interval(suseconds_t interval, std::function<bool()> const& fn)
	 * Evaluates fn() at a rate of once every passing of the given interval until it returns a true result.
	 * The interval is given in microseconds and is timed using the CPU timestamp.
	 */
	void repeat_on_interval(suseconds_t interval, std::function<bool()> const& fn);
	/**
	 * repeat_on_interval(suseconds_t interval, std::function<bool()> const& fn)
	 * Evaluates fn() at a rate of once every passing of the given interval until it returns a true result, then returns true.
	 * The interval is given in microseconds and is timed using the CPU timestamp.
	 * If the function does not return a true result after the given maximum number of repetitions, stops repeating and returns false.
	 */
	bool repeat_on_interval(suseconds_t interval, std::function<bool()> const& fn, size_t max_reps);
};
#define tsci cpu_timer_info::instance
#endif