#include "av_pch.hpp"

#include "core/GUILayer.hpp"

#include "core/Common.hpp"
#include "core/Window.hpp"
#include "graphics/CommandBuffer.hpp"
#include "graphics/GraphicsContext.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/Swapchain.hpp"
#include "vulkan/vulkan_core.h"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <vulkan/vulkan.h>

namespace Alabaster {

	static std::vector<VkCommandBuffer> imgui_command_buffers;

	void GUILayer::create_renderpass()
	{
		const auto&& [color, depth] = Application::the().swapchain().get_formats();

		VkAttachmentDescription color_attachment_desc = {};
		color_attachment_desc.format = color;
		color_attachment_desc.samples = VK_SAMPLE_COUNT_1_BIT;
		color_attachment_desc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		color_attachment_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		color_attachment_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		color_attachment_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		color_attachment_desc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		color_attachment_desc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentDescription depth_attachment_desc {};
		depth_attachment_desc.format = depth;
		depth_attachment_desc.samples = VK_SAMPLE_COUNT_1_BIT;
		depth_attachment_desc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		depth_attachment_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depth_attachment_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		depth_attachment_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depth_attachment_desc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
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
		render_pass_info.attachmentCount = static_cast<uint32_t>(descriptions.size());
		render_pass_info.pSubpasses = &subpass_description;
		render_pass_info.subpassCount = 1;
		render_pass_info.pDependencies = &dependency;
		render_pass_info.dependencyCount = 1;

		vk_check(vkCreateRenderPass(GraphicsContext::the().device(), &render_pass_info, nullptr, &gui_renderpass));
	}

	bool GUILayer::initialise()
	{
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		ImGui::StyleColorsDark();

		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		auto& window = Application::the().get_window();

		auto& vulkan_context = GraphicsContext::the();
		auto device = vulkan_context.device();
		create_renderpass();

		// Create Descriptor Pool
		VkDescriptorPoolSize pool_sizes[] = { { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 }, { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 }, { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 100 }, { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 100 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 100 }, { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100 }, { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 100 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 100 }, { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 100 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 100 } };
		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 100 * IM_ARRAYSIZE(pool_sizes);
		pool_info.poolSizeCount = static_cast<uint32_t>(IM_ARRAYSIZE(pool_sizes));
		pool_info.pPoolSizes = pool_sizes;
		vk_check(vkCreateDescriptorPool(device, &pool_info, nullptr, &imgui_descriptor_pool));

		// Setup Platform/Renderer bindings
		ImGui_ImplGlfw_InitForVulkan(window->native(), true);
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = vulkan_context.instance();
		init_info.PhysicalDevice = vulkan_context.physical_device();
		init_info.Device = vulkan_context.device();
		init_info.Queue = vulkan_context.present_queue();
		init_info.DescriptorPool = imgui_descriptor_pool;
		init_info.MinImageCount = 2;
		auto& swapchain = Application::the().get_window()->get_swapchain();
		init_info.ImageCount = swapchain->get_image_count();
		init_info.CheckVkResultFn = vk_check;
		ImGui_ImplVulkan_Init(&init_info, gui_renderpass);

		imgui_command_buffers.resize(swapchain->get_image_count());
		for (uint32_t i = 0; i < swapchain->get_image_count(); i++) {
			VkCommandBufferAllocateInfo info {};
			info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			info.commandBufferCount = 1;
			info.commandPool = GraphicsContext::the().pool();
			info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
			vk_check(vkAllocateCommandBuffers(device, &info, &imgui_command_buffers[i]));
		}

		ImGui::GetIO().Fonts->AddFontDefault();

		{
			VkCommandBuffer command_buffer = GraphicsContext::the().get_command_buffer();
			ImGui_ImplVulkan_CreateFontsTexture(command_buffer);
			GraphicsContext::the().flush_command_buffer(command_buffer);

			vk_check(vkDeviceWaitIdle(GraphicsContext::the().device()));

			ImGui_ImplVulkan_DestroyFontUploadObjects();
		}

		return true;
	}

	void GUILayer::begin()
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGuiIO& io = ImGui::GetIO();

