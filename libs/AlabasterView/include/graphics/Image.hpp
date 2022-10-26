//
// Created by Edwin Carlsson on 2022-10-25.
//

#pragma once

#include <cstdint>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace Alabaster {

	struct ImageInfo {
		VkImage image = nullptr;
		VkImageView view = nullptr;
		VkSampler sampler = nullptr;
		VmaAllocation allocation = nullptr;
	};

	enum class ImageFormat : uint8_t {
		RGBA = 0,
		RGB,
		RBG,
		RBGA,
		SINGLE_CHANNEL,
	};

	struct ImageProps {
		uint32_t width;
		uint32_t height;
		uint32_t depth { 1 };
		ImageFormat format { ImageFormat::RGBA };
	};

	class Image {
	public:
		explicit Image(const ImageProps& props);
		Image(const void* data, size_t size);

	private:
		ImageProps image_props;
		ImageInfo image_info;
	};

	class Image2D : public Image {

	public:
		explicit Image2D(const ImageProps& props);
		Image2D(const void* data, size_t size);
	};

} // namespace Alabaster