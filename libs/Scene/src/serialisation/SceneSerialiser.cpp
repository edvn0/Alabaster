#include "scene_pch.hpp"

#include "serialisation/SceneSerialiser.hpp"

#include "component/Component.hpp"
#include "core/Logger.hpp"
#include "entity/Entity.hpp"
#include "scene/Scene.hpp"
#include "serialisation/ComponentSerialiser.hpp"
#include "uuid.h"

#include <fstream>

namespace SceneSystem {
	SceneSerialiser::SceneSerialiser(Scene& scene) noexcept
		: scene(scene)
	{
		serialise_to_json();
	};

	nlohmann::json::object_t SceneSerialiser::serialise_entity(Entity& entity)
	{
		auto output_object = nlohmann::json::object();
		if (entity.has_component<Component::Tag>()) {
			serialise_component<Component::Tag> {}(entity, output_object);
		}
		if (entity.has_component<Component::Mesh>()) {
			serialise_component<Component::Mesh> {}(entity, output_object);
		}
		if (entity.has_component<Component::Texture>()) {
			serialise_component<Component::Texture> {}(entity, output_object);
		}
		if (entity.has_component<Component::Transform>()) {
			serialise_component<Component::Transform> {}(entity, output_object);
		}
		if (entity.has_component<Component::BasicGeometry>()) {
			serialise_component<Component::BasicGeometry> {}(entity, output_object);
		}

		return output_object;
	}

	void SceneSerialiser::serialise_to_json()
	{
		using json = nlohmann::json;
		const auto& registry = scene.get_registry();

		output_json["scene_name"] = "Unnamed scene";

		std::vector<Entity> entities;
		registry.each([scene = &scene, &entities](const auto entity) { entities.push_back(Entity { scene, entity }); });

		auto all_serialised_entities = json::object();
		registry.each([scene = &scene, &all_serialised_entities, this](const entt::entity& entt_entity) {
			Entity entity { scene, entt_entity };
			const auto serialised_object = serialise_entity(entity);

			all_serialised_entities[uuids::to_string(entity.get_component<Component::ID>().identifier)] = serialised_object;
		});
		output_json["entities"] = all_serialised_entities;
	}

	void SceneSerialiser::write_to_dir() noexcept
	{
		has_written = true;

		const auto output_file = Alabaster::IO::scene(uuids::to_string(scene.get_name()));
		std::ofstream scene_output(output_file);
		if (!scene_output) {
			Alabaster::Log::warn("[SceneSerialiser] Could not write scene to {}.", output_file.string());
			return;
		}

		scene_output << std::setw(4) << output_json << std::endl;

		Alabaster::Log::info("[SceneSerialiser] Wrote scene to file {}", output_file.string());
	}

} // namespace SceneSystem
