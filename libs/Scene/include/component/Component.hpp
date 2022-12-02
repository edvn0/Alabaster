#pragma once

#include "core/Random.hpp"
#include "graphics/Mesh.hpp"
#include "graphics/Pipeline.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <uuid.h>

namespace SceneSystem::Component {

	struct ID {
		uuids::uuid identifier;

		ID();
		ID(uuids::uuid id);
		~ID() = default;

		std::string to_string() const;
		operator const char*() { return (const char*)&identifier; }
	};

	struct Tag {
		std::string tag;

		Tag(const std::string& tag);
		~Tag() = default;
	};

	struct Transform {
		glm::vec3 position;
		glm::quat rotation;
		glm::vec3 scale;

		Transform() noexcept
			: position(0)
			, rotation(0, 0, 0, 0)
			, scale(1) {};
		~Transform() = default;

		glm::mat4 to_matrix() const;

		template <typename Pos, typename Rot, typename Scale>
		Transform(Pos&& pos, Rot&& rot, Scale&& scale) noexcept
			: position(std::forward<Pos>(pos))
			, rotation(std::forward<Rot>(rot))
			, scale(std::forward<Scale>(scale)) {};
	};

	struct Mesh {
		std::shared_ptr<Alabaster::Mesh> mesh;

		Mesh(const std::shared_ptr<Alabaster::Mesh>& mesh);
		~Mesh() = default;
	};

	struct Pipeline {
		std::shared_ptr<Alabaster::Pipeline> pipeline = nullptr;

		Pipeline(const std::shared_ptr<Alabaster::Pipeline>& pipeline);
		~Pipeline() = default;
	};

	enum class Geometry { Rect, Quad, Circle };

	struct BasicGeometry {
		Geometry geometry;

		BasicGeometry(Geometry geometry)
			: geometry(geometry) {};
	};

	struct Texture {
		glm::vec4 colour { 1.0f };
		Texture(glm::vec4 col);
		Texture()
			: colour() {};
		~Texture() = default;
	};

	struct Camera {
		Camera() = default;
		~Camera() = default;
	};

	namespace {
		template <typename T, typename... U>
		concept IsAnyOf = (std::same_as<T, U> || ...);
	}

	template <typename T>
	concept IsComponent = IsAnyOf<T, Mesh, Transform, ID, Tag, Texture, BasicGeometry, Pipeline, Camera>;

} // namespace SceneSystem::Component
