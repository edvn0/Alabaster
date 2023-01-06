#include "av_pch.hpp"

#include "graphics/Image.hpp"

#include "core/Common.hpp"
#include "core/exceptions/AlabasterException.hpp"
#include "core/Logger.hpp"
#include "graphics/Allocator.hpp"
#include "graphics/CommandBuffer.hpp"
#include "graphics/GraphicsContext.hpp"
#include "platform/Vulkan/ImageUtilities.hpp"
#include "utilities/FileInputOutput.hpp"
#include "vulkan/vulkan_core.h"

#include <stb_image.h>
#include <vulkan/vulkan.h>

namespace Alabaster {

	Image::Image(const ImageSpecification& specification) noexcept
		: spec(specification)
	{
		assert_that(spec.width > 0 && spec.height > 0);
	}

	void Image::destroy()
	{
		if (destroyed) {
			return;
		}
		if (info.image) {
			vkDestroyImageView(GraphicsContext::the().device(), info.view, nullptr);
			vkDestroySampler(GraphicsContext::the().device(), info.sampler, nullptr);

			for (auto& view : per_layer_image_views) {
				if (view)
					vkDestroyImageView(GraphicsContext::the().device(), view, nullptr);
			}

			Allocator allocator(fmt::format("Image-{}", spec.debug_name));
			allocator.destroy_image(info.image, info.allocation);

			Log::warn("[Image] Destroy ImageView {}", (const void*)info.view);
			per_layer_image_views.clear();
		}
		destroyed = true;
	}

