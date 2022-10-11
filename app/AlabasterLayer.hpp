//
// Created by Edwin Carlsson on 2022-10-10.
//

#pragma once

#include "Alabaster.hpp"

#include <glm/glm.hpp>

struct AlabasterLayer : public Alabaster::Layer {
	void update(float ts) override;
	void ui(float ts) override;
	bool initialise() override;
	void destroy() override;

private:
	void create_vertex_buffer();

private:
	std::string_view name() override { return "AlabasterLayer"; }

	std::unique_ptr<Alabaster::Pipeline> graphics_pipeline;
	glm::vec2 viewport_size = { 0.0f, 0.0f };
	glm::vec2 viewport_bounds[2] = { { 0.0f, 0.0f }, { 0.0f, 0.0f } };
	bool viewport_focused = false, viewport_hovered = false;
	bool is_dockspace_open { true };

	VkBuffer vertex_buffers[1]{nullptr};

	VkBuffer vb;
	VkDeviceMemory vb_mem;

	Alabaster::VertexBuffer vertex_buffer;
};
