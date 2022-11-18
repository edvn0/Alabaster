#include "av_pch.hpp"

#include "utilities/FileInputOutput.hpp"

#include <sstream>

namespace Alabaster::IO {

	std::filesystem::path root;

	static constexpr auto filter_file_types
		= [](const auto& fd) { return fd.path().extension() == ".DS_Store" || fd.path().extension() == ".gitkeep"; };

	void init_with_cwd(const std::filesystem::path& path) { root = path; }

	std::filesystem::path resources() { return root; }

	std::filesystem::path textures() { return root / std::filesystem::path { "textures" }; }

	std::filesystem::path shaders() { return root / std::filesystem::path { "shaders" }; }

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

	std::string read_file(std::filesystem::path&& filename, OpenMode mode) { return IO::read_file(filename, mode); }

	bool exists(const std::filesystem::path& path)
	{
		return std::filesystem::exists(path) || std::filesystem::exists(std::filesystem::current_path() / path);
	}

	bool is_file(const std::filesystem::path& path)
	{
		auto regular_file = std::filesystem::is_regular_file(path) || std::filesystem::is_regular_file(std::filesystem::current_path() / path);
		return IO::exists(path) && regular_file;
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

	std::filesystem::path slashed_to_fp(const std::string& slashed_string) { return independent_path(slashed_string); }

} // namespace Alabaster::IO