	void Image::invalidate()
	{
		release();

		Allocator allocator("Image2D");

		VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT; // TODO: this (probably) shouldn't be implied
		if (spec.usage == ImageUsage::Attachment) {
			if (Utilities::is_depth_format(spec.format)) {
				usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			} else {
				usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			}
		} else if (spec.usage == ImageUsage::Texture) {
			usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		} else if (spec.usage == ImageUsage::Storage) {
			usage |= VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		}

		VkImageAspectFlags aspect_mask = Utilities::is_depth_format(spec.format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		if (spec.format == ImageFormat::DEPTH24STENCIL8)
			aspect_mask |= VK_IMAGE_ASPECT_STENCIL_BIT;

		VkFormat vulkan_format = Utilities::vulkan_image_format(spec.format);

		VkImageCreateInfo image_create_info {};
		image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image_create_info.imageType = VK_IMAGE_TYPE_2D;
		image_create_info.format = vulkan_format;
		image_create_info.extent.width = spec.width;
		image_create_info.extent.height = spec.height;
		image_create_info.extent.depth = 1;
		image_create_info.mipLevels = spec.mips;
		image_create_info.arrayLayers = spec.layers;
		image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
		image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
		image_create_info.usage = usage;
		info.allocation = allocator.allocate_image(image_create_info, VMA_MEMORY_USAGE_GPU_ONLY, info.image);

		VkImageViewCreateInfo image_view_create_info {};
		image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		image_view_create_info.viewType = spec.layers > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
		image_view_create_info.format = vulkan_format;
		image_view_create_info.flags = 0;
		image_view_create_info.subresourceRange = {};
		image_view_create_info.subresourceRange.aspectMask = aspect_mask;
		image_view_create_info.subresourceRange.baseMipLevel = 0;
		image_view_create_info.subresourceRange.levelCount = spec.mips;
		image_view_create_info.subresourceRange.baseArrayLayer = 0;
		image_view_create_info.subresourceRange.layerCount = spec.layers;
		image_view_create_info.image = info.image;
		vk_check(vkCreateImageView(GraphicsContext::the().device(), &image_view_create_info, nullptr, &info.view));

		VkSamplerCreateInfo sampler_create_info {};
		sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		sampler_create_info.maxAnisotropy = 1.0f;
		if (Utilities::is_integer_based(spec.format)) {
			sampler_create_info.magFilter = VK_FILTER_NEAREST;
			sampler_create_info.minFilter = VK_FILTER_NEAREST;
			sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		} else {
			sampler_create_info.magFilter = VK_FILTER_LINEAR;
			sampler_create_info.minFilter = VK_FILTER_LINEAR;
			sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		}

		sampler_create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		sampler_create_info.addressModeV = sampler_create_info.addressModeU;
		sampler_create_info.addressModeW = sampler_create_info.addressModeU;
		sampler_create_info.mipLodBias = 0.0f;
		sampler_create_info.maxAnisotropy = 1.0f;
		sampler_create_info.minLod = 0.0f;
		sampler_create_info.maxLod = 100.0f;
		sampler_create_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		vk_check(vkCreateSampler(GraphicsContext::the().device(), &sampler_create_info, nullptr, &info.sampler));

		if (spec.usage == ImageUsage::Storage) {
			ImmediateCommandBuffer buffer { "Image Transition" };
			VkImageSubresourceRange subresource_range {};
			subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			subresource_range.baseMipLevel = 0;
			subresource_range.levelCount = spec.mips;
			subresource_range.layerCount = spec.layers;

			Utilities::transition_image_layout(info.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, &buffer, &subresource_range);
		}

		update_descriptor();
	}

	void Image::release()
	{
		if (info.image == nullptr)
			return;

		vkDestroyImageView(GraphicsContext::the().device(), info.view, nullptr);
		vkDestroySampler(GraphicsContext::the().device(), info.sampler, nullptr);

		for (auto& view : per_mip_image_views) {
			if (view.second)
				vkDestroyImageView(GraphicsContext::the().device(), view.second, nullptr);
		}
		for (auto& view : per_layer_image_views) {
			if (view)
				vkDestroyImageView(GraphicsContext::the().device(), view, nullptr);
		}
		Allocator allocator(fmt::format("Image-{}", spec.debug_name));
		allocator.destroy_image(info.image, info.allocation);
		info.image = nullptr;
		info.view = nullptr;
		info.sampler = nullptr;
		per_layer_image_views.clear();
		per_mip_image_views.clear();
	}

	void Image::create_per_layer_image_view()
	{
		VkImageAspectFlags aspect_mask = Utilities::is_depth_format(spec.format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		if (spec.format == ImageFormat::DEPTH24STENCIL8) {
			aspect_mask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}

		const VkFormat vulkan_format = Utilities::vulkan_image_format(spec.format);

		per_layer_image_views.resize(spec.layers);
		for (std::uint32_t layer = 0; layer < spec.layers; layer++) {
			VkImageViewCreateInfo image_view_create_info = {};
			image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			image_view_create_info.format = vulkan_format;
			image_view_create_info.flags = 0;
			image_view_create_info.subresourceRange = {};
			image_view_create_info.subresourceRange.aspectMask = aspect_mask;
			image_view_create_info.subresourceRange.baseMipLevel = 0;
			image_view_create_info.subresourceRange.levelCount = spec.mips;
			image_view_create_info.subresourceRange.baseArrayLayer = layer;
			image_view_create_info.subresourceRange.layerCount = 1;
			image_view_create_info.image = info.image;
			vk_check(vkCreateImageView(GraphicsContext::the().device(), &image_view_create_info, nullptr, &per_layer_image_views[layer]));
		}
	}

	VkImageView Image::get_mip_image_view(uint32_t mip)
	{
		if (per_mip_image_views.find(mip) == per_mip_image_views.end()) {
			VkImageAspectFlags aspect_mask = Utilities::is_depth_format(spec.format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
			if (spec.format == ImageFormat::DEPTH24STENCIL8)
				aspect_mask |= VK_IMAGE_ASPECT_STENCIL_BIT;

			VkFormat vulkan_format = Utilities::vulkan_image_format(spec.format);

			VkImageViewCreateInfo image_view_create_info = {};
			image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			image_view_create_info.format = vulkan_format;
			image_view_create_info.flags = 0;
			image_view_create_info.subresourceRange = {};
			image_view_create_info.subresourceRange.aspectMask = aspect_mask;
			image_view_create_info.subresourceRange.baseMipLevel = mip;
			image_view_create_info.subresourceRange.levelCount = 1;
			image_view_create_info.subresourceRange.baseArrayLayer = 0;
			image_view_create_info.subresourceRange.layerCount = 1;
			image_view_create_info.image = info.image;

			vk_check(vkCreateImageView(GraphicsContext::the().device(), &image_view_create_info, nullptr, &per_mip_image_views[mip]));
			return per_mip_image_views.at(mip);
		}

		return per_mip_image_views.at(mip);
	}

	void Image::update_descriptor()
	{
		if (spec.format == ImageFormat::DEPTH24STENCIL8 || spec.format == ImageFormat::DEPTH32F || spec.format == ImageFormat::DEPTH32FSTENCIL8UINT)
			descriptor_image_info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
		else if (spec.usage == ImageUsage::Storage)
			descriptor_image_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		else
			descriptor_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		if (spec.usage == ImageUsage::Storage)
			descriptor_image_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

		descriptor_image_info.imageView = info.view;
		descriptor_image_info.sampler = info.sampler;
	}

	void Image::create_per_specific_layer_image_views(const std::vector<uint32_t>& layer_indices)
	{
		VkImageAspectFlags aspect_mask = Utilities::is_depth_format(spec.format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		if (spec.format == ImageFormat::DEPTH24STENCIL8)
			aspect_mask |= VK_IMAGE_ASPECT_STENCIL_BIT;

		const VkFormat vulkan_format = Utilities::vulkan_image_format(spec.format);

		if (per_layer_image_views.empty())
			per_layer_image_views.resize(spec.layers);

		for (uint32_t layer : layer_indices) {
			VkImageViewCreateInfo image_view_create_info = {};
			image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			image_view_create_info.format = vulkan_format;
			image_view_create_info.flags = 0;
			image_view_create_info.subresourceRange = {};
			image_view_create_info.subresourceRange.aspectMask = aspect_mask;
			image_view_create_info.subresourceRange.baseMipLevel = 0;
			image_view_create_info.subresourceRange.levelCount = spec.mips;
			image_view_create_info.subresourceRange.baseArrayLayer = layer;
			image_view_create_info.subresourceRange.layerCount = 1;
			image_view_create_info.image = info.image;
			vk_check(vkCreateImageView(GraphicsContext::the().device(), &image_view_create_info, nullptr, &per_layer_image_views[layer]));
		}
	}

} // namespace Alabaster
