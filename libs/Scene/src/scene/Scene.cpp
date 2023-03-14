#include "scene_pch.hpp"

#include "scene/Scene.hpp"

#include "cache/ResourceCache.hpp"
#include "component/Component.hpp"
#include "core/Application.hpp"
#include "core/Input.hpp"
#include "core/Random.hpp"
#include "core/Window.hpp"
#include "entity/Entity.hpp"
#include "graphics/Camera.hpp"
#include "graphics/CommandBuffer.hpp"
#include "graphics/Framebuffer.hpp"
#include "graphics/GraphicsContext.hpp"
#include "graphics/IndexBuffer.hpp"
#include "graphics/Mesh.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/PushConstantRange.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/Renderer3D.hpp"
#include "graphics/Vertex.hpp"
#include "graphics/VertexBufferLayout.hpp"
#include "serialisation/SceneDeserialiser.hpp"
#include "serialisation/SceneSerialiser.hpp"

#include <GLFW/glfw3.h>
#include <imgui/imgui.h>

namespace SceneSystem {

	static glm::vec4 pos { -5, 5, 5, 1.0f };
	static glm::vec4 col { 255 / 255.0, 153 / 255.0, 51 / 255.0, 255.0f / 255.0 };
	static float ambience { 1.0f };

	template <class Position = glm::vec3> static constexpr auto axes(const auto& renderer, const Position& position)
	{
		renderer->line(position, position + glm::vec3 { 1, 0, 0 }, { 1, 0, 0, 1 });
		renderer->line(position, position + glm::vec3 { 0, -1, 0 }, { 0, 1, 0, 1 });
		renderer->line(position, position + glm::vec3 { 0, 0, -1 }, { 0, 0, 1, 1 });
	};

