#include "av_pch.hpp"

#include "graphics/VulkanSwapChain.hpp"

#include "core/Common.hpp"
#include "graphics/GraphicsContext.hpp"
#include "graphics/Swapchain.hpp"
#include "platform/Vulkan/ImageUtilities.hpp"

#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>

namespace Alabaster {

	VkFormat find_supported_format(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
	{
		for (VkFormat format : candidates) {
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(GraphicsContext::the().physical_device(), format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
				return format;
			} else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
				return format;
			}
		}

		throw std::runtime_error("failed to find supported format!");
	}

	VkFormat find_depth_format()
	{
		auto format = find_supported_format({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
		;
		return format;
	}

	std::tuple<VkFormat, VkFormat> VulkanSwapChain::get_formats() { return { color_format, VK_FORMAT_D32_SFLOAT }; }

	void VulkanSwapChain::init(GLFWwindow* window_handle)
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
	}

	void VulkanSwapChain::create(uint32_t* in_width, uint32_t* in_height, bool in_vsync)
	{
		this->vsync = in_vsync;

		VkSwapchainKHR old_swapchain = swap_chain;

		// Get physical device surface properties and formats
		VkSurfaceCapabilitiesKHR surf_caps;
		vk_check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(GraphicsContext::the().physical_device(), surface, &surf_caps));

		// Get available present modes
		uint32_t present_mode_count;
		vk_check(vkGetPhysicalDeviceSurfacePresentModesKHR(GraphicsContext::the().physical_device(), surface, &present_mode_count, NULL));
		assert_that(present_mode_count > 0);
		std::vector<VkPresentModeKHR> present_modes(present_mode_count);
		vk_check(
			vkGetPhysicalDeviceSurfacePresentModesKHR(GraphicsContext::the().physical_device(), surface, &present_mode_count, present_modes.data()));

		VkExtent2D swapchain_extent = {};
		// If width (and height) equals the special value 0xFFFFFFFF, the size of the surface will be set by the swapchain
		if (surf_caps.currentExtent.width == (uint32_t)-1) {
			// If the surface size is undefined, the size is set to
			// the size of the images requested.
			swapchain_extent.width = *in_width;
			swapchain_extent.height = *in_height;
		} else {
			// If the surface size is defined, the swap chain size must match
			swapchain_extent = surf_caps.currentExtent;
			*in_width = surf_caps.currentExtent.width;
			*in_height = surf_caps.currentExtent.height;
		}

		this->width = *in_width;
		this->height = *in_height;

		extent = swapchain_extent;

		// Select a present mode for the swapchain

		// The VK_PRESENT_MODE_FIFO_KHR mode must always be present as per spec
		// This mode waits for the vertical blank ("v-sync")
		VkPresentModeKHR swapchain_present_mode = VK_PRESENT_MODE_FIFO_KHR;

		// If v-sync is not requested, try to find a mailbox mode
		// It's the lowest latency non-tearing present mode available
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

		// Determine the number of images
		uint32_t desired_number_of_swapchain_images = surf_caps.minImageCount + 1;
		if ((surf_caps.maxImageCount > 0) && (desired_number_of_swapchain_images > surf_caps.maxImageCount)) {
			desired_number_of_swapchain_images = surf_caps.maxImageCount;
		}

		// Find the transformation of the surface
		VkSurfaceTransformFlagsKHR pre_transform;
		if (surf_caps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
			// We prefer a non-rotated transform
			pre_transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		} else {
			pre_transform = surf_caps.currentTransform;
		}

		// Find a supported composite alpha format (not all devices support alpha opaque)
		VkCompositeAlphaFlagBitsKHR composite_alpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		// Simply select the first composite alpha format available
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
		// Setting clipped to VK_TRUE allows the implementation to discard rendering outside of the surface area
		swapchain_ci.clipped = VK_TRUE;
		swapchain_ci.compositeAlpha = composite_alpha;

		// Enable transfer source on swap chain images if supported
		if (surf_caps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) {
			swapchain_ci.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		}

		// Enable transfer destination on swap chain images if supported
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
		// Get the swap chain images
		images.resize(image_count);
		vulkan_images.resize(image_count);
		vk_check(vkGetSwapchainImagesKHR(device, swap_chain, &image_count, vulkan_images.data()));

		// Get the swap chain buffers containing the image and imageview
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

		if (depth_image) {
			depth_image->destroy(device);
		}

		depth_image.reset(new DepthImage());

		Utilities::create_image(
			extent.width, extent.height, VK_FORMAT_D32_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, depth_image);
		Utilities::create_image_view(VK_FORMAT_D32_SFLOAT, VK_IMAGE_ASPECT_DEPTH_BIT, depth_image);

		// Create command buffers
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

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Synchronization Objects
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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

		VkFormat depth_format = VK_FORMAT_D32_SFLOAT;

		// Render Pass
		VkAttachmentDescription color_attachment_desc = {};
		// Color attachment
		color_attachment_desc.format = color_format;
		color_attachment_desc.samples = VK_SAMPLE_COUNT_1_BIT;
		color_attachment_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		color_attachment_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		color_attachment_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_attachment_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		color_attachment_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		color_attachment_desc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		// Render Pass
		VkAttachmentDescription depth_attachment_desc = {};
		// Color attachment
		depth_attachment_desc.format = depth_format;
		depth_attachment_desc.samples = VK_SAMPLE_COUNT_1_BIT;
		depth_attachment_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depth_attachment_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depth_attachment_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depth_attachment_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depth_attachment_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depth_attachment_desc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference color_reference = {};
		color_reference.attachment = 0;
		color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depth_reference = {};
		depth_reference.attachment = 1;
		depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass_description = {};
		subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass_description.colorAttachmentCount = 1;
		subpass_description.pColorAttachments = &color_reference;
		subpass_description.inputAttachmentCount = 0;
		subpass_description.pInputAttachments = nullptr;
		subpass_description.preserveAttachmentCount = 0;
		subpass_description.pPreserveAttachments = nullptr;
		subpass_description.pResolveAttachments = nullptr;
		subpass_description.pDepthStencilAttachment = &depth_reference;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 2> attachments = { color_attachment_desc, depth_attachment_desc };

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
				VkImageView fb_attachments[2] = { images[i].ImageView, depth_image->view };
				frame_buffer_create_info.pAttachments = fb_attachments;
				frame_buffer_create_info.attachmentCount = 2;
				vk_check(vkCreateFramebuffer(device, &frame_buffer_create_info, nullptr, &framebuffers[i]));
			}
		}
	}

	void VulkanSwapChain::destroy()
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

		depth_image->destroy(device);

		vkDeviceWaitIdle(device);
	}

	void VulkanSwapChain::on_resize(uint32_t in_width, uint32_t in_height)
	{
		vkDeviceWaitIdle(device);
		create(&in_width, &in_height, this->vsync);
		vkDeviceWaitIdle(device);
	}

	void VulkanSwapChain::begin_frame()
	{
		current_image_index = acquire_next_image();

		vk_check(vkResetCommandPool(device, command_buffers[current_buffer_index].CommandPool, 0));
	}

	void VulkanSwapChain::present()
	{

		const uint64_t default_fence_timeout = 100000000000;

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

		// Present the current buffer to the swap chain
		// Pass the semaphore signaled by the command buffer submission from the submit info as the wait semaphore for swap chain presentation
		// This ensures that the image is not presented to the windowing system until all commands have been submitted
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

	uint32_t VulkanSwapChain::acquire_next_image()
	{
		uint32_t image_index;
		vk_check(vkAcquireNextImageKHR(device, swap_chain, UINT64_MAX, semaphores.PresentComplete, (VkFence) nullptr, &image_index));
		return image_index;
	}

	void VulkanSwapChain::find_image_format_and_color_space()
	{
		// Get list of supported surface formats
		std::uint32_t format_count;
		vk_check(vkGetPhysicalDeviceSurfaceFormatsKHR(GraphicsContext::the().physical_device(), surface, &format_count, NULL));
		assert_that(format_count > 0);

		std::vector<VkSurfaceFormatKHR> surface_formats(format_count);
		vk_check(vkGetPhysicalDeviceSurfaceFormatsKHR(GraphicsContext::the().physical_device(), surface, &format_count, surface_formats.data()));

		// If the surface format list only includes one entry with VK_FORMAT_UNDEFINED,
		// there is no preferered format, so we assume VK_FORMAT_B8G8R8A8_UNORM
		if ((format_count == 1) && (surface_formats[0].format == VK_FORMAT_UNDEFINED)) {
			color_format = VK_FORMAT_B8G8R8A8_UNORM;
			color_space = surface_formats[0].colorSpace;
		} else {
			// iterate over the list of available imagesrface format and
			bool found_b8_g8_r8_a8_unorm = false;
			for (auto&& surface_format : surface_formats) {
				if (surface_format.format == VK_FORMAT_R8G8B8A8_UNORM) {
					color_format = surface_format.format;
					color_space = surface_format.colorSpace;
					found_b8_g8_r8_a8_unorm = true;
					break;
				}
			}

			// in case VK_FORMAT_B8G8R8A8_UNORM is not available
			// select the first available color format
			if (!found_b8_g8_r8_a8_unorm) {
				color_format = surface_formats[0].format;
				color_space = surface_formats[0].colorSpace;
			}
		}
	}

} // namespace Alabaster
