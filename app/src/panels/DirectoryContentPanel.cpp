//
// Created by Edwin Carlsson on 2022-12-17.
//

#include "panels/DirectoryContentPanel.hpp"

#include "Alabaster.hpp"
#include "AssetManager.hpp"
#include "platform/Vulkan/ImageUtilities.hpp"
#include "utilities/FileInputOutput.hpp"

#ifdef ALABASTER_WINDOWS
#include <wchar.h>
#endif

namespace App {

	struct path_hash {
		std::size_t operator()(const std::filesystem::path& path) const { return std::hash<std::string> {}(canonical(path).string()); }
	};

	std::unordered_map<std::filesystem::path, std::vector<std::filesystem::path>, path_hash> path_and_content_cache;

	DirectoryContentPanel::DirectoryContentPanel(std::filesystem::path initial)
		: initial(initial)
		, current(initial)
		, directory_icon(*AssetManager::asset<Alabaster::Texture>("directory_icon.png"))
		, file_icon(*AssetManager::asset<Alabaster::Texture>("file_icon.png"))
	{
	}

	bool DirectoryContentPanel::traverse_down(const std::filesystem::path into_directory, bool force_reload)
	{
		bool could = true;

		if (!Alabaster::IO::exists(into_directory)) {
			could = false;
		}

		std::swap(previous, current);
		current = into_directory;

		if (current != previous) {
			changed = true;
		}

		if (!path_and_content_cache.contains(current)) {
			path_and_content_cache[current] = get_files_in_directory(current);
		} else {
			if (force_reload) {
				path_and_content_cache[current] = get_files_in_directory(current);
			}
		}

		if (could)
			depth_from_initial++;
		return could;
	}

	bool DirectoryContentPanel::traverse_up(bool force_reload)
	{
		bool could = true;

		if (!Alabaster::IO::exists(previous)) {
			could = false;
		}

		if (current == initial) {
			previous = initial;
		}

		std::swap(current, previous);

		if (current != previous) {
			changed = true;
		}

		if (!path_and_content_cache.contains(current)) {
			path_and_content_cache[current] = get_files_in_directory(current);
		} else {
			if (force_reload) {
				path_and_content_cache[current] = get_files_in_directory(current);
			}
		}

		if (could)
			depth_from_initial--;
		return could;
	}

	void DirectoryContentPanel::on_update(float ts)
	{
		if (changed) {
			current_directory_content = path_and_content_cache[current];
			changed = false;
		}
	}

	void DirectoryContentPanel::ui(float ts)
	{
		ImGui::Begin("Directory Content");

		if (ImGui::ArrowButton("GoUpOneLevel", ImGuiDir_Left)) {
			traverse_up();
		}

		static float padding = 16.0f;
		static float thumbnail_size = 128.0f;
		float cell_size = thumbnail_size + padding;

		float panel_width = ImGui::GetContentRegionAvail().x;
		auto column_count = static_cast<int>(panel_width / cell_size);
		if (column_count < 1) {
			column_count = 1;
		}

		ImGui::Columns(column_count, 0, false);

		for (auto& directory_entry : current_directory_content) {
			const auto& path = directory_entry;
			std::string filename_string = path.filename().string();

			const auto& data = filename_string.data();
			ImGui::PushID(data);
			const auto& icon = is_directory(path) ? directory_icon : file_icon;
			const auto is_image = Alabaster::Utilities::is_image_by_extension<std::filesystem::path>()(path);
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			const auto& image_info = icon.get_descriptor_info();
			Alabaster::UI::image(image_info, { thumbnail_size, thumbnail_size });

#ifdef ALABASTER_MACOS
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
				auto relative_path = std::filesystem::relative(path, Alabaster::IO::resources());
				const char* item_path = relative_path.c_str();
				const auto size = (std::strlen(item_path) + 1) * sizeof(char);
				ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", item_path, size);
				ImGui::EndDragDropSource();
			}
#else
			if (ImGui::BeginDragDropSource()) {
				auto relative_path = std::filesystem::relative(path, g_AssetPath);
				const wchar_t* itemPath = relative_path.c_str();
				ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t));
				ImGui::EndDragDropSource();
			}
#endif

			ImGui::PopStyleColor();
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
				if (is_directory(directory_entry)) {
					traverse_down(directory_entry);
				}
			}
			ImGui::TextWrapped("%s", filename_string.c_str());

			ImGui::NextColumn();

			ImGui::PopID();
		}

		ImGui::Columns(1);

		ImGui::End();
	}

	void DirectoryContentPanel::on_event(Alabaster::Event& event) { }

	void DirectoryContentPanel::on_init()
	{
		path_and_content_cache[current] = get_files_in_directory(current);
		current_directory_content = path_and_content_cache[current];
	}

	void DirectoryContentPanel::on_destroy() { }

	std::vector<std::filesystem::path> DirectoryContentPanel::get_files_in_directory(const std::filesystem::path& for_path)
	{
		std::vector<std::filesystem::path> found;
		for (const auto& entry : std::filesystem::directory_iterator { for_path }) {
			found.push_back(entry.path());
		}
		return found;
	}

} // namespace App
