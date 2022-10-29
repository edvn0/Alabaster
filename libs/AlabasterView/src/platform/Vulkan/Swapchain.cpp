#include "av_pch.hpp"

#include "graphics/Swapchain.hpp"

#include "core/Application.hpp"
#include "core/Common.hpp"
#include "core/Logger.hpp"
#include "core/Window.hpp"
#include "graphics/GraphicsContext.hpp"
#include "vulkan/vulkan_core.h"

#include <GLFW/glfw3.h>

namespace Alabaster {

	static constexpr auto default_fence_timeout = std::numeric_limits<uint64_t>::max();

	void Swapchain::init(GLFWwindow* window)
	{
		auto& context = GraphicsContext::the();
		const auto& instance = context.instance();
		sc_handle = window;
		glfwCreateWindowSurface(instance, sc_handle, nullptr, &vk_surface);
	}

	void Swapchain::construct(uint32_t width, uint32_t height)
	{
		sc_width = width;
		sc_height = height;

		// Check that pixel screen size can contain SC images...
		int window_width, window_height;
		glfwGetWindowSize(sc_handle, &window_width, &window_height);

		if (window_width < sc_width || window_height < sc_height) { }

		glfwGetFramebufferSize(sc_handle, &pixel_size_x, &pixel_size_y);

		auto capabilities = query();
		verify(capabilities);

		choose_format(capabilities);
		choose_present_mode(capabilities);
		choose_extent(capabilities);

		create_swapchain(capabilities);
		retrieve_images();

		for (auto& cmd_buffer : command_buffers)
			vkDestroyCommandPool(GraphicsContext::the().device(), cmd_buffer.command_pool, nullptr);

		VkCommandPoolCreateInfo pci = {};
		pci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		pci.queueFamilyIndex = GraphicsContext::the().graphics_queue_family();
		pci.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		VkCommandBufferAllocateInfo command_buffer_allocate_info {};
		command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		command_buffer_allocate_info.commandBufferCount = 1;

		command_buffers.resize(image_count);
		for (auto& cmd_buffer : command_buffers) {
			vk_check(vkCreateCommandPool(GraphicsContext::the().device(), &pci, nullptr, &cmd_buffer.command_pool));

			command_buffer_allocate_info.commandPool = cmd_buffer.command_pool;
			vk_check(vkAllocateCommandBuffers(GraphicsContext::the().device(), &command_buffer_allocate_info, &cmd_buffer.buffer));
		}

		create_synchronisation_objects();

		VkPipelineStageFlags pipeline_stage_flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		std::vector<VkSemaphore> wait_semaphores;
		std::vector<VkSemaphore> signal_semaphores;
		for (const auto& [a, b, c] : sync_objects) {
			wait_semaphores.push_back(a);
			signal_semaphores.push_back(b);
		}

		submit_info = {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.pWaitDstStageMask = &pipeline_stage_flags;
		submit_info.waitSemaphoreCount = wait_semaphores.size();
		submit_info.pWaitSemaphores = wait_semaphores.data();
		submit_info.signalSemaphoreCount = signal_semaphores.size();
		submit_info.pSignalSemaphores = signal_semaphores.data();

		VkAttachmentDescription color_attachment_desc = {};
		color_attachment_desc.format = format.format;
		color_attachment_desc.samples = VK_SAMPLE_COUNT_1_BIT;
		color_attachment_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		color_attachment_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		color_attachment_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_attachment_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		color_attachment_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		color_attachment_desc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference color_reference = {};
		color_reference.attachment = 0;
		color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass_description = {};
		subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass_description.colorAttachmentCount = 1;
		subpass_description.pColorAttachments = &color_reference;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo render_pass_info = {};
		render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		render_pass_info.attachmentCount = 1;
		render_pass_info.pAttachments = &color_attachment_desc;
		render_pass_info.subpassCount = 1;
		render_pass_info.pSubpasses = &subpass_description;
		render_pass_info.dependencyCount = 1;
		render_pass_info.pDependencies = &dependency;

		vk_check(vkCreateRenderPass(GraphicsContext::the().device(), &render_pass_info, nullptr, &vk_render_pass));

		for (auto& framebuffer : frame_buffers)
			vkDestroyFramebuffer(GraphicsContext::the().device(), framebuffer, nullptr);

		VkFramebufferCreateInfo framebuffer_create_info = {};
		framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_create_info.renderPass = vk_render_pass;
		framebuffer_create_info.attachmentCount = 1;
		framebuffer_create_info.width = pixel_size_x;
		framebuffer_create_info.height = pixel_size_y;
		framebuffer_create_info.layers = 1;

		frame_buffers.resize(image_count);
		for (uint32_t i = 0; i < frame_buffers.size(); i++) {
			framebuffer_create_info.pAttachments = &images.views[i];
			vk_check(vkCreateFramebuffer(GraphicsContext::the().device(), &framebuffer_create_info, nullptr, &frame_buffers[i]));
		}

		Log::info("[Swapchain] Successfully created the swapchain ({}, {}) and retrieved its {} images.", pixel_size_x, pixel_size_y, image_count);
	}

	void Swapchain::begin_frame()
	{
		current_image_index = get_next_image();
		vk_check(vkResetCommandPool(GraphicsContext::the().device(), command_buffers[frame()].command_pool, 0));
	}

	void Swapchain::present()
	{
		VkPipelineStageFlags wait_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		VkSubmitInfo present_submit_info = {};
		present_submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		present_submit_info.pWaitDstStageMask = &wait_stage_mask;
		present_submit_info.pWaitSemaphores = &sync_objects[frame()].image_available;
		present_submit_info.waitSemaphoreCount = 1;
		present_submit_info.pSignalSemaphores = &sync_objects[frame()].render_finished;
		present_submit_info.signalSemaphoreCount = 1;
		present_submit_info.pCommandBuffers = &command_buffers[frame()].buffer;
		present_submit_info.commandBufferCount = 1;

		vk_check(vkResetFences(GraphicsContext::the().device(), 1, &sync_objects[frame()].in_flight_fence));
		vk_check(vkQueueSubmit(GraphicsContext::the().graphics_queue(), 1, &present_submit_info, sync_objects[frame()].in_flight_fence));

		VkResult result;
		{
			VkPresentInfoKHR present_info {};
			present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

			present_info.waitSemaphoreCount = 1;
			present_info.pWaitSemaphores = &sync_objects[frame()].render_finished;

			present_info.pSwapchains = &vk_swapchain;
			present_info.swapchainCount = 1;

			present_info.pImageIndices = &current_image_index;
			result = vkQueuePresentKHR(GraphicsContext::the().present_queue(), &present_info);
		}

		if (result != VK_SUCCESS) {
			if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
				on_resize(sc_width, sc_height);
			} else {
				Log::error("[Swapchain] Validation failed in present.");
				vk_check(result);
			}
		}

		current_frame = (frame() + 1) % image_count;
		vk_check(vkWaitForFences(GraphicsContext::the().device(), 1, &sync_objects[frame()].in_flight_fence, VK_TRUE, default_fence_timeout));
	}

