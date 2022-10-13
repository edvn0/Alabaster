#include "Alabaster.hpp"
#include "AlabasterLayer.hpp"
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
	{ { -0.5, -0.5, 1, 0 }, { 1, 0, 0, 1 } },
	{ { -0.5, 0.5, 1, 0 }, { 0, 1, 0, 1 } },
	{ { 0.5, 0.5, 1, 0 }, { 0, 0, 1, 1 } },
	{ { 0.5, -0.5, 1, 0 }, { 0, 0, 0, 1 } },
};

const std::vector<uint32_t> indices = { 0, 1, 2, 2, 3, 0 };

bool AlabasterLayer::initialise()
{
	PipelineSpecification spec {
		.shader = Shader("app/resources/shaders/main"),
		.debug_name = "Test",
		.render_pass = Application::the().get_window()->get_swapchain()->get_render_pass(),
		.wireframe = false,
		.backface_culling = true,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.depth_test = true,
		.depth_write = true,
		.vertex_layout
		= VertexBufferLayout { VertexBufferElement(ShaderDataType::Float4, "position"), VertexBufferElement(ShaderDataType::Float4, "colour") },
		.instance_layout = {},
	};

	vertex_buffer = new VertexBuffer(vertices.data(), vertices.size() * sizeof(Vertex));
	index_buffer = new IndexBuffer(indices.data(), indices.size() * sizeof(uint32_t));
	create_vertex_buffer();

	graphics_pipeline = std::make_unique<Pipeline>(spec);
	graphics_pipeline->invalidate();
	return true;
}

void AlabasterLayer::update(float ts)
{
	static size_t frame_number { 0 };

	static constexpr auto frame_to_rgb = [](size_t frame) {
		float r = sin((frame % 255) / 255.0);
		float g = cos((frame % 255) / 255.0);
		float b = (frame % 255) / 255.0;
		VkClearValue clear = { { { r, g, b, 1.0f } } };
		return clear;
	};

	const auto& swapchain = Application::the().get_window()->get_swapchain();
	VkCommandBufferBeginInfo cmd_bbi = {};
	cmd_bbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmd_bbi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	cmd_bbi.pNext = nullptr;

	auto buffer = swapchain->get_current_drawbuffer();
	vk_check(vkBeginCommandBuffer(buffer, &cmd_bbi));

	auto extent = swapchain->swapchain_extent();

	VkRenderPassBeginInfo render_pass_info {};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	render_pass_info.renderPass = swapchain->get_render_pass();
	render_pass_info.framebuffer = swapchain->get_current_framebuffer();
	render_pass_info.renderArea.offset = { 0, 0 };
	render_pass_info.renderArea.extent = extent;

	VkClearValue clear = frame_to_rgb(frame_number);
	render_pass_info.clearValueCount = 1;
	render_pass_info.pClearValues = &clear;

	vkCmdBeginRenderPass(buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline->get_vulkan_pipeline());

	VkViewport viewport {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)extent.width;
	viewport.height = -(float)extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(buffer, 0, 1, &viewport);

	VkRect2D scissor {};
	scissor.offset = { 0, 0 };
	scissor.extent = extent;
	vkCmdSetScissor(buffer, 0, 1, &scissor);

	std::array<VkBuffer, 1> vbs;
	vbs[0] = vertex_buffer->get_vulkan_buffer();
	VkDeviceSize offsets { 0 };
	vkCmdBindVertexBuffers(buffer, 0, 1, &vb, &offsets);

	vkCmdBindIndexBuffer(buffer, index_buffer->get_vulkan_buffer(), 0, VK_INDEX_TYPE_UINT32);

	vkCmdDrawIndexed(buffer, indices.size(), 1, 0, 0, 0);

	vkCmdEndRenderPass(buffer);

	vkEndCommandBuffer(buffer);

	Layer::update(ts);

	frame_number++;
}

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
					Application::the().get_window()->close();
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
	vertex_buffer->destroy();
	delete vertex_buffer;
	index_buffer->destroy();
	delete index_buffer;
	graphics_pipeline->destroy();

	vkDestroyBuffer(GraphicsContext::the().device(), vb, nullptr);
	vkFreeMemory(GraphicsContext::the().device(), vb_mem, nullptr);

	Layer::destroy();
}

void AlabasterLayer::create_vertex_buffer()
{
	VkBufferCreateInfo buffer_info {};
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.size = sizeof(vertices[0]) * vertices.size();
	buffer_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(GraphicsContext::the().device(), &buffer_info, nullptr, &vb) != VK_SUCCESS) {
		throw std::runtime_error("failed to create vertex buffer!");
	}

	VkMemoryRequirements mem_requirements;
	vkGetBufferMemoryRequirements(GraphicsContext::the().device(), vb, &mem_requirements);

	VkMemoryAllocateInfo allocation_info {};
	allocation_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocation_info.allocationSize = mem_requirements.size;
	allocation_info.memoryTypeIndex
		= find_memory_type(mem_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	vk_check(vkAllocateMemory(GraphicsContext::the().device(), &allocation_info, nullptr, &vb_mem));

	vkBindBufferMemory(GraphicsContext::the().device(), vb, vb_mem, 0);

	void* data;
	vkMapMemory(GraphicsContext::the().device(), vb_mem, 0, buffer_info.size, 0, &data);
	memcpy(data, vertices.data(), (size_t)buffer_info.size);
	vkUnmapMemory(GraphicsContext::the().device(), vb_mem);
}
