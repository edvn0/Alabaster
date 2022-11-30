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
#include "graphics/GraphicsContext.hpp"
#include "graphics/IndexBuffer.hpp"
#include "graphics/Mesh.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/PushConstantRange.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/Renderer3D.hpp"
#include "graphics/Vertex.hpp"
#include "graphics/VertexBufferLayout.hpp"
#include "serialisation/SceneSerialiser.hpp"

#include <imgui/imgui.h>

namespace SceneSystem {

	static glm::vec4 pos { -5, 5, 5, 1.0f };
	static glm::vec4 col { 255 / 255.0, 153 / 255.0, 51 / 255.0, 255.0f / 255.0 };
	static float ambience { 1.0f };

	static constexpr auto axes = [](const auto& renderer, auto&& pos) {
		renderer->line(pos, pos + glm::vec3 { 1, 0, 0 }, { 1, 0, 0, 1 });
		renderer->line(pos, pos + glm::vec3 { 0, -1, 0 }, { 0, 1, 0, 1 });
		renderer->line(pos, pos + glm::vec3 { 0, 0, -1 }, { 0, 0, 1, 1 });
	};

	auto create_renderpass()
	{
		const auto&& [color, depth] = Alabaster::Application::the().swapchain().get_formats();

		VkAttachmentDescription color_attachment_desc = {};
		color_attachment_desc.format = color;
		color_attachment_desc.samples = VK_SAMPLE_COUNT_1_BIT;
		color_attachment_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		color_attachment_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		color_attachment_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		color_attachment_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
		color_attachment_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		color_attachment_desc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription depth_attachment_desc {};
		depth_attachment_desc.samples = VK_SAMPLE_COUNT_1_BIT;
		depth_attachment_desc.format = depth;
		depth_attachment_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depth_attachment_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depth_attachment_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depth_attachment_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
		depth_attachment_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depth_attachment_desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		VkAttachmentReference color_reference = {};
		color_reference.attachment = 0;
		color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depth_reference = {};
		depth_reference.attachment = 1;
		depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass_description = {};
		subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass_description.colorAttachmentCount = 1;
		subpass_description.pColorAttachments = &color_reference;
		subpass_description.pDepthStencilAttachment = &depth_reference;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 2> descriptions { color_attachment_desc, depth_attachment_desc };
		VkRenderPassCreateInfo render_pass_info = {};
		render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		render_pass_info.pAttachments = descriptions.data();
		render_pass_info.attachmentCount = static_cast<std::uint32_t>(descriptions.size());
		render_pass_info.pSubpasses = &subpass_description;
		render_pass_info.subpassCount = 1;
		render_pass_info.pDependencies = &dependency;
		render_pass_info.dependencyCount = 1;

		VkRenderPass pass;
		Alabaster::vk_check(vkCreateRenderPass(Alabaster::GraphicsContext::the().device(), &render_pass_info, nullptr, &pass));
		return pass;
	}

