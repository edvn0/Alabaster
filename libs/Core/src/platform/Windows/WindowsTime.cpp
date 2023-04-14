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
		static auto replace = [](auto& data, char what, char with) mutable { std::ranges::replace(data.begin(), data.end(), what, with); };

		replace(time_stamp, ' ', '_');
		replace(time_stamp, ':', '_');
		replace(time_stamp, '\\', '_');
		replace(time_stamp, '/', '_');
		replace(time_stamp, '.', '_');
		replace(time_stamp, ',', '_');
		replace(time_stamp, 'å', 'a');
		replace(time_stamp, 'ö', 'o');
		replace(time_stamp, 'ä', 'a');

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
