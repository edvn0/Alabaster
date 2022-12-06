#pragma once

#include "graphics/Allocator.hpp"
#include "graphics/DepthImage.hpp"

#include <vector>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

struct GLFWwindow;

namespace Alabaster {

	class VulkanSwapChain {
	public:
		VulkanSwapChain() = default;

		void init(GLFWwindow* window_handle);
		void create(uint32_t* width, uint32_t* height, bool vsync);
		void destroy();

		void on_resize(uint32_t width, uint32_t height);

		void begin_frame();
		void end_frame() { present(); };
		void present();

		uint32_t get_image_count() const { return image_count; }

		uint32_t get_width() const { return width; }
		uint32_t get_height() const { return height; }

		VkRenderPass get_render_pass() { return render_pass; }

		VkFramebuffer get_current_framebuffer() { return get_framebuffer(current_image_index); }
		VkFramebuffer get_current_framebuffer() const { return get_framebuffer(current_image_index); }
		VkCommandBuffer get_current_drawbuffer() { return get_drawbuffer(current_buffer_index); }

		VkFormat get_color_format() { return color_format; }

		uint32_t get_current_buffer_index() const { return current_buffer_index; }

		VkFramebuffer get_framebuffer(uint32_t index) { return framebuffers[index]; }
		VkFramebuffer get_framebuffer(uint32_t index) const { return framebuffers[index]; }

		VkCommandBuffer get_drawbuffer(uint32_t index) { return command_buffers[index].CommandBuffer; }

		VkSemaphore get_render_complete_semaphore() { return semaphores.RenderComplete; }

		void set_vsync(const bool enabled) { vsync = enabled; }

		std::uint32_t frame() const { return current_buffer_index; }
		auto image() const { return images[frame()].ImageView; }
		auto swapchain_extent() const { return extent; }
		float aspect_ratio() const { return static_cast<float>(extent.width) / static_cast<float>(extent.height); }

		VkCommandBuffer get_current_drawbuffer() const;
		VkCommandBuffer get_drawbuffer(std::uint32_t frame) const;
		std::tuple<VkImageView, VkImage> get_current_image() const { return { images[frame()].ImageView, images[frame()].Image }; }
		VkRenderPass get_render_pass() const;
		VkFormat get_format() { return color_format; }

		std::tuple<VkFormat, VkFormat> get_formats();

	private:
		uint32_t acquire_next_image();

		void find_image_format_and_color_space();

	private:
		VkInstance instance = nullptr;
		VkDevice device;
		bool vsync = false;

		VkFormat color_format;
		VkColorSpaceKHR color_space;
		VkExtent2D extent;

		VkSwapchainKHR swap_chain = nullptr;
		uint32_t image_count = 0;
		std::vector<VkImage> vulkan_images;

		struct SwapchainImage {
			VkImage Image = nullptr;
			VkImageView ImageView = nullptr;
		};
		std::vector<SwapchainImage> images;

		struct {
			VkImage Image = nullptr;
			VmaAllocation MemoryAlloc = nullptr;
			VkImageView ImageView = nullptr;
		} depth_stencil;

		std::vector<VkFramebuffer> framebuffers;

		struct SwapchainCommandBuffer {
			VkCommandPool CommandPool = nullptr;
			VkCommandBuffer CommandBuffer = nullptr;
		};
		std::vector<SwapchainCommandBuffer> command_buffers;

		struct {
			// Swap chain
			VkSemaphore PresentComplete = nullptr;
			// Command buffer
			VkSemaphore RenderComplete = nullptr;
		} semaphores;
		VkSubmitInfo submit_info;

		std::vector<VkFence> wait_fences;

		VkRenderPass render_pass = nullptr;
		uint32_t current_buffer_index = 0;
		uint32_t current_image_index = 0;

		uint32_t queue_node_index = UINT32_MAX;
		uint32_t width = 0, height = 0;

		std::unique_ptr<DepthImage> depth_image;

		VkSurfaceKHR surface;
	};
} // namespace Alabaster
