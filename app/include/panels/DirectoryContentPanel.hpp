//
// Created by Edwin Carlsson on 2022-12-17.
//

#pragma once

#include "Panel.hpp"
#include "graphics/Texture.hpp"

#include <AssetManager.hpp>
#include <filesystem>
#include <mutex>

extern "C" {
struct ImVec2;
}

struct path_hash {
	std::size_t operator()(const std::optional<std::filesystem::path>& path) const { return path ? std::filesystem::hash_value(path.value()) : 0; }
};

namespace App {

	class DirectoryContentPanel : public Panel {
	public:
		DirectoryContentPanel(const std::filesystem::path& initial = "resources");
		~DirectoryContentPanel() override = default;

		bool traverse_down(const std::filesystem::path& into_directory, bool force_reload = false);
		bool traverse_up(bool force_reload = false);
		bool can_traverse_up() const;

		void on_update(float ts) override;
		void ui() override;
		void on_event(Alabaster::Event& event) override;
		void initialise(AssetManager::FileWatcher&) override;
		void on_destroy() override;

		auto& get_current() { return current; }
		std::vector<std::filesystem::path> get_files_in_directory(const std::filesystem::path& for_path) const;
		void draw_file_or_directory(const std::filesystem::path& path, const ImVec2& size);
		void register_file_watcher(AssetManager::FileWatcher&);

	private:
		void reload();

		const std::filesystem::path initial;
		std::filesystem::path current {};
		std::filesystem::path previous { initial };
		std::uint32_t depth_from_initial { 0 };
		bool changed { false };

		std::shared_ptr<Alabaster::Texture> directory_icon { AssetManager::asset<Alabaster::Texture>("directory_icon.png") };
		std::shared_ptr<Alabaster::Texture> file_icon { AssetManager::asset<Alabaster::Texture>("file_icon.png") };

		std::mutex mutex;
		std::vector<std::filesystem::path> current_directory_content;

		static constexpr auto icons_max_size = 200;

		std::unordered_map<std::filesystem::path, bool, path_hash> file_type_cache;
		std::unordered_map<std::filesystem::path, std::vector<std::filesystem::path>, path_hash> path_and_content_cache {};
		std::unordered_map<std::filesystem::path, std::shared_ptr<Alabaster::Texture>, path_hash> icons {};
		std::unordered_set<std::filesystem::path, path_hash> do_not_try_icons {};
	};

} // namespace App
