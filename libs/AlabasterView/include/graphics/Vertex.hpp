//
// Created by Edwin Carlsson on 2022-10-27.
//

#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

namespace Alabaster {

	struct Vertex {
		glm::vec4 position;
		glm::vec4 colour;
		glm::vec2 uv;

		bool operator==(const Vertex& other) const { return position == other.position && colour == other.colour && uv == other.uv; }
	};

	using Index = uint32_t;

} // namespace Alabaster

template <> struct std::hash<Alabaster::Vertex> {
	size_t operator()(const Alabaster::Vertex& vertex) const
	{
		return ((hash<glm::vec4>()(vertex.position) ^ (hash<glm::vec4>()(vertex.colour) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.uv) << 1);
	}
};
