#include "AlabasterLayer.hpp"

#include "Alabaster.hpp"
#include "AssetManager.hpp"
#include "graphics/CommandBuffer.hpp"

#include <imgui.h>
#include <optional>
#include <vulkan/vulkan.h>

using namespace Alabaster;

static std::uint32_t quads { 1 };

static glm::vec4 pos { -5, 5, 5, 1.0f };
static glm::vec4 col { 255 / 255.0, 153 / 255.0, 51 / 255.0, 255.0f / 255.0 };
static float ambience { 1.0f };

static constexpr auto axes = [](auto& renderer, auto&& pos, float size = 2.0f) {
	renderer.line(pos, pos + glm::vec3 { 1, 0, 0 }, { 1, 0, 0, 1 });
	renderer.line(pos, pos + glm::vec3 { 0, -1, 0 }, { 0, 1, 0, 1 });
	renderer.line(pos, pos + glm::vec3 { 0, 0, -1 }, { 0, 0, 1, 1 });
};

auto create_renderpass(bool first = false)
{
	const auto&& [color, depth] = Application::the().swapchain().get_formats();

	VkAttachmentDescription color_attachment_desc = {};
	color_attachment_desc.format = color;
	color_attachment_desc.samples = VK_SAMPLE_COUNT_1_BIT;

	if (first) {
		color_attachment_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	} else {
		color_attachment_desc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	}
	color_attachment_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

	if (first) {
		color_attachment_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	} else {
		color_attachment_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	}
	color_attachment_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;

	if (first) {
		color_attachment_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		color_attachment_desc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	} else {
		color_attachment_desc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		color_attachment_desc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}

	VkAttachmentDescription depth_attachment_desc {};
	depth_attachment_desc.samples = VK_SAMPLE_COUNT_1_BIT;
	depth_attachment_desc.format = depth;

	if (first) {
		depth_attachment_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	} else {
		depth_attachment_desc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	}
	depth_attachment_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

	if (first) {
		depth_attachment_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	} else {
		depth_attachment_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	}
	depth_attachment_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;

	if (first) {
		depth_attachment_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depth_attachment_desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	} else {
		depth_attachment_desc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depth_attachment_desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	}

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
	vk_check(vkCreateRenderPass(GraphicsContext::the().device(), &render_pass_info, nullptr, &pass));
	return pass;
}

bool AlabasterLayer::initialise()
{
	first_renderpass = create_renderpass(true);
	sun_renderpass = create_renderpass(false);

	viking_room_model = Mesh::from_file("viking_room.obj");
	sphere_model = Mesh::from_file("sphere.obj");
	cube_model = Mesh::from_file("cube.obj");

	// sponza_model = Mesh::from_file("sponza.obj");

	PipelineSpecification viking_spec { .shader = AssetManager::ResourceCache::the().shader("viking"),
		.debug_name = "Viking Pipeline",
		.render_pass = renderer.get_render_pass(),
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.vertex_layout
		= VertexBufferLayout { VertexBufferElement(ShaderDataType::Float3, "position"), VertexBufferElement(ShaderDataType::Float4, "colour"),
			VertexBufferElement(ShaderDataType::Float3, "normal"), VertexBufferElement(ShaderDataType::Float3, "tangent"),
			VertexBufferElement(ShaderDataType::Float3, "bitangent"), VertexBufferElement(ShaderDataType::Float2, "uvs") },
		.ranges = PushConstantRanges { PushConstantRange(PushConstantKind::Both, sizeof(PC)) } };
	viking_pipeline = Pipeline::create(viking_spec);

	PipelineSpecification sun_spec { .shader = AssetManager::ResourceCache::the().shader("mesh"),
		.debug_name = "Sun Pipeline",
		.render_pass = renderer.get_render_pass(),
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.vertex_layout
		= VertexBufferLayout { VertexBufferElement(ShaderDataType::Float3, "position"), VertexBufferElement(ShaderDataType::Float4, "colour"),
			VertexBufferElement(ShaderDataType::Float3, "normal"), VertexBufferElement(ShaderDataType::Float3, "tangent"),
			VertexBufferElement(ShaderDataType::Float3, "bitangent"), VertexBufferElement(ShaderDataType::Float2, "uvs") },
		.ranges = PushConstantRanges { PushConstantRange(PushConstantKind::Both, sizeof(PC)) } };
	sun_pipeline = Pipeline::create(sun_spec);

	command_buffer = CommandBuffer::from_swapchain();

	editor_scene = std::make_unique<SceneSystem::Scene>(camera);

	return true;
}

