#pragma once

#include <string>
#include <string_view>

namespace Alabaster {

	template <typename FloatLike = double> class CPUProfiler {
	public:
		CPUProfiler(std::string_view tag, std::string_view dir = "reports");
		~CPUProfiler();

	private:
		FloatLike start_time;
		std::string_view tag;
		std::string_view dir;
		std::string file_name;
	};

} // namespace Alabaster