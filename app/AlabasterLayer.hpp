//
// Created by Edwin Carlsson on 2022-10-10.
//

#pragma once

#include "Alabaster.hpp"
#include "AssetManager.hpp"
#include "graphics/Camera.hpp"
#include "graphics/Renderer.hpp"
#include "scene/Scene.hpp"
#include "SceneSystem.hpp"

#include <glm/glm.hpp>
#include <memory>

using namespace Alabaster;

struct AlabasterLayer final : public Layer {
	~AlabasterLayer() override {};
	AlabasterLayer() {

	};

	void update(float ts) final;
	void ui(float ts) final;
	void ui() final;
	bool initialise() final;
	void destroy() final;
	void on_event(Event& event) final;

	void draw_entity_node(SceneSystem::Entity& entity);
	void draw_components(SceneSystem::Entity& entity);
	template <SceneSystem::Component::IsComponent T> void display_add_component_entry(const std::string& entry_name)
	{
		if (!selected_entity.has_component<T>()) {
			if (ImGui::MenuItem(entry_name.c_str())) {
				selected_entity.add_component<T>();
				ImGui::CloseCurrentPopup();
			}
		}
	}

private:
	std::string_view name() override { return "AlabasterLayer"; }

private:
	std::unique_ptr<SceneSystem::Scene> editor_scene;

	SceneSystem::Entity selected_entity {};

	glm::vec2 viewport_size = { 0.0f, 0.0f };
	glm::vec2 viewport_bounds[2] = { { 0.0f, 0.0f }, { 0.0f, 0.0f } };
	bool viewport_focused { false };
	bool viewport_hovered { false };

	std::unique_ptr<CommandBuffer> command_buffer;
};