	void Swapchain::on_resize(uint32_t w, uint32_t h)
	{
		vkDeviceWaitIdle(GraphicsContext::the().device());
		construct(w, h);
		vkDeviceWaitIdle(GraphicsContext::the().device());
	}

	void Swapchain::cleanup_swapchain()
	{
		for (size_t i = 0; i < frame_buffers.size(); i++) {
			vkDestroyFramebuffer(GraphicsContext::the().device(), frame_buffers[i], nullptr);
		}

		for (size_t i = 0; i < images.views.size(); i++) {
			vkDestroyImageView(GraphicsContext::the().device(), images.views[i], nullptr);
		}

		vkDestroySwapchainKHR(GraphicsContext::the().device(), vk_swapchain, nullptr);
	}

	VkFramebuffer Swapchain::get_current_framebuffer() const { return frame_buffers[frame()]; };

	uint32_t Swapchain::get_width() const { return pixel_size_x; }

	uint32_t Swapchain::get_height() const { return pixel_size_y; }

	uint32_t Swapchain::get_image_count() { return image_count; }

	VkRenderPass Swapchain::get_render_pass() { return vk_render_pass; };

	VkCommandBuffer Swapchain::get_current_drawbuffer() const { return command_buffers[frame()].buffer; }

	uint32_t Swapchain::get_next_image()
	{
		vkWaitForFences(GraphicsContext::the().device(), 1, &sync_objects[frame()].in_flight_fence, VK_TRUE, UINT64_MAX);
		vkResetFences(GraphicsContext::the().device(), 1, &sync_objects[frame()].in_flight_fence);

		uint32_t image_index;
		vk_check(vkAcquireNextImageKHR(GraphicsContext::the().device(), vk_swapchain, default_fence_timeout, sync_objects[frame()].image_available,
			(VkFence) nullptr, &image_index));

		return image_index;
	}

