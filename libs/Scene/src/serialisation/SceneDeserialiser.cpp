//
// Created by Edwin Carlsson on 2022-12-12.
//

#include "scene_pch.hpp"

#include "serialisation/SceneDeserialiser.hpp"

#include "core/exceptions/AlabasterException.hpp"
#include "serialisation/ComponentDeserialiser.hpp"

#include <exception>

namespace SceneSystem {

	using Alabaster::AlabasterException;

	class DeserialiserNotImplementedException : public AlabasterException {
	public:
		using AlabasterException::AlabasterException;
	};

	class ComponentMissingInJsonNodeException : public AlabasterException {
	public:
		using AlabasterException::AlabasterException;
	};

	class DeserialisationFailedException : public AlabasterException {
	public:
		using AlabasterException::AlabasterException;
	};

} // namespace SceneSystem

namespace SceneSystem {

	using namespace SceneSystem::Component;

	namespace {
		static std::unordered_set<std::string> unmapped_deserialisation;
	}

	template <IsComponent T> static constexpr auto handle_component(const nlohmann::json& json_node, auto& entity)
	{
		try {
			constexpr const auto name = component_name<T>;

			if constexpr (std::empty(name))
				throw DeserialiserNotImplementedException("You have not implemented deserialisation");

			if (!json_node.contains(name))
				throw ComponentMissingInJsonNodeException("Could not find key for this object");

			if (const nlohmann::json& component = json_node[name]; component.is_object() || component.is_string() || component.is_array()) {
				try {
					deserialise_component<T>()(component, entity);
				} catch (const std::exception& exc) {
					throw DeserialisationFailedException("Could not deserialise this component with the deserialiser. Message: {}", exc.what());
				};
			}
		} catch (const DeserialiserNotImplementedException&) {
			unmapped_deserialisation.emplace(typeid(T).name());
			return;
		} catch (const ComponentMissingInJsonNodeException&) {
			return;
		} catch (const DeserialisationFailedException& e) {
			Alabaster::Log::warn("{}", e.what());
			return;
		}
	}

	void SceneDeserialiser::deserialise()
	{
		std::ifstream json_file(scene_path);
		if (!json_file) {
			throw Alabaster::AlabasterException("Could not open scene file.");
		}

		scene.clear();

		nlohmann::json data = nlohmann::json::parse(json_file);

		std::string scene_name = data["scene_name"];

		const auto& json_entities = data["entities"];

		for (const auto& json_entity : json_entities) {
			auto created_entity = scene.create_entity("Unnamed entity");
			if (json_entity.is_object()) {
				handle_component<Tag>(json_entity, created_entity);
				handle_component<Mesh>(json_entity, created_entity);
				handle_component<Transform>(json_entity, created_entity);
				handle_component<ID>(json_entity, created_entity);
				handle_component<Light>(json_entity, created_entity);
				handle_component<Tag>(json_entity, created_entity);
				handle_component<Pipeline>(json_entity, created_entity);
				handle_component<BasicGeometry>(json_entity, created_entity);
				handle_component<Texture>(json_entity, created_entity);
				handle_component<SphereIntersectible>(json_entity, created_entity);
			}
		}

		for (const auto& type_ids : unmapped_deserialisation) {
			Alabaster::Log::info("Unmapped deserialiser: {}", type_ids);
		}
	}

} // namespace SceneSystem
