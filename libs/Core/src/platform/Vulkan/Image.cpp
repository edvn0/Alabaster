#include "av_pch.hpp"

#include "graphics/Image.hpp"

#include "core/Common.hpp"
#include "core/exceptions/AlabasterException.hpp"
#include "core/Logger.hpp"
#include "graphics/Allocator.hpp"
#include "graphics/GraphicsContext.hpp"
#include "platform/Vulkan/ImageUtilities.hpp"
#include "utilities/FileInputOutput.hpp"
#include "vulkan/vulkan_core.h"

#include <stb_image.h>
#include <vulkan/vulkan.h>

namespace Alabaster {

	Image::Image(const std::filesystem::path& path, ImageFormat format)
	{
		Log::info("[Image] Creating image with path: {}", path.string());

		int w, h, actual_channels;
		uint8_t* data = stbi_load(path.string().data(), &w, &h, &actual_channels, STBI_rgb_alpha);
		image_props.width = static_cast<std::uint32_t>(w);
		image_props.height = static_cast<std::uint32_t>(h);
		image_props.channels = static_cast<std::uint32_t>(STBI_rgb_alpha);

		Log::info("[Image] w: {}, h: {}, channels: {}", w, h, actual_channels);
		invalidate(data);
	}

	Image::Image(const ImageProps& props)
		: image_props(props)
	{
		assert_that(props.path, "You need to supply a path to an image file.");
		const auto path = props.path.value();

		int w, h, actual_channels;
		uint8_t* data = stbi_load(path.string().data(), &w, &h, &actual_channels, STBI_rgb_alpha);

		assert_that(data != nullptr, "Image data is nullptr.");

		image_props.width = static_cast<std::uint32_t>(w);
		image_props.height = static_cast<std::uint32_t>(h);
		image_props.channels = static_cast<std::uint32_t>(STBI_rgb_alpha);

		Log::info("[Image] Creating image with path: {}\n\t[Image] Properties: {}x{}px, {} channels.", path.string(), w, h, actual_channels);

		pixel_data = data;
	}

	void Image::invalidate(CommandBuffer& buffer)
	{
		if (!pixel_data) {
			throw AlabasterException("Pixel data has not been loaded, either correctly, or at all.");
		}

		Allocator allocator("Image");

		VkDeviceSize buffer_size = static_cast<VkDeviceSize>(image_props.width * image_props.height * image_props.channels);

		VkBufferCreateInfo buffer_create_info {};
		buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_create_info.size = buffer_size;
		buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		VkBuffer staging_buffer;
		VmaAllocation staging_buffer_allocation = allocator.allocate_buffer(buffer_create_info, VMA_MEMORY_USAGE_CPU_TO_GPU, staging_buffer);

		auto* dest = allocator.map_memory<stbi_uc>(staging_buffer_allocation);
		std::memcpy(dest, pixel_data, buffer_size);
		allocator.unmap_memory(staging_buffer_allocation);

		stbi_image_free(pixel_data);

		chosen_format = VK_FORMAT_R8G8B8A8_SRGB;

		VkImageCreateInfo image_create_info = {};
		image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image_create_info.imageType = VK_IMAGE_TYPE_2D;
		image_create_info.format = chosen_format;
		image_create_info.extent.width = image_props.width;
		image_create_info.extent.height = image_props.height;
		image_create_info.extent.depth = 1;
		image_create_info.mipLevels = image_props.mips;
		image_create_info.arrayLayers = image_props.layers;
		image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
		image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
		image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		image_info.allocation = allocator.allocate_image(image_create_info, VMA_MEMORY_USAGE_AUTO_PREFER_HOST, image_info.image);

		Utilities::transition_image_layout(image_info.image, chosen_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &buffer);
		Utilities::copy_buffer_to_image(staging_buffer, image_info, image_props.width, image_props.height, &buffer);
		Utilities::transition_image_layout(
			image_info.image, chosen_format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, &buffer);

		//	Allocator alloc;
		//	allocator.destroy_buffer(staging_buffer, staging_buffer_allocation);

		buffer.add_destruction_callback([staging_buffer_allocation, staging_buffer](
											Allocator& allocator) { allocator.destroy_buffer(staging_buffer, staging_buffer_allocation); });

		Log::info("[Image] invalidated image: {}", image_props.path.value().string());
		create_view();
		create_sampler();
	}

