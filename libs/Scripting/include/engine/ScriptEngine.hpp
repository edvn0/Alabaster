#pragma once

#include "engine/ScriptingEngine.hpp"
#include "entity/Entity.hpp"

#include <SceneForward.hpp>
#include <memory>
#include <uuid.h>

namespace Scripting {

	class ScriptEngine {
		using EntityUUIDMap = std::unordered_map<uuids::uuid, SceneSystem::ScriptableEntity>;

	public:
		~ScriptEngine();
		ScriptEngine();

		void set_scene(SceneSystem::Scene*);
		const SceneSystem::Scene* get_scene() const;

		void entity_on_create(SceneSystem::Entity& entity);
		void entity_on_update(SceneSystem::Entity& entity, float ts);
		void entity_on_delete(SceneSystem::Entity& entity);

	private:
		std::unique_ptr<SceneSystem::Scene> current_scene;
		EntityUUIDMap entity_map;
		ScriptingEngine engine;
	};

} // namespace Scripting
