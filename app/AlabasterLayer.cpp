#include "AlabasterLayer.hpp"

#include "Alabaster.hpp"
#include "AssetManager.hpp"
#include "component/Component.hpp"
#include "core/GUILayer.hpp"
#include "core/Logger.hpp"
#include "entity/Entity.hpp"
#include "graphics/CommandBuffer.hpp"
#include "imgui_internal.h"

#include <imgui.h>
#include <optional>
#include <string.h>
#include <vulkan/vulkan.h>

using namespace Alabaster;
using namespace SceneSystem;
using namespace Component;

static std::uint32_t quads { 1 };

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

static void draw_quat_control(const std::string& label, glm::quat& values, float reset_value = 0.0f, float column_width = 100.0f)
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

	return true;
}

void AlabasterLayer::on_event(Event& e)
{
	editor_scene->on_event(e);

	EventDispatcher dispatch(e);
	dispatch.dispatch<KeyPressedEvent>([](KeyPressedEvent& key_event) {
		const auto key_code = key_event.get_key_code();
		if (key_code == Key::Escape) {
			Application::the().exit();
			return true;
		}

		if (Input::key(Key::G)) {
			Logger::cycle_levels();
			return true;
		}

		return false;
	});
}

void AlabasterLayer::update(float ts) { editor_scene->update(ts); }

void AlabasterLayer::ui() { }

void AlabasterLayer::ui(float ts)
{
	editor_scene->ui(ts);

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
	auto viewport_min_region = ImGui::GetWindowContentRegionMin();
	auto viewport_max_region = ImGui::GetWindowContentRegionMax();
	auto viewport_offset = ImGui::GetWindowPos();
	viewport_bounds[0] = { viewport_min_region.x + viewport_offset.x, viewport_min_region.y + viewport_offset.y };
	viewport_bounds[1] = { viewport_max_region.x + viewport_offset.x, viewport_max_region.y + viewport_offset.y };

	viewport_focused = ImGui::IsWindowFocused();
	viewport_hovered = ImGui::IsWindowHovered();

	if (!viewport_hovered)
		Application::the().gui_layer().block_events();
}

void AlabasterLayer::draw_components(Entity& entity)
{
	if (entity.has_component<Component::Tag>()) {
		auto& tag = entity.get_component<Component::Tag>().tag;

		char buffer[500];
		memset(buffer, 0, sizeof(buffer));
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
		draw_quat_control("Rotation", component.rotation);
		draw_vec3_control("Scale", component.scale, 1.0f);
	});
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
		bool opened = ImGui::TreeNodeEx(entity.get_component<Component::ID>(), flags, "%s", tag.c_str());
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

void AlabasterLayer::destroy() { editor_scene->shutdown(); }
