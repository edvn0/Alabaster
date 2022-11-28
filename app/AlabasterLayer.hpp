//
// Created by Edwin Carlsson on 2022-10-10.
//

#pragma once

#include "Alabaster.hpp"
#include "AssetManager.hpp"
#include "graphics/Camera.hpp"
#include "graphics/Renderer.hpp"
#include "SceneSystem.hpp"

#include <glm/glm.hpp>
#include <memory>

using namespace Alabaster;

struct AlabasterLayer final : public Layer {
	~AlabasterLayer() override {};
	AlabasterLayer()
		: camera(CameraType::FirstPerson, 1600 / 900.0f, 0.1, 20, 45.0f)
		, editor(45.0f, 1600, 900, 0.1, 1000)
		, renderer(editor) {

		};

	void update(float ts) final;
	void ui(float ts) final;
	void ui() final;
	bool initialise() final;
	void destroy() final;
	void on_event(Event& event) final;

private:
	std::string_view name() override { return "AlabasterLayer"; }

private:
	SimpleCamera camera;
	EditorCamera editor;
	Renderer3D renderer;

	std::unique_ptr<SceneSystem::Scene> editor_scene;

	VkRenderPass sun_renderpass { nullptr };
	VkRenderPass first_renderpass { nullptr };

	glm::vec2 viewport_size = { 0.0f, 0.0f };
	glm::vec2 viewport_bounds[2] = { { 0.0f, 0.0f }, { 0.0f, 0.0f } };
	bool viewport_focused { false };
	bool viewport_hovered { false };

	std::unique_ptr<Mesh> viking_room_model;
	std::unique_ptr<Mesh> sphere_model;
	std::unique_ptr<Mesh> cube_model;
	std::unique_ptr<Mesh> sponza_model;

	std::unique_ptr<CommandBuffer> command_buffer;

	std::unique_ptr<Pipeline> viking_pipeline;
	std::unique_ptr<Pipeline> sun_pipeline;
};
