#pragma once

#include "component/Component.hpp"
#include "entity/Entity.hpp"
#include "serialisation/JsonSubcomponentSerialiser.hpp"
#include "uuid.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <nlohmann/json.hpp>

namespace SceneSystem {

	template <Component::IsComponent T> struct serialise_component {
		void operator()(Entity&, nlohmann::json&) {};
	};

	template <> struct serialise_component<Component::ID> {
		void operator()(Entity& entity, auto& out) { out["id"] = entity.get_component<Component::ID>().identifier; }
	};

	template <> struct serialise_component<Component::Tag> {
		void operator()(Entity& entity, auto& out) { out["tag"] = entity.get_component<Component::Tag>().tag; }
	};

	template <> struct serialise_component<Component::Texture> {
		void operator()(Entity& entity, auto& out)
		{
			auto texture_object = nlohmann::json::object();
			texture_object["colour"] = entity.get_component<Component::Texture>().colour;

			out["texture"] = texture_object;
		};
	};

	template <> struct serialise_component<Component::Mesh> {
		void operator()(Entity& entity, auto& out)
		{
			auto mesh_object = nlohmann::json::object();
			mesh_object["asset_path"] = entity.get_component<Component::Mesh>().mesh->get_asset_path().string();

			out["mesh"] = mesh_object;
		};
	};

	template <> struct serialise_component<Component::BasicGeometry> {
		void operator()(Entity& entity, auto& out)
		{
			auto mesh_object = nlohmann::json::object();
			mesh_object["basic_geometry"] = entity.get_component<Component::BasicGeometry>().geometry;

			out["mesh"] = mesh_object;
		};
	};

	template <> struct serialise_component<Component::Transform> {
		void operator()(Entity& entity, auto& out)
		{
			auto transform = nlohmann::json::object();
			const Component::Transform& entity_transform = entity.get_component<Component::Transform>();
			transform["position"] = entity_transform.position;
			transform["rotation"] = entity_transform.rotation;
			transform["scale"] = entity_transform.scale;

			out["transform"] = transform;
		};
	};

} // namespace SceneSystem
