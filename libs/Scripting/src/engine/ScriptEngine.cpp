#include "script_pch.hpp"

#include "engine/ScriptEngine.hpp"

#include "component/Component.hpp"
#include "component/ScriptEntity.hpp"
#include "engine/ScriptingEngine.hpp"
#include "entity/Entity.hpp"
#include "platform/python/PythonEntity.hpp"
#include "scene/Scene.hpp"

#include <AssetManager.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/string_cast.hpp>
#include <pybind11/embed.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>

namespace Scripting {

	namespace detail {
		template <typename T> static auto id(T&& entity)
		{
			return std::forward<T>(entity).template get_component<SceneSystem::Component::ID>().identifier;
		}
		template <typename T> static auto id_string(T&& entity) { return uuids::to_string(id(std::forward<T>(entity))); }
		template <typename T> static auto script_name(T&& entity)
		{
			return std::forward<T>(entity).template get_component<SceneSystem::Component::ScriptBehaviour>().script_name;
		}

		static constexpr auto valid(auto& scene) { Alabaster::assert_that(scene, "Scene must never be null"); }
	} // namespace detail

	pybind11::scoped_interpreter& ScriptEngine::get_script_interpreter()
	{
		static pybind11::scoped_interpreter interpreter;
		return interpreter;
	};

	std::string_view EntityUtility::script_name(SceneSystem::Entity e) const { return detail::script_name(e); }

	uuids::uuid EntityUtility::id(SceneSystem::Entity e) const { return detail::id(e); }

	void ScriptEngine::valid_scene() { detail::valid(current_scene); }

	ScriptEngine::~ScriptEngine() { ScriptingEngine::initialise(); }

	ScriptEngine::ScriptEngine()
		: ScriptEngine(nullptr, std::make_unique<EntityUtility>())
	{
	}

	ScriptEngine::ScriptEngine(SceneSystem::Scene* scene, std::unique_ptr<IEntityUtility> utility)
		: current_scene(scene)
		, entity_utility(std::move(utility))
	{
		ScriptingEngine::initialise();
	}

	void ScriptEngine::set_scene(SceneSystem::Scene* scene) { current_scene = scene; }

	const SceneSystem::Scene* ScriptEngine::get_scene() const { return current_scene; }

	void ScriptEngine::entity_on_update(SceneSystem::Entity entity, const float ts)
	{
		valid_scene();
		entity_map.at(entity_utility->id(entity)).on_update(ts);
	}

	void ScriptEngine::entity_on_delete(SceneSystem::Entity entity)
	{
		valid_scene();
		entity_map.at(entity_utility->id(entity)).on_delete();
	}

	void ScriptEngine::register_file_watcher(AssetManager::FileWatcher& watcher)
	{
		watcher.on_modified([](const auto& info) {
			if (info.to_path().extension() != ".py")
				return;

			ScriptingEngine::reload_script(info.to_path().filename().string());
		});
	}

	ScriptableEntity::ScriptableEntity(const std::string_view in_script_name, SceneSystem::Entity in_entity)
		: script_name(in_script_name)
		, entity(in_entity)
	{
	}

	static std::unordered_map<std::string_view, std::shared_ptr<SceneSystem::ScriptEntity>> entity_map {};

	void ScriptableEntity::call_method(ScriptMethod method, void* args, std::size_t args_size)
	{
		ScriptingEngine::call_method(script_name, entity, method, args, args_size);
	}

	void ScriptableEntity::on_create() { call_method(ScriptMethod::OnCreate); }

	void ScriptableEntity::on_update(float ts)
	{
		void* step = &ts;
		call_method(ScriptMethod::OnUpdate, step, sizeof(float));
	}

	void ScriptableEntity::on_delete() { call_method(ScriptMethod::OnDelete); }

} // namespace Scripting
