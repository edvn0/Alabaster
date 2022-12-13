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

	class DeserialiserNotImplementedException : AlabasterException {
	public:
		DeserialiserNotImplementedException(auto&& exc)
			: AlabasterException(exc) {};
	};

	class ComponentMissingInJsonNodeException : AlabasterException {
	public:
		ComponentMissingInJsonNodeException(auto&& exc)
			: AlabasterException(exc) {};
	};

	class DeserialisationFailedException : AlabasterException {
	public:
		DeserialisationFailedException(auto&& exc)
			: AlabasterException(exc) {};
	};

} // namespace SceneSystem

namespace SceneSystem {

	using namespace SceneSystem::Component;

	static std::unordered_set<std::string> unmapped_deserialisation;

	template <IsComponent T> static constexpr auto handle_component(const nlohmann::json& json_node, auto& entity)
	{
		try {
			constexpr const auto name = component_name<T>;

			if constexpr (std::empty(name))
				throw DeserialiserNotImplementedException("You have not implemented deserialisation");

			if (!json_node.contains(name))
				throw ComponentMissingInJsonNodeException("Could not find key for this object");

			if (const auto component = json_node[name]; component.is_object()) {
				try {
					deserialise_component<T>()(component, entity);
				} catch (const std::exception& e) {
					throw DeserialisationFailedException("Could not deserialise this component with the deserialiser.");
				};
			}
		} catch (const DeserialiserNotImplementedException& e) {
			unmapped_deserialisation.emplace(typeid(T).name());
			return;
		} catch (const ComponentMissingInJsonNodeException& f) {
			return;
		} catch (const DeserialisationFailedException& g) {
			return;
		}
	}

	void SceneDeserialiser::deserialise(const std::filesystem::path& scene_path, Scene& out)
	{
		std::ifstream json_file(scene_path);
		if (!json_file) {
			throw Alabaster::AlabasterException("Could not open scene file.");
		}

		nlohmann::json data = nlohmann::json::parse(json_file);

		std::string scene_name = data["scene_name"];

		auto json_entities = data["entities"];

		for (const auto& json_entity : json_entities) {
			auto created_entity = Entity { &out, "Unnamed entity" };
			if (json_entity.is_object()) {
				handle_component<Mesh>(json_entity, created_entity);
				handle_component<Transform>(json_entity, created_entity);
				handle_component<ID>(json_entity, created_entity);
				handle_component<Light>(json_entity, created_entity);
				handle_component<Tag>(json_entity, created_entity);
				handle_component<Pipeline>(json_entity, created_entity);
				handle_component<BasicGeometry>(json_entity, created_entity);
				handle_component<Texture>(json_entity, created_entity);
			}
		}

		for (const auto& type_ids : unmapped_deserialisation) {
			Alabaster::Log::info("Unmapped deserialiser: {}", type_ids);
		}
	}

} // namespace SceneSystem
