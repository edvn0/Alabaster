#pragma once

#include "entity/Entity.hpp"

#include <entt/entt.hpp>
#include <string_view>

namespace SceneSystem {

	class Scene;

	/// @brief Base class for two way client binding (API)
	class ScriptEntity {
	public:
		virtual ~ScriptEntity() = default;
		ScriptEntity() = default;
		explicit ScriptEntity(SceneSystem::Entity entity)
			: script_entity(entity) {};

		template <Component::IsComponent T> T& get_component() { return script_entity.get_component<T>(); }
		template <Component::IsComponent T> const T& get_component() const { return script_entity.get_component<T>(); }

		virtual Component::Transform& get_transform() { return script_entity.get_component<Component::Transform>(); };
		virtual Component::Tag& get_tag() { return script_entity.get_component<Component::Tag>(); };

		virtual void on_update(const float) {
			// NO-OP
		};
		virtual void on_create() {
			// NO-OP
		};
		virtual void on_delete() {
			// NO-OP
		};

		virtual void set_entity(Entity e) { script_entity = e; };

		Entity script_entity;
		friend Scene;
	};

} // namespace SceneSystem
