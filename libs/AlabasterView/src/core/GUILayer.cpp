#include "av_pch.hpp"

#include "core/GUILayer.hpp"

#include "core/Common.hpp"
#include "core/Window.hpp"
#include "graphics/GraphicsContext.hpp"
#include "graphics/Swapchain.hpp"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <vulkan/vulkan.h>

namespace Alabaster {

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

		// Setup Platform/Renderer backends
		auto& window = Application::the().get_window();

		auto& vulkan_context = GraphicsContext::the();
		auto device = vulkan_context.device();

		VkDescriptorPool imgui_descriptor_pool;

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
		init_info.QueueFamily = vulkan_context.graphics_queue_family();
		init_info.Queue = vulkan_context.graphics_queue();
		init_info.PipelineCache = nullptr;
		init_info.DescriptorPool = imgui_descriptor_pool;
		init_info.Allocator = nullptr;
		init_info.MinImageCount = 2;
		auto& swapchain = Application::the().get_window()->get_swapchain();
		init_info.ImageCount = swapchain->get_image_count();
		init_info.CheckVkResultFn = vk_check;
		ImGui_ImplVulkan_Init(&init_info, swapchain->get_render_pass());

		return true;
	}

} // namespace Alabaster
