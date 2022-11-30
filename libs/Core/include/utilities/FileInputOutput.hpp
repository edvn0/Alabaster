#pragma once

#include "core/Common.hpp"
#include "core/exceptions/AlabasterException.hpp"
#include "core/OpenMode.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_set>

namespace Alabaster::IO {

	void init_with_cwd(const std::filesystem::path& path);

	std::filesystem::path resources();
	std::filesystem::path textures();
	std::filesystem::path shaders();
	std::filesystem::path fonts();

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

	template <typename Path = std::filesystem::path> std::filesystem::path scene(const Path& path)
	{
		const auto out = IO::resources() / std::filesystem::path { "scene" } / std::filesystem::path { path };
		return std::filesystem::path { out.string() + ".json" };
	}

	std::string read_file(const std::filesystem::path& filename, OpenMode mode = OpenMode::Read | OpenMode::Binary | OpenMode::AtEnd);

	std::string read_file(std::filesystem::path&& filename, OpenMode mode = OpenMode::Read | OpenMode::Binary | OpenMode::AtEnd);

	bool exists(const std::filesystem::path& path);

	bool is_file(const std::filesystem::path& path);

	std::filesystem::path independent_path(const std::string& path);

	std::filesystem::path slashed_to_fp(const std::string& slashed_string);

	inline std::optional<std::filesystem::path> get_resource_root()
	{
		constexpr const auto sanity_checks = [] {
			const auto cwd = std::filesystem::current_path();
			const auto app_dir_exists = std::filesystem::exists(cwd / std::filesystem::path { "app" });
			const auto resources_dir_exists = std::filesystem::exists(cwd / std::filesystem::path { "resources" });

			const auto app_shaders_exists = std::filesystem::exists(
				cwd / std::filesystem::path { "app" } / std::filesystem::path { "resources" } / std::filesystem::path { "shaders" });
			const auto app_models_exists = std::filesystem::exists(
				cwd / std::filesystem::path { "app" } / std::filesystem::path { "resources" } / std::filesystem::path { "models" });
			const auto app_textures_exists = std::filesystem::exists(
				cwd / std::filesystem::path { "app" } / std::filesystem::path { "resources" } / std::filesystem::path { "textures" });

			const auto resources_shaders_exists
				= std::filesystem::exists(std::filesystem::path { "resources" } / std::filesystem::path { "shaders" });
			const auto resources_models_exists = std::filesystem::exists(std::filesystem::path { "resources" } / std::filesystem::path { "models" });
			const auto resources_textures_exists
				= std::filesystem::exists(std::filesystem::path { "resources" } / std::filesystem::path { "textures" });

			if (!app_dir_exists && !resources_dir_exists) {
				Alabaster::Log::error("Your CWD is: {}, and Alabaster could not find the 'app' directory there.", cwd.string());
				throw AlabasterException();
			}

			// We might have app or resources

			if (!app_shaders_exists && !resources_shaders_exists) {
				Alabaster::Log::error("Your CWD is: {}, and Alabaster could not find the 'app/shaders' directory there.", cwd.string());
				throw AlabasterException();
			}

			if (!app_models_exists && !resources_models_exists) {
				Alabaster::Log::error("Your CWD is: {}, and Alabaster could not find the 'app/models' directory there.", cwd.string());
				throw AlabasterException();
			}

			if (!app_textures_exists && !resources_textures_exists) {
				Alabaster::Log::error("Your CWD is: {}, and Alabaster could not find the 'app/textures' directory there.", cwd.string());
				throw AlabasterException();
			}

			if (app_dir_exists) {
				return std::filesystem::path { "app" };
			}
			if (resources_dir_exists) {
				return std::filesystem::path { "resources" };
			}

			Alabaster::Log::error("Something really strange happened.");
			throw AlabasterException();
		};

		try {
			return sanity_checks();
		} catch (const AlabasterException&) {
			return {};
		}
	}

	template <typename Printable> static inline bool write_file(const std::filesystem::path& filename, Printable& printable)
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

	template <typename Printable> static inline bool write_file(const std::filesystem::path& filename, const char* buffer, std::size_t size)
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