		int w, h;
		int display_w, display_h;
		glfwGetWindowSize(Application::the().get_window()->native(), &w, &h);
		glfwGetFramebufferSize(Application::the().get_window()->native(), &display_w, &display_h);
		io.DisplaySize = ImVec2((float)w, (float)h);
		if (w > 0 && h > 0)
			io.DisplayFramebufferScale = ImVec2((float)display_w / w, (float)display_h / h);

		ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
		if (!platform_io.Monitors.empty() && platform_io.Monitors[0].DpiScale > 1.0f) {
			io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
			io.DisplaySize = ImVec2((float)display_w, (float)display_h);
		}
	}

	void GUILayer::end()
	{
		ImGui::Render();

		static constexpr VkClearColorValue clear_colour { 0.1f, 0.1f, 0.1f, 1.0f };

		const auto& swapchain = Application::the().get_window()->get_swapchain();
		std::array<VkClearValue, 2> clear_values {};
		clear_values[0].color = clear_colour;
		clear_values[1].depthStencil = { .depth = 1.0f, .stencil = 0 };

		uint32_t width = swapchain->get_width();
		uint32_t height = swapchain->get_height();

		uint32_t command_buffer_index = swapchain->frame();

		VkCommandBuffer draw_command_buffer = swapchain->get_drawbuffer(command_buffer_index);

		VkRenderPassBeginInfo render_pass_begin_info = {};
		render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_begin_info.renderPass = gui_renderpass;
		render_pass_begin_info.renderArea.offset.x = 0;
		render_pass_begin_info.renderArea.offset.y = 0;
		render_pass_begin_info.renderArea.extent.width = width;
		render_pass_begin_info.renderArea.extent.height = height;
		render_pass_begin_info.clearValueCount = clear_values.size();
		render_pass_begin_info.pClearValues = clear_values.data();
		render_pass_begin_info.framebuffer = swapchain->get_current_framebuffer();

		vkCmdBeginRenderPass(draw_command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

		const auto& imgui_buffer = imgui_command_buffers[command_buffer_index];
		{
			VkCommandBufferInheritanceInfo inheritance_info = {};
			inheritance_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
			inheritance_info.renderPass = gui_renderpass;
			inheritance_info.framebuffer = swapchain->get_current_framebuffer();
			inheritance_info.subpass = 0;

			VkCommandBufferBeginInfo cbi = {};
			cbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			cbi.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
			cbi.pInheritanceInfo = &inheritance_info;
			vk_check(vkBeginCommandBuffer(imgui_buffer, &cbi));

			VkViewport viewport = {};
			viewport.x = 0.0f;
			viewport.y = static_cast<float>(height);
			viewport.height = -static_cast<float>(height);
			viewport.width = static_cast<float>(width);
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(imgui_buffer, 0, 1, &viewport);

			VkRect2D scissor = {};
			scissor.extent.width = width;
			scissor.extent.height = height;
			scissor.offset.x = 0;
			scissor.offset.y = 0;
			vkCmdSetScissor(imgui_buffer, 0, 1, &scissor);

			ImDrawData* main_draw_data = ImGui::GetDrawData();
			ImGui_ImplVulkan_RenderDrawData(main_draw_data, imgui_buffer);

			vk_check(vkEndCommandBuffer(imgui_buffer));
		}

		vkCmdExecuteCommands(draw_command_buffer, 1, &imgui_buffer);

		vkCmdEndRenderPass(draw_command_buffer);

		vkEndCommandBuffer(draw_command_buffer);

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}

	void GUILayer::ui() { }

	void GUILayer::ui(float timestep)
	{
		ImGui::Begin("Test");
		if (ImGui::Button("Test")) {
			Log::info("Here");
		}
		ImGui::End();
	}

	GUILayer::~GUILayer() = default;

	void GUILayer::destroy()
	{
		const auto& device = GraphicsContext::the().device();
		vk_check(vkDeviceWaitIdle(device));
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
		vk_check(vkDeviceWaitIdle(device));

		vkDestroyRenderPass(device, gui_renderpass, nullptr);
		vkDestroyDescriptorPool(device, imgui_descriptor_pool, nullptr);
		Log::info("[GUILayer] Destroyed layer.");
	};

} // namespace Alabaster
