#include "av_pch.hpp"

#include "utilities/Time.hpp"

namespace Alabaster::Time {

	std::string formatted_time()
	{
		const auto time = local_time();
		std::stringstream ss;
		ss << std::put_time(time, "%c %Z");

		auto time_stamp { ss.str() };

		std::replace(time_stamp.begin(), time_stamp.end(), ' ', '_');
		std::replace(time_stamp.begin(), time_stamp.end(), ':', '_');

		return time_stamp;
	}

	tm* local_time()
	{
		const auto t = std::time(nullptr);
		return std::localtime(&t);
	}

} // namespace Alabaster::Time
