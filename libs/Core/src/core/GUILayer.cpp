#include "av_pch.hpp"

#include "core/GUILayer.hpp"

#include "core/Common.hpp"
#include "core/Window.hpp"
#include "core/events/KeyEvent.hpp"
#include "graphics/CommandBuffer.hpp"
#include "graphics/GraphicsContext.hpp"
#include "graphics/Shader.hpp"
#include "ui/ImGuizmo.hpp"
#include "utilities/FileInputOutput.hpp"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

namespace Alabaster {

	void GUILayer::on_event(Event& event)
	{
		if (should_block) {
			const ImGuiIO& io = ImGui::GetIO();
			event.handled |= event.is_in_category(EventCategoryMouse) && io.WantCaptureMouse;
			event.handled |= event.is_in_category(EventCategoryKeyboard) && io.WantCaptureKeyboard;
		}
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
		auto& device = vulkan_context.device();

		// Create Descriptor Pool
		static const std::array<VkDescriptorPoolSize, 11> pool_sizes = { VkDescriptorPoolSize { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			VkDescriptorPoolSize { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 }, VkDescriptorPoolSize { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			VkDescriptorPoolSize { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 100 }, VkDescriptorPoolSize { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 100 },
			VkDescriptorPoolSize { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 100 }, VkDescriptorPoolSize { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100 },
			VkDescriptorPoolSize { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 100 }, VkDescriptorPoolSize { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 100 },
			VkDescriptorPoolSize { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 100 },
			VkDescriptorPoolSize { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 100 } };
		constexpr auto size = static_cast<std::uint32_t>(pool_sizes.size());
		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 100 * size;
		pool_info.poolSizeCount = size;
		pool_info.pPoolSizes = pool_sizes.data();
		vk_check(vkCreateDescriptorPool(device, &pool_info, nullptr, &imgui_descriptor_pool));

		// Setup Platform/Renderer bindings
		ImGui_ImplGlfw_InitForVulkan(window->native(), true);
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = vulkan_context.instance();
		init_info.PhysicalDevice = vulkan_context.physical_device();
		init_info.Device = vulkan_context.device();
		init_info.Queue = vulkan_context.graphics_queue();
		init_info.DescriptorPool = imgui_descriptor_pool;
		init_info.MinImageCount = 2;
		auto& swapchain = Application::the().get_window()->get_swapchain();
		init_info.ImageCount = swapchain->get_image_count();
		init_info.CheckVkResultFn = vk_check;
		ImGui_ImplVulkan_Init(&init_info, swapchain->get_render_pass());

		const std::filesystem::path path = Alabaster::IO::font("FreePixel.ttf");
		ImGui::GetIO().Fonts->AddFontFromFileTTF(path.string().c_str(), 14.0f);

		{
			ImGui_ImplVulkan_CreateFontsTexture(ImmediateCommandBuffer { "Fonts Texture" }.get_buffer());

			vk_check(vkDeviceWaitIdle(GraphicsContext::the().device()));

			ImGui_ImplVulkan_DestroyFontUploadObjects();
		}

		ImGuizmo::SetImGuiContext(ImGui::GetCurrentContext());

		return true;
	}

	void GUILayer::begin() const
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();
	}

	void GUILayer::end() const
	{
		ImGui::Render();

		static constexpr VkClearColorValue clear_colour { { 0.1f, 0.1f, 0.1f, 0.0f } };
		static constexpr VkClearDepthStencilValue depth_stencil_clear { .depth = 1.0f, .stencil = 0 };

		const auto& swapchain = Application::the().get_window()->get_swapchain();
		std::array<VkClearValue, 2> clear_values {};
		clear_values[0].color = clear_colour;
		clear_values[1].depthStencil = depth_stencil_clear;

		const auto width = swapchain->get_width();
		const auto height = swapchain->get_height();

		const VkCommandBuffer draw_command_buffer = swapchain->get_current_drawbuffer();

		VkCommandBufferBeginInfo begin_info = {};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		begin_info.pNext = nullptr;
		vkBeginCommandBuffer(draw_command_buffer, &begin_info);

		VkRenderPassBeginInfo render_pass_begin_info = {};
		render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_begin_info.renderPass = swapchain->get_render_pass();
		render_pass_begin_info.renderArea.offset.x = 0;
		render_pass_begin_info.renderArea.offset.y = 0;
		render_pass_begin_info.renderArea.extent.width = width;
		render_pass_begin_info.renderArea.extent.height = height;
		render_pass_begin_info.clearValueCount = static_cast<std::uint32_t>(clear_values.size());
		render_pass_begin_info.pClearValues = clear_values.data();
		render_pass_begin_info.framebuffer = swapchain->get_current_framebuffer();

		vkCmdBeginRenderPass(draw_command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

		const auto& imgui_buffer = imgui_command_buffer;
		{
			const auto& command_buffer = imgui_buffer->get_buffer();
			VkCommandBufferInheritanceInfo inheritance_info = {};
			inheritance_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
			inheritance_info.renderPass = swapchain->get_render_pass();
			inheritance_info.framebuffer = swapchain->get_current_framebuffer();

			VkCommandBufferBeginInfo cbi = {};
			cbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			cbi.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
			cbi.pInheritanceInfo = &inheritance_info;
			imgui_buffer->begin(&cbi);

			VkViewport viewport;
			viewport.x = 0.0f;
			viewport.y = static_cast<float>(height);
			viewport.height = -static_cast<float>(height);
			viewport.width = static_cast<float>(width);
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(command_buffer, 0, 1, &viewport);

			VkRect2D scissor;
			scissor.extent.width = width;
			scissor.extent.height = height;
			scissor.offset.x = 0;
			scissor.offset.y = 0;
			vkCmdSetScissor(command_buffer, 0, 1, &scissor);

			static float scale_x { -1.0f };
			static float scale_y { -1.0f };
			static bool scaled_already { false };

			ImDrawData* main_draw_data = ImGui::GetDrawData();
			if (!scaled_already) {
				const auto&& [sx, sy] = Application::the().get_window()->framebuffer_scale();
				scale_x = sx;
				scale_y = sy;
				scaled_already = true;

				const auto scale = ImGui::GetWindowDpiScale();
				Alabaster::Log::info("[GUILayer] Scale: {}, new framebuffer scale: {} by {}", scale, sx, sy);
			}

			main_draw_data->FramebufferScale = { scale_x, scale_y };
			// UI scale and translate via push constants
			ImGui_ImplVulkan_RenderDrawData(main_draw_data, command_buffer);

			imgui_buffer->end_with_no_reset();
		}

		vkCmdExecuteCommands(draw_command_buffer, 1, &imgui_buffer->get_buffer());

		vkCmdEndRenderPass(draw_command_buffer);

		vk_check(vkEndCommandBuffer(draw_command_buffer));

		if (const ImGuiIO& io = ImGui::GetIO(); io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}

	void GUILayer::ui(float) { }

	void GUILayer::destroy()
	{
		const auto& device = GraphicsContext::the().device();
		vk_check(vkDeviceWaitIdle(device));
		imgui_command_buffer->destroy();

		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
		vk_check(vkDeviceWaitIdle(device));

		vkDestroyDescriptorPool(device, imgui_descriptor_pool, nullptr);
		Log::info("[GUILayer] Destroyed layer.");
	}

} // namespace Alabaster
