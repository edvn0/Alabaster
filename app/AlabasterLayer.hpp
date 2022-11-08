//
// Created by Edwin Carlsson on 2022-10-10.
//

#pragma once

#include "Alabaster.hpp"

#include <glm/glm.hpp>

struct AlabasterLayer final : public Alabaster::Layer {
	~AlabasterLayer() override = default;
	AlabasterLayer()
		: camera(Alabaster::CameraType::FirstPerson, 1920 / 1060.0f, 0.1, 20, 45.0f)
		, renderer(camera) {};

	void update(float ts) final;
	void ui(float ts) final;
	void ui() final;
	bool initialise() final;
	void destroy() final;
	void on_event(Alabaster::Event& event) final;

private:
	std::string_view name() override { return "AlabasterLayer"; }
	Alabaster::SimpleCamera camera;
	Alabaster::Renderer3D renderer;

	std::unique_ptr<Alabaster::Pipeline> graphics_pipeline;
	std::unique_ptr<Alabaster::Pipeline> viking_room_pipeline;
	glm::vec2 viewport_size = { 0.0f, 0.0f };
	glm::vec2 viewport_bounds[2] = { { 0.0f, 0.0f }, { 0.0f, 0.0f } };
	bool viewport_focused = false, viewport_hovered = false;
	bool is_dockspace_open { true };

	VkRenderPass render_pass { nullptr };

	std::unique_ptr<Alabaster::VertexBuffer> vertex_buffer;
	std::unique_ptr<Alabaster::IndexBuffer> index_buffer;
	std::unique_ptr<Alabaster::Texture2D> aeroplane_texture;
	std::unique_ptr<Alabaster::Texture2D> black_texture;
	std::unique_ptr<Alabaster::Mesh> viking_room_model;
	std::unique_ptr<Alabaster::Mesh> square_model;
};
