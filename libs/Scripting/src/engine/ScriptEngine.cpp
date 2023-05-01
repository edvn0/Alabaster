#include "script_pch.hpp"

#include "engine/ScriptEngine.hpp"

#include "component/Component.hpp"
#include "component/ScriptEntity.hpp"
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

	void ScriptEngine::entity_on_create(SceneSystem::Entity&) { valid(current_scene); }

	void ScriptEngine::entity_on_update(SceneSystem::Entity&, float) { valid(current_scene); }

	void ScriptEngine::entity_on_delete(SceneSystem::Entity&) { valid(current_scene); }

} // namespace Scripting
