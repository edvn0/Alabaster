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
#include <Scripting.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>

namespace SceneSystem {

	using namespace std::string_view_literals;

	static constexpr auto mouse_picking_interval_ms = 100.0;
	static constexpr auto script_update_interval_ms = 16.0;

	template <class Position = glm::vec3> static constexpr auto axes(const auto& renderer, const Position& position, auto length = 1.0f)
	{
		renderer->line(position, position + glm::vec3 { length, 0, 0 }, { 1, 0, 0, 1 });
		renderer->line(position, position + glm::vec3 { 0, -length, 0 }, { 0, 1, 0, 1 });
		renderer->line(position, position + glm::vec3 { 0, 0, -length }, { 0, 0, 1, 1 });
	}

	Scene::Scene() noexcept
	{
		selected_entity = std::make_unique<Entity>();
		hovered_entity = std::make_unique<Entity>();
	}

	void Scene::initialise(AssetManager::FileWatcher& watcher)
	{
		const auto&& [w, h] = Alabaster::Application::the().get_window().size();

		scene_camera = std::make_unique<Alabaster::EditorCamera>(45.0f, static_cast<float>(w), static_cast<float>(h), 0.1f, 1000.0f);
		scene_renderer = std::make_unique<Alabaster::Renderer3D>(scene_camera.get());
		command_buffer = Alabaster::CommandBuffer::create(3);
		selected_entity = std::make_unique<Entity>();

		engine = std::make_unique<Scripting::ScriptEngine>();
		engine->set_scene(this);
		engine->register_file_watcher(watcher);

		Alabaster::FramebufferSpecification fbs;
		fbs.width = w;
		fbs.height = h;
		fbs.attachments = { Alabaster::ImageFormat::RGBA, Alabaster::ImageFormat::DEPTH32F };
		fbs.samples = 1;
		fbs.clear_colour = { 0.0f, 0.0f, 0.0f, 1.0f };
		fbs.debug_name = "Geometry";
		fbs.clear_depth_on_load = true;
		framebuffer = std::make_unique<Alabaster::Framebuffer>(fbs);
	}

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
		glfwGetWindowPos(Alabaster::Application::the().get_window().native(), &x_offset, &y_offset);

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
			mouse_picking_accumulator = 0;
		}

		script_update_accumulator += ts;
		if (script_update_accumulator >= script_update_interval_ms) {
			const auto scripts = registry.view<Component::Behaviour>();
			scripts.each([this](const auto entity, Component::Behaviour& behaviour) {
				if (!behaviour.is_valid() || behaviour.entity)
					return;
				behaviour.create(behaviour);
				behaviour.entity->set_entity(Entity { this, entity });
				behaviour.entity->on_create();
			});
			scripts.each([ts](Component::Behaviour& behaviour) {
				if (!behaviour.is_valid())
					return;
				behaviour.entity->on_update(ts);
			});
			const auto script_engine_scripts = registry.view<Component::ScriptBehaviour>();
			for (const auto entity : script_engine_scripts) {
				engine->entity_on_create(this, entity);
			}

			for (const auto entity : script_engine_scripts) {
				engine->entity_on_update(Entity { this, entity }, ts);
			}

			script_update_accumulator = 0;
		}

		scene_camera->on_update(ts);
	}

	void Scene::render()
	{
		command_buffer->begin();
		scene_renderer->begin_scene();
		scene_renderer->reset_stats();
		draw_entities_in_scene();
		scene_renderer->end_scene(*command_buffer, *framebuffer);
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
		mesh_view.each([&renderer = scene_renderer](const auto& transform, const auto& mesh, const auto& texture, const auto& pipeline) {
			if (mesh.valid()) {
				renderer->mesh(mesh.mesh, transform.to_matrix(), pipeline.pipeline, texture.colour);
			}
		});

		const auto light_view = registry.view<const Component::Transform, const Component::Light, Component::Texture, const Component::Mesh>(
			entt::exclude<Component::PointLight>);
		light_view.each([&renderer = scene_renderer](const auto& transform, const auto& light, auto& texture, const auto& mesh) {
			texture.colour = light.ambience;
			if (mesh.valid()) {
				renderer->mesh(mesh.mesh, transform.to_matrix(), nullptr, texture.colour);
			}
			renderer->set_light_data(transform.position, texture.colour, light.ambience);
		});

		const auto point_light_view = registry.view<const Component::Transform, const Component::PointLight, const Component::Mesh>();
		point_light_view.each([&renderer = scene_renderer](const auto& transform, const auto& light, const auto& mesh) {
			renderer->submit_point_light_data({ glm::vec4(transform.position, 1.0), light.ambience });
			if (mesh.valid()) {
				renderer->mesh(mesh.mesh, transform.to_matrix(), nullptr, light.ambience);
			}
		});
		scene_renderer->commit_point_light_data();

		const auto basic_geometry_view = registry.view<const Component::Transform, const Component::BasicGeometry, const Component::Texture>();
		basic_geometry_view.each(
			[&renderer = scene_renderer, quad_texture_index = quad_texture_index](const auto& transform, const auto& geom, const auto& texture) {
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
			scene_renderer->set_camera(*scene_camera);
			// framebuffer->resize(e.width(), e.height());
			return false;
		});
	}

	Scene::~Scene()
	{
		auto scripts = registry.view<Component::Behaviour>();
		scripts.each([](Component::Behaviour& behaviour) {
			if (!behaviour.is_valid())
				return;

			behaviour.entity->on_delete();
			behaviour.destroy(behaviour);
		});

		const auto script_engine_scripts = registry.view<Component::ScriptBehaviour>();
		for (const auto entity : script_engine_scripts) {
			engine->entity_on_delete(Entity { this, entity });
		}

		registry.clear();
	}

	void Scene::ui() { }

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
