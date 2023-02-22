#include "scene_pch.hpp"

#include "serialisation/SceneSerialiser.hpp"

#include "component/Component.hpp"
#include "core/Logger.hpp"
#include "entity/Entity.hpp"
#include "scene/Scene.hpp"
#include "serialisation/ComponentSerialiser.hpp"
#include "utilities/Time.hpp"
#include "uuid.h"

namespace SceneSystem {

	SceneSerialiser::SceneSerialiser(Scene& input_scene) noexcept
		: scene(input_scene)
	{
		serialise_to_json();
	}

	template <Component::IsComponent T> static constexpr auto handle_component_serialisation(auto& entity, auto& json)
	{
		if (entity.template has_component<T>()) {
			serialise_component<T>()(entity, json);
		}
	}

	nlohmann::json SceneSerialiser::serialise_entity(Entity& entity)
	{
		auto output_object = nlohmann::json::object();
		handle_component_serialisation<Component::ID>(entity, output_object);
		handle_component_serialisation<Component::Tag>(entity, output_object);
		handle_component_serialisation<Component::Mesh>(entity, output_object);
		handle_component_serialisation<Component::Texture>(entity, output_object);
		handle_component_serialisation<Component::Transform>(entity, output_object);
		handle_component_serialisation<Component::BasicGeometry>(entity, output_object);
		return output_object;
	}

	void SceneSerialiser::serialise_to_json()
	{
		using json = nlohmann::json;
		const auto& registry = scene.get_registry();

		time_stamp = Alabaster::Time::formatted_time();

		output_json["scene_name"] = fmt::format("{}", time_stamp);

		std::vector<Entity> entities;
		registry.each([scene = &scene, &entities](const auto entity) { entities.push_back(Entity { scene, entity }); });

		auto all_serialised_entities = json::array();
		registry.each([scene = &scene, &all_serialised_entities, this](const entt::entity& entt_entity) {
			Entity entity { scene, entt_entity };
			const auto serialised_object = serialise_entity(entity);

			all_serialised_entities.push_back(serialised_object);
		});
		output_json["entities"] = all_serialised_entities;
	}

	void SceneSerialiser::write_to_dir() noexcept
	{
		has_written = true;

		const auto output_file = Alabaster::IO::scene(time_stamp + "_" + to_string(scene.get_name()));
		std::ofstream scene_output(output_file);
		if (!scene_output) {
			Alabaster::Log::warn("[SceneSerialiser] Could not write scene to {}.", output_file.string());
			return;
		}

		scene_output << std::setw(4) << output_json << '\n';

		Alabaster::Log::info("[SceneSerialiser] Wrote scene to file {}", output_file.string());
	}

} // namespace SceneSystem
