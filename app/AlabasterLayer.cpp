#include "Alabaster.hpp"
#include "AlabasterLayer.hpp"
#include "graphics/Renderer.hpp"
#include "vulkan/vulkan_core.h"

#include <imgui.h>

using namespace Alabaster;

uint32_t find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memory_properties;
	vkGetPhysicalDeviceMemoryProperties(GraphicsContext::the().physical_device(), &memory_properties);

	for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {
		const auto filter = (type_filter & (1 << i));
		if (filter && (memory_properties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}

struct Vertex {
	glm::vec4 position;
	glm::vec4 colour;
};

const std::vector<Vertex> vertices = {
	{ { -0.5, -0.5, 0, 1 }, { 0, 0, 1, 1 } },
	{ { 0.5, -0.5, 0, 1 }, { 0, 0, 1, 1 } },
	{ { 0.5, 0.5, 0, 1 }, { 0, 0, 1, 1 } },
	{ { -0.5, 0.5, 0, 1 }, { 0, 0, 1, 1 } },
};

const std::vector<uint32_t> indices = { 0, 1, 2, 2, 3, 0 };

void AlabasterLayer::create_renderpass()
{
	VkAttachmentDescription colorAttachment {};
	colorAttachment.format = Application::the().get_window()->get_swapchain()->get_format();
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkSubpassDependency dependency {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	vk_check(vkCreateRenderPass(GraphicsContext::the().device(), &renderPassInfo, nullptr, &render_pass));
}

bool AlabasterLayer::initialise()
{
	create_renderpass();

	PipelineSpecification spec {
		.shader = Shader("app/resources/shaders/main"),
		.debug_name = "Test",
		.render_pass = render_pass,
		.wireframe = false,
		.backface_culling = false,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.depth_test = false,
		.depth_write = false,
		.vertex_layout
		= VertexBufferLayout { VertexBufferElement(ShaderDataType::Float4, "position"), VertexBufferElement(ShaderDataType::Float4, "colour") },
		.instance_layout = {},
	};

	vertex_buffer = std::make_unique<VertexBuffer>(vertices.data(), vertices.size() * sizeof(Vertex));
	index_buffer = std::make_unique<IndexBuffer>(indices.data(), indices.size());

	aeroplane_texture = std::make_unique<Texture2D>("textures/aeroplane.png");
	uint32_t black = 0x00000000;
	black_texture = std::make_unique<Texture2D>(&black, sizeof(uint32_t));

	graphics_pipeline = std::make_unique<Pipeline>(spec);
	graphics_pipeline->invalidate();
	return true;
}

void AlabasterLayer::on_event(Event& e)
{
	EventDispatcher dispatch(e);
	dispatch.dispatch<KeyPressedEvent>([](KeyPressedEvent& key_event) {
		const auto key_code = key_event.get_key_code();
		if (key_code == Key::Escape) {
			Application::the().exit();
			return true;
		}

		return false;
	});
}

void AlabasterLayer::update(float ts)
{
	static size_t frame_number { 0 };
	Renderer::submit([this] {
		const auto& swapchain = Application::the().get_window()->get_swapchain();
		VkCommandBufferBeginInfo command_buffer_begin_info = {};
		command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		command_buffer_begin_info.pNext = nullptr;

		const auto& buffer = swapchain->get_current_drawbuffer();
		vk_check(vkBeginCommandBuffer(buffer, &command_buffer_begin_info));

		auto extent = swapchain->swapchain_extent();

		VkRenderPassBeginInfo render_pass_info {};
		render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_info.renderPass = render_pass;
		render_pass_info.framebuffer = swapchain->get_current_framebuffer();
		render_pass_info.renderArea.offset = { 0, 0 };
		render_pass_info.renderArea.extent = extent;

		std::array<VkClearValue, 2> clear_values {};
		clear_values[0].color = { 0, 1, 0, 1 };
		clear_values[1].depthStencil = { .depth = -1.0f, .stencil = 0 };

		render_pass_info.clearValueCount = clear_values.size();
		render_pass_info.pClearValues = clear_values.data();

		vkCmdBeginRenderPass(buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline->get_vulkan_pipeline());

		VkViewport viewport {};
		viewport.x = 0.0f;
		viewport.y = static_cast<float>(extent.height);
		viewport.width = static_cast<float>(extent.width);
		viewport.height = static_cast<float>(extent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(buffer, 0, 1, &viewport);

		VkRect2D scissor {};
		scissor.offset = { 0, 0 };
		scissor.extent = extent;
		vkCmdSetScissor(buffer, 0, 1, &scissor);

		std::array<VkBuffer, 1> vbs {};
		vbs[0] = vertex_buffer->get_vulkan_buffer();
		VkDeviceSize offsets { 0 };
		vkCmdBindVertexBuffers(buffer, 0, 1, vbs.data(), &offsets);

		vkCmdBindIndexBuffer(buffer, index_buffer->get_vulkan_buffer(), 0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(buffer, index_buffer->count(), 1, 0, 0, 0);

		vkCmdEndRenderPass(buffer);

		vkEndCommandBuffer(buffer);
	});

	Layer::update(ts);

	frame_number++;
}

void AlabasterLayer::ui() { }

void AlabasterLayer::ui(float ts)
{
	static bool opt_fullscreen_persistant = true;
	bool opt_fullscreen = opt_fullscreen_persistant;
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

	// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
	// because it would be confusing to have two docking targets within each others.
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

	// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the
	// pass-thru hole, so we ask Begin() to not render a background.
	if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		window_flags |= ImGuiWindowFlags_NoBackground;

	// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
	// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
	// all active windows docked into it will lose their parent and become undocked.
	// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
	// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
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
			ImGui::Text("Frametime: %s", std::to_string(frametime).c_str());
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
	vkDestroyRenderPass(GraphicsContext::the().device(), render_pass, nullptr);

	vertex_buffer->destroy();
	index_buffer->destroy();
	graphics_pipeline->destroy();

	Log::info("[AlabasterLayer] Destroyed layer.");
	Layer::destroy();
}
