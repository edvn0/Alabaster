#pragma once

#include "component/Component.hpp"
#include "scene/Scene.hpp"

#include <entt/entt.hpp>
#include <utility>

namespace SceneSystem {

	class Entity {
	public:
		explicit Entity(Scene& scene, entt::entity entity_handle, std::string name = "Unnamed entity");
		explicit Entity(Scene& scene, std::string name = "Unnamed entity")
			: Entity(scene, entt ::null, name) {};
		Entity(const Entity& other);
		~Entity() {};

		template <Component::IsComponent... T> bool has_any() { return scene.registry.any_of<T...>(entity_handle); }

		template <Component::IsComponent... T> bool has_all() { return scene.registry.all_of<T...>(entity_handle); }

		template <Component::IsComponent T> bool has_component() { return scene.registry.any_of<T>(entity_handle); }

		template <Component::IsComponent T> T& get_component() { return scene.registry.get<T>(entity_handle); }

		template <Component::IsComponent T> T& get_component() const { return scene.registry.get<T>(entity_handle); }

		template <Component::IsComponent T, typename... Args> auto& put_component(Args&&... args)
		{
			if (has_component<T>()) {
				auto& component = get_component<T>();
				component = T(std::forward<Args>(args)...);
				return component;
			}

			return add_component<T>(std::forward<Args>(args)...);
		}

		template <Component::IsComponent T, typename... Args> auto& add_component(Args&&... args)
		{
			return scene.registry.emplace<T>(entity_handle, std::forward<Args>(args)...);
		}

		template <Component::IsComponent T> auto remove_component() { scene.registry.remove<T>(entity_handle); }

	private:
		Scene& scene;
		entt::entity entity_handle;

		friend Scene;
	};

} // namespace SceneSystem
