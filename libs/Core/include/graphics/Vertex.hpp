//
// Created by Edwin Carlsson on 2022-10-27.
//

#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include "graphics/VertexBufferLayout.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

namespace Alabaster {

	static constexpr auto vertex_is_same = [](auto&& a, auto&& b) -> bool { return glm::all(glm::epsilonEqual(a, b, 0.00001f)); };

	struct Vertex {
		glm::vec3 position;
		glm::vec4 colour;
		glm::vec3 normal;
		glm::vec3 tangent;
		glm::vec3 bitangent;
		glm::vec2 uv;

		bool operator==(const Vertex& other) const { return vertex_is_same(position, other.position) && colour == other.colour && uv == other.uv; }
	};

	using Index = uint32_t;

	// clang-format off
	namespace Layout::Default {
		static const auto vertex_attributes()
		{
			return VertexBufferLayout 
				{ 
					VertexBufferElement(ShaderDataType::Float3, "position"),
					VertexBufferElement(ShaderDataType::Float4, "colour"), 
					VertexBufferElement(ShaderDataType::Float3, "normal"),
					VertexBufferElement(ShaderDataType::Float3, "tangent"), 
					VertexBufferElement(ShaderDataType::Float3, "bitangent"),
					VertexBufferElement(ShaderDataType::Float2, "uvs") 
				};
		}
	} // namespace Layout::Default
	// clang-format on

} // namespace Alabaster

template <> struct std::hash<Alabaster::Vertex> {
	size_t operator()(const Alabaster::Vertex& vertex) const
	{
		return ((hash<glm::vec3>()(vertex.position) ^ (hash<glm::vec4>()(vertex.colour) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.uv) << 1)
			^ (hash<glm::vec3>()(vertex.normal) << 1) ^ (hash<glm::vec3>()(vertex.tangent) << 1) ^ (hash<glm::vec3>()(vertex.bitangent) << 1);
	}
};
