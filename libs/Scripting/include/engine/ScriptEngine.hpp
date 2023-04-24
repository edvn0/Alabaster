#pragma once

#include "engine/ScriptingEngine.hpp"
#include "entity/Entity.hpp"
#include "uuid.h"

#include <SceneForward.hpp>
#include <memory>

namespace Scripting {

	class ScriptEngine {
		using EntityUUIDMap = std::unordered_map<uuids::uuid, SceneSystem::Entity>;

	public:
		~ScriptEngine();
		ScriptEngine();

		void set_scene(SceneSystem::Scene*);
		const SceneSystem::Scene* get_scene() const;

		void entity_on_create(SceneSystem::Entity& entity);
		void entity_on_update(SceneSystem::Entity& entity, float ts);
		void entity_on_delete(SceneSystem::Entity& entity);

	private:
		EntityUUIDMap entity_map;
		ScriptingEngine engine;
		std::unique_ptr<SceneSystem::Scene> current_scene;
	};

} // namespace Scripting
