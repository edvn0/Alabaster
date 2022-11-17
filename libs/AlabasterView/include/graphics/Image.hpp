//
// Created by Edwin Carlsson on 2022-10-25.
//

#pragma once

#include "utilities/FileInputOutput.hpp"
#include "vulkan/vulkan_core.h"

#include <cstdint>
#include <filesystem>
#include <memory>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace Alabaster {

	struct ImageInfo {
		VkImage image { nullptr };
		VkImageView view { nullptr };
		VkSampler sampler { nullptr };
		VmaAllocation allocation { nullptr };
	};

	enum class ImageFormat : uint8_t {
		RGBA = 0,
		RGB,
		RBG,
		RBGA,
		SINGLE_CHANNEL,
	};

	enum class ImageUsage : uint8_t { Attachment = 0, Texture, Storage };

	struct ImageProps {
		uint32_t width;
		uint32_t height;
		uint32_t mips { 1 };
		uint32_t layers { 1 };
		uint32_t depth { 1 };
		uint32_t channels { 3 };
		ImageFormat format { ImageFormat::RGBA };
	};

	class Image {
	public:
		Image(const std::filesystem::path& path, ImageFormat format = ImageFormat::RBGA);
		void destroy();

		const ImageInfo& operator*() const { return image_info; }

		const VkDescriptorImageInfo vulkan_image_info() const
		{
			VkDescriptorImageInfo info {};
			info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			info.imageView = image_info.view;
			info.sampler = image_info.sampler;
			return info;
		};

	private:
		void invalidate(void* data);

	private:
		ImageProps image_props;
		ImageUsage image_usage;
		ImageInfo image_info;

		bool destroyed { false };

	public:
		static std::unique_ptr<Image> create(const std::filesystem::path& filename) { return std::make_unique<Image>(IO::texture(filename)); }
	};

} // namespace Alabaster
