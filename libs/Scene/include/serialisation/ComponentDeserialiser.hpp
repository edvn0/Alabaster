#pragma once

#include "component/Component.hpp"
#include "entity/Entity.hpp"
#include "graphics/Camera.hpp"
#include "graphics/Mesh.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/Texture.hpp"
#include "serialisation/JsonSubcomponentSerialiser.hpp"
#include "uuid.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <nlohmann/json.hpp>

namespace SceneSystem {

	using namespace nlohmann;
	template <Component::IsComponent T> struct deserialise_component {
		void operator()(const json&, Entity&) {};
	};

	template <> struct deserialise_component<Component::ID> {
		void operator()(const json& json, Entity& out)
		{
			auto id = json["id"].get<uuids::uuid>();
			out.put_component<Component::ID>(id);
		}
	};

	template <> struct deserialise_component<Component::Tag> {
		void operator()(const json& json, Entity& out)
		{
			auto tag = json["tag"].get<std::string>();
			out.put_component<Component::Tag>(tag);
		}
	};

	template <> struct deserialise_component<Component::Transform> {
		void operator()(const json& json, Entity& out)
		{
			const auto& transform = json;
			const auto position = transform["position"].get<glm::vec3>();
			const auto rotation = transform["rotation"].get<glm::quat>();
			const auto scale = transform["scale"].get<glm::vec3>();

			out.put_component<Component::Transform>(position, rotation, scale);
		}
	};

	template <> struct deserialise_component<Component::SphereIntersectible> {
		void operator()(const json&, Entity& out) { out.put_component<Component::SphereIntersectible>(); }
	};

	template <> struct deserialise_component<Component::Camera> {
		void operator()(const json& json, Entity& out)
		{
			const auto camera_type = json.get<Component::ComponentCameraType>();

			out.put_component<Component::Camera>(camera_type);
		}
	};

	template <> struct deserialise_component<Component::QuadIntersectible> {
		void operator()(const json& json, Entity& out)
		{
			const auto world_pos = json["world_position"].get<glm::vec3>();
			const auto normal = json["normal"].get<glm::vec3>();
			out.put_component<Component::QuadIntersectible>(world_pos, normal);
		};
	};

	template <> struct deserialise_component<Component::Light> {
		void operator()(const json& json, Entity& out)
		{
			const auto ambience = json["ambience"].get<glm::vec4>();
			out.put_component<Component::Light>(ambience);
		}
	};

	template <> struct deserialise_component<Component::PointLight> {
		void operator()(const json& json, Entity& out)
		{
			const auto ambience = json["ambience"].get<glm::vec4>();
			out.put_component<Component::PointLight>(ambience);
		}
	};

	template <> struct deserialise_component<Component::Behaviour> {
		void operator()(const json& json, Entity& out)
		{
			auto& component = out.put_component<Component::Behaviour>();
			component.name = json["name"].get<std::string>();
		}
	};

	template <> struct deserialise_component<Component::BasicGeometry> {
		void operator()(const json& json, Entity& out)
		{
			auto& component = out.put_component<Component::BasicGeometry>();
			component.geometry
				= Alabaster::enum_value<Component::Geometry>(json["basic_geometry"].get<std::string>()).value_or(Component::Geometry::Quad);
		}
	};

	template <> struct deserialise_component<Component::ScriptBehaviour> {
		void operator()(const json& json, Entity& out)
		{
			auto& component = out.put_component<Component::ScriptBehaviour>();
			component.script_name = json["script_name"].get<std::string>();
		}
	};

} // namespace SceneSystem
