//
// Created by Edwin Carlsson on 2022-12-17.
//

#pragma once

#include "graphics/Texture.hpp"
#include "Panel.hpp"

#include <filesystem>

namespace App {

	class DirectoryContentPanel : public Panel {
	public:
		DirectoryContentPanel(std::filesystem::path initial = "resources");
		~DirectoryContentPanel() override = default;

		bool traverse_down(const std::filesystem::path into_directory, bool force_reload = false);
		bool traverse_up(bool force_reload = false);

		void on_update(float ts) override;
		void ui(float ts) override;
		void on_event(Alabaster::Event& event) override;
		void on_init() override;
		void on_destroy() override;

		auto& get_current() { return current; }
		std::vector<std::filesystem::path> get_files_in_directory(const std::filesystem::path& for_path);

	private:
		const std::filesystem::path initial;
		std::filesystem::path current {};
		std::filesystem::path previous { initial };
		std::uint32_t depth_from_initial { 0 };
		bool changed { false };

		const Alabaster::Texture& directory_icon;
		const Alabaster::Texture& file_icon;

		std::vector<std::filesystem::path> current_directory_content;
	};

} // namespace App