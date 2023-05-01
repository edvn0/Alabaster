#include "scene_pch.hpp"

#include "component/Component.hpp"

#include "component/ScriptEntity.hpp"
#include "core/UUID.hpp"

namespace SceneSystem {

	Component::ID::ID()
		: identifier(Alabaster::UUID::random_uuid())
	{
	}

	Component::ID::ID(uuids::uuid id)
		: identifier(id)
	{
	}

	std::string Component::ID::to_string() const { return uuids::to_string(identifier); }

	glm::mat4 Component::Transform::to_matrix() const
	{
		return glm::translate(glm::mat4(1.0f), position) * glm::mat4(rotation) * glm::scale(glm::mat4(1.0f), scale);
	}

	Component::Mesh::Mesh(const std::shared_ptr<Alabaster::Mesh>& in_mesh)
		: mesh(in_mesh)
	{
	}

	Component::Pipeline::Pipeline(const std::shared_ptr<Alabaster::Pipeline>& in_pipeline)
		: pipeline(in_pipeline)
	{
	}

	Component::Tag::Tag(const std::string& in_tag)
		: tag(in_tag)
	{
	}

	bool Component::SphereIntersectible::intersects_with(
		const glm::vec3& ray_direction_wor, const glm::vec3& ray_origin_wor, float& intersection_distance) const
	{
		const auto sphere_centre_wor = world_position;
		const auto sphere_radius = radius;
		const auto dist_to_sphere = ray_origin_wor - sphere_centre_wor;
		float b = glm::dot(ray_direction_wor, dist_to_sphere);
		float c = glm::dot(dist_to_sphere, dist_to_sphere) - sphere_radius * sphere_radius;
		float b_squared_minus_c = b * b - c;
		if (b_squared_minus_c < 0.0f) {
			return false;
		}
		if (b_squared_minus_c > 0.0f) {
			float t_a = -b + glm::sqrt(b_squared_minus_c);
			float t_b = -b - glm::sqrt(b_squared_minus_c);
			intersection_distance = t_b;
			if (t_a < 0.0) {
				if (t_b < 0.0) {
					return false;
				}
			} else if (t_b < 0.0) {
				intersection_distance = t_a;
			}

			return true;
		}
		if (0.0f == b_squared_minus_c) {
			float t = -b + glm::sqrt(b_squared_minus_c);
			if (t < 0.0f) {
				return false;
			}
			intersection_distance = t;
			return true;
		}
		return false;
	};

	void Component::Behaviour::setup_entity_destruction()
	{
		destroy = [](Behaviour& behaviour) {
			delete behaviour.entity;
			behaviour.entity = nullptr;
		};
	}

} // namespace SceneSystem
