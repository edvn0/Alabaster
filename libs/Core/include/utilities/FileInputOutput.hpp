#pragma once

#include "core/Common.hpp"
#include "core/OpenMode.hpp"
#include "core/exceptions/AlabasterException.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_set>

namespace Alabaster::IO {

	void init_with_cwd(const std::filesystem::path& path);

	std::filesystem::path resources();
	std::filesystem::path textures();
	std::filesystem::path shaders();
	std::filesystem::path models();
	std::filesystem::path fonts();
	std::filesystem::path scenes();
	std::filesystem::path scripts();
	std::filesystem::path editor_resources();

	template <typename Path = std::filesystem::path> std::filesystem::path shader(const Path& path)
	{
		return IO::shaders() / std::filesystem::path { path };
	}
	template <typename Path = std::filesystem::path> std::filesystem::path model(const Path& path)
	{
		return IO::models() / std::filesystem::path { path };
	}
	template <typename Path = std::filesystem::path> std::filesystem::path texture(const Path& path)
	{
		return IO::textures() / std::filesystem::path { path };
	}
	template <typename Path = std::filesystem::path> std::filesystem::path font(const Path& path)
	{
		return IO::fonts() / std::filesystem::path { path };
	}
	template <typename Path = std::filesystem::path> std::filesystem::path scene(const Path& path)
	{
		return IO::scenes() / std::filesystem::path { path };
	}
	template <typename Path = std::filesystem::path> std::filesystem::path script(const Path& path)
	{
		return IO::scripts() / std::filesystem::path { path };
	}

	std::string read_file(const std::filesystem::path& filename, OpenMode mode = OpenMode::Read | OpenMode::Binary | OpenMode::AtEnd);

	std::string read_file(std::filesystem::path&& filename, OpenMode mode = OpenMode::Read | OpenMode::Binary | OpenMode::AtEnd);

	bool exists(const std::filesystem::path& path);

	bool is_file(const std::filesystem::path& path);

	std::filesystem::path independent_path(const std::string& path);

	std::filesystem::path slashed_to_fp(const std::string& slashed_string);

	std::optional<std::filesystem::path> get_resource_root();

	template <typename T>
	concept Printable = requires(T t, std::ofstream& of) {
		{
			t.write_to(of)
		} -> std::same_as<bool>;
	};

	static inline bool write_file(const std::filesystem::path& filename, Printable auto& printable)
	{
		std::ofstream output_stream(filename);
		if (!output_stream) {
			Log::error("Could not write output file: {}", filename.string());
			return false;
		}

		if (printable.write_to(output_stream)) {
			return true;
		}
		Log::error("Could not write to output file stream.");
		return false;
	}

	static inline bool write_file(const std::filesystem::path& filename, const char* buffer, std::size_t size)
	{
		std::ofstream output_stream(filename);
		if (!output_stream) {
			Log::error("Could not write output file: {}", filename.string());
			return false;
		}

		output_stream.write(buffer, size);
		Log::error("Could not write to output file stream.");
		return false;
	}

} // namespace Alabaster::IO
