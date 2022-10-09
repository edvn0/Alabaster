#include "av_pch.hpp"

#include "graphics/Swapchain.hpp"

#include "core/Application.hpp"
#include "core/Common.hpp"
#include "core/Logger.hpp"
#include "core/Window.hpp"
#include "graphics/GraphicsContext.hpp"

#include <GLFW/glfw3.h>

namespace Alabaster {

	void Swapchain::construct(GLFWwindow* handle, uint32_t width, uint32_t height)
	{
		auto& context = GraphicsContext::the();
		auto instance = context.instance();
		auto device = context.device();

		glfwCreateWindowSurface(instance, handle, nullptr, &vk_surface);

		auto capabilities = query();
		verify(capabilities);

		choose_format(capabilities);
		choose_present_mode(capabilities);
		choose_extent(capabilities);

		create_swapchain(capabilities);
		retrieve_images();

		Log::info("Successfully created the swapchain and retrieved its {} images.", image_count);
		Log::info("SC Info: [Format: {}. Present Mode: {}. Extent: {} by {}]", enum_name(format.format), enum_name(present_format), extent.width,
			extent.height);
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
		auto&& [capabilities, a, b] = in;

		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			extent = capabilities.currentExtent;
		} else {
			auto&& [width, height] = Application::the().get_window()->framebuffer_extent();

			VkExtent2D framebuffer_extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

			framebuffer_extent.width = std::clamp(framebuffer_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			framebuffer_extent.height = std::clamp(framebuffer_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			extent = framebuffer_extent;
		}
	}

	void Swapchain::create_swapchain(const Capabilities& capabilities)
	{
		image_count = capabilities.capabilities.minImageCount + 1;
		if (capabilities.capabilities.maxImageCount > 0 && image_count > capabilities.capabilities.maxImageCount) {
			image_count = capabilities.capabilities.maxImageCount;
		}

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

		swapchain_create_info.preTransform = capabilities.capabilities.currentTransform;
		swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapchain_create_info.presentMode = present_format;
		swapchain_create_info.clipped = VK_TRUE;
		swapchain_create_info.oldSwapchain = VK_NULL_HANDLE;

		vk_check(vkCreateSwapchainKHR(GraphicsContext::the().device(), &swapchain_create_info, nullptr, &vk_swapchain));
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
			view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			view_create_info.subresourceRange.baseMipLevel = 0;
			view_create_info.subresourceRange.levelCount = 1;
			view_create_info.subresourceRange.baseArrayLayer = 0;
			view_create_info.subresourceRange.layerCount = 1;

			vk_check(vkCreateImageView(GraphicsContext::the().device(), &view_create_info, nullptr, &views[i]));
		}
	}

	Swapchain::~Swapchain()
	{
		for (auto view : images.views) {
			vkDestroyImageView(GraphicsContext::the().device(), view, nullptr);
		}

		vkDestroySurfaceKHR(GraphicsContext::the().instance(), vk_surface, nullptr);
		vkDestroySwapchainKHR(GraphicsContext::the().device(), vk_swapchain, nullptr);
	}

} // namespace Alabaster