	void Scene::build_scene()
	{
		using namespace Alabaster;
		first_renderpass = create_renderpass();

		auto viking_room_model = Mesh::from_file("viking_room.obj");
		auto sphere_model = Mesh::from_file("sphere.obj");
		auto cube_model = Mesh::from_file("cube.obj");

		PipelineSpecification viking_spec { .shader = AssetManager::ResourceCache::the().shader("viking"),
			.debug_name = "Viking Pipeline",
			.render_pass = scene_renderer->get_render_pass(),
			.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			.vertex_layout
			= VertexBufferLayout { VertexBufferElement(ShaderDataType::Float3, "position"), VertexBufferElement(ShaderDataType::Float4, "colour"),
				VertexBufferElement(ShaderDataType::Float3, "normal"), VertexBufferElement(ShaderDataType::Float3, "tangent"),
				VertexBufferElement(ShaderDataType::Float3, "bitangent"), VertexBufferElement(ShaderDataType::Float2, "uvs") },
			.ranges = PushConstantRanges { PushConstantRange(PushConstantKind::Both, sizeof(PC)) } };
		std::shared_ptr<Alabaster::Pipeline> viking_pipeline = Pipeline::create(viking_spec);

		PipelineSpecification sun_spec { .shader = AssetManager::ResourceCache::the().shader("mesh"),
			.debug_name = "Sun Pipeline",
			.render_pass = scene_renderer->get_render_pass(),
			.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			.vertex_layout
			= VertexBufferLayout { VertexBufferElement(ShaderDataType::Float3, "position"), VertexBufferElement(ShaderDataType::Float4, "colour"),
				VertexBufferElement(ShaderDataType::Float3, "normal"), VertexBufferElement(ShaderDataType::Float3, "tangent"),
				VertexBufferElement(ShaderDataType::Float3, "bitangent"), VertexBufferElement(ShaderDataType::Float2, "uvs") },
			.ranges = PushConstantRanges { PushConstantRange(PushConstantKind::Both, sizeof(PC)) } };
		std::shared_ptr<Alabaster::Pipeline> sun_pipeline = Pipeline::create(sun_spec);

		for (std::uint32_t i = 0; i < 100; i++) {
			Entity entity { *this, fmt::format("Sphere-{}", i) };
			entity.add_component<Component::Mesh>(sphere_model);
			auto& transform = entity.get_component<Component::Transform>();
			transform.position = sphere_vector3(30);
			entity.add_component<Component::Texture>(glm::vec4(1.0f));
			entity.add_component<Component::Pipeline>(viking_pipeline);
		}

		{
			Entity floor { *this, "Floor" };
			floor.add_component<Component::BasicGeometry>(Component::Geometry::Quad);
			auto& floor_transform = floor.get_component<Component::Transform>();
			floor_transform.scale = { 200, 200, .2 };
			floor_transform.position.y += 30;
			floor_transform.rotation = glm::rotate(glm::mat4 { 1.0f }, glm::radians(90.0f), { 1, 0, 0 });
			floor.add_component<Component::Texture>(glm::vec4 { 0.3f, 0.2f, 0.3f, 0.7f });
		}

		{
			struct {
				glm::vec3 pos;
				glm::vec4 col;
				glm::vec3 scale;
				float rotation;
			} quad_data[9];

			quad_data[0] = { { 0, 0, -30 }, { 0.2, 0.3, 0.1, 1.0f }, { 10.0, 10.0, .3f }, glm::radians(90.0f) };
			quad_data[1] = { { 30, 0, -30 }, { 0.2, 0.3, 0.1, 1.0f }, { 10.0, 10.0, .3f }, glm::radians(90.0f) };
			quad_data[2] = { { -30, 0, -30 }, { 0.2, 0.3, 0.1, 1.0f }, { 10.0, 10.0, .3f }, glm::radians(90.0f) };

			quad_data[3] = { { 0, 0, 0 }, { 0.2, 0.3, 0.1, 1.0f }, { 10.0, 10.0, .3f }, glm::radians(90.0f) };
			quad_data[4] = { { 30, 0, 0 }, { 0.2, 0.3, 0.1, 1.0f }, { 10.0, 10.0, .3f }, glm::radians(90.0f) };
			quad_data[5] = { { -30, 0, 0 }, { 0.2, 0.3, 0.1, 1.0f }, { 10.0, 10.0, .3f }, glm::radians(90.0f) };

			quad_data[6] = { { 0, 0, 30 }, { 0.2, 0.3, 0.1, 1.0f }, { 10.0, 10.0, .3f }, glm::radians(90.0f) };
			quad_data[7] = { { 30, 0, 30 }, { 0.2, 0.3, 0.1, 1.0f }, { 10.0, 10.0, .3f }, glm::radians(90.0f) };
			quad_data[8] = { { -30, 0, 30 }, { 0.2, 0.3, 0.1, 1.0f }, { 10.0, 10.0, .3f }, glm::radians(90.0f) };

			for (std::uint32_t i = 0; i < 9; i++) {
				Entity entity { *this, fmt::format("Quad-{}", i) };
				entity.add_component<Component::BasicGeometry>(Component::Geometry::Quad);

				auto& transform = entity.get_component<Component::Transform>();
				transform.position = quad_data[i].pos;
				transform.scale = quad_data[i].scale;
				transform.rotation = glm::rotate(glm::mat4 { 1.0f }, quad_data[i].rotation, { 1, 0, 0 });

				entity.add_component<Component::Texture>(quad_data[i].col);
			}
		}

		{
			Entity viking { *this, "Viking" };
			auto rot = glm::rotate(glm::mat4 { 1.0f }, glm::radians(90.0f), glm::vec3 { 1, 0, 0 });
			viking.add_component<Component::Mesh>(viking_room_model);
			viking.add_component<Component::Pipeline>(viking_pipeline);
			viking.add_component<Component::Texture>(glm::vec4 { 1, 1, 1, 1 });
			auto& viking_transform = viking.get_transform();
			viking_transform.rotation = rot;
			viking_transform.scale = { 15, 15, 15 };
		}
	}

