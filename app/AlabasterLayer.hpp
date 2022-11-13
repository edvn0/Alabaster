//
// Created by Edwin Carlsson on 2022-10-10.
//

#pragma once

#include "Alabaster.hpp"
#include "graphics/Camera.hpp"

#include <glm/glm.hpp>

using namespace Alabaster;

struct AlabasterLayer final : public Layer {
	~AlabasterLayer() override = default;
	AlabasterLayer()
		: camera(CameraType::FirstPerson, 1600 / 900.0f, 0.1, 20, 45.0f)
		, editor(45.0f, 1600, 900, 0.1, 20)
		, renderer(editor) {};

	void update(float ts) final;
	void ui(float ts) final;
	void ui() final;
	bool initialise() final;
	void destroy() final;
	void on_event(Event& event) final;

private:
	std::string_view name() override { return "AlabasterLayer"; }
	SimpleCamera camera;
	EditorCamera editor;
	Renderer3D renderer;

	glm::vec2 viewport_size = { 0.0f, 0.0f };
	glm::vec2 viewport_bounds[2] = { { 0.0f, 0.0f }, { 0.0f, 0.0f } };
	bool viewport_focused = false, viewport_hovered = false;
	bool is_dockspace_open { true };

	std::unique_ptr<Mesh> viking_room_model;
	std::unique_ptr<Mesh> sphere_model;
};
