#pragma once

#include "graphics/CommandBuffer.hpp"

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

struct GLFWwindow;

namespace Alabaster {

	class Swapchain {
	public:
		Swapchain() = default;
		~Swapchain();

		void destroy();

		void init(GLFWwindow* window);
		void construct(uint32_t width, uint32_t height);
		void on_resize(uint32_t w, uint32_t h);

		void present();

		void begin_frame();
		void end_frame() { present(); }

		void wait();

		uint32_t frame() const { return current_frame; }
		auto image() const { return images.views[frame()]; }
		auto swapchain_extent() const { return extent; }
		float aspect_ratio() const { return static_cast<float>(extent.width) / static_cast<float>(extent.height); }

		VkCommandBuffer get_current_drawbuffer() const;
		VkCommandBuffer get_drawbuffer(uint32_t frame) const;
		VkFramebuffer get_current_framebuffer() const;
		uint32_t get_width() const;
		uint32_t get_height() const;
		uint32_t get_image_count();
		VkRenderPass get_render_pass() const;
		VkFormat get_format() { return format.format; }

	private:
		GLFWwindow* sc_handle;
		uint32_t sc_width;
		uint32_t sc_height;
		uint32_t current_image_index;

		int pixel_size_x;
		int pixel_size_y;

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
			std::vector<VkImage> images {};
			std::vector<VkImageView> views {};
		} images;

		struct Sync {
			VkSemaphore image_available;
			VkSemaphore render_finished;
			VkFence in_flight_fence;
		};

		std::vector<CommandBuffer> command_buffers;
		std::vector<Sync> sync_objects;
		std::vector<VkFramebuffer> frame_buffers;

		uint32_t current_frame { 0 };

		VkSubmitInfo submit_info {};

	private:
		Capabilities query();

		uint32_t get_next_image();

		void choose_format(const Capabilities&);
		void choose_present_mode(const Capabilities&);
		void choose_extent(const Capabilities&);
		void create_swapchain(const Capabilities&);
		void create_synchronisation_objects();
		void create_command_structures();
		void retrieve_images();
		void cleanup_swapchain();
	};

} // namespace Alabaster
