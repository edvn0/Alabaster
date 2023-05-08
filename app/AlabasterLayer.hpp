//
// Created by Edwin Carlsson on 2022-10-10.
//

#pragma once

#include "graphics/Texture.hpp"
#include "panels/Panel.hpp"
#include "scene/Scene.hpp"

#include <Alabaster.hpp>
#include <AssetManager.hpp>
#include <ImGuizmo.h>
#include <SceneSystem.hpp>
#include <memory>

class AlabasterLayer final : public Alabaster::Layer {
public:
	AlabasterLayer() = default;
	~AlabasterLayer() override = default;

	void update(float ts) override;
	void render() override;
	void ui() override;
	bool initialise(AssetManager::FileWatcher&) override;
	void destroy() override;
	void on_event(Alabaster::Event& event) override;

private:
	void handle_drag_drop() const;
	void menu_bar() const;
	void viewport();
	void take_step();
	void ui_toolbar();
	void build_scene(SceneSystem::Scene&);

	void serialise_scene();

	std::string_view name() override { return "AlabasterLayer"; }

	glm::vec2 viewport_size = { 0.0f, 0.0f };
	std::array<glm::vec2, 2> viewport_bounds = { glm::vec2 { 0.0f, 0.0f }, glm::vec2 { 0.0f, 0.0f } };
	bool viewport_focused { false };
	bool viewport_hovered { false };
	ImGuizmo::OPERATION gizmo_type { ImGuizmo::OPERATION::TRANSLATE };
	SceneSystem::SceneState scene_state { SceneSystem::SceneState::Edit };

	// Taken from "https://www.flaticon.com/authors/jungsa" and "https://www.flaticon.com/packs/music-multimedia-7?word=play%20pause"
	std::shared_ptr<Alabaster::Texture> icon_play { AssetManager::asset<Alabaster::Texture>("play-button.png") };
	std::shared_ptr<Alabaster::Texture> icon_simulate { AssetManager::asset<Alabaster::Texture>("simulate-button.png") };
	std::shared_ptr<Alabaster::Texture> icon_edit { AssetManager::asset<Alabaster::Texture>("edit-button.png") };
	std::shared_ptr<Alabaster::Texture> icon_stop { AssetManager::asset<Alabaster::Texture>("stop-button.png") };
	std::shared_ptr<Alabaster::Texture> icon_pause { AssetManager::asset<Alabaster::Texture>("pause-button.png") };
	std::shared_ptr<Alabaster::Texture> icon_step { AssetManager::asset<Alabaster::Texture>("step-button.png") };

	std::vector<std::unique_ptr<App::Panel>> panels;
	std::unique_ptr<SceneSystem::Scene> editor_scene;
};
