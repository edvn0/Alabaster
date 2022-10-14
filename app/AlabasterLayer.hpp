//
// Created by Edwin Carlsson on 2022-10-10.
//

#pragma once

#include "Alabaster.hpp"

#include <glm/glm.hpp>

struct AlabasterLayer final : public Alabaster::Layer {
	~AlabasterLayer() override = default;
	void update(float ts) final;
	void ui(float ts) final;
	bool initialise() final;
	void destroy() final;

private:
	std::string_view name() override { return "AlabasterLayer"; }

	std::unique_ptr<Alabaster::Pipeline> graphics_pipeline;
	glm::vec2 viewport_size = { 0.0f, 0.0f };
	glm::vec2 viewport_bounds[2] = { { 0.0f, 0.0f }, { 0.0f, 0.0f } };
	bool viewport_focused = false, viewport_hovered = false;
	bool is_dockspace_open { true };

	std::unique_ptr<Alabaster::VertexBuffer> vertex_buffer;
	std::unique_ptr<Alabaster::IndexBuffer> index_buffer;
};
