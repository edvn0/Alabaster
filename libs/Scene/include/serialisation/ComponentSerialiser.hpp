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

	template <typename Func> static constexpr auto map(const auto& vec_ts, Func&& apply)
	{
		auto output = json::array();
		for (const auto& t : vec_ts) {
			output.push_back(apply(t));
		}
		return output;
	}

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

	template <> struct serialise_component<Component::Pipeline> {
		void operator()(Entity& entity, auto& out)
		{
			const auto& pipeline_component = entity.get_component<Component::Pipeline>();
			auto pipeline = json::object();
			out["pipeline"] = pipeline;
			if (!pipeline_component.pipeline)
				return;

			const auto& spec = pipeline_component.pipeline->get_specification();
			pipeline["shader"] = spec.shader->get_path().string();
			pipeline["shader_owned_by_pipeline"] = spec.shader_owned_by_pipeline;
			pipeline["debug_name"] = spec.debug_name;
			pipeline["wireframe"] = spec.wireframe;
			pipeline["backface_culling"] = spec.backface_culling;
			pipeline["topology"] = Alabaster::enum_name(spec.topology);
			pipeline["depth_test"] = spec.depth_test;
			pipeline["depth_write"] = spec.depth_write;

			static constexpr auto default_vertex_buffer_element_mapper = [](const Alabaster::VertexBufferElement& in) {
				auto element = json::object();
				element["name"] = in.name;
				element["shader_data_type"] = Alabaster::enum_name(in.shader_data_type);
				element["size"] = in.size;
				element["offset"] = in.offset;
				element["normalised"] = in.normalised;
				return element;
			};

			pipeline["vertex_layout"] = map(spec.vertex_layout.get_elements(), default_vertex_buffer_element_mapper);
			pipeline["instance_layout"] = map(spec.instance_layout.get_elements(), default_vertex_buffer_element_mapper);
			if (spec.ranges) {
				auto ranges = *spec.ranges;
				pipeline["ranges"] = map(ranges.get_input_ranges(), [](const Alabaster::PushConstantRange& range) {
					auto element = json::object();
					element["size"] = range.size;
					element["flags"] = Alabaster::enum_name(range.flags);
					element["offset"] = range.offset;
					return element;
				});
			}
			pipeline["line_width"] = spec.line_width;
		};
	};

} // namespace SceneSystem
