#pragma once

#include "entity/Entity.hpp"

namespace SceneSystem {

	class Scene;

	class ScriptEntity {
	public:
		virtual ~ScriptEntity() = default;
		ScriptEntity() = default;

		template <Component::IsComponent T> T& get_component() { return entity.get_component<T>(); }

		template <Component::IsComponent T> const T& get_component() const { return entity.get_component<T>(); }

	protected:
		virtual void on_update(const float ts) = 0;
		virtual void on_create() = 0;
		virtual void on_delete() = 0;

		Entity entity;
		friend Scene;
	};

} // namespace SceneSystem
