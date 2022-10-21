#include "av_pch.hpp"

#include "core/CPUProfiler.hpp"

#include "core/Clock.hpp"
#include "utilities/FileInputOutput.hpp"

namespace Alabaster {

	template <typename FloatLike> struct Profile {
		FloatLike time_taken;
		std::string_view tag;

		bool write_to(std::ostream& out)
		{
			out << tag << "time_taken: [" << std::to_string(time_taken) << "]";
			return true;
		}
	};

	template <typename FloatLike>
	CPUProfiler<FloatLike>::CPUProfiler(std::string_view tag, std::string_view dir)
		: tag(tag)
		, dir(dir)
	{
		start_time = Clock::get_ms<FloatLike>();
		file_name = std::string { tag } + ".prof";
	};

	template <typename FloatLike> CPUProfiler<FloatLike>::~CPUProfiler()
	{
		FloatLike time_taken = Clock::get_ms<FloatLike>() - start_time;
		Profile<FloatLike> profile { time_taken, tag };
		IO::write_file(IO::resources() / std::filesystem::path { "profiles" } / std::filesystem::path { file_name }, profile);
	}

	template class CPUProfiler<double>;
	template class CPUProfiler<float>;

} // namespace Alabaster
