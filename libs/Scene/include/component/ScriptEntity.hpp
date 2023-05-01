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

		template <Component::IsComponent T> T& get_component() { return entity.get_component<T>(); }
		template <Component::IsComponent T> const T& get_component() const { return entity.get_component<T>(); }

		virtual Component::Transform& get_transform() { return entity.get_component<Component::Transform>(); };
		virtual Component::Tag& get_tag() { return entity.get_component<Component::Tag>(); };

		virtual void on_update(const float ts) = 0;
		virtual void on_create() = 0;
		virtual void on_delete() = 0;

		Entity entity;
		friend Scene;
	};

	/// @brief Class for scripting binding to CPP
	class ScriptableEntity {
	public:
		std::string_view name;
	};

} // namespace SceneSystem
