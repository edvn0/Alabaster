#pragma once

#include <vulkan/vulkan.h>

struct GLFWwindow;

namespace Alabaster {

	class Swapchain {
	public:
		Swapchain() = default;
		~Swapchain();

		void construct(GLFWwindow*, uint32_t width, uint32_t height);

		inline uint32_t get_image_count() { return image_count; }

		inline VkRenderPass get_render_pass() { return vk_render_pass; };

	private:
		VkSwapchainKHR vk_swapchain;
		VkSurfaceKHR vk_surface;
		VkRenderPass vk_render_pass;
		uint32_t image_count { 3 };
	};

} // namespace Alabaster
