#include "av_pch.hpp"

#include "utilities/FileInputOutput.hpp"

#include <sstream>

namespace Alabaster::IO {

	std::filesystem::path resources() { return std::filesystem::path { "app" } / std::filesystem::path { "resources" }; }

	std::string read_file(const std::filesystem::path& filename, OpenMode mode)
	{
		std::ifstream stream(filename, static_cast<unsigned int>(mode));
		verify(stream);

		auto size = stream.tellg();
		verify(size > 0, "Size of file must be greater than zero.");

		std::vector<char> buffer;
		buffer.resize(size);

		stream.seekg(0);

		stream.read(buffer.data(), size);

		return std::string(buffer.begin(), buffer.end());
	}

	bool exists(const std::filesystem::path& path)
	{
		return std::filesystem::exists(path) || std::filesystem::exists(std::filesystem::current_path() / path);
	}

	std::filesystem::path independent_path(const std::string& path)
	{
		verify(path.find("/") != std::string::npos);

		auto vector = [&path] {
			std::stringstream stream(path);
			std::string item;
			std::vector<std::string> split_strings;
			while (std::getline(stream, item, '/')) {
				split_strings.push_back(item);
			}
			return split_strings;
		}();

		std::filesystem::path result;
		for (auto&& subpath : vector) {
			result /= subpath;
		}
		return result;
	}

} // namespace Alabaster::IO
