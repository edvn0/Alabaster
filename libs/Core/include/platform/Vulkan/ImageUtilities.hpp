#pragma once

#include "core/exceptions/AlabasterException.hpp"
#include "graphics/CommandBuffer.hpp"
#include "graphics/GraphicsContext.hpp"
#include "graphics/Image.hpp"

#include <vulkan/vulkan.h>

namespace Alabaster::Utilities {

	static inline void transition_image_layout(
		VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout, const std::unique_ptr<CommandBuffer>& buffer = nullptr)
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

	void copy_buffer_to_image(
		VkBuffer buffer, const ImageInfo& image_info, uint32_t w, uint32_t h, const std::unique_ptr<CommandBuffer>& cmd_buffer = nullptr)
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

} // namespace Alabaster::Utilities
