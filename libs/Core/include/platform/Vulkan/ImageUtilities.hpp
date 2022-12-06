#pragma once

#include "core/exceptions/AlabasterException.hpp"
#include "graphics/CommandBuffer.hpp"
#include "graphics/DepthImage.hpp"
#include "graphics/GraphicsContext.hpp"
#include "graphics/Image.hpp"

#include <vulkan/vulkan.h>

namespace Alabaster::Utilities {

	inline void create_image(
		std::uint32_t width, std::uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlagBits bits, DepthImage& image)
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
		image.allocation = allocator.allocate_image(image_info, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, image.image);
	}

	inline void create_image(std::uint32_t width, std::uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlagBits bits,
		std::unique_ptr<DepthImage>& image)
	{
		create_image(width, height, format, tiling, bits, *image.get());
	}

	inline void create_image_view(VkFormat format, VkImageAspectFlagBits bits, DepthImage& image)
	{
		VkImageViewCreateInfo view_info {};
		view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		view_info.image = image.image;
		view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		view_info.format = format;
		view_info.subresourceRange.aspectMask = bits;
		view_info.subresourceRange.baseMipLevel = 0;
		view_info.subresourceRange.levelCount = 1;
		view_info.subresourceRange.baseArrayLayer = 0;
		view_info.subresourceRange.layerCount = 1;

		vk_check(vkCreateImageView(GraphicsContext::the().device(), &view_info, nullptr, &image.view));
	}

	inline void create_image_view(VkFormat format, VkImageAspectFlagBits bits, std::unique_ptr<DepthImage>& image)
	{
		create_image_view(format, bits, *image.get());
	}

	inline void transition_image_layout(VkImage image, VkImageLayout old_layout, VkImageLayout new_layout, CommandBuffer* buffer)
	{
		const auto& command_buffer = buffer ? buffer->get_buffer() : GraphicsContext::the().get_command_buffer();

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

		if (!buffer) {
			GraphicsContext::the().flush_command_buffer(command_buffer);
		}
	}

	inline void transition_image_layout(
		VkImage image, VkImageLayout old_layout, VkImageLayout new_layout, const std::unique_ptr<CommandBuffer>& buffer = nullptr)
	{
		transition_image_layout(image, old_layout, new_layout, buffer.get());
	}

	inline void copy_buffer_to_image(VkBuffer buffer, const ImageInfo& image_info, std::uint32_t w, std::uint32_t h, CommandBuffer* cmd_buffer)
	{
		const auto command_buffer = cmd_buffer ? cmd_buffer->get_buffer() : GraphicsContext::the().get_command_buffer();

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

		if (!buffer) {
			GraphicsContext::the().flush_command_buffer(command_buffer);
		}
	}

	inline void copy_buffer_to_image(
		VkBuffer buffer, const ImageInfo& image_info, std::uint32_t w, std::uint32_t h, const std::unique_ptr<CommandBuffer>& cmd_buffer = nullptr)
	{
		copy_buffer_to_image(buffer, image_info, w, h, cmd_buffer.get());
	}

} // namespace Alabaster::Utilities
