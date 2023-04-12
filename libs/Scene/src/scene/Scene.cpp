#include "scene_pch.hpp"

#include "scene/Scene.hpp"

#include "cache/ResourceCache.hpp"
#include "component/Component.hpp"
#include "component/ScriptEntity.hpp"
#include "entity/Entity.hpp"
#include "serialisation/SceneDeserialiser.hpp"
#include "serialisation/SceneSerialiser.hpp"

#include <Alabaster.hpp>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>

namespace SceneSystem {

	static constexpr auto mouse_picking_interval_ms = 100.0;

	template <class Position = glm::vec3> static constexpr auto axes(const auto& renderer, const Position& position, auto length = 1.0f)
	{
		renderer->line(position, position + glm::vec3 { length, 0, 0 }, { 1, 0, 0, 1 });
		renderer->line(position, position + glm::vec3 { 0, -length, 0 }, { 0, 1, 0, 1 });
		renderer->line(position, position + glm::vec3 { 0, 0, -length }, { 0, 0, 1, 1 });
	};

	void Scene::build_scene()
	{
		using namespace Alabaster;
		auto sphere_model = Mesh::from_file("sphere_subdivided.obj");
		auto simple_sphere_model = Mesh::from_file("sphere.obj");
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

		PipelineSpecification sun_spec { .shader = AssetManager::asset<Alabaster::Shader>("mesh_light"),
			.debug_name = "Sun Pipeline",
			.render_pass = framebuffer->get_renderpass(),
			.topology = Topology::TriangleList,
			.vertex_layout
			= VertexBufferLayout { VertexBufferElement(ShaderDataType::Float3, "position"), VertexBufferElement(ShaderDataType::Float4, "colour"),
				VertexBufferElement(ShaderDataType::Float3, "normal"), VertexBufferElement(ShaderDataType::Float3, "tangent"),
				VertexBufferElement(ShaderDataType::Float3, "bitangent"), VertexBufferElement(ShaderDataType::Float2, "uvs") },
			.ranges = PushConstantRanges { PushConstantRange(PushConstantKind::Both, scene_renderer->default_push_constant_size()) } };
		auto sun_pipeline = Pipeline::create(sun_spec);

		Entity sphere_one = create_entity(fmt::format("Sphere-{}", 0));
		sphere_one.add_component<Component::Mesh>(sphere_model);
		auto& sphere_one_transform = sphere_one.get_component<Component::Transform>();
		sphere_one_transform.scale = glm::vec3 { 0.1f };
		sphere_one_transform.position = { -0.35, -0.5, .5f };
		sphere_one.add_component<Component::Texture>(Alabaster::random_vec4(0, 1));
		sphere_one.add_component<Component::Pipeline>(sun_pipeline);
		sphere_one.add_component<Component::SphereIntersectible>();
		class MoveScript : public ScriptEntity {
		public:
			~MoveScript() override = default;
			void on_create() override { Alabaster::Log::info("Created entity!!!"); }
			void on_delete() override { Alabaster::Log::info("Deleted entity!!!"); }
			void on_update(const float ts)
			{
				auto& component = get_component<Component::Transform>();
				component.position += ts * 0.1;
			}
		};
		sphere_one.add_component<Component::Behaviour>().bind<MoveScript>("MoveScript");

		Entity sphere_two = create_entity(fmt::format("Sphere-{}", 1));
		sphere_two.add_component<Component::Mesh>(sphere_model);
		auto& sphere_two_transform = sphere_two.get_component<Component::Transform>();
		sphere_two_transform.scale = glm::vec3 { 0.1f };
		sphere_two_transform.position = { .35, -0.5, .5f };
		sphere_two.add_component<Component::Texture>(Alabaster::random_vec4(0, 1));
		sphere_two.add_component<Component::Pipeline>(sun_pipeline);
		sphere_two.add_component<Component::SphereIntersectible>();

		Entity floor = create_entity("Floor");
		floor.add_component<Component::BasicGeometry>(Component::Geometry::Quad);
		auto& floor_transform = floor.get_component<Component::Transform>();
		floor_transform.scale = { 5, 5, .2 };
		floor_transform.rotation = glm::rotate(glm::mat4 { 1.0f }, glm::radians(90.0f), { 1, 0, 0 });
		floor.add_component<Component::Texture>(glm::vec4 { 0.3f, 0.2f, 0.3f, 0.7f });

		static std::array point_lights { create_entity("LightOne"), create_entity("LightTwo"), create_entity("LightThree"),
			create_entity("LightFour"), create_entity("LightFive"), create_entity("LightSix"), create_entity("LightSeven"),
			create_entity("LightEight"), create_entity("LightNine"), create_entity("LightTen") };

		class MoveInCircle : public ScriptEntity {
		public:
			~MoveInCircle() override = default;
			explicit MoveInCircle(float in_radius, float in_height)
				: radius(in_radius)
				, height(in_height)
			{
			}
			void on_create() override { Alabaster::Log::info("Created entity!!!"); }
			void on_delete() override { Alabaster::Log::info("Deleted entity!!!"); }
			void on_update(const float ts)
			{
				auto& component = get_component<Component::Transform>();
				auto& pos = component.position;
				const auto cos = glm::cos(glm::radians(position_index));
				const auto sin = glm::sin(glm::radians(position_index));
				position_index += ts;

				pos = { sin * radius, height, cos * radius };
			}

		private:
			float radius;
			float height;
			float position_index = 0;
		};
		for (auto& point_light : point_lights) {
			auto& transform = point_light.get_transform();

			constexpr auto height = -0.5f;
			constexpr auto radius = 2.0f;

			static int index = 0;
			constexpr auto division = (2 * glm::pi<float>()) / point_lights.size();
			const auto cos = glm::cos(index * division);
			const auto sin = glm::sin(index * division);
			index++;

			transform.position = { sin * radius, height, cos * radius };
			auto ambience = random_vec4(0, 1);
			ambience.w = 1;
			point_light.add_component<Component::PointLight>(ambience);
			point_light.add_component<Component::Mesh>(simple_sphere_model);
			point_light.add_behaviour<MoveInCircle>("MoveInCircle", radius, height);
		}

		auto sun = create_entity("The Sun");
		sun.add_component<Component::Light>(glm::vec4 { 252., 144., 3., 255 });
		sun.add_component<Component::Mesh>(sphere_model);
		sun.add_component<Component::Texture>(glm::vec4 { 252., 144., 3., 255 });
		auto& sun_transform = sun.get_transform();
		sun_transform.position = { -3, -1.5, -1 };
		sun_transform.scale = glm::vec3 { 0.5 };
	}

