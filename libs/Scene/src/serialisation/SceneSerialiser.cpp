#include "scene_pch.hpp"

#include "serialisation/SceneSerialiser.hpp"

#include "component/Component.hpp"
#include "core/Logger.hpp"
#include "core/Utilities.hpp"
#include "entity/Entity.hpp"
#include "scene/Scene.hpp"
#include "serialisation/ComponentSerialiser.hpp"
#include "utilities/Time.hpp"
#include "uuid.h"

#include <AssetManager.hpp>

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

	nlohmann::json serialise_entity(Entity& entity)
	{
		auto output_object = nlohmann::json::object();
		handle_component_serialisation<Component::ID>(entity, output_object);
		handle_component_serialisation<Component::Tag>(entity, output_object);
		handle_component_serialisation<Component::Mesh>(entity, output_object);
		handle_component_serialisation<Component::SphereIntersectible>(entity, output_object);
		handle_component_serialisation<Component::Camera>(entity, output_object);
		handle_component_serialisation<Component::Texture>(entity, output_object);
		handle_component_serialisation<Component::Transform>(entity, output_object);
		handle_component_serialisation<Component::BasicGeometry>(entity, output_object);
		handle_component_serialisation<Component::Pipeline>(entity, output_object);
		return output_object;
	}

	void SceneSerialiser::serialise_to_json()
	{
		using json = nlohmann::json;
		auto& registry = scene.get_registry();

		time_stamp = Alabaster::Time::formatted_time();

		std::string name = { fmt::format("{}", time_stamp) };

		output_json["scene_name"] = name;

		std::vector<Entity> entities;
		auto view = registry.view<const Component::Tag>();
		view.each([&scene = scene, &entities](
					  entt::entity entity, const Component::Tag& tag) mutable { entities.push_back(scene.create_entity(entity, tag.tag)); });

		if (registry.size() != entities.size()) {
			Alabaster::Log::info("We could not create these entities..");
		}

		AssetManager::ThreadPool pool { 16 };
		auto all_serialised_entities = json::array();
		using namespace Alabaster::Utilities;
		for (auto chunked = split_into(entities, 16); auto& chunk : chunked) {
			futures.emplace_back(pool.push([chunk = std::move(chunk)](int) mutable {
				auto serialised_entities = json::array();
				for (auto& entity : chunk) {
					serialised_entities.push_back(serialise_entity(entity));
				}
				return serialised_entities;
			}));
		}

		pool.stop(true);
	}

	void SceneSerialiser::write_to_dir() noexcept
	{
		has_written = true;

		json entities_array = json::array();
		try {
			for (auto& entity_array_future : futures) {
				for (auto array = entity_array_future.get(); const auto& entity_array : array) {
					entities_array.push_back(entity_array);
				}
			}
		} catch (const std::future_error& error) {
			Alabaster::Log::error("Could not serialise scene because the thread pool threw. Will not write file. More info: {}", error.what());
			return;
		} catch (const std::exception& error) {
			Alabaster::Log::error("Could not serialise scene because {}. Will not write file.", error.what());
			return;
		}

		output_json["entities"] = entities_array;

		try {
			const auto filename = time_stamp + "_" + scene.get_name() + ".scene";
			const auto output_file = Alabaster::IO::scene(filename);
			std::ofstream scene_output(output_file);
			if (!scene_output) {
				Alabaster::Log::warn("[SceneSerialiser] Could not write scene to {}.", output_file.string());
				return;
			}

			scene_output << std::setw(4) << output_json << '\n';
			Alabaster::Log::info("[SceneSerialiser] Wrote scene to file {}", output_file.string());
		} catch (const std::exception& e) {
			Alabaster::Log::warn("[SceneSerialiser] Could not serialise because: {}", e.what());
		}
	}

} // namespace SceneSystem