void AlabasterLayer::on_event(Event& e)
{
	editor_scene->on_event(e);
	editor.on_event(e);

	EventDispatcher dispatch(e);
	dispatch.dispatch<KeyPressedEvent>([](KeyPressedEvent& key_event) {
		const auto key_code = key_event.get_key_code();
		if (key_code == Key::Escape) {
			Application::the().exit();
			return true;
		}

		if (key_code == Key::C) {
			quads++;
			return true;
		}

		if (key_code == Key::U) {
			pos.x += 0.1;
			pos.y += 0.1;
		}
		if (key_code == Key::H) {
			pos.x -= 0.1;
			pos.y -= 0.1;
		}

		if (Input::key(Key::G)) {
			Logger::cycle_levels();
			return true;
		}

		return false;
	});
}

void AlabasterLayer::update(float ts)
{
	static std::size_t frame_number { 0 };

	editor_scene->update(ts);

	editor.on_update(ts);
	command_buffer->begin();
	{
		renderer.reset_stats();
		renderer.begin_scene();
		renderer.set_light_data(pos, col, ambience);

		renderer.quad({ 0, 0, -30 }, { 0.2, 0.3, 0.1, 1.0f }, { 10.0, 10.0, .3f }, 90.0f);
		renderer.quad({ 30, 0, -30 }, { 0.2, 0.3, 0.1, 1.0f }, { 10.0, 10.0, .3f }, 90.0f);
		renderer.quad({ -30, 0, -30 }, { 0.2, 0.3, 0.1, 1.0f }, { 10.0, 10.0, .3f }, 90.0f);

		renderer.quad({ 0, 0, 0 }, { 0.2, 0.3, 0.1, 1.0f }, { 10.0, 10.0, .3f }, 90.0f);
		renderer.quad({ 30, 0, 0 }, { 0.2, 0.3, 0.1, 1.0f }, { 10.0, 10.0, .3f }, 90.0f);
		renderer.quad({ -30, 0, 0 }, { 0.2, 0.3, 0.1, 1.0f }, { 10.0, 10.0, .3f }, 90.0f);

		renderer.quad({ 0, 0, 30 }, { 0.2, 0.3, 0.1, 1.0f }, { 10.0, 10.0, .3f }, 90.0f);
		renderer.quad({ 30, 0, 30 }, { 0.2, 0.3, 0.1, 1.0f }, { 10.0, 10.0, .3f }, 90.0f);
		renderer.quad({ -30, 0, 30 }, { 0.2, 0.3, 0.1, 1.0f }, { 10.0, 10.0, .3f }, 90.0f);

		axes(renderer, glm::vec3 { 0, -0.1, 0 });

		renderer.line(4, { 0, 0, 0 }, { 3, 3, 3 }, { 1, 0, 0, 1 });
		auto rot = glm::rotate(glm::mat4 { 1.0f }, glm::radians(90.0f), glm::vec3 { 1, 0, 0 });
		renderer.mesh(viking_room_model, viking_pipeline, glm::vec3 { 0, -2.0f, 0 }, rot, glm::vec4 { 1 }, { 10, 10, 10 });

		renderer.text("Test Test", glm::vec3 { 0, 0, 0 });
		renderer.end_scene(command_buffer, first_renderpass);
	}
	{
		renderer.reset_stats();
		renderer.begin_scene();

		static double x = 0.0;
		x += 0.8;

		static double y = 0.0;
		y += 0.8;

		float cosx = 30 * glm::cos(glm::radians(x));
		float z = 30 * glm::sin(glm::radians(y));
		float pos_y = (cosx * z) / 30;

		pos = { cosx, pos_y, z, 1.0f };

		renderer.set_light_data(pos, col, ambience);
		renderer.mesh(sphere_model, sun_pipeline, pos);

		for (int i = 0; i < 45; i++) {
			renderer.mesh(sphere_model, sphere_vector3(30));
		}

		renderer.quad({ 15, 15, 0 }, { 0.2, 0.3, 0.1, 1.0f }, { 10.0, 10.0, .3f }, 30.0f);

		renderer.end_scene(command_buffer, sun_renderpass);
	}

	frame_number++;
}

void AlabasterLayer::ui() { }

void AlabasterLayer::ui(float ts) { editor_scene->ui(ts); }

void AlabasterLayer::destroy()
{
	renderer.destroy();

	vkDestroyRenderPass(GraphicsContext::the().device(), sun_renderpass, nullptr);
	vkDestroyRenderPass(GraphicsContext::the().device(), first_renderpass, nullptr);
}
