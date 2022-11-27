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

	void Scene::build_scene()
	{
		sphere = Alabaster::Mesh::from_file("sphere.obj");

		for (std::uint32_t i = 0; i < 100; i++) {
			auto entity = Entity(*this);
			entity.add_component<Component::Mesh>(sphere);
			entity.add_component<Component::Texture>(glm::vec4(1.0f));
		}
	}

	Scene::Scene(Alabaster::Camera& camera)
		: registry()
		, scene_renderer(new Alabaster::Renderer3D(camera))
		, command_buffer(new Alabaster::CommandBuffer(3))
	{

		build_scene();
	}

	Scene::~Scene() { scene_renderer->destroy(); };

	void Scene::update(float)
	{
		command_buffer->begin();
		scene_renderer->begin_scene();
		auto view = registry.view<Component::Transform, Component::Mesh, Component::Texture>();
		view.each(
			[&renderer = scene_renderer](const Component::Transform& transform, const Component::Mesh& mesh, const Component::Texture& texture) {
				const auto t = transform.to_matrix();
				const auto colour = texture.colour;
				renderer->mesh(mesh.mesh, t, nullptr, colour);
			});
		scene_renderer->end_scene(command_buffer);
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
			ImGui::Text(fmt::format("ID: {}, Name: {}", id.identifier, std::string(tag.tag)).c_str());
			ImGui::PopID();
		});

		ImGui::End();
	}

} // namespace SceneSystem