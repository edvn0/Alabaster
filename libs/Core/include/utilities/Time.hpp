#pragma once

#include <ctime>
#include <iomanip>
#include <sstream>

namespace Alabaster::Time {

	std::string formatted_time();

	tm* local_time();

} // namespace Alabaster::Time
