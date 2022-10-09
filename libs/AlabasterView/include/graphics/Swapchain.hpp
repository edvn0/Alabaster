#pragma once

#include <vulkan/vulkan.h>

struct GLFWwindow;

namespace Alabaster {

	class Swapchain {
	public:
		Swapchain() = default;
		~Swapchain();

		void construct(GLFWwindow*, uint32_t width, uint32_t height);

		uint32_t get_image_count() { return image_count; }

		VkRenderPass get_render_pass() { return vk_render_pass; };

	private:
		VkSwapchainKHR vk_swapchain;
		VkSurfaceKHR vk_surface;
		VkRenderPass vk_render_pass;
		uint32_t image_count { 3 };

		VkSurfaceFormatKHR format;
		VkPresentModeKHR present_format;
		VkExtent2D extent;

		struct Capabilities {
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> present_modes;

			operator bool() const { return !(formats.empty() && present_modes.empty()); }
		};

		struct {
			std::vector<VkImage> images;
			std::vector<VkImageView> views;
		} images;

	private:
		Capabilities query();
		void choose_format(const Capabilities&);
		void choose_present_mode(const Capabilities&);
		void choose_extent(const Capabilities&);
		void create_swapchain(const Capabilities&);
		void retrieve_images();
	};

} // namespace Alabaster