	void Image::invalidate(void* data)
	{
		Allocator allocator("Image");

		VkDeviceSize buffer_size = image_props.width * image_props.height * image_props.channels;

		VkBufferCreateInfo buffer_create_info {};
		buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_create_info.size = buffer_size;
		buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		VkBuffer staging_buffer;
		VmaAllocation staging_buffer_allocation = allocator.allocate_buffer(buffer_create_info, VMA_MEMORY_USAGE_CPU_TO_GPU, staging_buffer);

		auto* dest = allocator.map_memory<stbi_uc>(staging_buffer_allocation);
		std::memcpy(dest, data, buffer_size);
		allocator.unmap_memory(staging_buffer_allocation);

		stbi_image_free(data);

		chosen_format = VK_FORMAT_R8G8B8A8_SRGB;

		VkImageCreateInfo image_create_info = {};
		image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image_create_info.imageType = VK_IMAGE_TYPE_2D;
		image_create_info.format = chosen_format;
		image_create_info.extent.width = image_props.width;
		image_create_info.extent.height = image_props.height;
		image_create_info.extent.depth = 1;
		image_create_info.mipLevels = image_props.mips;
		image_create_info.arrayLayers = image_props.layers;
		image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
		image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
		image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		image_info.allocation = allocator.allocate_image(image_create_info, VMA_MEMORY_USAGE_AUTO_PREFER_HOST, image_info.image);

		Utilities::transition_image_layout(image_info.image, chosen_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		Utilities::copy_buffer_to_image(staging_buffer, image_info, image_props.width, image_props.height);
		Utilities::transition_image_layout(
			image_info.image, chosen_format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		allocator.destroy_buffer(staging_buffer, staging_buffer_allocation);

		create_view();
		create_sampler();
	}

	void Image::create_view()
	{
		// VIEW SAMPLER

		VkImageViewCreateInfo view_info = {};
		view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		view_info.viewType = image_props.layers > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
		view_info.format = chosen_format;
		view_info.flags = 0;
		view_info.subresourceRange = {};
		view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		view_info.subresourceRange.baseMipLevel = 0;
		view_info.subresourceRange.levelCount = image_props.mips;
		view_info.subresourceRange.baseArrayLayer = 0;
		view_info.subresourceRange.layerCount = image_props.layers;
		view_info.image = image_info.image;
		vkCreateImageView(GraphicsContext::the().device(), &view_info, nullptr, &image_info.view);
	}

	void Image::create_sampler()
	{
		VkSamplerCreateInfo sampler_info = {};
		sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		sampler_info.maxAnisotropy = 1.0f;
		sampler_info.magFilter = VK_FILTER_LINEAR;
		sampler_info.minFilter = VK_FILTER_LINEAR;
		sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

		sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		sampler_info.addressModeV = sampler_info.addressModeU;
		sampler_info.addressModeW = sampler_info.addressModeU;
		sampler_info.mipLodBias = 0.0f;
		sampler_info.maxAnisotropy = 1.0f;
		sampler_info.minLod = 0.0f;
		sampler_info.maxLod = 100.0f;
		sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		vk_check(vkCreateSampler(GraphicsContext::the().device(), &sampler_info, nullptr, &image_info.sampler));
	}

	void Image::destroy()
	{
		vkDestroyImageView(GraphicsContext::the().device(), image_info.view, nullptr);
		vkDestroySampler(GraphicsContext::the().device(), image_info.sampler, nullptr);

		Allocator allocator("VulkanImage2D");
		allocator.destroy_image(image_info.image, image_info.allocation);

		destruction(allocator);
		destruction = nullptr;

		if (image_props.path) {
			Log::info("[Image] Destroying image {}.", (*image_props.path).string());
		} else {
			Log::info("[Image] Destroying image with dimensions [{} x {}].", image_props.width, image_props.height);
		}
	}

} // namespace Alabaster
