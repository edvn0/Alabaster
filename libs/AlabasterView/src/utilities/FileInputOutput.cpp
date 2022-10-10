#include "av_pch.hpp"

#include "utilities/FileInputOutput.hpp"

namespace Alabaster::IO {

	std::string read_file(const std::filesystem::path& filename)
	{
		constexpr auto read_size = std::size_t(4096);
		auto stream = std::ifstream(filename);
		stream.exceptions(std::ios_base::badbit);

		std::string out;
		std::string buf(read_size, '\0');
		while (stream.read(&buf[0], read_size)) {
			out.append(buf, 0, stream.gcount());
		}
		out.append(buf, 0, stream.gcount());
		return out;
	}

} // namespace Alabaster::IO
