#pragma once

#include "component/Component.hpp"
#include "CoreForward.hpp"
#include "graphics/Camera.hpp"
#include "uuid.h"

#include <entt/entt.hpp>
#include <vulkan/vulkan.h>

namespace SceneSystem {

	class Entity;

	class Scene {
	public:
		Scene();
		~Scene();

		void update(float ts);
		void initialise();
		void on_event(Alabaster::Event& event);
		void shutdown();
		void ui(float ts);

	public:
		void delete_entity(const std::string& tag);
		void delete_entity(const uuids::uuid& uuid);
		void delete_entity(const Entity& entity);
		void create_entity();
		void create_entity(std::string_view name);
		const auto& get_registry() const { return registry; }

		template <Component::IsComponent... T> auto all_with() { return registry.view<T...>(); }
		void for_each_entity(auto&& func) { registry.each(func); }

		auto get_name() const { return Component::ID().identifier; }

		const std::shared_ptr<Alabaster::Image>& final_image() const;

	private:
		void build_scene();

	private:
		entt::registry registry;

		std::shared_ptr<Alabaster::EditorCamera> scene_camera;
		std::shared_ptr<Alabaster::Framebuffer> framebuffer;
		std::unique_ptr<Alabaster::Renderer3D> scene_renderer;
		std::unique_ptr<Alabaster::CommandBuffer> command_buffer;

		friend Entity;
	};

} // namespace SceneSystem
