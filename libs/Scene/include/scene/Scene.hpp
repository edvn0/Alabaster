#pragma once

#include "component/Component.hpp"
#include "CoreForward.hpp"

#include <entt/entt.hpp>

namespace SceneSystem {

	class Entity;

	class Scene {
	public:
		Scene(Alabaster::Camera& camera);
		~Scene();

		void update(float ts);
		void on_event(Alabaster::Event& event);
		void ui(float ts);

	public:
		void delete_entity(const std::string& tag);
		void delete_entity(const uuids::uuid& uuid);

	private:
		void build_scene();

	private:
		entt::registry registry;
		std::unique_ptr<Alabaster::Renderer3D> scene_renderer;
		std::unique_ptr<Alabaster::CommandBuffer> command_buffer;

		std::unique_ptr<Alabaster::Mesh> sphere;

		friend Entity;
	};

} // namespace SceneSystem
