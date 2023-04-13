#pragma once

#include "core/Random.hpp"
#include "graphics/Camera.hpp"
#include "graphics/Mesh.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/Texture.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/norm.hpp>
#include <uuid.h>

namespace SceneSystem {
	class ScriptEntity;
}

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
		bool intersects_with(const glm::vec3& ray_direction_wor, const glm::vec3& ray_origin_wor, float& intersection_distance) const override;

		glm::vec3 world_position { 0 };
		float radius { 0.f };
	};
	template <> inline constexpr std::string_view component_name<Component::SphereIntersectible> = "sphere_intersectible";

	struct QuadIntersectible : public RayIntersectible {
		QuadIntersectible() = default;
		QuadIntersectible(const glm::vec3& in_world_pos, const glm::vec3& in_normal)
			: world_position(in_world_pos)
			, normal(in_normal) {};
		~QuadIntersectible() override = default;

		void update(const glm::vec3& position, const glm::vec3&, const glm::quat&) override { world_position = position; }

		void set_normal(const glm::vec3& in_normal) { normal = in_normal; }

		bool intersects_with(const glm::vec3& ray_direction_wor, const glm::vec3& ray_origin_wor, float& intersection_distance) const override
		{
			float denom = glm::dot(normal, ray_direction_wor);
			if (!glm::epsilonEqual(denom, 0.f, 0.00001f)) {
				glm::vec3 quad_ray_origin_vec = world_position - ray_origin_wor;
				intersection_distance = glm::dot(quad_ray_origin_vec, normal) / denom;
				return intersection_distance >= 0;
			}

			return false;
		};

		glm::vec3 world_position { 0 };
		glm::vec3 normal { 0.f, 1.f, 0.f };
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
		const std::shared_ptr<Alabaster::Texture> texture { nullptr };

		template <typename T>
		explicit Texture(const T& col, const std::shared_ptr<Alabaster::Texture>& tex = nullptr)
			: colour(col)
			, texture(tex)
		{
			// If the norm is > 1, we normalize to RGB, I.e. divide by 255.
			// l1 against 4 components is fine (max of sum of four components is 4)
			if ((col.x + col.y + col.z + col.w) > 4.0f)
				colour = colour / 255.0f;
		}

		explicit Texture() = default;
		~Texture() = default;
	};
	template <> inline constexpr std::string_view component_name<Component::Texture> = "texture";

	struct Light {
		glm::vec4 ambience { 1.0f };

		template <typename T>
		Light(const T& amb)
			: ambience(amb)
		{
			// If the norm is > 1, we normalize to RGB, I.e. divide by 255.
			// l1 against 4 components is fine (max of sum of four components is 4)
			if ((ambience.x + ambience.y + ambience.z + ambience.w) > 4.0f)
				ambience = ambience / 255.0f;
		}
		Light() = default;
		~Light() = default;
	};
	template <> inline constexpr std::string_view component_name<Component::Light> = "light";

	struct Camera {
		Camera() = default;
		Camera(Alabaster::CameraType type)
			: camera_type(type) {};
		~Camera() = default;

		Alabaster::CameraType camera_type = Alabaster::CameraType::Perspective;
	};
	template <> inline constexpr std::string_view component_name<Component::Camera> = "camera";

	struct PointLight {
		glm::vec4 ambience { 1.0f };

		template <typename T>
		PointLight(const T& amb)
			: ambience(amb)
		{
			// If the l1 norm is > 1, we normalize to RGB, I.e. divide by 255.
			// l1 against 4 components is fine (max of sum of four components is 4)
			if ((ambience.x + ambience.y + ambience.z + ambience.w) > 4.0f)
				ambience = ambience / 255.0f;
		}

		PointLight() = default;
		~PointLight() = default;
	};
	template <> inline constexpr std::string_view component_name<Component::PointLight> = "point_light";

	template <typename T>
	concept IsScriptable = requires(T t, float ts) {
		{
			t.on_update(ts)
		} -> std::same_as<void>;
		{
			t.on_create()
		} -> std::same_as<void>;
		{
			t.on_delete()
		} -> std::same_as<void>;
	};
	struct Behaviour {
		ScriptEntity* entity { nullptr };
		std::string_view name;

		Behaviour() = default;

		std::function<void(Behaviour&)> create;
		std::function<void(Behaviour&)> destroy;

		template <IsScriptable T, typename... Args> void bind(std::string_view current_name, Args&&... args)
		{
			name = std::move(current_name);
			create = [... arg = std::forward<Args>(args)](
						 Behaviour& behaviour) { behaviour.entity = static_cast<ScriptEntity*>(new T(std::forward<Args>(arg)...)); };
			setup_entity_destruction();
		}

	private:
		void setup_entity_destruction();
	};
	template <> inline constexpr std::string_view component_name<Component::Behaviour> = "behaviour";

	namespace Detail {
		template <typename T, typename... U>
		concept IsAnyOf = (std::same_as<T, U> || ...);
	}

	template <typename T>
	concept IsComponent = Detail::IsAnyOf<T, Mesh, Transform, ID, Tag, Texture, BasicGeometry, Pipeline, Camera, Light, PointLight,
		SphereIntersectible, QuadIntersectible, Behaviour>;

	template <typename T>
	concept IsValidComponent = IsComponent<T> && requires(T component) {
		{
			component.is_valid()
		} -> std::same_as<bool>;
	};

} // namespace SceneSystem::Component
