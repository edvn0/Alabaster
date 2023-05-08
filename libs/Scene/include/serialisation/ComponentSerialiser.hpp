#pragma once

#include "component/Component.hpp"
#include "entity/Entity.hpp"
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

	template <typename Func> static auto map(const auto& vec_ts, Func&& apply)
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
		void operator()(Entity& entity, auto& out)
		{
			out[Component::component_name<Component::ID>] = entity.get_component<Component::ID>().identifier;
		}
	};

	template <> struct serialise_component<Component::Tag> {
		void operator()(Entity& entity, auto& out) { out[Component::component_name<Component::Tag>] = entity.get_component<Component::Tag>().tag; }
	};

	template <> struct serialise_component<Component::Texture> {
		void operator()(Entity& entity, auto& out)
		{
			auto texture_object = json::object();
			texture_object["colour"] = entity.get_component<Component::Texture>().colour;

			out[Component::component_name<Component::Texture>] = texture_object;
		};
	};

	template <> struct serialise_component<Component::Mesh> {
		void operator()(Entity& entity, auto& out)
		{
			auto mesh_object = json::object();
			mesh_object["asset_path"] = entity.get_component<Component::Mesh>().mesh->get_asset_path().string();

			out[Component::component_name<Component::Mesh>] = mesh_object;
		};
	};

	template <> struct serialise_component<Component::BasicGeometry> {
		void operator()(Entity& entity, auto& out)
		{
			auto mesh_object = json::object();
			mesh_object["basic_geometry"] = Alabaster::enum_name(entity.get_component<Component::BasicGeometry>().geometry);

			out[Component::component_name<Component::BasicGeometry>] = mesh_object;
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

			out[Component::component_name<Component::Transform>] = transform;
		};
	};

	template <> struct serialise_component<Component::SphereIntersectible> {
		void operator()(Entity& entity, auto& out)
		{
			const auto& sphere_intersectible_component = entity.get_component<Component::SphereIntersectible>();
			auto sphere_intersectible = json::object();
			sphere_intersectible["radius"] = sphere_intersectible_component.radius;
			sphere_intersectible["world_position"] = sphere_intersectible_component.world_position;
			out[Component::component_name<Component::SphereIntersectible>] = sphere_intersectible;
		};
	};

	template <> struct serialise_component<Component::QuadIntersectible> {
		void operator()(Entity& entity, auto& out)
		{
			const auto& quad_intersectible_component = entity.get_component<Component::QuadIntersectible>();
			auto quad_intersectible = json::object();
			quad_intersectible["normal"] = quad_intersectible_component.normal;
			quad_intersectible["world_position"] = quad_intersectible_component.world_position;
			out[Component::component_name<Component::QuadIntersectible>] = quad_intersectible;
		};
	};

	template <> struct serialise_component<Component::Light> {
		void operator()(Entity& entity, auto& out)
		{
			auto light = json::object();
			auto light_component = entity.get_component<Component::Light>();
			light["ambiance"] = light_component.ambience;
			out[Component::component_name<Component::Light>] = light;
		}
	};

	template <> struct serialise_component<Component::PointLight> {
		void operator()(Entity& entity, auto& out)
		{
			auto point_light = json::object();
			auto point_light_component = entity.get_component<Component::PointLight>();
			point_light["ambience"] = point_light_component.ambience;
			out[Component::component_name<Component::PointLight>] = point_light;
		}
	};

	template <> struct serialise_component<Component::Behaviour> {
		void operator()(Entity& entity, auto& out)
		{
			auto behaviour = json::object();
			auto behaviour_component = entity.get_component<Component::Behaviour>();
			behaviour["name"] = behaviour_component.name;
			out[Component::component_name<Component::Behaviour>] = behaviour;
		}
	};

	template <> struct serialise_component<Component::ScriptBehaviour> {
		void operator()(Entity& entity, auto& out)
		{
			auto script_behaviour = json::object();
			script_behaviour["script_name"] = entity.get_component<Component::ScriptBehaviour>().script_name;
			out[Component::component_name<Component::ScriptBehaviour>] = script_behaviour;
		}
	};

	template <> struct serialise_component<Component::Camera> {
		void operator()(Entity& entity, auto& out)
		{
			const auto& camera_component = entity.get_component<Component::Camera>();
			auto camera = json::object();
			camera["type"] = Alabaster::enum_name(camera_component.camera_type);
			out[Component::component_name<Component::Camera>] = camera;
		};
	};

	template <> struct serialise_component<Component::Pipeline> {
		void operator()(Entity& entity, auto& out)
		{
			const auto& pipeline_component = entity.get_component<Component::Pipeline>();
			auto pipeline = json::object();
			out[Component::component_name<Component::Pipeline>] = pipeline;
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