	void Scene::build_scene()
	{
		using namespace Alabaster;
		auto viking_room_model = Mesh::from_file("viking_room.obj");
		auto sphere_model = Mesh::from_file("sphere.obj");
		auto cube_model = Mesh::from_file("cube.obj");

		const auto&& [w, h] = Application::the().get_window()->size();
		FramebufferSpecification fbs;
		fbs.width = w;
		fbs.height = h;
		fbs.attachments = { ImageFormat::RGBA, ImageFormat::DEPTH32F };
		fbs.samples = 1;
		fbs.clear_colour = { 0.0f, 0.0f, 0.0f, 1.0f };
		fbs.debug_name = "Geometry";
		fbs.clear_depth_on_load = true;
		framebuffer = Framebuffer::create(fbs);

		PipelineSpecification viking_spec { .shader = *AssetManager::asset<Alabaster::Shader>("viking"),
			.debug_name = "Viking Pipeline",
			.render_pass = framebuffer->get_renderpass(),
			.topology = Topology::TriangleList,
			.vertex_layout
			= VertexBufferLayout { VertexBufferElement(ShaderDataType::Float3, "position"), VertexBufferElement(ShaderDataType::Float4, "colour"),
				VertexBufferElement(ShaderDataType::Float3, "normal"), VertexBufferElement(ShaderDataType::Float3, "tangent"),
				VertexBufferElement(ShaderDataType::Float3, "bitangent"), VertexBufferElement(ShaderDataType::Float2, "uvs") },
			.ranges = PushConstantRanges { PushConstantRange(PushConstantKind::Both, sizeof(PC)) } };
		auto viking_pipeline = Pipeline::create(viking_spec);

		PipelineSpecification sun_spec { .shader = *AssetManager::asset<Alabaster::Shader>("mesh"),
			.debug_name = "Sun Pipeline",
			.render_pass = framebuffer->get_renderpass(),
			.topology = Topology::TriangleList,
			.vertex_layout
			= VertexBufferLayout { VertexBufferElement(ShaderDataType::Float3, "position"), VertexBufferElement(ShaderDataType::Float4, "colour"),
				VertexBufferElement(ShaderDataType::Float3, "normal"), VertexBufferElement(ShaderDataType::Float3, "tangent"),
				VertexBufferElement(ShaderDataType::Float3, "bitangent"), VertexBufferElement(ShaderDataType::Float2, "uvs") },
			.ranges = PushConstantRanges { PushConstantRange(PushConstantKind::Both, sizeof(PC)) } };
		auto sun_pipeline = Pipeline::create(sun_spec);

		for (std::uint32_t i = 0; i < 200; i++) {
			Entity entity = create_entity(fmt::format("Sphere-{}", i));
			entity.add_component<Component::Mesh>(sphere_model);
			Component::Transform& transform = entity.get_component<Component::Transform>();
			transform.position = sphere_vector3(Random::get<float>(30.f, 90.f));
			entity.add_component<Component::Texture>(glm::vec4(1.0f));
			entity.add_component<Component::Pipeline>(sun_pipeline);
			entity.add_component<Component::SphereIntersectible>();
		}

		Entity floor = create_entity("Floor");
		floor.add_component<Component::BasicGeometry>(Component::Geometry::Quad);
		Component::Transform& floor_transform = floor.get_component<Component::Transform>();
		floor_transform.scale = { 200, 200, .2 };
		floor_transform.position.y += 30;
		floor_transform.rotation = glm::rotate(glm::mat4 { 1.0f }, glm::radians(90.0f), { 1, 0, 0 });
		floor.add_component<Component::Texture>(glm::vec4 { 0.3f, 0.2f, 0.3f, 0.7f });

		struct Plane {
			glm::vec3 pos;
			glm::vec4 col;
			glm::vec3 scale;
			float rotation;
		};
		std::array<Plane, 9> quad_data {};
		quad_data[0] = { { 0, 0, -30 }, { 0.2, 0.3, 0.1, 1.0f }, { 10.0, 10.0, .3f }, glm::radians(90.0f) };
		quad_data[1] = { { 30, 0, -30 }, { 0.2, 0.3, 0.1, 1.0f }, { 10.0, 10.0, .3f }, glm::radians(90.0f) };
		quad_data[2] = { { -30, 0, -30 }, { 0.2, 0.3, 0.1, 1.0f }, { 10.0, 10.0, .3f }, glm::radians(90.0f) };

		quad_data[3] = { { 0, 0, 0 }, { 0.2, 0.3, 0.1, 1.0f }, { 10.0, 10.0, .3f }, glm::radians(90.0f) };
		quad_data[4] = { { 30, 0, 0 }, { 0.2, 0.3, 0.1, 1.0f }, { 10.0, 10.0, .3f }, glm::radians(90.0f) };
		quad_data[5] = { { -30, 0, 0 }, { 0.2, 0.3, 0.1, 1.0f }, { 10.0, 10.0, .3f }, glm::radians(90.0f) };

		quad_data[6] = { { 0, 0, 30 }, { 0.2, 0.3, 0.1, 1.0f }, { 10.0, 10.0, .3f }, glm::radians(90.0f) };
		quad_data[7] = { { 30, 0, 30 }, { 0.2, 0.3, 0.1, 1.0f }, { 10.0, 10.0, .3f }, glm::radians(90.0f) };
		quad_data[8] = { { -30, 0, 30 }, { 0.2, 0.3, 0.1, 1.0f }, { 10.0, 10.0, .3f }, glm::radians(90.0f) };

		for (std::uint32_t quad_index = 0; quad_index < quad_data.size(); quad_index++) {
			Entity entity = create_entity(fmt::format("Quad-{}", quad_index));
			entity.add_component<Component::BasicGeometry>(Component::Geometry::Quad);

			Component::Transform& transform = entity.get_component<Component::Transform>();
			transform.position = quad_data[quad_index].pos;
			transform.scale = quad_data[quad_index].scale;
			transform.rotation = glm::rotate(glm::mat4 { 1.0f }, quad_data[quad_index].rotation, { 1, 0, 0 });

			entity.add_component<Component::Texture>(quad_data[quad_index].col);
		}

		Entity viking = create_entity("Viking Room");
		auto rot = glm::rotate(glm::mat4 { 1.0f }, glm::radians(90.0f), glm::vec3 { 1, 0, 0 });
		viking.add_component<Component::Mesh>(viking_room_model);
		viking.add_component<Component::Pipeline>(viking_pipeline);
		viking.add_component<Component::Texture>(glm::vec4 { 0.6, 0.1, 1, 1 }, AssetManager::asset<Alabaster::Texture>("viking_room.png"));

		auto& viking_transform = viking.get_transform();
		viking_transform.rotation = rot;
		viking_transform.scale = { 15, 15, 15 };

		Entity sun = create_entity("The Sun");
		sun.add_component<Component::Light>();
		sun.add_component<Component::Mesh>(sphere_model);
		sun.add_component<Component::Texture>(glm::vec4 { 1, 1, 1, 1 });
		auto& sun_transform = sun.get_transform();
		sun_transform.scale = { 3, 3, 3 };
	}

