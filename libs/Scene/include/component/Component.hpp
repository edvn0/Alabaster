#pragma once

#include "core/Random.hpp"
#include "graphics/Mesh.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace SceneSystem::Component {

	struct ID {
		std::size_t identifier;

		ID()
			: identifier(Alabaster::Random::get<std::size_t>())
		{
		}
		~ID() = default;
	};

	struct Tag {
		std::string tag;

		Tag(const std::string& tag)
			: tag(tag)
		{
		}
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

		template <typename Pos, typename Rot, typename Scale>
		Transform(Pos&& pos, Rot&& rot, Scale&& scale) noexcept
			: position(std::forward<Pos>(pos))
			, rotation(std::forward<Rot>(rot))
			, scale(std::forward<Scale>(scale)) {};

		const glm::mat4 to_matrix() const
		{
			return glm::translate(glm::mat4(1.0f), position) * glm::mat4(rotation) * glm::scale(glm::mat4(1.0f), scale);
		}
	};

	struct Mesh {
		const Alabaster::Mesh* mesh;

		Mesh(const Alabaster::Mesh*)
			: mesh(mesh)
		{
		}
	};

	template <typename... T> struct Components {
	};
	using AllComponents = Components<Mesh, Transform, ID, Tag>;

	template <typename T, typename... U>
	concept IsAnyOf = (std::same_as<T, U> || ...);

	template <typename T>
	concept IsComponent = IsAnyOf<T, Mesh, Transform, ID, Tag>;

} // namespace SceneSystem::Component