	Swapchain::Capabilities Swapchain::query()
	{
		Capabilities details {};
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(GraphicsContext::the().physical_device(), vk_surface, &details.capabilities);

		uint32_t format_count;
		vkGetPhysicalDeviceSurfaceFormatsKHR(GraphicsContext::the().physical_device(), vk_surface, &format_count, nullptr);

		if (format_count != 0) {
			details.formats.resize(format_count);
			vkGetPhysicalDeviceSurfaceFormatsKHR(GraphicsContext::the().physical_device(), vk_surface, &format_count, details.formats.data());
		}

		uint32_t present_mode_count;
		vkGetPhysicalDeviceSurfacePresentModesKHR(GraphicsContext::the().physical_device(), vk_surface, &present_mode_count, nullptr);

		if (present_mode_count != 0) {
			details.present_modes.resize(present_mode_count);
			vkGetPhysicalDeviceSurfacePresentModesKHR(
				GraphicsContext::the().physical_device(), vk_surface, &present_mode_count, details.present_modes.data());
		}

		return details;
	}

	void Swapchain::choose_format(const Capabilities& capabilities)
	{
		for (const auto& available : capabilities.formats) {
			if (available.format == VK_FORMAT_B8G8R8A8_SRGB && available.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				format = available;
				return;
			}
		}

		format = capabilities.formats[0];
	}

	void Swapchain::choose_present_mode(const Capabilities& capabilities)
	{
		for (const auto& available : capabilities.present_modes) {
			if (available == VK_PRESENT_MODE_MAILBOX_KHR) {
				present_format = available;
				return;
			}
		}

		present_format = VK_PRESENT_MODE_FIFO_KHR;
	}

