#pragma once

#include "CoreForward.hpp"
#include "component/Component.hpp"
#include "graphics/Camera.hpp"
#include "graphics/CommandBuffer.hpp"
#include "graphics/Renderer3D.hpp"

#include <entt/entt.hpp>
#include <uuid.h>
#include <vulkan/vulkan.h>

namespace SceneSystem {

	class Entity;

	enum class SceneState : std::uint8_t { Play, Simulate, Edit };

	class Scene {
	public:
		Scene() noexcept;
		~Scene();

		void update(float ts);
		void initialise();
		void on_event(Alabaster::Event& event);
		void shutdown();
		void ui(float ts);

		template <class Bound> void update_viewport_sizes(const glm::vec2& sizes, Bound&& bounds, const glm::vec2& offset)
		{
			viewport_size = sizes;
			viewport_bounds = std::forward<Bound>(bounds);
			viewport_offset = offset;
		}

		void draw_entities_in_scene(float ts);
		void update_intersectibles();

		void delete_entity(const std::string& tag);
		void delete_entity(const uuids::uuid& uuid);
		void delete_entity(const Entity& entity);
		Entity create_entity(const std::string& name);
		Entity create_entity(const Entity& name);
		Entity create_entity(entt::entity name);
		Entity create_entity(entt::entity name, const std::string& tag_name);

		const auto& get_registry() const { return registry; }
		auto& get_registry() { return registry; }

		template <Component::IsComponent... T> auto all_with() { return registry.view<T...>(); }

		template <typename Func> void for_each_entity(Func&& func) { registry.each(std::forward<Func>(func)); }

		auto get_name() const { return Component::ID().identifier; }

		const std::shared_ptr<Alabaster::Image>& final_image() const;

		const Entity* get_selected_entity() const { return selected_entity.get(); }
		Entity* get_selected_entity() { return selected_entity.get(); }

		void update_selected_entity();

		const auto& get_camera() const { return scene_camera; }
		auto& get_camera() { return scene_camera; }

		void clear() { registry.clear(); };

		bool is_paused() const { return paused; }
		void set_paused(bool in_pause) { paused = in_pause; }

	private:
		void pick_entity(const glm::vec3& ray_world);
		void pick_mouse();
		void build_scene();

		entt::registry registry;

		std::array<glm::vec2, 2> viewport_bounds { glm::vec2 { 0 }, glm::vec2 { 0 } }; // top left, bottom right XY vectors
		glm::vec2 viewport_size { 0 };
		glm::vec2 viewport_offset { 0 };

		std::unique_ptr<Entity> selected_entity { nullptr };
		std::unique_ptr<Entity> hovered_entity { nullptr };

		std::shared_ptr<Alabaster::EditorCamera> scene_camera;
		std::shared_ptr<Alabaster::Framebuffer> framebuffer;
		std::unique_ptr<Alabaster::Renderer3D> scene_renderer;
		std::unique_ptr<Alabaster::CommandBuffer> command_buffer;

		bool paused { false };

		friend Entity;
	};

} // namespace SceneSystem
