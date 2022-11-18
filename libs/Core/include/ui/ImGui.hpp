#pragma once

#include <imgui.h>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace Alabaster::UI {

	struct VulkanImage {
		VkImage image;
		VkSampler sampler;
		VkImageView view;
		VkImageLayout layout;
	};

	void image(const VulkanImage&, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1));

} // namespace Alabaster::UI
