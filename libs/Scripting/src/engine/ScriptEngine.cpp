#include "script_pch.hpp"

#include "engine/ScriptEngine.hpp"

#include "component/Component.hpp"
#include "engine/ScriptingEngine.hpp"
#include "entity/Entity.hpp"
#include "scene/Scene.hpp"

namespace Scripting {

	template <typename T> static auto id(T&& entity) { return entity.template get_component<SceneSystem::Component::ID>().identifier; }

	static constexpr auto valid(auto& scene) { Alabaster::assert_that(scene, "Scene must never be null"); }

	ScriptEngine::~ScriptEngine() { engine.destroy(); }

	ScriptEngine::ScriptEngine()
		: current_scene(nullptr)
	{
		engine.initialise();
	}

	void ScriptEngine::set_scene(SceneSystem::Scene* scene) { current_scene.reset(scene); }

	const SceneSystem::Scene* ScriptEngine::get_scene() const { return current_scene.get(); }

	void ScriptEngine::entity_on_create(SceneSystem::Entity& entity)
	{
		valid(current_scene);
		entity_map[id(entity)] = entity;
	}

	void ScriptEngine::entity_on_update(SceneSystem::Entity&, float) { valid(current_scene); }

	void ScriptEngine::entity_on_delete(SceneSystem::Entity& entity)
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