	Scene::Scene() noexcept { selected_entity = std::make_unique<Entity>(); };

	Scene::~Scene() = default;

	void Scene::pick_entity(const glm::vec3& ray_wor)
	{
		const auto camera_position = scene_camera->get_position();
		entt::entity found_entity = entt::null;
		float t_dist = 1000.0f;
		auto mesh_view = registry.view<const Component::SphereIntersectible>();
		mesh_view.each(
			[&camera_position, &ray_wor, &t_dist, &found_entity](const entt::entity& entity, const Component::SphereIntersectible& intersectible) {
				float distance = 1000.0f;
				const bool intersected = intersectible.intersects_with(ray_wor, camera_position, distance);
				if (intersected && distance < t_dist) {
					t_dist = distance;
					found_entity = entity;
				}
			});

		if (found_entity != entt::null) {
			*selected_entity = Entity(this, found_entity);
		} else {
			*selected_entity = {};
		}
	}

	template <class Vec = glm::vec3> static constexpr auto xy(const Vec& vec) { return vec.xy; }

	void Scene::pick_mouse()
	{
		const auto mouse_pos = Alabaster::Input::mouse_position();
		const auto size = viewport_size;

		int x_offset;
		int y_offset;
		glfwGetWindowPos(Alabaster::Application::the().get_window()->native(), &x_offset, &y_offset);

		const auto offset_x = static_cast<float>(x_offset);
		const auto offset_y = static_cast<float>(y_offset);
		const auto vp_x = viewport_bounds[0].x;
		const auto vp_y = viewport_bounds[0].y;

		glm::vec2 offset { offset_x, offset_y };
		glm::vec2 vp { vp_x, vp_y };

		const auto mouse_vp = (mouse_pos + offset) - vp;
		if (mouse_vp.x < 0.0f || mouse_vp.y < 0.0f || mouse_vp.x > size.x || mouse_vp.y > size.y) {
			*selected_entity = {};
			return;
		}

		float x = (2.0f * mouse_vp.x) / size.x - 1.0f;
		float y = 1.0f - (2.0f * mouse_vp.y) / size.y;
		float z = 1.0f;
		glm::vec3 ray_nds { x, -y, z };

		glm::vec4 ray_clip(ray_nds.x, ray_nds.y, -1.0, 1.0);

		glm::vec4 ray_eye = glm::inverse(scene_camera->get_projection_matrix()) * ray_clip;
		ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);

		glm::vec3 ray_wor = glm::inverse(scene_camera->get_view_matrix()) * ray_eye;
		ray_wor = glm::normalize(ray_wor);

