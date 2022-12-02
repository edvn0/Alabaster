#include "scene_pch.hpp"

#include "serialisation/ComponentSerialiser.hpp"

namespace SceneSystem {

	template <> struct serialise_component<Component::ID> {
		void operator()(Entity& entity, auto& out)
		{
			auto id_object = nlohmann::json::object();
			id_object["id"] = entity.get_component<Component::ID>().identifier;

			out["id"] = id_object;
		}
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
			const auto& t = entity.get_component<Component::Transform>();
			transform["position"] = t.position;
			transform["rotation"] = t.rotation;
			transform["scale"] = t.scale;

			out["transform"] = transform;
		};
	};
} // namespace SceneSystem
