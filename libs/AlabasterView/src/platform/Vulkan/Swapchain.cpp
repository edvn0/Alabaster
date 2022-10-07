#include "av_pch.hpp"

#include "graphics/Swapchain.hpp"

#include "graphics/GraphicsContext.hpp"

#include <GLFW/glfw3.h>

namespace Alabaster {

	void Swapchain::construct(GLFWwindow* handle, uint32_t width, uint32_t height)
	{
		auto& context = GraphicsContext::the();
		auto instance = context.instance();
		auto device = context.device();

		glfwCreateWindowSurface(instance, handle, nullptr, &vk_surface);
		VkSwapchainCreateInfoKHR create_info {};
		create_info.surface = vk_surface;
	}

	Swapchain::~Swapchain() { vkDestroySurfaceKHR(GraphicsContext::the().instance(), vk_surface, nullptr); }

} // namespace Alabaster
