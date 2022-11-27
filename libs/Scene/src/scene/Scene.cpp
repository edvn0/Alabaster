#include "scene_pch.hpp"

#include "scene/Scene.hpp"

#include "core/Application.hpp"
#include "core/Window.hpp"
#include "entity/Entity.hpp"
#include "graphics/Camera.hpp"
#include "graphics/CommandBuffer.hpp"
#include "graphics/GraphicsContext.hpp"
#include "graphics/IndexBuffer.hpp"
#include "graphics/Mesh.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/PushConstantRange.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/Renderer3D.hpp"
#include "graphics/Vertex.hpp"
#include "graphics/VertexBufferLayout.hpp"

#include <imgui/imgui.h>

namespace SceneSystem {

	static int global_key = 0;

	Scene::Scene(Alabaster::Camera& camera)
		: registry()
		, scene_renderer(new Alabaster::Renderer3D(camera))
	{
	}

	Scene::~Scene() { scene_renderer->destroy(); };

	void Scene::update(float ts)
	{

		auto view = registry.view<Component::Transform>();

		view.each([](const Component::Transform& transform) { transform.to_matrix(); });
	}

	void Scene::on_event(Alabaster::Event& event) { }

	void Scene::ui(float ts)
	{

		auto view = registry.view<Component::ID, Component::Tag>();

		ImGui::Begin("IDs");
		if (ImGui::Button("Add Entity")) {
			Entity { *this };
		}
		view.each([](const Component::ID& id, const Component::Tag& tag) {
			ImGui::PushID(id.identifier);
			ImGui::Text(fmt::format("ID: {}, Name: {}", id.identifier, std::string(tag.tag)).c_str());
			ImGui::PopID();
		});

		ImGui::End();
	}

} // namespace SceneSystem