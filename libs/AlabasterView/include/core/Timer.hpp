//
// Created by Edwin Carlsson on 2022-10-18.
//

#pragma once

#include "core/Clock.hpp"

namespace Alabaster {

	enum class ClockGranularity : unsigned int { SECONDS = 0, MILLIS = 1, NANOS = 2 };

	template <ClockGranularity in, typename FloatLike = double> class Timer {
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

		FloatLike elapsed()
		{
			constexpr auto out = [](auto&& f, auto&& start) { return static_cast<FloatLike>(f() - static_cast<FloatLike>(start)); };
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

} // namespace Alabaster