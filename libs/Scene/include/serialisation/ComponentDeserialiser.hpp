#pragma once

#include "component/Component.hpp"
#include "entity/Entity.hpp"
#include "serialisation/JsonSubcomponentSerialiser.hpp"
#include "uuid.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <nlohmann/json.hpp>

namespace SceneSystem {

	template <Component::IsComponent T> struct deserialise_component {
		void operator()(const nlohmann::json&, Entity&) {};
	};

	template <> struct deserialise_component<Component::ID> {
		void operator()(const nlohmann::json& json, Entity& out)
		{
			auto id = json.get<uuids::uuid>();
			out.put_component<Component::ID>(id);
		}
	};

	template <> struct deserialise_component<Component::Transform> {
		void operator()(const nlohmann::json& json, Entity& out)
		{
			const auto& transform = json;
			const auto position = transform["position"].get<glm::vec3>();
			const auto rotation = transform["rotation"].get<glm::quat>();
			const auto scale = transform["scale"].get<glm::vec3>();

			out.put_component<Component::Transform>(position, rotation, scale);
		}
	};

	template <> struct deserialise_component<Component::SphereIntersectible> {
		void operator()(const nlohmann::json&, Entity& out) { out.put_component<Component::SphereIntersectible>(); }
	};

	template <> struct deserialise_component<Component::Camera> {
		void operator()(const nlohmann::json& json, Entity& out)
		{
			const auto camera_type = json.get<Alabaster::CameraType>();

			out.put_component<Component::Camera>(camera_type);
		}
	};

} // namespace SceneSystem
