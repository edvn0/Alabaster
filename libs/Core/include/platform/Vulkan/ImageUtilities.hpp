#pragma once

#include "core/exceptions/AlabasterException.hpp"
#include "graphics/CommandBuffer.hpp"
#include "graphics/DepthImage.hpp"
#include "graphics/GraphicsContext.hpp"
#include "graphics/Image.hpp"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace Alabaster::Utilities {

	inline void insert_image_memory_barrier(VkCommandBuffer command_buffer, VkImage image, VkAccessFlags src_access_mask,
		VkAccessFlags dst_access_mask, VkImageLayout old_image_layout, VkImageLayout new_image_layout, VkPipelineStageFlags src_stage_mask,
		VkPipelineStageFlags dst_stage_mask, VkImageSubresourceRange subresource_range)
	{
		VkImageMemoryBarrier image_memory_barrier {};
		image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		image_memory_barrier.srcAccessMask = src_access_mask;
		image_memory_barrier.dstAccessMask = dst_access_mask;
		image_memory_barrier.oldLayout = old_image_layout;
		image_memory_barrier.newLayout = new_image_layout;
		image_memory_barrier.image = image;
		image_memory_barrier.subresourceRange = subresource_range;

		vkCmdPipelineBarrier(command_buffer, src_stage_mask, dst_stage_mask, 0, 0, nullptr, 0, nullptr, 1, &image_memory_barrier);
	}

	inline void set_image_layout(VkCommandBuffer command_buffer, VkImage image, VkImageLayout old_image_layout, VkImageLayout new_image_layout,
		VkImageSubresourceRange subresource_range, VkPipelineStageFlags src_stage_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VkPipelineStageFlags dst_stage_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT)
	{
		VkImageMemoryBarrier image_memory_barrier = {};
		image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		image_memory_barrier.oldLayout = old_image_layout;
		image_memory_barrier.newLayout = new_image_layout;
		image_memory_barrier.image = image;
		image_memory_barrier.subresourceRange = subresource_range;

		switch (old_image_layout) {
		case VK_IMAGE_LAYOUT_UNDEFINED:
			image_memory_barrier.srcAccessMask = 0;
			break;

		case VK_IMAGE_LAYOUT_PREINITIALIZED:
			image_memory_barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			image_memory_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			image_memory_barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			image_memory_barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;
		default:
			break;
		}

		switch (new_image_layout) {
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;

		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			image_memory_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			image_memory_barrier.dstAccessMask = image_memory_barrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			if (image_memory_barrier.srcAccessMask == 0) {
				image_memory_barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
			}
			image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;
		default:
			break;
		}

		vkCmdPipelineBarrier(command_buffer, src_stage_mask, dst_stage_mask, 0, 0, nullptr, 0, nullptr, 1, &image_memory_barrier);
	}

	inline void set_image_layout(VkCommandBuffer cmdbuffer, VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldImageLayout,
		VkImageLayout newImageLayout, VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT)
	{
		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = aspectMask;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = 1;
		subresourceRange.layerCount = 1;
		set_image_layout(cmdbuffer, image, oldImageLayout, newImageLayout, subresourceRange, srcStageMask, dstStageMask);
	}

	inline auto is_integer_based(auto) { return false; }

	inline uint32_t get_image_format_bpp(ImageFormat format)
	{
		switch (format) {
		case ImageFormat::RED8UN:
			return 1;
		case ImageFormat::RED8UI:
			return 1;
		case ImageFormat::RED16UI:
			return 2;
		case ImageFormat::RED32UI:
			return 4;
		case ImageFormat::RED32F:
			return 4;
		case ImageFormat::RGB:
		case ImageFormat::SRGB:
			return 3;
		case ImageFormat::RGBA:
			return 4;
		case ImageFormat::RGBA16F:
			return 2 * 4;
		case ImageFormat::RGBA32F:
			return 4 * 4;
		case ImageFormat::B10R11G11UF:
			return 4;
		default:
			throw AlabasterException("Test");
		}
		return 0;
	}

	inline bool is_integer_based(const ImageFormat format)
	{
		switch (format) {
		case ImageFormat::RED16UI:
		case ImageFormat::RED32UI:
		case ImageFormat::RED8UI:
		case ImageFormat::DEPTH32FSTENCIL8UINT:
			return true;
		case ImageFormat::DEPTH32F:
		case ImageFormat::RED8UN:
		case ImageFormat::RGBA32F:
		case ImageFormat::B10R11G11UF:
		case ImageFormat::RG16F:
		case ImageFormat::RG32F:
		case ImageFormat::RED32F:
		case ImageFormat::RG8:
		case ImageFormat::RGBA:
		case ImageFormat::RGBA16F:
		case ImageFormat::RGB:
		case ImageFormat::SRGB:
		case ImageFormat::DEPTH24STENCIL8:
			return false;
		default:
			throw AlabasterException("Test");
		}
		return false;
	}

	inline uint32_t calculate_mip_count(uint32_t width, uint32_t height) { return (uint32_t)std::floor(std::log2(glm::min(width, height))) + 1; }

	inline uint32_t get_image_memory_size(ImageFormat format, uint32_t width, uint32_t height)
	{
		return width * height * get_image_format_bpp(format);
	}

	inline bool is_depth_format(ImageFormat format)
	{
		if (format == ImageFormat::DEPTH24STENCIL8 || format == ImageFormat::DEPTH32F || format == ImageFormat::DEPTH32FSTENCIL8UINT)
			return true;

		return false;
	}

	inline auto vulkan_image_format(ImageFormat in)
	{
		switch (in) {
		case ImageFormat::RGBA:
			return VK_FORMAT_R8G8B8A8_SRGB;
		case ImageFormat::RGB:
			return VK_FORMAT_R8G8B8_SRGB;
		case ImageFormat::DEPTH32F:
			return VK_FORMAT_D32_SFLOAT;
		case ImageFormat::DEPTH24STENCIL8:
			return VK_FORMAT_D24_UNORM_S8_UINT;
		case ImageFormat::DEPTH32FSTENCIL8UINT:
			return VK_FORMAT_D32_SFLOAT_S8_UINT;
		default:
			throw AlabasterException("We do not support RBG/RBGA.");
		};
	}

	inline VkFormat find_supported_format(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
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

	inline VkFormat find_depth_format()
	{
		auto format = find_supported_format({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
		return format;
	}

	inline void create_image(
		std::uint32_t width, std::uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlagBits bits, DepthImage* image)
	{
		VkImageCreateInfo image_info {};
		image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image_info.imageType = VK_IMAGE_TYPE_2D;
		image_info.extent.width = width;
		image_info.extent.height = height;
		image_info.extent.depth = 1;
		image_info.mipLevels = 1;
		image_info.arrayLayers = 1;
		image_info.format = format;
		image_info.tiling = tiling;
		image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		image_info.usage = bits;
		image_info.samples = VK_SAMPLE_COUNT_1_BIT;
		image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		Allocator allocator("Create Image");
		image->allocation = allocator.allocate_image(image_info, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, image->image);
	}

	inline VkSamplerAddressMode vulkan_sampler_wrap(TextureWrap wrap)
	{
		switch (wrap) {
		case TextureWrap::Clamp:
			return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		case TextureWrap::Repeat:
			return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		default:
			throw AlabasterException("We do not support this wrap mode");
		}
	}

	inline VkFilter vulkan_sampler_filter(TextureFilter filter)
	{
		switch (filter) {
		case TextureFilter::Linear:
			return VK_FILTER_LINEAR;
		case TextureFilter::Nearest:
			return VK_FILTER_NEAREST;
		case TextureFilter::Cubic:
			return VK_FILTER_CUBIC_IMG;
		default:
			throw AlabasterException("We do not support this filter mode");
		}
		return (VkFilter)0;
	}

	inline size_t get_memory_size(ImageFormat format, uint32_t width, uint32_t height)
	{
		switch (format) {
		case ImageFormat::RED16UI:
			return width * height * sizeof(uint16_t);
		case ImageFormat::RG16F:
			return width * height * 2 * sizeof(uint16_t);
		case ImageFormat::RG32F:
			return width * height * 2 * sizeof(float);
		case ImageFormat::RED32F:
			return width * height * sizeof(float);
		case ImageFormat::RED8UN:
			return width * height;
		case ImageFormat::RED8UI:
			return width * height;
		case ImageFormat::RGBA:
			return width * height * 4;
		case ImageFormat::RGBA32F:
			return width * height * 4 * sizeof(float);
		case ImageFormat::B10R11G11UF:
			return width * height * sizeof(float);
		default:
			throw AlabasterException("We do not support this image format as memory type.");
		}
		return 0;
	}

	inline void create_image(std::uint32_t width, std::uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlagBits bits,
		std::unique_ptr<DepthImage>& image)
	{
		create_image(width, height, format, tiling, bits, image.get());
	}

	inline void create_image_view(VkFormat format, VkImageAspectFlagBits bits, DepthImage* image)
	{
		VkImageViewCreateInfo view_info {};
		view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		view_info.image = image->image;
		view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		view_info.format = format;
		view_info.subresourceRange.aspectMask = bits;
		view_info.subresourceRange.baseMipLevel = 0;
		view_info.subresourceRange.levelCount = 1;
		view_info.subresourceRange.baseArrayLayer = 0;
		view_info.subresourceRange.layerCount = 1;

		vk_check(vkCreateImageView(GraphicsContext::the().device(), &view_info, nullptr, &image->view));
	}

	inline void create_image_view(VkFormat format, VkImageAspectFlagBits bits, std::unique_ptr<DepthImage>& image)
	{
		create_image_view(format, bits, image.get());
	}

	inline void transition_image_layout(
		VkImage image, VkImageLayout old_layout, VkImageLayout new_layout, CommandBuffer* buffer, VkImageSubresourceRange* range = nullptr)
	{
		const auto& command_buffer = buffer ? buffer->get_buffer() : ImmediateCommandBuffer();

		VkImageMemoryBarrier barrier {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = old_layout;
		barrier.newLayout = new_layout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		if (range) {
			barrier.subresourceRange = *range;
		}

		VkPipelineStageFlags source_stage;
		VkPipelineStageFlags dst_stage;

		if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		} else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		} else {
			throw AlabasterException("Unsupported layout transition!");
		}

		vkCmdPipelineBarrier(command_buffer, source_stage, dst_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
	}

	inline void transition_image_layout(
		VkImage image, VkImageLayout old_layout, VkImageLayout new_layout, const std::unique_ptr<CommandBuffer>& buffer = nullptr)
	{
		transition_image_layout(image, old_layout, new_layout, buffer.get());
	}

	inline void transition_image_layout(VkImage image, VkImageLayout old_layout, VkImageLayout new_layout,
		const std::unique_ptr<CommandBuffer>& buffer = nullptr, VkImageSubresourceRange* range = nullptr)
	{
		transition_image_layout(image, old_layout, new_layout, buffer.get(), range);
	}

	inline void copy_buffer_to_image(VkBuffer buffer, const ImageInfo& image_info, std::uint32_t w, std::uint32_t h, CommandBuffer* cmd_buffer)
	{
		const auto command_buffer = cmd_buffer ? cmd_buffer->get_buffer() : ImmediateCommandBuffer();

		VkBufferImageCopy region {};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { w, h, 1 };

		vkCmdCopyBufferToImage(command_buffer, buffer, image_info.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	}

	inline void copy_buffer_to_image(
		VkBuffer buffer, const ImageInfo& image_info, std::uint32_t w, std::uint32_t h, const std::unique_ptr<CommandBuffer>& cmd_buffer = nullptr)
	{
		copy_buffer_to_image(buffer, image_info, w, h, cmd_buffer.get());
	}

} // namespace Alabaster::Utilities
