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

	private:
		std::unique_ptr<Alabaster::Renderer3D> scene_renderer;
		entt::registry registry;

		friend Entity;
	};

} // namespace SceneSystem