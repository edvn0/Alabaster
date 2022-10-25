#pragma once

#include <string>
#include <string_view>

namespace Alabaster {

	template <typename FloatLike> class CPUProfiler {
	public:
		CPUProfiler(std::string_view tag, std::string_view dir = "reports");
		~CPUProfiler();

	private:
		void write_profile();

	private:
		FloatLike start_time;
		std::string_view tag;
		std::string_view dir;
		std::string file_name;
	};

} // namespace Alabaster