#include "AlabasterLayer.hpp"

#include "Alabaster.hpp"
#include "AssetManager.hpp"
#include "graphics/CommandBuffer.hpp"
#include "vulkan/vulkan_core.h"

#include <imgui.h>
#include <initializer_list>
#include <optional>
#include <string_view>
#include <vulkan/vulkan.h>

using namespace Alabaster;

static std::uint32_t quads { 1 };

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
	color_attachment_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	if (first) {
		color_attachment_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		color_attachment_desc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	} else {
		color_attachment_desc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		color_attachment_desc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
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
	depth_attachment_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

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

	// sponza_model = Mesh::from_file("sponza.obj");
	// auto shader = AssetManager::ShaderCache::the().get_from_cache("mesh");

	PipelineSpecification viking_spec {
		.shader = AssetManager::ResourceCache::the().shader("mesh_light"),
		.debug_name = "Viking Pipeline",
		.render_pass = renderer.get_render_pass(),
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.vertex_layout
		= VertexBufferLayout { VertexBufferElement(ShaderDataType::Float3, "position"), VertexBufferElement(ShaderDataType::Float4, "colour"),
			VertexBufferElement(ShaderDataType::Float3, "normal"), VertexBufferElement(ShaderDataType::Float3, "tangent"),
			VertexBufferElement(ShaderDataType::Float3, "bitangent"), VertexBufferElement(ShaderDataType::Float2, "uvs") },
		.ranges = PushConstantRanges { PushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, sizeof(glm::mat4)),
			PushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, sizeof(glm::vec4)) },
	};
	viking_pipeline = Pipeline::create(viking_spec);

	command_buffer = CommandBuffer::from_swapchain();

	return true;
}

void AlabasterLayer::on_event(Event& e)
{
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

	renderer.reset_stats();
	editor.on_update(ts);
	command_buffer->begin();
	{
		renderer.begin_scene();

		renderer.quad({ 0, 0, -30 }, glm::vec4 { 0.2, 0.3, 0.1, 1.0f }, { 10.0, 10.0, .3f }, 90.0f);
		renderer.quad({ 30, 0, -30 }, glm::vec4 { 0.2, 0.3, 0.1, 1.0f }, { 10.0, 10.0, .3f }, 90.0f);
		renderer.quad({ -30, 0, -30 }, glm::vec4 { 0.2, 0.3, 0.1, 1.0f }, { 10.0, 10.0, .3f }, 90.0f);

		renderer.quad({ 0, 0, 0 }, glm::vec4 { 0.2, 0.3, 0.1, 1.0f }, { 10.0, 10.0, .3f }, 90.0f);
		renderer.quad({ 30, 0, 0 }, glm::vec4 { 0.2, 0.3, 0.1, 1.0f }, { 10.0, 10.0, .3f }, 90.0f);
		renderer.quad({ -30, 0, 0 }, glm::vec4 { 0.2, 0.3, 0.1, 1.0f }, { 10.0, 10.0, .3f }, 90.0f);

		renderer.quad({ 0, 0, 30 }, glm::vec4 { 0.2, 0.3, 0.1, 1.0f }, { 10.0, 10.0, .3f }, 90.0f);
		renderer.quad({ 30, 0, 30 }, glm::vec4 { 0.2, 0.3, 0.1, 1.0f }, { 10.0, 10.0, .3f }, 90.0f);
		renderer.quad({ -30, 0, 30 }, glm::vec4 { 0.2, 0.3, 0.1, 1.0f }, { 10.0, 10.0, .3f }, 90.0f);

		axes(renderer, glm::vec3 { 0, -0.1, 0 });

		renderer.line(4, { 0, 0, 0 }, { 3, 3, 3 }, { 1, 0, 0, 1 });
		auto rot = glm::rotate(glm::mat4 { 1.0f }, glm::radians(90.0f), glm::vec3 { 1, 0, 0 });
		renderer.mesh(viking_room_model, viking_pipeline, glm::vec3 { 0, -2.0f, 0 }, rot, glm::vec4 { 1 }, { 2, 2, 2 });

		renderer.text("Test Test", glm::vec3 { 0, 0, 0 });
		renderer.end_scene(command_buffer, first_renderpass);
	}
	{
		renderer.begin_scene();
		renderer.mesh(sphere_model, nullptr, { -5, 5, 5 });
		renderer.end_scene(command_buffer, sun_renderpass);
	}
	command_buffer->end();
	command_buffer->submit();

	frame_number++;
}

void AlabasterLayer::ui() { }

void AlabasterLayer::ui(float ts)
{
	static bool persistent = false;
	bool opt_fullscreen = persistent;
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	if (opt_fullscreen) {
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	}

	if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		window_flags |= ImGuiWindowFlags_NoBackground;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("DockSpace", &is_dockspace_open, window_flags);
	ImGui::PopStyleVar();
	{
		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// DockSpace
		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		float min_window_size_x = style.WindowMinSize.x;
		style.WindowMinSize.x = 370.0f;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		style.WindowMinSize.x = min_window_size_x;

		if (ImGui::BeginMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("New", "Ctrl+N")) { }
				if (ImGui::MenuItem("Open...", "Ctrl+O")) { }
				if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S")) { }
				if (ImGui::MenuItem("Exit")) {
					Application::the().exit();
				}
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		ImGui::Begin("Stats");
		{
			ImGui::Text("Renderer Stats:");
			std::string name = "None";
			ImGui::Text("Hovered Entity: %s", name.c_str());
			auto frametime = Application::the().frametime();
			ImGui::Text("Frame time: %s", std::to_string(frametime).c_str());
		}
		ImGui::End();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2 { 0, 0 });
		{
			ImGui::Begin("Viewport");
			{
				auto viewport_min_region = ImGui::GetWindowContentRegionMin();
				auto viewport_max_region = ImGui::GetWindowContentRegionMax();
				auto viewport_offset = ImGui::GetWindowPos();
				viewport_bounds[0] = { viewport_min_region.x + viewport_offset.x, viewport_min_region.y + viewport_offset.y };
				viewport_bounds[1] = { viewport_max_region.x + viewport_offset.x, viewport_max_region.y + viewport_offset.y };

				viewport_focused = ImGui::IsWindowFocused();
				viewport_hovered = ImGui::IsWindowHovered();

				ImVec2 viewport_panel_size = ImGui::GetContentRegionAvail();
				viewport_size = { viewport_panel_size.x, viewport_panel_size.y };

				ImVec2 vp_size = ImVec2 { viewport_size.x, viewport_size.y };

				ImGui::End();
			}
		}
		ImGui::PopStyleVar();
	}
	ImGui::End();
}

void AlabasterLayer::destroy()
{
	renderer.destroy();
	viking_room_model->destroy();
	sphere_model->destroy();
	sponza_model->destroy();
	command_buffer->destroy();

	vkDestroyRenderPass(GraphicsContext::the().device(), sun_renderpass, nullptr);
	vkDestroyRenderPass(GraphicsContext::the().device(), first_renderpass, nullptr);
}
