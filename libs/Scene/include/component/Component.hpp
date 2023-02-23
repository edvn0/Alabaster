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
		explicit ID(uuids::uuid id);
		~ID() = default;

		std::string to_string() const;
	};
	template <> inline constexpr std::string_view component_name<Component::ID> = "id";

	struct Tag {
		std::string tag = "Empty entity";

		Tag() = default;
		explicit Tag(const std::string& tag);
		~Tag() = default;
	};
	template <> inline constexpr std::string_view component_name<Component::Tag> = "tag";

	struct RayIntersectible {
		RayIntersectible() = default;
		virtual ~RayIntersectible() = default;

		virtual void update(const glm::vec3& position, const glm::vec3& scale, const glm::quat& rotation) = 0;
		virtual bool intersects_with(const glm::vec3& direction, const glm::vec3& origin, float& distance) const = 0;
	};

	struct SphereIntersectible : public RayIntersectible {
		SphereIntersectible() = default;
		SphereIntersectible(const glm::vec3& in_world_pos, float in_radius)
			: world_position(in_world_pos)
			, radius(in_radius) {};
		~SphereIntersectible() override = default;

		void update(const glm::vec3& position, const glm::vec3& scale, const glm::quat&) override
		{
			world_position = position;
			radius = scale.x;
		}

		// This is taken from https://github.com/capnramses/antons_opengl_tutorials_book/blob/master/07_ray_picking/main.cpp
		bool intersects_with(const glm::vec3& ray_direction_wor, const glm::vec3& ray_origin_wor, float& intersection_distance) const override
		{
			const auto sphere_centre_wor = world_position;
			const auto sphere_radius = radius;
			// work out components of quadratic
			glm::vec3 dist_to_sphere = ray_origin_wor - sphere_centre_wor;
			float b = glm::dot(ray_direction_wor, dist_to_sphere);
			float c = glm::dot(dist_to_sphere, dist_to_sphere) - sphere_radius * sphere_radius;
			float b_squared_minus_c = b * b - c;
			// check for "imaginary" answer. == ray completely misses sphere
			if (b_squared_minus_c < 0.0f) {
				return false;
			}
			// check for ray hitting twice (in and out of the sphere)
			if (b_squared_minus_c > 0.0f) {
				// get the 2 intersection distances along ray
				float t_a = -b + glm::sqrt(b_squared_minus_c);
				float t_b = -b - glm::sqrt(b_squared_minus_c);
				intersection_distance = t_b;
				// if behind viewer, throw one or both away
				if (t_a < 0.0) {
					if (t_b < 0.0) {
						return false;
					}
				} else if (t_b < 0.0) {
					intersection_distance = t_a;
				}

				return true;
			}
			// check for ray hitting once (skimming the surface)
			if (0.0f == b_squared_minus_c) {
				// if behind viewer, throw away
				float t = -b + glm::sqrt(b_squared_minus_c);
				if (t < 0.0f) {
					return false;
				}
				intersection_distance = t;
				return true;
			}
			// note: could also check if ray origin is inside sphere radius
			return false;
		};

	private:
		glm::vec3 world_position { 0 };
		float radius { 0.f };
	};
	template <> inline constexpr std::string_view component_name<Component::SphereIntersectible> = "sphere_intersectable";

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

		Mesh() = default;
		explicit Mesh(const std::shared_ptr<Alabaster::Mesh>& mesh);
		~Mesh() = default;
	};
	template <> inline constexpr std::string_view component_name<Component::Mesh> = "mesh";

	struct Pipeline {
		std::shared_ptr<Alabaster::Pipeline> pipeline = nullptr;

		Pipeline() = default;
		explicit Pipeline(const std::shared_ptr<Alabaster::Pipeline>& pipeline);
		~Pipeline() = default;
	};
	template <> inline constexpr std::string_view component_name<Component::Pipeline> = "pipeline";

	enum class Geometry { Rect, Quad, Circle };

	struct BasicGeometry {
		Geometry geometry = Geometry::Quad;

		BasicGeometry() = default;
		explicit BasicGeometry(Geometry geom)
			: geometry(geom) {};
	};
	template <> inline constexpr std::string_view component_name<Component::BasicGeometry> = "basic_geometry";

	struct Texture {
		glm::vec4 colour { 1.0f };
		explicit Texture(glm::vec4 col);
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
	concept IsComponent = Detail::IsAnyOf<T, Mesh, Transform, ID, Tag, Texture, BasicGeometry, Pipeline, Camera, Light, SphereIntersectible>;

} // namespace SceneSystem::Component
