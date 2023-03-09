#include "av_pch.hpp"

#include "graphics/Texture.hpp"

#include "graphics/GraphicsContext.hpp"
#include "platform/Vulkan/ImageUtilities.hpp"

#include <stb_image.h>

namespace Alabaster {

	Texture::Texture(const std::filesystem::path& tex_path, const TextureProperties props)
		: path(tex_path)
		, properties(props)
	{
		bool loaded = load_image(path.string());
		if (!loaded) {
			throw AlabasterException("Could not load image.");
		}

		ImageSpecification image_spec;
		image_spec.format = format;
		image_spec.width = width;
		image_spec.height = height;
		image_spec.mips = properties.generate_mips ? Texture::get_mip_level_count() : 1;
		image_spec.debug_name = properties.debug_name;
		image = Image::create(image_spec);

		invalidate();
	}

	Texture::Texture(ImageFormat input_format, uint32_t w, uint32_t h, const void* data, const TextureProperties props)
		: width(w)
		, height(h)
		, properties(props)
		, format(input_format)
	{
		if (height == 0) {
			bool loaded = load_image(data, width);
			if (!loaded) {
				throw AlabasterException("Could not load image.");
			}
		} else if (data) {
			auto size = static_cast<std::uint32_t>(Utilities::get_memory_size(format, width, height));
			image_data = Buffer::copy(data, size);
		} else {
			auto size = static_cast<std::uint32_t>(Utilities::get_memory_size(format, width, height));
			image_data.allocate(size);
			image_data.zero_initialise();
		}

		ImageSpecification image_spec;
		image_spec.format = format;
		image_spec.width = width;
		image_spec.height = height;
		image_spec.mips = properties.generate_mips ? Texture::get_mip_level_count() : 1;
		image_spec.debug_name = properties.debug_name;
		if (properties.storage)
			image_spec.usage = ImageUsage::Storage;
		image = Image::create(image_spec);

		invalidate();
	}

	Texture::~Texture()
	{
		if (!destroyed) {
			destroy();
		}
	}

	void Texture::destroy()
	{
		if (image)
			image->release();

		image_data.release();

		destroyed = true;
	}

	bool Texture::load_image(const void* data, uint32_t size)
	{
		int w, h, channels;

		if (stbi_is_hdr_from_memory(static_cast<const stbi_uc*>(data), static_cast<int>(size))) {
			image_data.data
				= (byte*)stbi_loadf_from_memory(static_cast<const stbi_uc*>(data), static_cast<int>(size), &w, &h, &channels, STBI_rgb_alpha);
			image_data.size = width * height * 4 * sizeof(float);
			format = ImageFormat::RGBA32F;
		} else {
			image_data.data = stbi_load_from_memory(static_cast<const stbi_uc*>(data), static_cast<int>(size), &w, &h, &channels, STBI_rgb_alpha);
			image_data.size = width * height * 4;
			format = ImageFormat::RGBA;
		}

		if (!image_data.data)
			return false;

		width = w;
		height = h;
		return true;
	}

	bool Texture::load_image(const std::string& in_path)
	{
		int w, h, channels;

		if (stbi_is_hdr(in_path.c_str())) {
			image_data.data = (byte*)stbi_loadf(path.string().c_str(), &w, &h, &channels, 4);
			image_data.size = w * h * 4 * sizeof(float);
			format = ImageFormat::RGBA32F;
		} else {
			// stbi_set_flip_vertically_on_load(1);
			image_data.data = stbi_load(path.string().c_str(), &w, &h, &channels, 4);
			image_data.size = w * h * 4;
			format = ImageFormat::RGBA;
		}

		if (!image_data.data)
			return false;

		width = w;
		height = h;
		return true;
	}

	void Texture::resize(const glm::uvec2& size) { resize(size.x, size.y); }

	void Texture::resize(const uint32_t w, const uint32_t h)
	{
		width = w;
		height = h;

		invalidate();
	}

