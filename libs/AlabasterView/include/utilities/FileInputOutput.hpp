#pragma once

#include "core/Common.hpp"
#include "core/OpenMode.hpp"

#include <filesystem>
#include <fstream>
#include <string>
#include <unordered_set>

namespace Alabaster::IO {

	void init_with_cwd(const std::filesystem::path& path);

	std::filesystem::path resources();
	template <typename Path = std::filesystem::path> std::filesystem::path shader(const Path& path)
	{
		return IO::resources() / std::filesystem::path { "shaders" } / std::filesystem::path { path };
	}
	template <typename Path = std::filesystem::path> std::filesystem::path model(const Path& path)
	{
		return IO::resources() / std::filesystem::path { "models" } / std::filesystem::path { path };
	}
	template <typename Path = std::filesystem::path> std::filesystem::path texture(const Path& path)
	{
		return IO::resources() / std::filesystem::path { "textures" } / std::filesystem::path { path };
	}

	std::string read_file(const std::filesystem::path& filename, OpenMode mode = OpenMode::Read | OpenMode::Binary | OpenMode::AtEnd);
	std::string read_file(std::filesystem::path&& filename, OpenMode mode = OpenMode::Read | OpenMode::Binary | OpenMode::AtEnd);
	bool exists(const std::filesystem::path& path);
	bool is_file(const std::filesystem::path& path);
	std::filesystem::path independent_path(const std::string& path);
	std::filesystem::path slashed_to_fp(const std::string& slashed_string);

	template <typename Output = std::string, bool Recursive = false>
	std::vector<Output> in_directory(const std::filesystem::path& path, std::unordered_set<std::string> extensions)
	{
		static constexpr auto entry_to_string = [](const auto& input) { return input.path().extension().string(); };
		std::vector<Output> output;
		if constexpr (Recursive) {
			for (const auto& fd : std::filesystem::recursive_directory_iterator { path }) {
				if (extensions.count(entry_to_string(fd))) {
					output.push_back(static_cast<Output>(fd.path().string()));
				}
			}
		} else {
			for (const auto& fd : std::filesystem::directory_iterator { path }) {
				if (extensions.count(entry_to_string(fd))) {
					output.push_back(static_cast<Output>(fd.path().string()));
				}
			}
		}
		return output;
	}

	template <typename Printable> static inline bool write_file(const std::filesystem::path& filename, Printable& printable)
	{
		std::ofstream output_stream(filename);
		if (!output_stream) {
			Log::error("Could not write output file: {}", filename);
			return false;
		}

		if (printable.write_to(output_stream)) {
			return true;
		}
		Log::error("Could not write to output file stream.");
		return false;
	}

	template <typename Printable> static inline bool write_file(const std::filesystem::path& filename, const char* buffer, size_t size)
	{
		std::ofstream output_stream(filename);
		if (!output_stream) {
			Log::error("Could not write output file: {}", filename);
			return false;
		}

		output_stream.write(buffer, size);
		Log::error("Could not write to output file stream.");
		return false;
	}

} // namespace Alabaster::IO
