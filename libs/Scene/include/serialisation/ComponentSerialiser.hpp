#pragma once

#include "component/Component.hpp"
#include "entity/Entity.hpp"
#include "serialisation/JsonSubcomponentSerialiser.hpp"
#include "uuid.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <nlohmann/json.hpp>

namespace SceneSystem {

	using namespace nlohmann;

	template <Component::IsComponent T> struct serialise_component {
		void operator()(Entity&, json&) {};
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
			auto texture_object = json::object();
			texture_object["colour"] = entity.get_component<Component::Texture>().colour;

			out["texture"] = texture_object;
		};
	};

	template <> struct serialise_component<Component::Mesh> {
		void operator()(Entity& entity, auto& out)
		{
			auto mesh_object = json::object();
			mesh_object["asset_path"] = entity.get_component<Component::Mesh>().mesh->get_asset_path().string();

			out["mesh"] = mesh_object;
		};
	};

	template <> struct serialise_component<Component::BasicGeometry> {
		void operator()(Entity& entity, auto& out)
		{
			auto mesh_object = json::object();
			mesh_object["basic_geometry"] = entity.get_component<Component::BasicGeometry>().geometry;

			out["mesh"] = mesh_object;
		};
	};

	template <> struct serialise_component<Component::Transform> {
		void operator()(Entity& entity, auto& out)
		{
			auto transform = json::object();
			const Component::Transform& entity_transform = entity.get_component<Component::Transform>();
			transform["position"] = entity_transform.position;
			transform["rotation"] = entity_transform.rotation;
			transform["scale"] = entity_transform.scale;

			out["transform"] = transform;
		};
	};

	template <> struct serialise_component<Component::SphereIntersectible> {
		void operator()(Entity& entity, auto& out)
		{
			const auto& sphere_intersectible_component = entity.get_component<Component::SphereIntersectible>();
			auto sphere_intersectible = json::object();
			sphere_intersectible["radius"] = sphere_intersectible_component.radius;
			sphere_intersectible["world_position"] = sphere_intersectible_component.world_position;
			out["sphere_intersectible"] = sphere_intersectible;
		};
	};

	template <> struct serialise_component<Component::Camera> {
		void operator()(Entity& entity, auto& out)
		{
			const auto& camera_component = entity.get_component<Component::Camera>();
			auto camera = json::object();
			camera["type"] = Alabaster::enum_name(camera_component.camera_type);
			out["camera"] = camera;
		};
	};

} // namespace SceneSystem