	void Texture::invalidate()
	{
		const auto& vulkan_device = GraphicsContext::the().device();

		image->release();
		uint32_t mip_count = properties.generate_mips ? get_mip_level_count() : 1;

		ImageSpecification& image_spec = image->get_specification();
		image_spec.format = format;
		image_spec.width = width;
		image_spec.height = height;
		image_spec.mips = mip_count;
		if (!image_data)
			image_spec.usage = ImageUsage::Storage;

		image->invalidate();

		auto& info = image->get_info();

		if (image_data) {
			VkDeviceSize size = image_data.size;

			Allocator allocator("Texture2D - Staging");

			VkBufferCreateInfo buffer_create_info {};
			buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			buffer_create_info.size = size;
			buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			VkBuffer staging_buffer;
			VmaAllocation staging_buffer_allocation = allocator.allocate_buffer(buffer_create_info, VMA_MEMORY_USAGE_CPU_TO_GPU, staging_buffer);

			uint8_t* dest_data = allocator.map_memory<uint8_t>(staging_buffer_allocation);
			memcpy(dest_data, image_data.data, size);
			allocator.unmap_memory(staging_buffer_allocation);

			ImmediateCommandBuffer immediate_command_buffer { "Texture Transition" };
			immediate_command_buffer.add_destruction_callback(
				[staging_buffer, staging_buffer_allocation](Allocator& alloc) { alloc.destroy_buffer(staging_buffer, staging_buffer_allocation); });

			VkImageSubresourceRange subresource_range = {};
			subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			subresource_range.baseMipLevel = 0;
			subresource_range.levelCount = 1;
			subresource_range.layerCount = 1;

			VkImageMemoryBarrier image_memory_barrier {};
			image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			image_memory_barrier.image = info.image;
			image_memory_barrier.subresourceRange = subresource_range;
			image_memory_barrier.srcAccessMask = 0;
			image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

			vkCmdPipelineBarrier(*immediate_command_buffer, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1,
				&image_memory_barrier);

			VkBufferImageCopy buffer_copy_region = {};
			buffer_copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			buffer_copy_region.imageSubresource.mipLevel = 0;
			buffer_copy_region.imageSubresource.baseArrayLayer = 0;
			buffer_copy_region.imageSubresource.layerCount = 1;
			buffer_copy_region.imageExtent.width = width;
			buffer_copy_region.imageExtent.height = height;
			buffer_copy_region.imageExtent.depth = 1;
			buffer_copy_region.bufferOffset = 0;

			vkCmdCopyBufferToImage(
				*immediate_command_buffer, staging_buffer, info.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &buffer_copy_region);

			if (mip_count > 1) // Mips to generate
			{
				Utilities::insert_image_memory_barrier(*immediate_command_buffer, info.image, VK_ACCESS_TRANSFER_WRITE_BIT,
					VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, subresource_range);
			} else {
				Utilities::insert_image_memory_barrier(*immediate_command_buffer, info.image, VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_SHADER_READ_BIT,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, image->get_descriptor_info().imageLayout, VK_PIPELINE_STAGE_TRANSFER_BIT,
					VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, subresource_range);
			}

		} else {
			ImmediateCommandBuffer immediate_command_buffer { "Texture Image Layout" };

			VkImageSubresourceRange subresource_range = {};
			subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			subresource_range.layerCount = 1;
			subresource_range.levelCount = get_mip_level_count();

			Utilities::set_image_layout(
				*immediate_command_buffer, info.image, VK_IMAGE_LAYOUT_UNDEFINED, image->get_descriptor_info().imageLayout, subresource_range);
		}

		VkSamplerCreateInfo sampler {};
		sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		sampler.maxAnisotropy = 1.0f;
		sampler.magFilter = Utilities::vulkan_sampler_filter(properties.sampler_filter);
		sampler.minFilter = Utilities::vulkan_sampler_filter(properties.sampler_filter);
		sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		sampler.addressModeU = Utilities::vulkan_sampler_wrap(properties.sampler_wrap);
		sampler.addressModeV = Utilities::vulkan_sampler_wrap(properties.sampler_wrap);
		sampler.addressModeW = Utilities::vulkan_sampler_wrap(properties.sampler_wrap);
		sampler.mipLodBias = 0.0f;
		sampler.compareOp = VK_COMPARE_OP_NEVER;
		sampler.minLod = 0.0f;
		sampler.maxLod = static_cast<float>(mip_count);

		sampler.maxAnisotropy = 1.0;
		sampler.anisotropyEnable = VK_FALSE;
		sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

		vk_check(vkCreateSampler(vulkan_device, &sampler, nullptr, &info.sampler));

		if (!properties.storage) {
			VkImageViewCreateInfo view {};
			view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			view.viewType = VK_IMAGE_VIEW_TYPE_2D;
			view.format = Utilities::vulkan_image_format(format);
			view.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
			view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			view.subresourceRange.baseMipLevel = 0;
			view.subresourceRange.baseArrayLayer = 0;
			view.subresourceRange.layerCount = 1;
			view.subresourceRange.levelCount = mip_count;
			view.image = info.image;
			vk_check(vkCreateImageView(vulkan_device, &view, nullptr, &info.view));

			image->update_descriptor();
		}

		if (image_data && properties.generate_mips && mip_count > 1)
			generate_mips();

		stbi_image_free(image_data.data);
		image_data = Buffer();
	}

