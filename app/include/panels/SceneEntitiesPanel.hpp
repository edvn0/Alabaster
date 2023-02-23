#pragma once

#include "core/Logger.hpp"
#include "entity/Entity.hpp"
#include "panels/Panel.hpp"
#include "scene/Scene.hpp"

#include <imgui.h>

namespace App {

	class SceneEntitiesPanel : public App::Panel {
	public:
		SceneEntitiesPanel(SceneSystem::Scene* input_scene)
			: scene(input_scene)
		{
		}

		void update_scene(SceneSystem::Scene* new_scene) { scene = new_scene; }
		void draw_components(SceneSystem::Entity& entity);
		template <SceneSystem::Component::IsComponent T> void display_add_component_entry(const std::string& entry_name)
		{
			if (!selected_entity.has_component<T>() && ImGui::MenuItem(entry_name.c_str())) {
				selected_entity.add_component<T>();
				ImGui::CloseCurrentPopup();
			}
		}
		void draw_entity_node(SceneSystem::Entity& entity);

		~SceneEntitiesPanel() override = default;
		void on_update(float ts) override;
		void ui(float ts) override;
		void on_event(Alabaster::Event& event) override;
		void on_init() override { Alabaster::Log::info("[SceneEntitiesPanel] Initialised."); };
		void on_destroy() override { Alabaster::Log::info("[SceneEntitiesPanel] Destroyed."); };

	private:
		SceneSystem::Scene* scene;
		SceneSystem::Entity selected_entity;
	};

} // namespace App