#include "scene_pch.hpp"

#include "scene/Scene.hpp"

#include "cache/ResourceCache.hpp"
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

	static glm::vec4 pos { -5, 5, 5, 1.0f };
	static glm::vec4 col { 255 / 255.0, 153 / 255.0, 51 / 255.0, 255.0f / 255.0 };
	static float ambience { 1.0f };

	void Scene::build_scene()
	{
		sphere = Alabaster::Mesh::from_file("sphere.obj");

		for (std::uint32_t i = 0; i < 100; i++) {
			auto entity = Entity(*this);
			entity.add_component<Component::Mesh>(sphere);
			auto& transform = entity.get_component<Component::Transform>();
			transform.position = Alabaster::sphere_vector3(30);
			transform.scale = { 0.2, 0.2, 0.2 };
			entity.add_component<Component::Texture>(glm::vec4(1.0f));
		}
	}

	Scene::Scene(Alabaster::Camera& camera)
		: registry()
		, scene_renderer(new Alabaster::Renderer3D(camera))
		, command_buffer(Alabaster::CommandBuffer::from_swapchain())
	{

		build_scene();
	}

	Scene::~Scene() { scene_renderer->destroy(); };

	void Scene::update(float)
	{
		command_buffer->begin({}, false);
		scene_renderer->begin_scene();
		scene_renderer->reset_stats();
		scene_renderer->set_light_data(pos, col, ambience);
		auto view = registry.view<Component::Transform, Component::Mesh, Component::Texture>();
		view.each([&renderer = scene_renderer](const Component::Transform& transform, const Component::Mesh& mesh,
					  const Component::Texture& texture) { renderer->mesh(mesh.mesh, transform.to_matrix(), nullptr, texture.colour); });
		scene_renderer->end_scene(command_buffer);
		command_buffer->submit();
	}

	void Scene::on_event(Alabaster::Event&) { }

	void Scene::ui(float)
	{
		auto view = registry.view<Component::ID, Component::Tag>();

		ImGui::Begin("IDs");
		if (ImGui::Button("Add Entity")) {
			Entity entity { *this };
		}
		view.each([](const Component::ID& id, const Component::Tag& tag) {
			ImGui::PushID(static_cast<int>(id.identifier));
			ImGui::Text("ID: %s, Name: %s", std::to_string(id.identifier).c_str(), std::string(tag.tag).c_str());
			ImGui::PopID();
		});

		ImGui::End();
	}

} // namespace SceneSystem
