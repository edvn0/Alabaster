#include "AlabasterLayer.hpp"

#include "Alabaster.hpp"
#include "AssetManager.hpp"
#include "component/Component.hpp"
#include "core/Common.hpp"
#include "core/exceptions/AlabasterException.hpp"
#include "core/GUILayer.hpp"
#include "core/Logger.hpp"
#include "entity/Entity.hpp"
#include "graphics/CommandBuffer.hpp"
#include "imgui_internal.h"
#include "panels/DirectoryContentPanel.hpp"
#include "vulkan/vulkan_core.h"

#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <optional>
#include <string.h>
#include <vulkan/vulkan.h>

using namespace Alabaster;
using namespace SceneSystem;
using namespace Component;

static std::uint32_t quads { 1 };
static bool is_dockspace_open { true };

static bool global_imgui_is_blocking { false };

static glm::vec4 pos { -5, 5, 5, 1.0f };
static glm::vec4 col { 255 / 255.0, 153 / 255.0, 51 / 255.0, 255.0f / 255.0 };
static float ambience { 1.0f };

static float font_size = 11.0f;
static float frame_padding = 0.5f;

template <IsComponent T> static void draw_component(Entity& entity, const std::string& name, auto&& ui_function)
{
	const ImGuiTreeNodeFlags tree_node_flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth
		| ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
	if (entity.has_component<T>()) {
		auto& component = entity.get_component<T>();
		ImVec2 content_region_available = ImGui::GetContentRegionAvail();

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2 { 4, 4 });
		float line_height = font_size + frame_padding * 2.0f;
		ImGui::Separator();
		bool open = ImGui::TreeNodeEx((const void*)&entity.get_component<ID>().identifier, tree_node_flags, "%s", name.c_str());
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

	ImColor colors[] = { ImColor(255, 0, 0), ImColor(255, 255, 0), ImColor(0, 255, 0), ImColor(0, 255, 255), ImColor(0, 0, 255), ImColor(255, 0, 255),
		ImColor(255, 0, 0) };

	for (int i = 0; i < 6; ++i) {
		draw_list->AddRectFilledMultiColor(ImVec2(picker_pos.x + sv_picker_size.x + 10, picker_pos.y + i * (sv_picker_size.y / 6)),
			ImVec2(picker_pos.x + sv_picker_size.x + 10 + hue_picker_width, picker_pos.y + (i + 1) * (sv_picker_size.y / 6)), colors[i], colors[i],
			colors[i + 1], colors[i + 1]);
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

	ImColor colors[] = { ImColor(255, 0, 0), ImColor(255, 255, 0), ImColor(0, 255, 0), ImColor(0, 255, 255), ImColor(0, 0, 255), ImColor(255, 0, 255),
		ImColor(255, 0, 0) };

	for (int i = 0; i < 6; ++i) {
		draw_list->AddRectFilledMultiColor(ImVec2(picker_pos.x + sv_picker_size.x + 10, picker_pos.y + i * (sv_picker_size.y / 6)),
			ImVec2(picker_pos.x + sv_picker_size.x + 10 + hue_picker_width, picker_pos.y + (i + 1) * (sv_picker_size.y / 6)), colors[i], colors[i],
			colors[i + 1], colors[i + 1]);
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

bool AlabasterLayer::initialise()
{
	editor_scene = std::make_unique<Scene>();
	editor_scene->initialise();

	panels.push_back(std::make_unique<App::DirectoryContentPanel>(IO::resources()));

	for (const auto& panel : panels) {
		panel->on_init();
	}
	return true;
}

void AlabasterLayer::on_event(Event& e)
{
	for (const auto& panel : panels) {
		panel->on_event(e);
	}

	editor_scene->on_event(e);

	EventDispatcher dispatch(e);
	dispatch.dispatch<MouseScrolledEvent>([](MouseScrolledEvent& event) { return global_imgui_is_blocking; });

	dispatch.dispatch<KeyPressedEvent>([](KeyPressedEvent& key_event) {
		const auto key_code = key_event.get_key_code();
		if (key_code == Key::Escape) {
			Application::the().exit();
			return true;
		}

		if (key_code == Key::F) {
			UI::empty_cache();
			return false;
		}

		if (key_code == Key::G) {
			Logger::cycle_levels();
			return false;
		}

		return false;
	});
}

void AlabasterLayer::update(float ts)
{
	editor_scene->update(ts);
	for (const auto& panel : panels) {
		panel->on_update(ts);
	}
}

void AlabasterLayer::ui() { }

void AlabasterLayer::ui(float ts)
{
	editor_scene->ui(ts);

	static bool persistent = true;
	bool opt_fullscreen = persistent;
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	if (opt_fullscreen) {
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	}

	if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		window_flags |= ImGuiWindowFlags_NoBackground;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("DockSpace", &is_dockspace_open, window_flags);

	ImGui::PopStyleVar();
	{
		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// DockSpace
		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		float min_window_size_x = style.WindowMinSize.x;
		style.WindowMinSize.x = 370.0f;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpacew");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		style.WindowMinSize.x = min_window_size_x;

		if (ImGui::BeginMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("New", "Ctrl+N")) { }
				if (ImGui::MenuItem("Open...", "Ctrl+O")) { }
				if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S")) { }
				if (ImGui::MenuItem("Exit")) {
					Application::the().exit();
				}
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		for (const auto& panel : panels) {
			panel->ui(ts);
		}

		{
			ImGui::Begin("Scene Hierarchy");

			editor_scene->for_each_entity([&](auto entity_id) {
				Entity entity { editor_scene.get(), entity_id };
				draw_entity_node(entity);
			});

			// Right-click on blank space
			if (ImGui::BeginPopupContextWindow(0, 1)) {
				if (ImGui::MenuItem("Create Empty Entity"))
					editor_scene->create_entity("Empty Entity");

				ImGui::EndPopup();
			}

			ImGui::End();

			ImGui::Begin("Properties");
			if (selected_entity) {
				draw_components(selected_entity);
			}

			ImGui::End();
		}

		ImGui::Begin("Stats");
		{
			ImGui::Text("Renderer Stats:");
			std::string name = "None";
			ImGui::Text("Hovered Entity: %s", name.c_str());
			auto frametime = Application::the().frametime();
			ImGui::Text("Frame time: %s", std::to_string(frametime).c_str());
		}
		ImGui::End();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2 { 0, 0 });
		ImGui::Begin("Viewport");
		{
			auto viewport_min_region = ImGui::GetWindowContentRegionMin();
			auto viewport_max_region = ImGui::GetWindowContentRegionMax();
			auto viewport_offset = ImGui::GetWindowPos();
			viewport_bounds[0] = { viewport_min_region.x + viewport_offset.x, viewport_min_region.y + viewport_offset.y };
			viewport_bounds[1] = { viewport_max_region.x + viewport_offset.x, viewport_max_region.y + viewport_offset.y };

			viewport_focused = ImGui::IsWindowFocused();
			viewport_hovered = ImGui::IsWindowHovered();

			global_imgui_is_blocking = viewport_hovered;
			if (viewport_hovered) {
				Application::the().gui_layer().block_events();
			}

			ImVec2 viewport_panel_size = ImGui::GetContentRegionAvail();
			viewport_size = { viewport_panel_size.x, viewport_panel_size.y };

			const auto& img = editor_scene->final_image();
			UI::image(*img, { viewport_size.x, viewport_size.y });

			handle_drag_drop();

			ImGui::End();
		}
		ImGui::PopStyleVar();
	}
	ImGui::End();
}

namespace Filetype {
	enum Filetypes { PNG, TTF, JPEG, JPG, SPV, VERT, FRAG, OBJ };
}

template <Filetype::Filetypes Type> struct handle_filetype {
	void operator()(const std::filesystem::path& path) { Log::info("Filetype handler not implemented for {}", enum_name(Type)); };
};

template <> struct handle_filetype<Filetype::Filetypes::PNG> {
	void operator()(const std::filesystem::path& path)
	{
		if (path.extension() != ".png")
			return;

		TextureProperties props;
		const auto img = Alabaster::Texture::from_filename(path, props);
		(void)img;
		return;
	}
};

void AlabasterLayer::handle_drag_drop()
{
	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
			const char* path = static_cast<const char*>(payload->Data);
			const auto fp = std::filesystem::path { path };

			const auto filename = fp.filename();
			const auto extension = filename.extension();

			try {
				handle_filetype<Filetype::Filetypes::PNG>()(filename);
				handle_filetype<Filetype::Filetypes::TTF>()(filename);
				handle_filetype<Filetype::Filetypes::JPEG>()(filename);
				handle_filetype<Filetype::Filetypes::JPG>()(filename);
				handle_filetype<Filetype::Filetypes::SPV>()(filename);
				handle_filetype<Filetype::Filetypes::VERT>()(filename);
				handle_filetype<Filetype::Filetypes::FRAG>()(filename);
				handle_filetype<Filetype::Filetypes::OBJ>()(filename);

			} catch (const AlabasterException& e) {
				Log::info("[AlabasterLayer] {}", e.what());
			}
		}
		ImGui::EndDragDropTarget();
	}
}

