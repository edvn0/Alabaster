#include "AlabasterLayer.hpp"

#include "Alabaster.hpp"
#include "AssetManager.hpp"
#include "core/GUILayer.hpp"
#include "entity/Entity.hpp"
#include "graphics/CommandBuffer.hpp"

#include <imgui.h>
#include <optional>
#include <vulkan/vulkan.h>

using namespace Alabaster;

static std::uint32_t quads { 1 };

static glm::vec4 pos { -5, 5, 5, 1.0f };
static glm::vec4 col { 255 / 255.0, 153 / 255.0, 51 / 255.0, 255.0f / 255.0 };
static float ambience { 1.0f };

bool AlabasterLayer::initialise()
{
	command_buffer = CommandBuffer::from_swapchain();
	editor_scene = std::make_unique<SceneSystem::Scene>();
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

		if (key_code == Key::C) {
			quads++;
			return true;
		}

		if (key_code == Key::U) {
			pos.x += 0.1;
			pos.y += 0.1;
		}
		if (key_code == Key::H) {
			pos.x -= 0.1;
			pos.y -= 0.1;
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

	auto all_meshes = editor_scene->all_with<SceneSystem::Component::ID, SceneSystem::Component::Tag>();
	ImGui::Begin("All entities");
	if (ImGui::Button("Add Entity")) {
		editor_scene->create_entity();
	}
	all_meshes.each([](const SceneSystem::Component::ID& id, const SceneSystem::Component::Tag& tag) {
		ImGui::Text("ID: %s, Name: %s", id.to_string().c_str(), std::string(tag.tag).c_str());
	});
	ImGui::End();

	{
		ImGui::Begin("Scene Hierarchy");

		editor_scene->for_each_entity([&](auto entity_id) {
			SceneSystem::Entity entity { editor_scene.get(), entity_id };
			draw_entity_node(entity);
		});

		if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::IsWindowHovered())
			selected_entity = {};

		// Right-click on blank space
		if (ImGui::BeginPopupContextWindow(0, 1)) {
			if (ImGui::MenuItem("Create Empty Entity"))
				editor_scene->create_entity("Empty Entity");

			ImGui::EndPopup();
		}

		ImGui::End();

		ImGui::Begin("Properties");
		// if (selected_entity) {
		//	draw_components(selected_entity);
		// }

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

void AlabasterLayer::draw_entity_node(SceneSystem::Entity& entity)
{
	using namespace SceneSystem;
	auto& tag = entity.get_component<Component::Tag>().tag;

	ImGuiTreeNodeFlags flags = ((selected_entity == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
	flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
	bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, "%s", tag.c_str());
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
		bool opened = ImGui::TreeNodeEx((void*)9817239, flags, "%s", tag.c_str());
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