	Scene::Scene() noexcept
	{
		selected_entity = std::make_unique<Entity>();
		hovered_entity = std::make_unique<Entity>();
	};

	Scene::~Scene() = default;

	void Scene::step()
	{
		// TODO: We should step
		(void)this;
	}

	void Scene::pick_entity(const glm::vec3& ray_world)
	{
		const auto camera_position = scene_camera->get_position();
		entt::entity found_entity = entt::null;
		float t_dist = 1000.0f;

		const auto spheres = registry.view<const Component::SphereIntersectible>();
		spheres.each(
			[&camera_position, &ray_world, &t_dist, &found_entity](const entt::entity& entity, const Component::SphereIntersectible& sphere) {
				float distance = 1000.0f;
				if (const bool intersected = sphere.intersects_with(ray_world, camera_position, distance); intersected && distance < t_dist) {
					t_dist = distance;
					found_entity = entity;
				}
			});

		const auto quads = registry.view<const Component::QuadIntersectible>();
		quads.each([&camera_position, &ray_world, &t_dist, &found_entity](const entt::entity& entity, const Component::QuadIntersectible& quad) {
			float distance = 1000.0f;
			if (const bool intersected = quad.intersects_with(ray_world, camera_position, distance); intersected && distance < t_dist) {
				t_dist = distance;
				found_entity = entity;
			}
		});

		if (found_entity != entt::null) {
			*hovered_entity = Entity(this, found_entity);
		} else {
			*hovered_entity = {};
		}
	}

	template <class Vec = glm::vec3> static constexpr auto xy(const Vec& vec) { return vec.xy; }

	void Scene::update_selected_entity() const { *selected_entity = *hovered_entity; }

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
		mouse_picking_accumulator += ts;
		if (mouse_picking_accumulator >= mouse_picking_interval_ms) {
			pick_mouse();
			update_intersectibles();

			auto scripts = registry.view<Component::Behaviour>();
			scripts.each([this](const auto entity, Component::Behaviour& behaviour) {
				if (behaviour.entity)
					return;
				behaviour.create(behaviour);
				behaviour.entity->entity = Entity { this, entity };
				behaviour.entity->on_create();
			});

			scripts.each([ts](Component::Behaviour& behaviour) { behaviour.entity->on_update(ts); });

			mouse_picking_accumulator = 0;
		}

