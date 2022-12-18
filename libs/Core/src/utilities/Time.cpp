#include "av_pch.hpp"

#include "utilities/Time.hpp"

#ifdef ALABASTER_WINDOWS
#include <time.h>
#endif
#include <algorithm>

namespace Alabaster::Time {

	std::string formatted_time()
	{
		const auto time = local_time();
		std::stringstream ss;
		ss << std::put_time(&time, "%c %Z");

		auto time_stamp { ss.str() };

		std::replace(time_stamp.begin(), time_stamp.end(), ' ', '_');
		std::replace(time_stamp.begin(), time_stamp.end(), ':', '_');

		return time_stamp;
	}

	tm local_time()
	{
#ifdef ALABASTER_WINDOWS
		struct tm newtime;
		time_t now = time(0);
		localtime_s(&newtime, &now);
		return newtime;
#else
		auto t = time(nullptr);
		return *std::localtime(t);
#endif
	}

} // namespace Alabaster::Time
