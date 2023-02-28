#include "panels/SceneEntitiesPanel.hpp"

#include "component/Component.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_internal.h>

namespace App {

	static float font_size = 11.0f;
	static float frame_padding = 0.5f;

	static void draw_vec3_control(const std::string& label, glm::vec3& values, float reset_value = 0.0f, float column_width = 100.0f)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto bold_font = io.Fonts->Fonts[0];

		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, column_width);
		ImGui::Text("%s", label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2 { 0, 0 });

		float line_height = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 button_size = { line_height + 3.0f, line_height };

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4 { 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4 { 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4 { 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushFont(bold_font);
		if (ImGui::Button("X", button_size))
			values.x = reset_value;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4 { 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4 { 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4 { 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushFont(bold_font);
		if (ImGui::Button("Y", button_size))
			values.y = reset_value;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4 { 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4 { 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4 { 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushFont(bold_font);
		if (ImGui::Button("Z", button_size))
			values.z = reset_value;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();

		ImGui::Columns(1);

		ImGui::PopID();
	}

	static bool color_picker(const char* label, glm::vec3& col)
	{
		static constexpr float hue_picker_width = 20.0f;
		static constexpr float crosshair_size = 7.0f;
		static constexpr ImVec2 sv_picker_size = ImVec2(200, 200);

		ImColor color(col[0], col[1], col[2]);
		bool value_changed = false;

		ImDrawList* draw_list = ImGui::GetWindowDrawList();

		ImVec2 picker_pos = ImGui::GetCursorScreenPos();

		ImColor colors[] = { ImColor(255, 0, 0), ImColor(255, 255, 0), ImColor(0, 255, 0), ImColor(0, 255, 255), ImColor(0, 0, 255),
			ImColor(255, 0, 255), ImColor(255, 0, 0) };

		for (int i = 0; i < 6; ++i) {
			draw_list->AddRectFilledMultiColor(ImVec2(picker_pos.x + sv_picker_size.x + 10, picker_pos.y + i * (sv_picker_size.y / 6)),
				ImVec2(picker_pos.x + sv_picker_size.x + 10 + hue_picker_width, picker_pos.y + (i + 1) * (sv_picker_size.y / 6)), colors[i],
				colors[i], colors[i + 1], colors[i + 1]);
		}

		float hue, saturation, value;
		ImGui::ColorConvertRGBtoHSV(color.Value.x, color.Value.y, color.Value.z, hue, saturation, value);

		draw_list->AddLine(ImVec2(picker_pos.x + sv_picker_size.x + 8, picker_pos.y + hue * sv_picker_size.y),
			ImVec2(picker_pos.x + sv_picker_size.x + 12 + hue_picker_width, picker_pos.y + hue * sv_picker_size.y), ImColor(255, 255, 255));

		{
			const int step = 5;
			ImVec2 pos = ImVec2(0, 0);

			ImVec4 c00(1, 1, 1, 1);
			ImVec4 c10(1, 1, 1, 1);
			ImVec4 c01(1, 1, 1, 1);
			ImVec4 c11(1, 1, 1, 1);
			for (int y = 0; y < step; y++) {
				for (int x = 0; x < step; x++) {
					float s0 = (float)x / (float)step;
					float s1 = (float)(x + 1) / (float)step;
					float v0 = 1.0 - (float)(y) / (float)step;
					float v1 = 1.0 - (float)(y + 1) / (float)step;

					ImGui::ColorConvertHSVtoRGB(hue, s0, v0, c00.x, c00.y, c00.z);
					ImGui::ColorConvertHSVtoRGB(hue, s1, v0, c10.x, c10.y, c10.z);
					ImGui::ColorConvertHSVtoRGB(hue, s0, v1, c01.x, c01.y, c01.z);
					ImGui::ColorConvertHSVtoRGB(hue, s1, v1, c11.x, c11.y, c11.z);

					draw_list->AddRectFilledMultiColor(ImVec2(picker_pos.x + pos.x, picker_pos.y + pos.y),
						ImVec2(picker_pos.x + pos.x + sv_picker_size.x / step, picker_pos.y + pos.y + sv_picker_size.y / step),
						ImGui::ColorConvertFloat4ToU32(c00), ImGui::ColorConvertFloat4ToU32(c10), ImGui::ColorConvertFloat4ToU32(c11),
						ImGui::ColorConvertFloat4ToU32(c01));

					pos.x += sv_picker_size.x / step;
				}
				pos.x = 0;
				pos.y += sv_picker_size.y / step;
			}
		}

		float x = saturation * sv_picker_size.x;
		float y = (1 - value) * sv_picker_size.y;
		ImVec2 p(picker_pos.x + x, picker_pos.y + y);
		draw_list->AddLine(ImVec2(p.x - crosshair_size, p.y), ImVec2(p.x - 2, p.y), ImColor(255, 255, 255));
		draw_list->AddLine(ImVec2(p.x + crosshair_size, p.y), ImVec2(p.x + 2, p.y), ImColor(255, 255, 255));
		draw_list->AddLine(ImVec2(p.x, p.y + crosshair_size), ImVec2(p.x, p.y + 2), ImColor(255, 255, 255));
		draw_list->AddLine(ImVec2(p.x, p.y - crosshair_size), ImVec2(p.x, p.y - 2), ImColor(255, 255, 255));

		ImGui::InvisibleButton("saturation_value_selector", sv_picker_size);

		if (ImGui::IsItemActive() && ImGui::GetIO().MouseDown[0]) {
			ImVec2 mouse_pos_in_canvas = ImVec2(ImGui::GetIO().MousePos.x - picker_pos.x, ImGui::GetIO().MousePos.y - picker_pos.y);

			/**/ if (mouse_pos_in_canvas.x < 0)
				mouse_pos_in_canvas.x = 0;
			else if (mouse_pos_in_canvas.x >= sv_picker_size.x - 1)
				mouse_pos_in_canvas.x = sv_picker_size.x - 1;

			/**/ if (mouse_pos_in_canvas.y < 0)
				mouse_pos_in_canvas.y = 0;
			else if (mouse_pos_in_canvas.y >= sv_picker_size.y - 1)
				mouse_pos_in_canvas.y = sv_picker_size.y - 1;

			value = 1 - (mouse_pos_in_canvas.y / (sv_picker_size.y - 1));
			saturation = mouse_pos_in_canvas.x / (sv_picker_size.x - 1);
			value_changed = true;
		}

		ImGui::SetCursorScreenPos(ImVec2(picker_pos.x + sv_picker_size.x + 10, picker_pos.y));
		ImGui::InvisibleButton("hue_selector", ImVec2(hue_picker_width, sv_picker_size.y));

		if ((ImGui::IsItemHovered() || ImGui::IsItemActive()) && ImGui::GetIO().MouseDown[0]) {
			ImVec2 mouse_pos_in_canvas = ImVec2(ImGui::GetIO().MousePos.x - picker_pos.x, ImGui::GetIO().MousePos.y - picker_pos.y);

			/* Previous horizontal bar will represent hue=1 (bottom) as hue=0 (top). Since both colors are red, we clamp at (-2, above edge) to avoid
			 * visual continuities */
			/**/ if (mouse_pos_in_canvas.y < 0)
				mouse_pos_in_canvas.y = 0;
			else if (mouse_pos_in_canvas.y >= sv_picker_size.y - 2)
				mouse_pos_in_canvas.y = sv_picker_size.y - 2;

			hue = mouse_pos_in_canvas.y / (sv_picker_size.y - 1);
			value_changed = true;
		}

		color = ImColor::HSV(hue > 0 ? hue : 1e-6, saturation > 0 ? saturation : 1e-6, value > 0 ? value : 1e-6);
		col[0] = color.Value.x;
		col[1] = color.Value.y;
		col[2] = color.Value.z;
		return value_changed || ImGui::ColorEdit3(label, glm::value_ptr(col));
	}

	static bool color_picker(const char* label, glm::vec4& col)
	{
		static constexpr float hue_picker_width = 20.0f;
		static constexpr float crosshair_size = 7.0f;
		static constexpr ImVec2 sv_picker_size = ImVec2(200, 200);

		ImColor color(col[0], col[1], col[2], col[3]);
		bool value_changed = false;

		ImDrawList* draw_list = ImGui::GetWindowDrawList();

		ImVec2 picker_pos = ImGui::GetCursorScreenPos();

		ImColor colors[] = { ImColor(255, 0, 0), ImColor(255, 255, 0), ImColor(0, 255, 0), ImColor(0, 255, 255), ImColor(0, 0, 255),
			ImColor(255, 0, 255), ImColor(255, 0, 0) };

		for (int i = 0; i < 6; ++i) {
			draw_list->AddRectFilledMultiColor(ImVec2(picker_pos.x + sv_picker_size.x + 10, picker_pos.y + i * (sv_picker_size.y / 6)),
				ImVec2(picker_pos.x + sv_picker_size.x + 10 + hue_picker_width, picker_pos.y + (i + 1) * (sv_picker_size.y / 6)), colors[i],
				colors[i], colors[i + 1], colors[i + 1]);
		}

		float hue, saturation, value;
		ImGui::ColorConvertRGBtoHSV(color.Value.x, color.Value.y, color.Value.z, hue, saturation, value);

		draw_list->AddLine(ImVec2(picker_pos.x + sv_picker_size.x + 8, picker_pos.y + hue * sv_picker_size.y),
			ImVec2(picker_pos.x + sv_picker_size.x + 12 + hue_picker_width, picker_pos.y + hue * sv_picker_size.y), ImColor(255, 255, 255));

		{
			const int step = 5;
			ImVec2 pos = ImVec2(0, 0);

			ImVec4 c00(1, 1, 1, 1);
			ImVec4 c10(1, 1, 1, 1);
			ImVec4 c01(1, 1, 1, 1);
			ImVec4 c11(1, 1, 1, 1);
			for (int y = 0; y < step; y++) {
				for (int x = 0; x < step; x++) {
					float s0 = (float)x / (float)step;
					float s1 = (float)(x + 1) / (float)step;
					float v0 = 1.0 - (float)(y) / (float)step;
					float v1 = 1.0 - (float)(y + 1) / (float)step;

					ImGui::ColorConvertHSVtoRGB(hue, s0, v0, c00.x, c00.y, c00.z);
					ImGui::ColorConvertHSVtoRGB(hue, s1, v0, c10.x, c10.y, c10.z);
					ImGui::ColorConvertHSVtoRGB(hue, s0, v1, c01.x, c01.y, c01.z);
					ImGui::ColorConvertHSVtoRGB(hue, s1, v1, c11.x, c11.y, c11.z);

					draw_list->AddRectFilledMultiColor(ImVec2(picker_pos.x + pos.x, picker_pos.y + pos.y),
						ImVec2(picker_pos.x + pos.x + sv_picker_size.x / step, picker_pos.y + pos.y + sv_picker_size.y / step),
						ImGui::ColorConvertFloat4ToU32(c00), ImGui::ColorConvertFloat4ToU32(c10), ImGui::ColorConvertFloat4ToU32(c11),
						ImGui::ColorConvertFloat4ToU32(c01));

					pos.x += sv_picker_size.x / step;
				}
				pos.x = 0;
				pos.y += sv_picker_size.y / step;
			}
		}

		float x = saturation * sv_picker_size.x;
		float y = (1 - value) * sv_picker_size.y;
		ImVec2 p(picker_pos.x + x, picker_pos.y + y);
		draw_list->AddLine(ImVec2(p.x - crosshair_size, p.y), ImVec2(p.x - 2, p.y), ImColor(255, 255, 255));
		draw_list->AddLine(ImVec2(p.x + crosshair_size, p.y), ImVec2(p.x + 2, p.y), ImColor(255, 255, 255));
		draw_list->AddLine(ImVec2(p.x, p.y + crosshair_size), ImVec2(p.x, p.y + 2), ImColor(255, 255, 255));
		draw_list->AddLine(ImVec2(p.x, p.y - crosshair_size), ImVec2(p.x, p.y - 2), ImColor(255, 255, 255));

		ImGui::InvisibleButton("saturation_value_selector", sv_picker_size);

		if (ImGui::IsItemActive() && ImGui::GetIO().MouseDown[0]) {
			ImVec2 mouse_pos_in_canvas = ImVec2(ImGui::GetIO().MousePos.x - picker_pos.x, ImGui::GetIO().MousePos.y - picker_pos.y);

			/**/ if (mouse_pos_in_canvas.x < 0)
				mouse_pos_in_canvas.x = 0;
			else if (mouse_pos_in_canvas.x >= sv_picker_size.x - 1)
				mouse_pos_in_canvas.x = sv_picker_size.x - 1;

			/**/ if (mouse_pos_in_canvas.y < 0)
				mouse_pos_in_canvas.y = 0;
			else if (mouse_pos_in_canvas.y >= sv_picker_size.y - 1)
				mouse_pos_in_canvas.y = sv_picker_size.y - 1;

			value = 1 - (mouse_pos_in_canvas.y / (sv_picker_size.y - 1));
			saturation = mouse_pos_in_canvas.x / (sv_picker_size.x - 1);
			value_changed = true;
		}

		ImGui::SetCursorScreenPos(ImVec2(picker_pos.x + sv_picker_size.x + 10, picker_pos.y));
		ImGui::InvisibleButton("hue_selector", ImVec2(hue_picker_width, sv_picker_size.y));

		if ((ImGui::IsItemHovered() || ImGui::IsItemActive()) && ImGui::GetIO().MouseDown[0]) {
			ImVec2 mouse_pos_in_canvas = ImVec2(ImGui::GetIO().MousePos.x - picker_pos.x, ImGui::GetIO().MousePos.y - picker_pos.y);

			/* Previous horizontal bar will represent hue=1 (bottom) as hue=0 (top). Since both colors are red, we clamp at (-2, above edge) to avoid
			 * visual continuities */
			/**/ if (mouse_pos_in_canvas.y < 0)
				mouse_pos_in_canvas.y = 0;
			else if (mouse_pos_in_canvas.y >= sv_picker_size.y - 2)
				mouse_pos_in_canvas.y = sv_picker_size.y - 2;

			hue = mouse_pos_in_canvas.y / (sv_picker_size.y - 1);
			value_changed = true;
		}

		color = ImColor::HSV(hue > 0 ? hue : 1e-6, saturation > 0 ? saturation : 1e-6, value > 0 ? value : 1e-6);
		col[0] = color.Value.x;
		col[1] = color.Value.y;
		col[2] = color.Value.z;
		col[3] = color.Value.w;
		return value_changed || ImGui::ColorEdit4(label, glm::value_ptr(col));
	}

	static void draw_four_component_vector(const std::string& label, auto&& values, float reset_value = 0.0f, float column_width = 100.0f)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto bold_font = io.Fonts->Fonts[0];

		ImGui::PushID(label.c_str());

		{
			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, column_width);
			ImGui::Text("%s", label.c_str());
			ImGui::NextColumn();
		}

		{
			ImGui::PushMultiItemsWidths(4, ImGui::CalcItemWidth());
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2 { 0, 0 });
		}

		float line_height = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 button_size = { line_height + 3.0f, line_height };

		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4 { 0.8f, 0.1f, 0.15f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4 { 0.9f, 0.2f, 0.2f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4 { 0.8f, 0.1f, 0.15f, 1.0f });
			ImGui::PushFont(bold_font);
			if (ImGui::Button("X", button_size))
				values.x = reset_value;
			ImGui::PopFont();
			ImGui::PopStyleColor(3);

			ImGui::SameLine();
			ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
			ImGui::PopItemWidth();
			ImGui::SameLine();
		}

		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4 { 0.2f, 0.7f, 0.2f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4 { 0.3f, 0.8f, 0.3f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4 { 0.2f, 0.7f, 0.2f, 1.0f });
			ImGui::PushFont(bold_font);
			if (ImGui::Button("Y", button_size))
				values.y = reset_value;
			ImGui::PopFont();
			ImGui::PopStyleColor(3);

			ImGui::SameLine();
			ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
			ImGui::PopItemWidth();
			ImGui::SameLine();
		}

		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4 { 0.1f, 0.25f, 0.8f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4 { 0.2f, 0.35f, 0.9f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4 { 0.1f, 0.25f, 0.8f, 1.0f });
			ImGui::PushFont(bold_font);
			if (ImGui::Button("Z", button_size))
				values.z = reset_value;
			ImGui::PopFont();
			ImGui::PopStyleColor(3);

			ImGui::SameLine();
			ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
			ImGui::PopItemWidth();
			ImGui::SameLine();
		}

		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4 { 0.2f, 0.7f, 0.2f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4 { 0.3f, 0.8f, 0.3f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4 { 0.2f, 0.7f, 0.2f, 1.0f });
			ImGui::PushFont(bold_font);
			if (ImGui::Button("W", button_size))
				values.w = reset_value;
			ImGui::PopFont();
			ImGui::PopStyleColor(3);

			ImGui::SameLine();
			ImGui::DragFloat("##W", &values.w, 0.1f, 0.0f, 0.0f, "%.2f");
			ImGui::PopItemWidth();
		}

		ImGui::PopStyleVar();

		ImGui::Columns(1);

		ImGui::PopID();
	}

	template <SceneSystem::Component::IsComponent T>
	static void draw_component(SceneSystem::Entity& entity, const std::string& name, auto&& ui_function)
	{
		const ImGuiTreeNodeFlags tree_node_flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth
			| ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
		if (entity.has_component<T>()) {
			auto& component = entity.get_component<T>();
			ImVec2 content_region_available = ImGui::GetContentRegionAvail();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2 { 4, 4 });
			float line_height = font_size + frame_padding * 2.0f;
			ImGui::Separator();

			const auto leaf_id = (const void*)&entity.get_component<SceneSystem::Component::ID>().identifier;
			bool open = ImGui::TreeNodeEx(leaf_id, tree_node_flags, "%s", name.c_str());
			ImGui::PopStyleVar();
			ImGui::SameLine(content_region_available.x - line_height * 0.5f);
			if (ImGui::Button("+", ImVec2 { line_height, line_height })) {
				ImGui::OpenPopup("ComponentSettings");
			}

			bool remove_component = false;
			if (ImGui::BeginPopup("Component Settings")) {
				if (ImGui::MenuItem("Remove component"))
					remove_component = true;

				ImGui::EndPopup();
			}

			if (open) {
				ui_function(component);
				ImGui::TreePop();
			}

			if (remove_component)
				entity.remove_component<T>();
		}
	}

	void SceneEntitiesPanel::draw_components(SceneSystem::Entity& entity)
	{
		if (entity.has_component<SceneSystem::Component::Tag>()) {
			auto& tag = entity.get_tag().tag;

			std::string buffer;
			buffer.reserve(500);
			std::memset(buffer.data(), 0, sizeof(buffer));
			tag.copy(buffer.data(), 499);
			if (ImGui::InputText("##Tag", buffer.data(), sizeof(buffer))) {
				tag = std::string(buffer);
			}
		}

		ImGui::SameLine();
		ImGui::PushItemWidth(-1);

		if (ImGui::Button("Add Component"))
			ImGui::OpenPopup("AddComponent");

		if (ImGui::BeginPopup("AddComponent")) {
			display_add_component_entry<SceneSystem::Component::Camera>("Camera");
			display_add_component_entry<SceneSystem::Component::Texture>("Texture");

			ImGui::EndPopup();
		}

		ImGui::PopItemWidth();

		draw_component<SceneSystem::Component::Transform>(entity, "Transform", [](SceneSystem::Component::Transform& component) {
			draw_vec3_control("Translation", component.position);
			draw_four_component_vector("Rotation", component.rotation);
			draw_vec3_control("Scale", component.scale, 1.0f);
		});

		draw_component<SceneSystem::Component::Texture>(
			entity, "Texture", [](SceneSystem::Component::Texture& component) { color_picker("Colour", component.colour); });
	}

	void SceneEntitiesPanel::draw_entity_node(SceneSystem::Entity& entity)
	{
		const auto& tag = entity.get_tag().tag;

		ImGuiTreeNodeFlags flags = ((selected_entity == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
		const auto& id = entity.get_component<SceneSystem::Component::ID>();
		bool opened = ImGui::TreeNodeEx(id.identifier.as_bytes().data(), flags, "%s", tag.c_str());
		if (ImGui::IsItemClicked()) {
			selected_entity = entity;
		}

		bool entity_deleted = false;
		if (ImGui::BeginPopupContextItem()) {
			if (ImGui::MenuItem("Delete Entity"))
				entity_deleted = true;

			ImGui::EndPopup();
		}

		if (opened) {
			ImGuiTreeNodeFlags opened_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
			const auto leaf_id = (const char*)&entity.get_component<SceneSystem::Component::ID>().identifier;
			bool was_opened = ImGui::TreeNodeEx(leaf_id, opened_flags, "%s", tag.c_str());
			if (was_opened)
				ImGui::TreePop();
			ImGui::TreePop();
		}

		if (entity_deleted) {
			scene->delete_entity(entity);
			if (selected_entity == entity)
				selected_entity = {};
		}
	}

	void SceneEntitiesPanel::on_update(float ts) { }

	void SceneEntitiesPanel::ui(float ts)
	{
		ImGui::Begin("Scene Hierarchy");

		scene->for_each_entity([this](auto entity_id) {
			auto entity = scene->create_entity(entity_id);
			draw_entity_node(entity);
		});

		// Right-click on blank space
		if (ImGui::BeginPopupContextWindow(0, 1)) {
			if (ImGui::MenuItem("Create Empty Entity"))
				scene->create_entity("Empty Entity");

			ImGui::EndPopup();
		}

		ImGui::End();

		ImGui::Begin("Properties");
		if (selected_entity.is_valid()) {
			draw_components(selected_entity);
		}

		ImGui::End();
	}

	void SceneEntitiesPanel::on_event(Alabaster::Event& event) { }

} // namespace App