		pick_entity(ray_wor);
	}

	void Scene::update(float ts)
	{
		scene_camera->on_update(ts);
		command_buffer->begin();
		scene_renderer->begin_scene();
		scene_renderer->reset_stats();
		draw_entities_in_scene(ts);
		update_intersectibles();
		scene_renderer->end_scene(*command_buffer, framebuffer);
		command_buffer->end();
		command_buffer->submit();
	}

	void Scene::update_intersectibles()
	{
		auto mesh_view = registry.view<const Component::Transform, Component::SphereIntersectible>();
		mesh_view.each([](const Component::Transform& transform, Component::SphereIntersectible& intersectible) {
			intersectible.update(transform.position, transform.scale, transform.rotation);
		});
	}

	void Scene::draw_entities_in_scene(float ts)
	{
		static double x = 0.0;
		x += 0.8 * ts;

		static double y = 0.0;
		y += 0.8 * ts;

		auto light_view = registry.view<Component::Transform, const Component::Mesh, const Component::Texture, const Component::Light>();
		light_view.each([&renderer = scene_renderer](Component::Transform& transform, const Component::Mesh& mesh, const Component::Texture& texture,
							const Component::Light&) {
			double cosx = 30 * glm::cos(glm::radians(x));
			double z = 30 * glm::sin(glm::radians(y));
			double pos_y = (cosx * z) / 30;

			pos = { cosx, pos_y, z, 1.0f };
			transform.position = pos;

			renderer->mesh(mesh.mesh, transform.to_matrix(), texture.colour);
		});

		axes(scene_renderer, glm::vec3 { 0, -0.1, 0 });
		scene_renderer->set_light_data(pos, col, ambience);

		auto mesh_view = registry.view<Component::Transform, const Component::Mesh, const Component::Texture, const Component::Pipeline>();
		mesh_view.each(
			[&renderer = scene_renderer](const Component::Transform& transform, const Component::Mesh& mesh, const Component::Texture& texture,
				const Component::Pipeline& pipeline) { renderer->mesh(mesh.mesh, transform.to_matrix(), pipeline.pipeline, texture.colour); });

		auto quad_view = registry.view<const Component::Transform, const Component::BasicGeometry, const Component::Texture>();
		quad_view.each([&renderer = scene_renderer](
						   const Component::Transform& transform, const Component::BasicGeometry& geom, const Component::Texture& texture) {
			if (geom.geometry == Component::Geometry::Quad)
				renderer->quad(transform.to_matrix(), texture.colour);
		});
	}

	void Scene::on_event(Alabaster::Event& event)
	{
		scene_camera->on_event(event);

		Alabaster::EventDispatcher dispatch(event);
		dispatch.dispatch<Alabaster::WindowResizeEvent>([this](const Alabaster::WindowResizeEvent& e) {
			const auto vertical_fov = glm::degrees(scene_camera->get_vertical_fov());
			scene_camera.reset(new Alabaster::EditorCamera(
				vertical_fov, static_cast<float>(e.width()), static_cast<float>(e.height()), 0.1f, 1000.0f, scene_camera.get()));
			scene_renderer->set_camera(scene_camera);
			return false;
		});

		dispatch.dispatch<Alabaster::MouseButtonPressedEvent>([this](const Alabaster::MouseButtonPressedEvent& mouse_button_pressed_event) {
			if (mouse_button_pressed_event.get_mouse_button() == Alabaster::Mouse::Left) {
				pick_mouse();
			}
			return false;
		});
	}

	void Scene::shutdown()
	{
		registry.clear();

		if (scene_renderer)
			scene_renderer->destroy();

		if (framebuffer)
			framebuffer->destroy();

		if (command_buffer)
			command_buffer->destroy();

		SceneSerialiser serialiser(*this);
	}

	void Scene::initialise()
	{
		auto&& [w, h] = Alabaster::Application::the().get_window()->size();

		scene_camera = std::make_shared<Alabaster::EditorCamera>(74.0f, static_cast<float>(w), static_cast<float>(h), 0.1f, 1000.0f);
		scene_renderer = std::make_unique<Alabaster::Renderer3D>(scene_camera);
		command_buffer = std::make_unique<Alabaster::CommandBuffer>(3);
		selected_entity = std::make_unique<Entity>();

		build_scene();
	}

	void Scene::ui(float) { }

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

	void Scene::delete_entity(const Entity& entity) { registry.destroy(entity.entity_handle); }

	Entity Scene::create_entity(const std::string& name)
	{
		Entity entity { this, registry.create(), name };
		entity.add_component<Component::ID>();
		entity.add_component<Component::Transform>();
		auto& tag = entity.emplace_component<Component::Tag>();
		tag.tag = name.empty() ? "Entity" : name;

		return entity;
	}

	Entity Scene::create_entity(const Entity& name)
	{
		Entity entity { this, name.entity_handle, name.get_tag().tag };
		entity.add_component<Component::ID>();
		entity.add_component<Component::Transform>();
		auto& tag = entity.emplace_component<Component::Tag>();
		tag.tag = name.get_tag().tag.empty() ? "Entity" : name.get_tag().tag;

		return entity;
	}

	Entity Scene::create_entity(entt::entity name)
	{
		Entity entity { this, name };
		entity.add_component<Component::ID>();
		entity.add_component<Component::Transform>();

		return entity;
	}

	const std::shared_ptr<Alabaster::Image>& Scene::final_image() const { return framebuffer->get_image(); }

} // namespace SceneSystem
