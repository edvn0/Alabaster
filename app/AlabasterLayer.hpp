//
// Created by Edwin Carlsson on 2022-10-10.
//

#pragma once

#include "core/Application.hpp"

#include <glm/glm.hpp>

namespace Alabaster {
	class Pipeline;
}

struct AlabasterLayer : public Alabaster::Layer {
	void update(float ts) override;
	void ui(float ts) override;
	bool initialise() override;

private:
	std::string_view name() override { return "AlabasterLayer"; }

	Alabaster::Pipeline* graphics_pipeline;
	glm::vec2 viewport_size = { 0.0f, 0.0f };
	glm::vec2 viewport_bounds[2] = { { 0.0f, 0.0f }, { 0.0f, 0.0f } };
	bool viewport_focused = false, viewport_hovered = false;
	bool is_dockspace_open { true };
};