	void Swapchain::choose_extent(const Capabilities& in)
	{
		const auto& capabilities = in.capabilities;

		int width, height;
		glfwGetFramebufferSize(sc_handle, &width, &height);

		VkExtent2D framebuffer_extent = { .width = static_cast<uint32_t>(width), .height = static_cast<uint32_t>(height) };

		framebuffer_extent.width = std::clamp(framebuffer_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		framebuffer_extent.height = std::clamp(framebuffer_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		extent = framebuffer_extent;
	}

	void Swapchain::create_swapchain(const Capabilities& in)
	{
		const auto& capabilities = in.capabilities;

		image_count = capabilities.minImageCount + 1;

		static constexpr auto preferred = 4;
		if (image_count < preferred && preferred < capabilities.maxImageCount) {
			image_count = preferred;
		}

		if (capabilities.maxImageCount > 0 && image_count > capabilities.maxImageCount) {
			image_count = capabilities.maxImageCount;
		}

		auto old_sc = vk_swapchain;

		VkSwapchainCreateInfoKHR swapchain_create_info {};
		swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchain_create_info.surface = vk_surface;
		swapchain_create_info.minImageCount = image_count;
		swapchain_create_info.imageFormat = format.format;
		swapchain_create_info.imageColorSpace = format.colorSpace;
		swapchain_create_info.imageExtent = extent;
		swapchain_create_info.imageArrayLayers = 1;
		swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		auto graphics_family = GraphicsContext::the().graphics_queue_family();
		auto present_family = GraphicsContext::the().present_queue_family();
		uint32_t indices[] = { graphics_family, present_family };

		if (graphics_family != present_family) {
			swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			swapchain_create_info.queueFamilyIndexCount = 2;
			swapchain_create_info.pQueueFamilyIndices = indices;
		} else {
			swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		VkCompositeAlphaFlagBitsKHR composite_alpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		std::vector<VkCompositeAlphaFlagBitsKHR> composite_alpha_flags = {
			VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
		};
		for (auto& alpha_flag : composite_alpha_flags) {
			if (capabilities.supportedCompositeAlpha & alpha_flag) {
				composite_alpha = alpha_flag;
				break;
			};
		}

		swapchain_create_info.preTransform = capabilities.currentTransform;
		swapchain_create_info.compositeAlpha = composite_alpha;
		swapchain_create_info.presentMode = present_format;
		swapchain_create_info.clipped = VK_TRUE;

		vk_check(vkCreateSwapchainKHR(GraphicsContext::the().device(), &swapchain_create_info, nullptr, &vk_swapchain));

		if (old_sc)
			vkDestroySwapchainKHR(GraphicsContext::the().device(), old_sc, nullptr);

		for (auto& view : images.views)
			vkDestroyImageView(GraphicsContext::the().device(), view, nullptr);
	}

	void Swapchain::retrieve_images()
	{
		auto& [imgs, views] = images;

		vkGetSwapchainImagesKHR(GraphicsContext::the().device(), vk_swapchain, &image_count, nullptr);
		imgs.resize(image_count);
		vkGetSwapchainImagesKHR(GraphicsContext::the().device(), vk_swapchain, &image_count, imgs.data());

		views.resize(image_count);
		for (size_t i = 0; i < image_count; i++) {
			VkImageViewCreateInfo view_create_info {};
			view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			view_create_info.image = imgs[i];
			view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			view_create_info.format = format.format;
			view_create_info.components.r = VK_COMPONENT_SWIZZLE_R;
			view_create_info.components.g = VK_COMPONENT_SWIZZLE_G;
			view_create_info.components.b = VK_COMPONENT_SWIZZLE_B;
			view_create_info.components.a = VK_COMPONENT_SWIZZLE_A;
			view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			view_create_info.subresourceRange.baseMipLevel = 0;
			view_create_info.subresourceRange.levelCount = 1;
			view_create_info.subresourceRange.baseArrayLayer = 0;
			view_create_info.subresourceRange.layerCount = 1;

			vk_check(vkCreateImageView(GraphicsContext::the().device(), &view_create_info, nullptr, &views[i]));
		}
	}

	void Swapchain::create_synchronisation_objects()
	{
		sync_objects.resize(image_count);

		VkSemaphoreCreateInfo semaphore_info {};
		semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fence_info {};
		fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		const auto& device = GraphicsContext::the().device();

		for (size_t i = 0; i < image_count; i++) {
			vk_check(vkCreateSemaphore(device, &semaphore_info, nullptr, &sync_objects[i].image_available));
			vk_check(vkCreateSemaphore(device, &semaphore_info, nullptr, &sync_objects[i].render_finished));
			vk_check(vkCreateFence(device, &fence_info, nullptr, &sync_objects[i].in_flight_fence));
		}
	}

	Swapchain::~Swapchain() = default;

	void Swapchain::destroy()
	{
		auto vk_device = GraphicsContext::the().device();
		wait();

		if (vk_swapchain)
			vkDestroySwapchainKHR(vk_device, vk_swapchain, nullptr);

		for (auto& view : images.views)
			vkDestroyImageView(vk_device, view, nullptr);

		for (auto& cmd_buffer : command_buffers)
			vkDestroyCommandPool(vk_device, cmd_buffer.command_pool, nullptr);

		if (vk_render_pass)
			vkDestroyRenderPass(vk_device, vk_render_pass, nullptr);

		for (auto framebuffer : frame_buffers)
			vkDestroyFramebuffer(vk_device, framebuffer, nullptr);

		for (auto i = 0; i < sync_objects.size(); i++)
			if (sync_objects[i].image_available)
				vkDestroySemaphore(vk_device, sync_objects[i].image_available, nullptr);

		for (auto i = 0; i < sync_objects.size(); i++)
			if (sync_objects[i].render_finished)
				vkDestroySemaphore(vk_device, sync_objects[i].render_finished, nullptr);

		for (auto i = 0; i < sync_objects.size(); i++)
			vkDestroyFence(vk_device, sync_objects[i].in_flight_fence, nullptr);

		vkDestroySurfaceKHR(GraphicsContext::the().instance(), vk_surface, nullptr);

		vkDeviceWaitIdle(vk_device);

		Log::info("[Swapchain] Destroyed swapchain.");
	}

	void Swapchain::wait()
	{
		std::vector<VkFence> fences;
		for (const auto& frame : sync_objects) {
			fences.push_back(frame.in_flight_fence);
		}

		vkWaitForFences(GraphicsContext::the().device(), fences.size(), fences.data(), VK_TRUE, default_fence_timeout);
	}

} // namespace Alabaster
