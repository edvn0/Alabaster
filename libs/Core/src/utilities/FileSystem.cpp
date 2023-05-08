//
// Created by Edwin Carlsson on 2022-12-12.
//

#include "av_pch.hpp"

#include "filesystem/FileSystem.hpp"

#include "core/Logger.hpp"

namespace Alabaster::FileSystem {

	std::filesystem::path root;
	void init_with_cwd(const std::filesystem::path& path) { root = path; }

	std::filesystem::path resources() { return root; }
	std::filesystem::path executable() { return root / std::filesystem::path { ".." }; }

	std::filesystem::path textures() { return root / std::filesystem::path { "textures" }; }
	std::filesystem::path fonts() { return root / std::filesystem::path { "fonts" }; }
	std::filesystem::path shaders() { return root / std::filesystem::path { "shaders" }; }
	std::filesystem::path models() { return root / std::filesystem::path { "models" }; }
	std::filesystem::path scenes() { return root / std::filesystem::path { "scene" }; }
	std::filesystem::path scripts() { return root / std::filesystem::path { "scripts" }; }
	std::filesystem::path editor_resources() { return root / std::filesystem::path { "editor" }; }

	std::vector<std::filesystem::path> find_fonts_with_name(const std::string_view name) { return find_with_name(name, FileSystem::fonts()); }
	std::vector<std::filesystem::path> find_shaders_with_name(const std::string_view name) { return find_with_name(name, FileSystem::shaders()); }
	std::vector<std::filesystem::path> find_models_with_name(const std::string_view name) { return find_with_name(name, FileSystem::models()); }
	std::vector<std::filesystem::path> find_scenes_with_name(const std::string_view name) { return find_with_name(name, FileSystem::scenes()); }
	std::vector<std::filesystem::path> find_scripts_with_name(const std::string_view name) { return find_with_name(name, FileSystem::scripts()); }
	std::vector<std::filesystem::path> find_textures_with_name(const std::string_view name) { return find_with_name(name, FileSystem::textures()); }

	std::optional<std::filesystem::path> find_file(
		const std::string& name, const std::string& directory, const std::unordered_set<std::string>& extensions)
	{
		for (const auto& v : extensions)
			Alabaster::Log::info("[FileSystem] Extensions: {}", v);

		for (const auto& entry : std::filesystem::directory_iterator(directory)) {
			const auto filename = entry.path().filename();
			const auto extension = filename.extension().string();
			Alabaster::Log::info("[FileSystem] {} - {}", filename.string(), extension);
			if (!extensions.contains(extension))
				continue;

			const auto with_extension = name + extension;
			Alabaster::Log::info("[FileSystem] With extension? {} - {}", with_extension, filename.string());

			if (with_extension == filename)
				return { entry.path() };
		}
		return {};
	}

	std::vector<std::filesystem::path> find_with_name(const std::string_view name, std::filesystem::path directory)
	{
		std::regex regex(std::string { name });

		std::vector<std::filesystem::path> output;
		for (const auto& entry : std::filesystem::directory_iterator(directory)) {
			const auto filename = entry.path().filename();
			if (!std::regex_search(filename.string(), regex))
				continue;
			output.push_back(entry.path());
		}
		return output;
	}

} // namespace Alabaster::FileSystem