		scene_camera->on_update(ts);
	}

	void Scene::render()
	{
		command_buffer->begin();
		scene_renderer->begin_scene();
		scene_renderer->reset_stats();
		draw_entities_in_scene();
		scene_renderer->end_scene(*command_buffer, framebuffer);
		command_buffer->end();
		command_buffer->submit();
	}

	void Scene::update_intersectibles()
	{
		const auto sphere_intersectible_view = registry.view<const Component::Transform, Component::SphereIntersectible>();
		sphere_intersectible_view.each([](const Component::Transform& transform, Component::SphereIntersectible& intersectible) {
			intersectible.update(transform.position, transform.scale, transform.rotation);
		});
	}

	void Scene::draw_entities_in_scene()
	{
		axes(scene_renderer, glm::vec3 { -2.5, -0.1, 2.5 }, 5.0f);

		const auto mesh_view = registry.view<Component::Transform, const Component::Mesh, const Component::Texture, const Component::Pipeline>(
			entt::exclude<Component::Light>);
		mesh_view.each(
			[&renderer = scene_renderer](const Component::Transform& transform, const Component::Mesh& mesh, const Component::Texture& texture,
				const Component::Pipeline& pipeline) { renderer->mesh(mesh.mesh, transform.to_matrix(), pipeline.pipeline, texture.colour); });

		const auto light_view = registry.view<const Component::Transform, const Component::Light, Component::Texture, const Component::Mesh>(
			entt::exclude<Component::PointLight>);
		light_view.each([&renderer = scene_renderer](const Component::Transform& transform, const Component::Light& light,
							Component::Texture& texture, const Component::Mesh& mesh) {
			texture.colour = light.ambience;
			renderer->mesh(mesh.mesh, transform.to_matrix(), nullptr, texture.colour);
			renderer->set_light_data(transform.position, texture.colour, light.ambience);
		});

		const auto point_light_view = registry.view<Component::Transform, const Component::PointLight, const Component::Mesh>();
		const auto size = point_light_view.size_hint();
		std::size_t i = 0;
		point_light_view.each([&renderer = scene_renderer, size = size, i = i++](
								  Component::Transform& transform, const Component::PointLight& light, const Component::Mesh& mesh) {
			renderer->submit_point_light_data({ glm::vec4(transform.position, 1.0), light.ambience });
			renderer->mesh(mesh.mesh, transform.to_matrix(), nullptr, light.ambience);
		});
		scene_renderer->commit_point_light_data();

		const auto basic_geometry_view = registry.view<const Component::Transform, const Component::BasicGeometry, const Component::Texture>();
		basic_geometry_view.each([&renderer = scene_renderer, quad_texture_index = quad_texture_index](
									 const Component::Transform& transform, const Component::BasicGeometry& geom, const Component::Texture& texture) {
			const auto base_pos = transform.to_matrix();
			switch (geom.geometry) {
			case Component::Geometry::Rect: {
				renderer->quad(base_pos, texture.colour, quad_texture_index);
				return;
			}
			case Component::Geometry::Quad: {
				renderer->quad(base_pos, texture.colour, quad_texture_index);
				return;
			}
			case Component::Geometry::Circle: {
				// const auto base_pos = transform.to_matrix();
				// renderer->circle(base_pos, texture.colour, quad_texture_index);
				return;
			}
			}
		});
	}

	void Scene::on_event(Alabaster::Event& event)
	{
		scene_camera->on_event(event);

		Alabaster::EventDispatcher dispatch(event);
		dispatch.dispatch<Alabaster::KeyPressedEvent>([&quad_index = quad_texture_index](const Alabaster::KeyPressedEvent& key_event) {
			if (key_event.get_key_code() != Alabaster::Key::L)
				return false;

			quad_index++;
			quad_index = quad_index % 3;

			return false;
		});

		dispatch.dispatch<Alabaster::WindowResizeEvent>([this](const Alabaster::WindowResizeEvent& e) {
			if (e.width() == 0 || e.height() == 0)
				return true;

			const auto vertical_fov = glm::degrees(scene_camera->get_vertical_fov());
			scene_camera.reset(new Alabaster::EditorCamera(
				vertical_fov, static_cast<float>(e.width()), static_cast<float>(e.height()), 0.1f, 1000.0f, scene_camera.get()));
			scene_renderer->set_camera(scene_camera);
			// framebuffer->resize(e.width(), e.height());
			return false;
		});
	}

	void Scene::shutdown()
	{
		if (scene_renderer)
			scene_renderer->destroy();

		if (framebuffer)
			framebuffer->destroy();

		if (command_buffer)
			command_buffer->destroy();

		auto scripts = registry.view<Component::Behaviour>();
		scripts.each([](Component::Behaviour& behaviour) {
			behaviour.entity->on_delete();
			behaviour.destroy(behaviour);
		});

		registry.clear();
	}

	void Scene::initialise(AssetManager::FileWatcher&)
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

	Entity Scene::create_entity(entt::entity name, const std::string& tag_name)
	{
		Entity entity { this, name };
		entity.add_component<Component::ID>();
		entity.add_component<Component::Transform>();
		auto& tag = entity.add_component<Component::Tag>();
		tag.tag = tag_name;

		return entity;
	}

	const std::shared_ptr<Alabaster::Image>& Scene::final_image() const { return framebuffer->get_image(); }

} // namespace SceneSystem
