#include "av_pch.hpp"

#include "utilities/Time.hpp"

#include <algorithm>
#include <time.h>

namespace Alabaster::Time {

	std::string formatted_time()
	{
		const auto time = local_time();
		std::stringstream ss;
		ss << std::put_time(&time, "%c %Z");

		auto time_stamp { ss.str() };
		std::ranges::replace(time_stamp.begin(), time_stamp.end(), ' ', '_');
		std::ranges::replace(time_stamp.begin(), time_stamp.end(), ':', '_');
		std::ranges::replace(time_stamp.begin(), time_stamp.end(), '\\', '_');
		std::ranges::replace(time_stamp.begin(), time_stamp.end(), '/', '_');
		std::ranges::replace(time_stamp.begin(), time_stamp.end(), '.', '_');
		std::ranges::replace(time_stamp.begin(), time_stamp.end(), ',', '_');

		return time_stamp;
	}

	tm local_time()
	{
		struct tm newtime;
		time_t now = time(0);
		localtime_s(&newtime, &now);
		return newtime;
	}

} // namespace Alabaster::Time
