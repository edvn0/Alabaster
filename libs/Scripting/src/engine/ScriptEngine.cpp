#include "script_pch.hpp"

#include "engine/ScriptEngine.hpp"

#include "component/Component.hpp"
#include "engine/ScriptingEngine.hpp"
#include "scene/Scene.hpp"

namespace Scripting {

	template <typename T> static const auto id(T&& entity)
	{
		using namespace SceneSystem::Component;
		return entity.get_component<ID>().identifier;
	}

	static constexpr auto valid(auto& scene) { Alabaster::assert_that(scene, "Scene must never be null"); }

	ScriptEngine::~ScriptEngine() { }

	ScriptEngine::ScriptEngine()
		: engine(ScriptingEngine::create())
		, current_scene(nullptr)
	{
	}

	void ScriptEngine::set_scene(SceneSystem::Scene* scene) { current_scene.reset(scene); }

	void ScriptEngine::entity_on_create(SceneSystem::Entity entity)
	{
		valid(current_scene);
		entity_map[id(entity)] = entity;
	}

	void ScriptEngine::entity_on_update(SceneSystem::Entity entity, float) { valid(current_scene); }

	void ScriptEngine::entity_on_delete(SceneSystem::Entity entity)
	{
		valid(current_scene);

		const auto identifier = id(entity);
		for (auto it = entity_map.begin(); it != entity_map.end();) {
			if (id(it->second) == identifier) {
				it = entity_map.erase(it);
			} else {
				++it;
			}
		}
	}

} // namespace Scripting
