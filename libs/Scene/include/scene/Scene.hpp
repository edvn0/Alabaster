#pragma once

#include "component/Component.hpp"
#include "CoreForward.hpp"
#include "graphics/Camera.hpp"

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

	private:
		void build_scene();

	private:
		entt::registry registry;

		std::unique_ptr<Alabaster::EditorCamera> scene_camera;

		std::unique_ptr<Alabaster::Renderer3D> scene_renderer;
		std::unique_ptr<Alabaster::CommandBuffer> command_buffer;

		std::unique_ptr<Alabaster::Mesh> viking_room_model;
		std::unique_ptr<Alabaster::Mesh> sphere_model;
		std::unique_ptr<Alabaster::Mesh> cube_model;

		std::unique_ptr<Alabaster::Pipeline> viking_pipeline;
		std::unique_ptr<Alabaster::Pipeline> sun_pipeline;

		VkRenderPass first_renderpass { nullptr };

		friend Entity;
	};

} // namespace SceneSystem
