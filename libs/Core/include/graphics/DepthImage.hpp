#pragma once

#include "graphics/Allocator.hpp"

#include <vulkan/vulkan.h>

typedef struct VmaAllocation_T* VmaAllocation;

namespace Alabaster {

	struct DepthImage {
		VkImage image;
		VkImageView view;
		VmaAllocation allocation;

		void destroy(auto& device)
		{
			vkDestroyImageView(device, view, nullptr);
			Allocator allocator("Depth image destruction");
			allocator.destroy_image(image, allocation);
		}
	};

} // namespace Alabaster
