#include "av_pch.hpp"

#include "graphics/Swapchain.hpp"

#include "core/Common.hpp"
#include "graphics/GraphicsContext.hpp"
#include "platform/Vulkan/ImageUtilities.hpp"

#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>

namespace Alabaster {

	VkCommandBuffer Swapchain::get_current_drawbuffer() const { return get_drawbuffer(current_buffer_index); }

	VkCommandBuffer Swapchain::get_drawbuffer(std::uint32_t frame) const { return command_buffers[frame].CommandBuffer; }

	VkRenderPass Swapchain::get_render_pass() const { return render_pass; }

	std::tuple<VkFormat, VkFormat> Swapchain::get_formats() { return { color_format, VK_FORMAT_D32_SFLOAT }; }

	void Swapchain::init(GLFWwindow* window_handle)
	{
		instance = GraphicsContext::the().instance();
		device = GraphicsContext::the().device();

		glfwCreateWindowSurface(instance, window_handle, nullptr, &surface);

		// Get available queue family properties
		uint32_t queue_count;
		vkGetPhysicalDeviceQueueFamilyProperties(GraphicsContext::the().physical_device(), &queue_count, NULL);

		std::vector<VkQueueFamilyProperties> queue_props(queue_count);
		vkGetPhysicalDeviceQueueFamilyProperties(GraphicsContext::the().physical_device(), &queue_count, queue_props.data());

		// Iterate over each queue to learn whether it supports presenting:
		// Find a queue with present support
		// Will be used to present the swap chain images to the windowing system
		std::vector<VkBool32> supports_present(queue_count);
		for (uint32_t i = 0; i < queue_count; i++) {
			vkGetPhysicalDeviceSurfaceSupportKHR(GraphicsContext::the().physical_device(), i, surface, &supports_present[i]);
		}

		// Search for a graphics and a present queue in the array of queue
		// families, try to find one that supports both
		uint32_t graphics_queue_node_index = UINT32_MAX;
		uint32_t present_queue_node_index = UINT32_MAX;
		for (uint32_t i = 0; i < queue_count; i++) {
			if ((queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
				if (graphics_queue_node_index == UINT32_MAX) {
					graphics_queue_node_index = i;
				}

				if (supports_present[i] == VK_TRUE) {
					graphics_queue_node_index = i;
					present_queue_node_index = i;
					break;
				}
			}
		}

		if (present_queue_node_index == UINT32_MAX) {
			// If there's no queue that supports both present and graphics
			// try to find a separate present queue
			for (uint32_t i = 0; i < queue_count; ++i) {
				if (supports_present[i] == VK_TRUE) {
					present_queue_node_index = i;
					break;
				}
			}
		}

		queue_node_index = graphics_queue_node_index;

		find_image_format_and_color_space();

		Log::info("Color format: {}", enum_name(color_format));
	}

	void Swapchain::create(uint32_t* in_width, uint32_t* in_height, bool in_vsync)
	{
		this->vsync = in_vsync;

		VkSwapchainKHR old_swapchain = swap_chain;

		VkSurfaceCapabilitiesKHR surf_caps;
		vk_check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(GraphicsContext::the().physical_device(), surface, &surf_caps));

		uint32_t present_mode_count;
		vk_check(vkGetPhysicalDeviceSurfacePresentModesKHR(GraphicsContext::the().physical_device(), surface, &present_mode_count, NULL));
		assert_that(present_mode_count > 0);
		std::vector<VkPresentModeKHR> present_modes(present_mode_count);
		vk_check(
			vkGetPhysicalDeviceSurfacePresentModesKHR(GraphicsContext::the().physical_device(), surface, &present_mode_count, present_modes.data()));

		VkExtent2D swapchain_extent {};
		if (surf_caps.currentExtent.width == (uint32_t)-1) {
			swapchain_extent.width = *in_width;
			swapchain_extent.height = *in_height;
		} else {
			swapchain_extent = surf_caps.currentExtent;
			*in_width = surf_caps.currentExtent.width;
			*in_height = surf_caps.currentExtent.height;
		}

		this->width = *in_width;
		this->height = *in_height;

		extent = swapchain_extent;

		VkPresentModeKHR swapchain_present_mode = VK_PRESENT_MODE_FIFO_KHR;

		if (!in_vsync) {
			for (size_t i = 0; i < present_mode_count; i++) {
				if (present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
					swapchain_present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
					break;
				}
				if ((swapchain_present_mode != VK_PRESENT_MODE_MAILBOX_KHR) && (present_modes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)) {
					swapchain_present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
				}
			}
		}

		uint32_t desired_number_of_swapchain_images = surf_caps.minImageCount + 1;
		if ((surf_caps.maxImageCount > 0) && (desired_number_of_swapchain_images > surf_caps.maxImageCount)) {
			desired_number_of_swapchain_images = surf_caps.maxImageCount;
		}

		VkSurfaceTransformFlagsKHR pre_transform;
		if (surf_caps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
			pre_transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		} else {
			pre_transform = surf_caps.currentTransform;
		}

		VkCompositeAlphaFlagBitsKHR composite_alpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		std::vector<VkCompositeAlphaFlagBitsKHR> composite_alpha_flags = {
			VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
		};
		for (auto& composite_alpha_flag : composite_alpha_flags) {
			if (surf_caps.supportedCompositeAlpha & composite_alpha_flag) {
				composite_alpha = composite_alpha_flag;
				break;
			};
		}

		VkSwapchainCreateInfoKHR swapchain_ci = {};
		swapchain_ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchain_ci.pNext = NULL;
		swapchain_ci.surface = surface;
		swapchain_ci.minImageCount = desired_number_of_swapchain_images;
		swapchain_ci.imageFormat = color_format;
		swapchain_ci.imageColorSpace = color_space;
		swapchain_ci.imageExtent = { swapchain_extent.width, swapchain_extent.height };
		swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapchain_ci.preTransform = (VkSurfaceTransformFlagBitsKHR)pre_transform;
		swapchain_ci.imageArrayLayers = 1;
		swapchain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchain_ci.queueFamilyIndexCount = 0;
		swapchain_ci.pQueueFamilyIndices = NULL;
		swapchain_ci.presentMode = swapchain_present_mode;
		swapchain_ci.oldSwapchain = old_swapchain;
		swapchain_ci.clipped = VK_TRUE;
		swapchain_ci.compositeAlpha = composite_alpha;

		if (surf_caps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) {
			swapchain_ci.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		}

		if (surf_caps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
			swapchain_ci.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		}

		vk_check(vkCreateSwapchainKHR(device, &swapchain_ci, nullptr, &swap_chain));

		if (old_swapchain)
			vkDestroySwapchainKHR(device, old_swapchain, nullptr);

		for (auto& image : images)
			vkDestroyImageView(device, image.ImageView, nullptr);
		images.clear();

		vk_check(vkGetSwapchainImagesKHR(device, swap_chain, &image_count, NULL));
		images.resize(image_count);
		vulkan_images.resize(image_count);
		vk_check(vkGetSwapchainImagesKHR(device, swap_chain, &image_count, vulkan_images.data()));

		images.resize(image_count);
		for (uint32_t i = 0; i < image_count; i++) {
			VkImageViewCreateInfo color_attachment_view = {};
			color_attachment_view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			color_attachment_view.pNext = NULL;
			color_attachment_view.format = color_format;
			color_attachment_view.image = vulkan_images[i];
			color_attachment_view.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
			color_attachment_view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			color_attachment_view.subresourceRange.baseMipLevel = 0;
			color_attachment_view.subresourceRange.levelCount = 1;
			color_attachment_view.subresourceRange.baseArrayLayer = 0;
			color_attachment_view.subresourceRange.layerCount = 1;
			color_attachment_view.viewType = VK_IMAGE_VIEW_TYPE_2D;
			color_attachment_view.flags = 0;

			images[i].Image = vulkan_images[i];

			vk_check(vkCreateImageView(device, &color_attachment_view, nullptr, &images[i].ImageView));
		}

		{
			for (auto& command_buffer : command_buffers)
				vkDestroyCommandPool(device, command_buffer.CommandPool, nullptr);

			VkCommandPoolCreateInfo cmd_pool_info = {};
			cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			cmd_pool_info.queueFamilyIndex = queue_node_index;
			cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

			VkCommandBufferAllocateInfo command_buffer_allocate_info {};
			command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			command_buffer_allocate_info.commandBufferCount = 1;

			command_buffers.resize(image_count);
			for (auto& command_buffer : command_buffers) {
				vk_check(vkCreateCommandPool(device, &cmd_pool_info, nullptr, &command_buffer.CommandPool));

				command_buffer_allocate_info.commandPool = command_buffer.CommandPool;
				vk_check(vkAllocateCommandBuffers(device, &command_buffer_allocate_info, &command_buffer.CommandBuffer));
			}
		}

		if (!semaphores.RenderComplete || !semaphores.PresentComplete) {
			VkSemaphoreCreateInfo semaphore_create_info {};
			semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			vk_check(vkCreateSemaphore(device, &semaphore_create_info, nullptr, &semaphores.RenderComplete));
			vk_check(vkCreateSemaphore(device, &semaphore_create_info, nullptr, &semaphores.PresentComplete));
		}

		if (wait_fences.size() != image_count) {
			VkFenceCreateInfo fence_create_info {};
			fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

			wait_fences.resize(image_count);
			for (auto& fence : wait_fences) {
				vk_check(vkCreateFence(device, &fence_create_info, nullptr, &fence));
			}
		}

		VkPipelineStageFlags pipeline_stage_flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		submit_info = {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.pWaitDstStageMask = &pipeline_stage_flags;
		submit_info.waitSemaphoreCount = 1;
		submit_info.pWaitSemaphores = &semaphores.PresentComplete;
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores = &semaphores.RenderComplete;

		VkAttachmentDescription color_attachment_desc = {};
		color_attachment_desc.format = color_format;
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
		subpass_description.inputAttachmentCount = 0;
		subpass_description.preserveAttachmentCount = 0;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 1> attachments = { color_attachment_desc };

		VkRenderPassCreateInfo render_pass_info = {};
		render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		render_pass_info.attachmentCount = static_cast<std::uint32_t>(attachments.size());
		render_pass_info.pAttachments = attachments.data();
		render_pass_info.subpassCount = 1;
		render_pass_info.pSubpasses = &subpass_description;
		render_pass_info.dependencyCount = 1;
		render_pass_info.pDependencies = &dependency;

		if (render_pass) {
			vkDestroyRenderPass(device, render_pass, nullptr);
		}

		vk_check(vkCreateRenderPass(device, &render_pass_info, nullptr, &render_pass));

		{
			for (auto& framebuffer : framebuffers)
				vkDestroyFramebuffer(device, framebuffer, nullptr);

			VkFramebufferCreateInfo frame_buffer_create_info = {};
			frame_buffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			frame_buffer_create_info.renderPass = render_pass;
			frame_buffer_create_info.attachmentCount = 1;
			frame_buffer_create_info.width = this->width;
			frame_buffer_create_info.height = this->height;
			frame_buffer_create_info.layers = 1;

			framebuffers.resize(image_count);
			for (uint32_t i = 0; i < framebuffers.size(); i++) {
				VkImageView fb_attachments[1] = { images[i].ImageView };
				frame_buffer_create_info.pAttachments = fb_attachments;
				frame_buffer_create_info.attachmentCount = 1;
				vk_check(vkCreateFramebuffer(device, &frame_buffer_create_info, nullptr, &framebuffers[i]));
			}
		}
	}

	void Swapchain::destroy()
	{
		vkDeviceWaitIdle(device);

		if (swap_chain)
			vkDestroySwapchainKHR(device, swap_chain, nullptr);

		for (auto& image : images)
			vkDestroyImageView(device, image.ImageView, nullptr);

		for (auto& command_buffer : command_buffers)
			vkDestroyCommandPool(device, command_buffer.CommandPool, nullptr);

		if (render_pass)
			vkDestroyRenderPass(device, render_pass, nullptr);

		for (auto framebuffer : framebuffers)
			vkDestroyFramebuffer(device, framebuffer, nullptr);

		if (semaphores.RenderComplete)
			vkDestroySemaphore(device, semaphores.RenderComplete, nullptr);

		if (semaphores.PresentComplete)
			vkDestroySemaphore(device, semaphores.PresentComplete, nullptr);

		for (auto& fence : wait_fences)
			vkDestroyFence(device, fence, nullptr);

		vkDestroySurfaceKHR(GraphicsContext::the().instance(), surface, nullptr);

		vkDeviceWaitIdle(device);
	}

	void Swapchain::on_resize(uint32_t in_width, uint32_t in_height)
	{
		vkDeviceWaitIdle(device);
		create(&in_width, &in_height, this->vsync);
		vkDeviceWaitIdle(device);
	}

	void Swapchain::begin_frame()
	{
		current_image_index = acquire_next_image();

		if (current_image_index == 9877)
			return;

		vk_check(vkResetCommandPool(device, command_buffers[current_buffer_index].CommandPool, 0));
	}

	void Swapchain::present()
	{

		VkPipelineStageFlags wait_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		VkSubmitInfo present_submit_info = {};
		present_submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		present_submit_info.pWaitDstStageMask = &wait_stage_mask;
		present_submit_info.pWaitSemaphores = &semaphores.PresentComplete;
		present_submit_info.waitSemaphoreCount = 1;
		present_submit_info.pSignalSemaphores = &semaphores.RenderComplete;
		present_submit_info.signalSemaphoreCount = 1;
		present_submit_info.pCommandBuffers = &command_buffers[current_buffer_index].CommandBuffer;
		present_submit_info.commandBufferCount = 1;

		vk_check(vkResetFences(device, 1, &wait_fences[current_buffer_index]));
		vk_check(vkQueueSubmit(GraphicsContext::the().graphics_queue(), 1, &present_submit_info, wait_fences[current_buffer_index]));

		VkResult result;
		{
			VkPresentInfoKHR present_info = {};
			present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			present_info.pNext = NULL;
			present_info.swapchainCount = 1;
			present_info.pSwapchains = &swap_chain;
			present_info.pImageIndices = &current_image_index;
			present_info.pWaitSemaphores = &semaphores.RenderComplete;
			present_info.waitSemaphoreCount = 1;
			result = vkQueuePresentKHR(GraphicsContext::the().graphics_queue(), &present_info);
		}

		if (result != VK_SUCCESS) {
			if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
				on_resize(this->width, this->height);
			} else {
				vk_check(result);
			}
		}

		{
			current_buffer_index = (current_buffer_index + 1) % image_count;
			vk_check(vkWaitForFences(device, 1, &wait_fences[current_buffer_index], VK_TRUE, UINT64_MAX));
		}
	}

