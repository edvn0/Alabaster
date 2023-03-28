//
// Created by Edwin Carlsson on 2022-10-10.
//

#pragma once

#include "Alabaster.hpp"
#include "AssetManager.hpp"
#include "SceneSystem.hpp"
#include "graphics/Camera.hpp"
#include "graphics/Renderer.hpp"
#include "panels/Panel.hpp"
#include "scene/Scene.hpp"

#include <glm/glm.hpp>
#include <magic_enum.hpp>
#include <memory>

namespace Filetype {
	enum class Filetypes : std::uint8_t { PNG = 0, TTF, JPEG, JPG, SPV, VERT, FRAG, OBJ, JSON, SCENE };
}

template <Filetype::Filetypes Type> struct handle_filetype {
	using Scene = SceneSystem::Scene;
	void operator()(Scene& scene, [[maybe_unused]] const std::filesystem::path& path) const
	{
		Alabaster::Log::info("Filetype handler not implemented for {}", magic_enum::enum_name(Type));
	};
};

class AlabasterLayer final : public Alabaster::Layer {
public:
	AlabasterLayer() = default;
	~AlabasterLayer() override = default;

	void update(float ts) override;
	void ui(float ts) override;
	void ui() override {};
	bool initialise() override;
	void destroy() override;
	void on_event(Alabaster::Event& event) override;

private:
	void handle_drag_drop();
	void menu_bar() const;
	void viewport();

	void serialise_scene();

	std::string_view name() override { return "AlabasterLayer"; }

	std::unique_ptr<SceneSystem::Scene> editor_scene;
	std::vector<std::unique_ptr<App::Panel>> panels;

	glm::vec2 viewport_size = { 0.0f, 0.0f };
	std::array<glm::vec2, 2> viewport_bounds = { glm::vec2 { 0.0f, 0.0f }, glm::vec2 { 0.0f, 0.0f } };
	bool viewport_focused { false };
	bool viewport_hovered { false };
};
