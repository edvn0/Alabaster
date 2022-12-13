#pragma once

#include "core/exceptions/AlabasterException.hpp"
#include "graphics/CommandBuffer.hpp"
#include "graphics/DepthImage.hpp"
#include "graphics/GraphicsContext.hpp"
#include "graphics/Image.hpp"

#include <vulkan/vulkan.h>

namespace Alabaster::Utilities {

	void insert_image_memory_barrier(VkCommandBuffer command_buffer, VkImage image, VkAccessFlags src_access_mask, VkAccessFlags dst_access_mask,
		VkImageLayout old_image_layout, VkImageLayout new_image_layout, VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask,
		VkImageSubresourceRange subresource_range);

	void set_image_layout(VkCommandBuffer command_buffer, VkImage image, VkImageLayout old_image_layout, VkImageLayout new_image_layout,
		VkImageSubresourceRange subresource_range, VkPipelineStageFlags src_stage_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VkPipelineStageFlags dst_stage_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

	void set_image_layout(VkCommandBuffer command_buffer, VkImage image, VkImageAspectFlags aspect_mask, VkImageLayout old_image_layout,
		VkImageLayout new_image_layout, VkPipelineStageFlags src_stage_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VkPipelineStageFlags dst_stage_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

	bool is_integer_based(ImageFormat format);

	uint32_t get_image_format_bpp(ImageFormat format);

	bool is_integer_based(const ImageFormat format);

	uint32_t calculate_mip_count(uint32_t width, uint32_t height);

	uint32_t get_image_memory_size(ImageFormat format, uint32_t width, uint32_t height);

	bool is_depth_format(ImageFormat format);

	VkFormat vulkan_image_format(ImageFormat in);

	VkFormat find_supported_format(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

	VkFormat find_depth_format();

	void create_image(std::uint32_t width, std::uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlagBits bits, DepthImage* image);

	VkSamplerAddressMode vulkan_sampler_wrap(TextureWrap wrap);

	VkFilter vulkan_sampler_filter(TextureFilter filter);

	size_t get_memory_size(ImageFormat format, uint32_t width, uint32_t height);

	void create_image(std::uint32_t width, std::uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlagBits bits,
		std::unique_ptr<DepthImage>& image);

	void create_image_view(VkFormat format, VkImageAspectFlagBits bits, DepthImage* image);

	void create_image_view(VkFormat format, VkImageAspectFlagBits bits, std::unique_ptr<DepthImage>& image);

	void transition_image_layout(
		VkImage image, VkImageLayout old_layout, VkImageLayout new_layout, CommandBuffer* buffer, VkImageSubresourceRange* range = nullptr);

	void transition_image_layout(
		VkImage image, VkImageLayout old_layout, VkImageLayout new_layout, const std::unique_ptr<CommandBuffer>& buffer = nullptr);

	void transition_image_layout(VkImage image, VkImageLayout old_layout, VkImageLayout new_layout,
		const std::unique_ptr<CommandBuffer>& buffer = nullptr, VkImageSubresourceRange* range = nullptr);

	void copy_buffer_to_image(VkBuffer buffer, const ImageInfo& image_info, std::uint32_t w, std::uint32_t h, CommandBuffer* cmd_buffer);

	void copy_buffer_to_image(
		VkBuffer buffer, const ImageInfo& image_info, std::uint32_t w, std::uint32_t h, const std::unique_ptr<CommandBuffer>& cmd_buffer = nullptr);

} // namespace Alabaster::Utilities