	uint32_t Swapchain::acquire_next_image()
	{
		uint32_t image_index;
		auto result = vkAcquireNextImageKHR(device, swap_chain, UINT64_MAX, semaphores.PresentComplete, (VkFence) nullptr, &image_index);

		if (result != VK_SUCCESS) {
			if (result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR) {
				on_resize(width, height);
				return 9877; // Magic number
			} else {
				throw AlabasterException("Could not acquire new image.");
			}
		}

		return image_index;
	}

	void Swapchain::find_image_format_and_color_space()
	{
		std::uint32_t format_count;
		vk_check(vkGetPhysicalDeviceSurfaceFormatsKHR(GraphicsContext::the().physical_device(), surface, &format_count, NULL));
		assert_that(format_count > 0);

		std::vector<VkSurfaceFormatKHR> surface_formats(format_count);
		vk_check(vkGetPhysicalDeviceSurfaceFormatsKHR(GraphicsContext::the().physical_device(), surface, &format_count, surface_formats.data()));

		if (format_count == 1) {
			color_format = surface_formats[0].format;
			color_space = surface_formats[0].colorSpace;
		} else {
			bool found_wanted_format = false;
			for (auto&& surface_format : surface_formats) {
				if (surface_format.format == VK_FORMAT_R8G8B8A8_UNORM) {
					color_format = surface_format.format;
					color_space = surface_format.colorSpace;
					found_wanted_format = true;
					break;
				}
			}

			if (!found_wanted_format) {
				color_format = surface_formats[0].format;
				color_space = surface_formats[0].colorSpace;
			}
		}
	}

} // namespace Alabaster
