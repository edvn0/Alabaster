//
// Created by Edwin Carlsson on 2022-10-18.
//

#pragma once

#include "core/Clock.hpp"

namespace Alabaster {

	enum class ClockGranularity : unsigned int { SECONDS = 0, MILLIS = 1, NANOS = 2 };

	template <typename FloatLike = double> class Timer {
	public:
		Timer(ClockGranularity granularity = ClockGranularity::MILLIS)
		{
			switch (granularity) {
			case ClockGranularity::SECONDS: {
				start_time = Clock::get_seconds<FloatLike>();
				break;
			}
			case ClockGranularity::MILLIS: {
				start_time = Clock::get_ms<FloatLike>();
				break;
			}
			case ClockGranularity::NANOS: {
				start_time = Clock::get_nanos<FloatLike>();
				break;
			}
			}
		}

		auto elapsed()
		{
			switch (granularity) {
			case ClockGranularity::SECONDS: {
				return static_cast<FloatLike>(Clock::get_seconds<FloatLike>() - static_cast<FloatLike>(start_time));
			}
			case ClockGranularity::MILLIS: {
				return static_cast<FloatLike>(Clock::get_ms<FloatLike>() - static_cast<FloatLike>(start_time));
			}
			case ClockGranularity::NANOS: {
				return static_cast<FloatLike>(Clock::get_ms<FloatLike>() - static_cast<FloatLike>(start_time));
			}
			}
		}

	private:
		FloatLike start_time { 0.0 };
		ClockGranularity granularity;
	};

} // namespace Alabaster