	Buffer Texture::get_writeable_buffer() { return image_data; }

	const std::filesystem::path& Texture::get_path() const { return path; }

	uint32_t Texture::get_mip_level_count() const { return Utilities::calculate_mip_count(width, height); }

	std::pair<uint32_t, uint32_t> Texture::get_mip_size(uint32_t mip) const
	{
		uint32_t w = width;
		uint32_t h = height;
		while (mip != 0) {
			w /= 2;
			h /= 2;
			mip--;
		}

		return { w, h };
	}

	void Texture::generate_mips()
	{
		const auto& info = image->get_info();

		ImmediateCommandBuffer immediate_command_buffer { "Mip Generation" };

		const auto mip_levels = get_mip_level_count();
		for (uint32_t i = 1; i < mip_levels; i++) {
			VkImageBlit image_blit {};

			// Source
			image_blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			image_blit.srcSubresource.layerCount = 1;
			image_blit.srcSubresource.mipLevel = i - 1;
			image_blit.srcOffsets[1].x = int32_t(width >> (i - 1));
			image_blit.srcOffsets[1].y = int32_t(height >> (i - 1));
			image_blit.srcOffsets[1].z = 1;

			// Destination
			image_blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			image_blit.dstSubresource.layerCount = 1;
			image_blit.dstSubresource.mipLevel = i;
			image_blit.dstOffsets[1].x = int32_t(width >> i);
			image_blit.dstOffsets[1].y = int32_t(height >> i);
			image_blit.dstOffsets[1].z = 1;

			VkImageSubresourceRange mip_sub_range = {};
			mip_sub_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			mip_sub_range.baseMipLevel = i;
			mip_sub_range.levelCount = 1;
			mip_sub_range.layerCount = 1;

			// Prepare current mip level as image blit destination
			Utilities::insert_image_memory_barrier(immediate_command_buffer.get_buffer(), info.image, 0, VK_ACCESS_TRANSFER_WRITE_BIT,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, mip_sub_range);

			// Blit from previous level
			vkCmdBlitImage(immediate_command_buffer.get_buffer(), info.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, info.image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &image_blit, Utilities::vulkan_sampler_filter(properties.sampler_filter));

			// Prepare current mip level as image blit source for next level
			Utilities::insert_image_memory_barrier(immediate_command_buffer.get_buffer(), info.image, VK_ACCESS_TRANSFER_WRITE_BIT,
				VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, mip_sub_range);
		}

		// After the loop, all mip layers are in TRANSFER_SRC layout, so transition all to SHADER_READ
		VkImageSubresourceRange subresource_range = {};
		subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresource_range.layerCount = 1;
		subresource_range.levelCount = mip_levels;

		Utilities::insert_image_memory_barrier(immediate_command_buffer.get_buffer(), info.image, VK_ACCESS_TRANSFER_READ_BIT,
			VK_ACCESS_SHADER_READ_BIT,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, subresource_range);
	}

} // namespace Alabaster