void AlabasterLayer::draw_components(Entity& entity)
{
	if (entity.has_component<Component::Tag>()) {
		auto& tag = entity.get_component<Component::Tag>().tag;

		char buffer[500];
		std::memset(buffer, 0, sizeof(buffer));
		tag.copy(buffer, 499);
		buffer[499] = '\0';
		if (ImGui::InputText("##Tag", buffer, sizeof(buffer))) {
			tag = std::string(buffer);
		}
	}

	ImGui::SameLine();
	ImGui::PushItemWidth(-1);

	if (ImGui::Button("Add Component"))
		ImGui::OpenPopup("AddComponent");

	if (ImGui::BeginPopup("AddComponent")) {
		display_add_component_entry<Component::Camera>("Camera");
		display_add_component_entry<Component::Texture>("Texture");

		ImGui::EndPopup();
	}

	ImGui::PopItemWidth();

	draw_component<Component::Transform>(entity, "Transform", [](Component::Transform& component) {
		draw_vec3_control("Translation", component.position);
		draw_four_component_vector("Rotation", component.rotation);
		draw_vec3_control("Scale", component.scale, 1.0f);
	});

	draw_component<Component::Texture>(entity, "Texture", [](Component::Texture& component) { color_picker("Colour", component.colour); });
}

void AlabasterLayer::draw_entity_node(Entity& entity)
{
	auto& tag = entity.get_component<Component::Tag>().tag;

	ImGuiTreeNodeFlags flags = ((selected_entity == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
	flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
	const auto& id = entity.get_component<ID>();
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
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
		bool opened = ImGui::TreeNodeEx((const char*)&entity.get_component<ID>().identifier, flags, "%s", tag.c_str());
		if (opened)
			ImGui::TreePop();
		ImGui::TreePop();
	}

	if (entity_deleted) {
		editor_scene->delete_entity(entity);
		if (selected_entity == entity)
			selected_entity = {};
	}
}

void AlabasterLayer::destroy()
{
	editor_scene->shutdown();
	for (auto& panel : panels) {
		panel->on_destroy();
	}
}
