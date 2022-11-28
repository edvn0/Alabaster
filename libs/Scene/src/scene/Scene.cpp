#include "scene_pch.hpp"

#include "cache/ResourceCache.hpp"
#include "core/Application.hpp"
#include "core/Random.hpp"
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
#include "scene/Scene.hpp"

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
			float cosx = 30 * glm::cos(glm::radians(transform.position.x));
			float z = 30 * glm::sin(glm::radians(transform.position.y));
			float pos_y = (cosx * z) / 30;

			transform.position = { cosx, pos_y, z };
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
		command_buffer->begin(false);
		{
			scene_renderer->begin_scene();
			scene_renderer->reset_stats();
			scene_renderer->set_light_data(pos, col, ambience);
			auto view = registry.view<Component::Transform, const Component::Mesh, const Component::Texture>();
			view.each([&renderer = scene_renderer](Component::Transform& transform, const Component::Mesh& mesh, const Component::Texture& texture) {
				transform.position = Alabaster::sphere_vector3(30);
				renderer->mesh(mesh.mesh, transform.to_matrix(), texture.colour);
			});
			scene_renderer->end_scene(command_buffer);
		}
		command_buffer->submit();
	}

	void Scene::on_event(Alabaster::Event&) { }

	void Scene::ui(float)
	{
		auto view = registry.view<const Component::ID, const Component::Tag>();

		ImGui::Begin("IDs");
		if (ImGui::Button("Add Entity")) {
			Entity entity { *this };
		}
		view.each([](const Component::ID& id, const Component::Tag& tag) {
			ImGui::Text("ID: %s, Name: %s", id.to_string().c_str(), std::string(tag.tag).c_str());
		});

		ImGui::End();
	}

	void Scene::delete_entity(const std::string& tag)
	{
		registry.view<Component::Tag>().each([tag, &registry = registry](const auto entity, const Component::Tag& tag_component) mutable {
			if (tag_component.tag == tag) {
				registry.destroy(entity);
			}
		});
	}

	void Scene::delete_entity(const uuids::uuid& uuid)
	{
		registry.view<Component::ID>().each([uuid, &registry = registry](const auto entity, const Component::ID& id_component) mutable {
			if (id_component.identifier == uuid) {
				registry.destroy(entity);
			}
		});
	}

} // namespace SceneSystem
