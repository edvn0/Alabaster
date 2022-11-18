#pragma once

#include "graphics/Allocator.hpp"
#include "graphics/CommandBuffer.hpp"

#include <memory>
#include <vector>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

struct GLFWwindow;

namespace Alabaster {

	struct DepthImage {
		VkImage image;
		VkImageView view;
		VmaAllocation allocation;

		void destroy(auto& device)
		{
			vkDestroyImageView(device, view, nullptr);
			Allocator allocator("Depth image destruction");
			allocator.destroy_image(image, allocation);
		}
	};

	class Swapchain {
	public:
		Swapchain() = default;

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
		std::tuple<VkImageView, VkImage> get_current_image() const { return { images.views[frame()], images.images[frame()] }; }
		VkFramebuffer get_current_framebuffer() const;
		uint32_t get_width() const;
		uint32_t get_height() const;
		uint32_t get_image_count();
		VkRenderPass get_render_pass() const;
		VkFormat get_format() { return format.format; }

		std::tuple<VkFormat, VkFormat> get_formats();

	private:
		GLFWwindow* sc_handle;
		uint32_t sc_width;
		uint32_t sc_height;
		uint32_t current_image_index;

		bool unsafe_semaphore { false };

		VkSwapchainKHR vk_swapchain;
		VkSurfaceKHR vk_surface;
		VkRenderPass vk_render_pass;
		uint32_t image_count { 3 };

		VkSurfaceFormatKHR format;
		VkPresentModeKHR present_format;
		VkExtent2D extent;
		VkFormat depth_format;
		DepthImage depth_image;

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
			VkFence in_flight_fence;
		};

		struct SwapchainCommandBuffer {
			VkCommandPool pool;
			VkCommandBuffer buffer;
		};

		std::vector<SwapchainCommandBuffer> command_buffers;
		std::vector<Sync> sync_objects;
		VkSemaphore render_complete;
		VkSemaphore present_complete;
		std::vector<VkFramebuffer> frame_buffers;

		uint32_t current_frame { 0 };

		VkSubmitInfo submit_info {};

	private:
		Capabilities query();
		void cleanup_unsafe_semaphore();
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