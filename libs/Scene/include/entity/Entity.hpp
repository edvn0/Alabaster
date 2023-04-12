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
		explicit Entity(Scene* scene, entt::entity entity_handle, const std::string& name = "Unnamed entity");
		explicit Entity(Scene* scene, const std::string& name = "Unnamed entity");
		explicit Entity(const std::unique_ptr<Scene>& scene, const std::string& name = "Unnamed entity");
		explicit Entity(const std::shared_ptr<Scene>& scene, const std::string& name = "Unnamed entity");
		~Entity() = default;

		Entity(const Entity& other);
		Entity& operator=(const Entity& other) = default;

		template <Component::IsComponent... T> bool has_any() { return scene->registry.any_of<T...>(entity_handle); }

		template <Component::IsComponent... T> bool has_all() { return scene->registry.all_of<T...>(entity_handle); }

		template <Component::IsComponent... T> bool has_any() const { return scene->registry.any_of<T...>(entity_handle); }

		template <Component::IsComponent... T> bool has_all() const { return scene->registry.all_of<T...>(entity_handle); }

		template <Component::IsComponent T> bool has_component() const { return scene->registry.any_of<T>(entity_handle); }
		template <Component::IsComponent T> bool has_component() { return scene->registry.any_of<T>(entity_handle); }

		template <Component::IsComponent T> T& get_component() { return scene->registry.get<T>(entity_handle); }

		template <Component::IsComponent T> T& get_component() const { return scene->registry.get<T>(entity_handle); }

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
			if (has_component<T>())
				return get_component<T>();

			return emplace_component<T>(std::forward<Args>(args)...);
		}

		template <Component::IsComponent T, typename... Args> auto& add_component()
		{
			if (has_component<T>())
				return get_component<T>();

			return emplace_component<T>();
		}

		template <Component::IsScriptable Script, typename... Args> inline auto& add_behaviour(std::string_view name, Args&&... args)
		{
			using T = Component::Behaviour;
			if (has_component<T>())
				return get_component<T>();

			auto& component = add_component<T>();
			component.bind<Script>(std::move(name), std::forward<Args>(args)...);
			return component;
		}

		template <Component::IsComponent T> T& emplace_component() { return scene->registry.emplace<T>(entity_handle); }
		template <Component::IsComponent T, typename... Args> T& emplace_component(Args&&... args)
		{
			return scene->registry.emplace<T>(entity_handle, std::forward<Args>(args)...);
		}

		template <Component::IsComponent T> auto remove_component() { scene->registry.remove<T>(entity_handle); }

		bool operator==(const Entity& other) const { return entity_handle == other.entity_handle && scene == other.scene; }

		bool is_valid() const { return is_valid(entity_handle); }
		template <typename Handle> bool is_valid(Handle&& handle) const { return std::forward<Handle>(handle) != entt::null; }

		const auto& get_transform() const { return get_component<Component::Transform>(); }
		auto& get_transform() { return get_component<Component::Transform>(); }
		const auto& get_tag() const { return get_component<Component::Tag>(); }
		auto& get_tag() { return get_component<Component::Tag>(); }

	private:
		Scene* scene { nullptr };
		entt::entity entity_handle { entt::null };

		friend Scene;
	};

} // namespace SceneSystem
