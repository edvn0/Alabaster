#pragma once

#include "core/Random.hpp"
#include "graphics/Mesh.hpp"
#include "graphics/Pipeline.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <uuid.h>

namespace SceneSystem::Component {

	template <typename T> inline constexpr std::string_view component_name;

	struct ID {
		uuids::uuid identifier;

		ID();
		ID(uuids::uuid id);
		~ID() = default;

		std::string to_string() const;
	};
	template <> inline constexpr std::string_view component_name<Component::ID> = "id";

	struct Tag {
		std::string tag;

		Tag(const std::string& tag);
		~Tag() = default;
	};
	template <> inline constexpr std::string_view component_name<Component::Tag> = "tag";

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
		Transform(Pos&& pos, Rot&& rot, Scale&& scl) noexcept
			: position(std::forward<Pos>(pos))
			, rotation(std::forward<Rot>(rot))
			, scale(std::forward<Scale>(scl))
		{
		}
	};
	template <> inline constexpr std::string_view component_name<Component::Transform> = "transform";

	struct Mesh {
		std::shared_ptr<Alabaster::Mesh> mesh;

		Mesh(const std::shared_ptr<Alabaster::Mesh>& mesh);
		~Mesh() = default;
	};
	template <> inline constexpr std::string_view component_name<Component::Mesh> = "mesh";

	struct Pipeline {
		std::shared_ptr<Alabaster::Pipeline> pipeline = nullptr;

		Pipeline(const std::shared_ptr<Alabaster::Pipeline>& pipeline);
		~Pipeline() = default;
	};
	template <> inline constexpr std::string_view component_name<Component::Pipeline> = "pipeline";

	enum class Geometry { Rect, Quad, Circle };

	struct BasicGeometry {
		Geometry geometry;

		BasicGeometry(Geometry geom)
			: geometry(geom) {};
	};
	template <> inline constexpr std::string_view component_name<Component::BasicGeometry> = "basic_geometry";

	struct Texture {
		glm::vec4 colour { 1.0f };
		Texture(glm::vec4 col);
		Texture()
			: colour() {};
		~Texture() = default;
	};
	template <> inline constexpr std::string_view component_name<Component::Texture> = "texture";

	struct Light {
		bool is_light { true };

		Light() = default;
		~Light() = default;
	};
	template <> inline constexpr std::string_view component_name<Component::Light> = "light";

	struct Camera {
		Camera() = default;
		~Camera() = default;
	};
	template <> inline constexpr std::string_view component_name<Component::Camera> = "camera";

	namespace Detail {
		template <typename T, typename... U>
		concept IsAnyOf = (std::same_as<T, U> || ...);
	}

	template <typename T>
	concept IsComponent = Detail::IsAnyOf<T, Mesh, Transform, ID, Tag, Texture, BasicGeometry, Pipeline, Camera, Light>;

} // namespace SceneSystem::Component
