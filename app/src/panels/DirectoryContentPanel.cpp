//
// Created by Edwin Carlsson on 2022-12-17.
//

#include "panels/DirectoryContentPanel.hpp"

#include "Alabaster.hpp"
#include "AssetManager.hpp"
#include "graphics/Texture.hpp"
#include "platform/Vulkan/ImageUtilities.hpp"
#include "utilities/FileInputOutput.hpp"

namespace App {

	using namespace std::string_view_literals;

	DirectoryContentPanel::DirectoryContentPanel(const std::filesystem::path& initial)
		: initial(initial)
		, current(initial)
	{
	}

	bool DirectoryContentPanel::traverse_down(const std::filesystem::path& into_directory, bool force_reload)
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

	bool DirectoryContentPanel::can_traverse_up() const { return current != initial && depth_from_initial > 0; }

	void DirectoryContentPanel::ui()
	{
		bool force_reload = false;
		if (Alabaster::Input::all(Alabaster::Key::LeftControl, Alabaster::Key::LeftShift, Alabaster::Key::R)) {
			reload();
			force_reload = true;
		}

		ImGui::Begin("Directory Content");

		if (can_traverse_up() && ImGui::ArrowButton("##GoUpOneLevel", ImGuiDir_Left)) {
			traverse_up(force_reload);
		}

		static float padding = 16.0f;
		static float thumbnail_size = 128.0f;
		float cell_size = thumbnail_size + padding;

		float panel_width = ImGui::GetContentRegionAvail().x;
		auto column_count = static_cast<int>(panel_width / cell_size);
		if (column_count < 1) {
			column_count = 1;
		}

		if (ImGui::BeginTable("DirectoryContent", column_count)) {

			for (const auto& directory_entry : current_directory_content) {
				const auto& path = directory_entry;
				const auto filename = path.filename();
				std::string filename_string = filename.string();

				const auto* data = filename_string.data();
				ImGui::PushID(data);

				const auto is_image = Alabaster::Utilities::is_image_by_extension<std::filesystem::path>()(path);

				if (!is_image) {
					draw_file_or_directory(path, { thumbnail_size, thumbnail_size });
				} else {
					if (icons.contains(filename)) {
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
						const auto& image_info = icons[filename]->get_descriptor_info();
						Alabaster::UI::image(image_info, { thumbnail_size, thumbnail_size });
						ImGui::PopStyleColor();
					} else {
						if (!do_not_try_icons.contains(filename)) {
							try {
								if (icons.size() >= icons_max_size) {
									icons.clear();
								}

								icons[filename] = Alabaster::Texture::from_filename(filename);
							} catch (const Alabaster::AlabasterException&) {
								Alabaster::Log::info("Could not create icon from image {}, will not try to display again.", filename.string());
								do_not_try_icons.emplace(filename);
							}
						} else {
							draw_file_or_directory(path, { thumbnail_size, thumbnail_size });
						}
					}
				}
				Alabaster::UI::drag_drop(path);
				Alabaster::UI::handle_double_click([&directory_entry, force_reload, this] {
					if (is_directory(directory_entry)) {
						traverse_down(directory_entry, force_reload);
					}
				});

				ImGui::TextWrapped("%s", filename_string.c_str());

				ImGui::TableNextColumn();

				ImGui::PopID();
			}
		}

		ImGui::EndTable();

		ImGui::End();
	}

	void DirectoryContentPanel::on_event(Alabaster::Event& event) { }

	void DirectoryContentPanel::initialise(AssetManager::FileWatcher& watcher)
	{
		path_and_content_cache[current] = get_files_in_directory(current);
		current_directory_content = path_and_content_cache[current];

		watcher.on(AssetManager::FileStatuses::All, [this](const AssetManager::FileInformation& file_info) {
			std::filesystem::path file_path { file_info.path };
			if (const auto file_directory_for_modified_file = file_path.parent_path(); file_directory_for_modified_file != current)
				return;

			std::unique_lock lock { mutex };
			reload();
		});
	}

	void DirectoryContentPanel::reload()
	{
		path_and_content_cache[current] = get_files_in_directory(current);
		current_directory_content = path_and_content_cache[current];
	}

	void DirectoryContentPanel::on_destroy()
	{
		current_directory_content.clear();
		path_and_content_cache.clear();
	}

	std::vector<std::filesystem::path> DirectoryContentPanel::get_files_in_directory(const std::filesystem::path& for_path) const
	{
		std::vector<std::filesystem::path> found;
		for (const auto& entry : std::filesystem::directory_iterator { for_path }) {
			found.push_back(entry.path());
		}
		return found;
	}

	void DirectoryContentPanel::draw_file_or_directory(const std::filesystem::path& path, const ImVec2& size)
	{
		if (!file_type_cache.contains(path)) {
			file_type_cache[path] = is_directory(path);
		}
		const auto& icon = file_type_cache[path] ? directory_icon : file_icon;
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		const auto& image_info = icon->get_descriptor_info();
		Alabaster::UI::image(image_info, size);
		ImGui::PopStyleColor();
	}
} // namespace App
