//
// Created by Edwin Carlsson on 2022-10-18.
//

#pragma once

#include "core/Clock.hpp"

namespace Alabaster {

	enum class ClockGranularity : unsigned int { SECONDS = 0, MILLIS = 1, NANOS = 2 };

	template <typename FloatLike = double, ClockGranularity in = ClockGranularity::MILLIS> class Timer {
	public:
		Timer()
		{
			if constexpr (in == ClockGranularity::SECONDS) {
				start_time = Clock::get_seconds<FloatLike>();
			} else if constexpr (in == ClockGranularity::MILLIS) {
				start_time = Clock::get_ms<FloatLike>();
			} else {
				start_time = Clock::get_nanos<FloatLike>();
			}
		}

		~Timer() = default;

		[[nodiscard]] FloatLike elapsed()
		{
			constexpr auto out
				= [](auto&& clock_function, auto start) { return static_cast<FloatLike>(clock_function() - static_cast<FloatLike>(start)); };
			if constexpr (in == ClockGranularity::SECONDS) {
				return out(Clock::get_seconds<FloatLike>, start_time);
			} else if constexpr (in == ClockGranularity::MILLIS) {
				return out(Clock::get_ms<FloatLike>, start_time);
			} else {
				return out(Clock::get_nanos<FloatLike>, start_time);
			}
		}

	private:
		FloatLike start_time { 0.0 };
		ClockGranularity granularity { ClockGranularity::MILLIS };
	};

	template class Timer<float, ClockGranularity::MILLIS>;
	template class Timer<double, ClockGranularity::MILLIS>;
	template class Timer<float, ClockGranularity::NANOS>;
	template class Timer<double, ClockGranularity::NANOS>;
	template class Timer<float, ClockGranularity::SECONDS>;
	template class Timer<double, ClockGranularity::SECONDS>;

} // namespace Alabaster
