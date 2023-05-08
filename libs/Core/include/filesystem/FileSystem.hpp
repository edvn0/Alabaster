#pragma once

#include <filesystem>
#include <optional>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace Alabaster::FileSystem {
	void init_with_cwd(const std::filesystem::path& path);

	std::filesystem::path fonts();
	std::filesystem::path shaders();
	std::filesystem::path models();
	std::filesystem::path scenes();
	std::filesystem::path scripts();
	std::filesystem::path textures();

	std::filesystem::path resources();
	std::filesystem::path executable();
	std::filesystem::path editor_resources();

	/// @brief Find files matching the regex *<name>*.
	/// @param name: name of the file
	/// @param directory: name of the directory
	/// @return vector of files with matching names.
	std::vector<std::filesystem::path> find_with_name(const std::string_view name, std::filesystem::path directory);

	std::vector<std::filesystem::path> find_fonts_with_name(const std::string_view name);
	std::vector<std::filesystem::path> find_shaders_with_name(const std::string_view name);
	std::vector<std::filesystem::path> find_models_with_name(const std::string_view name);
	std::vector<std::filesystem::path> find_scenes_with_name(const std::string_view name);
	std::vector<std::filesystem::path> find_scripts_with_name(const std::string_view name);
	std::vector<std::filesystem::path> find_textures_with_name(const std::string_view name);

	std::optional<std::filesystem::path> find_file(
		const std::string& name, const std::string& directory, const std::unordered_set<std::string>& extensions);

	template <typename Path = std::filesystem::path> std::filesystem::path shader(const Path& path)
	{
		return FileSystem::shaders() / std::filesystem::path { path };
	}
	template <typename Path = std::filesystem::path> std::filesystem::path model(const Path& path)
	{
		return FileSystem::models() / std::filesystem::path { path };
	}
	template <typename Path = std::filesystem::path> std::filesystem::path texture(const Path& path)
	{
		return FileSystem::textures() / std::filesystem::path { path };
	}
	template <typename Path = std::filesystem::path> std::filesystem::path font(const Path& path)
	{
		return FileSystem::fonts() / std::filesystem::path { path };
	}
	template <typename Path = std::filesystem::path> std::filesystem::path scene(const Path& path)
	{
		return FileSystem::scenes() / std::filesystem::path { path };
	}
	template <typename Path = std::filesystem::path> std::filesystem::path script(const Path& path)
	{
		return FileSystem::scripts() / std::filesystem::path { path };
	}

	template <typename Output = std::string, typename Hasher = std::hash<std::string>, typename Equality = std::equal_to<std::string>,
		bool Recursive = false>
	std::vector<Output> in_directory(const std::filesystem::path& path, std::unordered_set<std::string, Hasher, Equality> extensions, bool sorted)
	{
		static constexpr auto entry_to_string = [](const auto& input) { return input.path().extension().string(); };
		static auto should_include = [&extensions](const auto& input) {
			if (extensions.contains("*")) {
				return true;
			}

			return extensions.contains(entry_to_string(input));
		};
		static auto add_to_output = [](const auto& fd, auto& output) mutable {
			if constexpr (std::is_same_v<Output, std::filesystem::path>) {
				output.push_back(static_cast<Output>(fd.path().string()));
			} else {
				output.push_back(static_cast<Output>(fd.path().string()));
			};
		};

		std::vector<Output> output;
		if constexpr (Recursive) {
			for (const auto& fd : std::filesystem::recursive_directory_iterator { path }) {
				if (should_include(fd)) {
					add_to_output(fd, output);
				}
			}
		} else {
			for (const auto& fd : std::filesystem::directory_iterator { path }) {
				if (should_include(fd)) {
					add_to_output(fd, output);
				}
			}
		}

		if (sorted) {
			std::sort(output.begin(), output.end());
		}

		return output;
	}

} // namespace Alabaster::FileSystem
