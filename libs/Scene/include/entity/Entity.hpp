#pragma once

#include "component/Component.hpp"
#include "scene/Scene.hpp"

#include <entt/entt.hpp>
#include <memory>
#include <utility>

namespace SceneSystem {

	class Entity {
	public:
		Entity() = default;
		explicit Entity(Scene* scene, entt::entity entity_handle, std::string name = "Unnamed entity");
		explicit Entity(Scene* scene, std::string name = "Unnamed entity")
			: Entity(scene, entt ::null, name) {};
		explicit Entity(const std::unique_ptr<Scene>& scene, std::string name = "Unnamed entity")
			: Entity(scene.get(), entt ::null, name) {};
		explicit Entity(const std::shared_ptr<Scene>& scene, std::string name = "Unnamed entity")
			: Entity(scene.get(), entt ::null, name) {};
		Entity(const Entity& other);
		~Entity() {};

		template <Component::IsComponent... T> bool has_any() { return scene->registry.any_of<T...>(entity_handle); }

		template <Component::IsComponent... T> bool has_all() { return scene->registry.all_of<T...>(entity_handle); }

		template <Component::IsComponent... T> bool has_any() const { return scene->registry.any_of<T...>(entity_handle); }

		template <Component::IsComponent... T> bool has_all() const { return scene->registry.all_of<T...>(entity_handle); }

		template <Component::IsComponent T> bool has_component() const { return scene->registry.any_of<T>(entity_handle); }
		template <Component::IsComponent T> bool has_component() { return scene->registry.any_of<T>(entity_handle); }

		template <Component::IsComponent T> T& get_component() { return scene->registry.get<T>(entity_handle); }

		template <Component::IsComponent T> T& get_component() const { return scene->registry.get<T>(entity_handle); }

		template <Component::IsComponent T, typename... Args> void put_component(Args&&... args)
		{
			if (has_component<T>()) {
				auto& component = get_component<T>();
				component = T(std::forward<Args>(args)...);
				return;
			}

			add_component<T>(std::forward<Args>(args)...);
		}

		template <Component::IsComponent T, typename... Args> void add_component(Args&&... args)
		{
			scene->registry.emplace<T>(entity_handle, std::forward<Args>(args)...);
		}

		template <Component::IsComponent T, typename... Args> void add_component() { scene->registry.emplace<T>(entity_handle); }

		template <Component::IsComponent T> auto remove_component() { scene->registry.remove<T>(entity_handle); }

		operator bool() const { return entity_handle != entt::null; }

		bool operator==(const Entity& other) const { return entity_handle == other.entity_handle && scene == other.scene; }

		bool operator!=(const Entity& other) const { return !(*this == other); }

	public:
		auto& get_transform() { return get_component<Component::Transform>(); }
		auto& get_tag() { return get_component<Component::Tag>(); }

	private:
		Scene* scene;
		entt::entity entity_handle { entt::null };

		friend Scene;
	};

} // namespace SceneSystem
