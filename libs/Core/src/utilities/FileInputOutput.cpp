#include "av_pch.hpp"

#include "utilities/FileInputOutput.hpp"

#include <sstream>

namespace Alabaster::IO {

	std::filesystem::path root;

	void init_with_cwd(const std::filesystem::path& path) { root = path; }

	std::filesystem::path resources() { return root; }

	std::filesystem::path textures() { return root / std::filesystem::path { "textures" }; }
	std::filesystem::path fonts() { return root / std::filesystem::path { "fonts" }; }
	std::filesystem::path shaders() { return root / std::filesystem::path { "shaders" }; }
	std::filesystem::path models() { return root / std::filesystem::path { "models" }; }
	std::filesystem::path scenes() { return root / std::filesystem::path { "scene" }; }
	std::filesystem::path scripts() { return root / std::filesystem::path { "scripts" }; }
	std::filesystem::path editor_resources() { return root / std::filesystem::path { "editor" }; }

	std::string read_file(const std::filesystem::path& filename, OpenMode mode)
	{
#ifdef ALABASTER_WINDOWS
		std::ifstream stream(filename, static_cast<int>(mode | OpenMode::AtEnd));
#else
		std::ifstream stream(filename, static_cast<std::ios_base::openmode>(mode | OpenMode::AtEnd));
#endif

		if (!stream) {
			throw AlabasterException("{}", "Could not open filestream to shader file.");
		}

		auto size = stream.tellg();
		verify(size > 0, "Size of file must be greater than zero.");

		std::vector<char> buffer;
		buffer.resize(size);

		stream.seekg(0);

		stream.read(buffer.data(), size);

		return std::string(buffer.begin(), buffer.end());
	}

	std::string read_file(std::filesystem::path&& filename, OpenMode mode) { return IO::read_file(filename, mode); }

	bool exists(const std::filesystem::path& path) { return std::filesystem::exists(path) || std::filesystem::exists(resources() / path); }

	bool is_file(const std::filesystem::path& path)
	{
		auto regular_file = std::filesystem::is_regular_file(path) || std::filesystem::is_regular_file(resources() / path);
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

	std::optional<std::filesystem::path> get_resource_root()
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
				throw AlabasterException("Your CWD is: {}, and Alabaster could not find the 'app' directory there.", cwd.string());
			}

			// We might have app or resources

			if (!app_shaders_exists && !resources_shaders_exists) {
				throw AlabasterException("Your CWD is: {}, and Alabaster could not find the 'app/shaders' directory there.", cwd.string());
			}

			if (!app_models_exists && !resources_models_exists) {
				throw AlabasterException("Your CWD is: {}, and Alabaster could not find the 'app/models' directory there.", cwd.string());
			}

			if (!app_textures_exists && !resources_textures_exists) {
				throw AlabasterException("Your CWD is: {}, and Alabaster could not find the 'app/textures' directory there.", cwd.string());
			}

			if (app_dir_exists) {
				return std::filesystem::path { "app" };
			}
			if (resources_dir_exists) {
				return std::filesystem::path { "resources" };
			}

			throw AlabasterException("Something really strange happened.");
		};

		return sanity_checks();
	}

} // namespace Alabaster::IO