	Scene::Scene()
		: registry()
	{
	}

	Scene::~Scene()
	{
		scene_renderer->destroy();
		vkDestroyRenderPass(Alabaster::GraphicsContext::the().device(), first_renderpass, nullptr);
	};

	void Scene::update(float ts)
	{
		scene_camera->on_update(ts);
		command_buffer->begin();
		{
			scene_renderer->begin_scene();
			scene_renderer->reset_stats();

			axes(scene_renderer, glm::vec3 { 0, -0.1, 0 });
			scene_renderer->set_light_data(pos, col, ambience);

			auto mesh_view = registry.view<Component::Transform, const Component::Mesh, const Component::Texture, const Component::Pipeline>();
			mesh_view.each([&renderer = scene_renderer](Component::Transform& transform, const Component::Mesh& mesh,
							   const Component::Texture& texture, const Component::Pipeline& pipeline) {
				// transform.position = Alabaster::sphere_vector3(30);
				renderer->mesh(mesh.mesh, transform.to_matrix(), pipeline.pipeline, texture.colour);
			});

			auto quad_view = registry.view<const Component::Transform, const Component::BasicGeometry, const Component::Texture>();
			quad_view.each([&renderer = scene_renderer](
							   const Component::Transform& transform, const Component::BasicGeometry& geom, const Component::Texture& texture) {
				if (geom.geometry == Component::Geometry::Quad)
					renderer->quad(transform.to_matrix(), texture.colour);
			});

			// Sun stuff
			static double x = 0.0;
			x += 0.8;

			static double y = 0.0;
			y += 0.8;

			double cosx = 30 * glm::cos(glm::radians(x));
			double z = 30 * glm::sin(glm::radians(y));
			double pos_y = (cosx * z) / 30;

			pos = { cosx, pos_y, z, 1.0f };
			// scene_renderer->mesh(sphere_model, sun_pipeline, pos);
			// Sun done

			// End todo

			scene_renderer->end_scene(command_buffer, first_renderpass);
		}
		command_buffer->submit();
	}

	void Scene::on_event(Alabaster::Event& event) { scene_camera->on_event(event); }

	void Scene::shutdown() { SceneSerialiser serialiser(*this); }

	void Scene::initialise()
	{
		auto&& [w, h] = Alabaster::Application::the().get_window()->size();

		scene_camera = std::make_unique<Alabaster::EditorCamera>(74.0f, static_cast<float>(w), static_cast<float>(h), 0.1f, 1000.0f);
		scene_renderer = std::make_unique<Alabaster::Renderer3D>(*scene_camera.get());
		command_buffer = Alabaster::CommandBuffer::from_swapchain();

		build_scene();
	}

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
