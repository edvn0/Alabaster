#include "av_pch.hpp"

#include "ui/ImGui.hpp"

#include <imgui_impl_vulkan.h>

namespace Alabaster::UI {

	void image(const VulkanImage& vk_image, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1)
	{
		if (!vk_image.view)
			return;
		const auto texture_id = ImGui_ImplVulkan_AddTexture(vk_image.sampler, vk_image.view, vk_image.layout);
		const auto as_imu64 = reinterpret_cast<ImU64>(texture_id);
		ImGui::Image(as_imu64, size, uv0, uv1, ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1));
	}

} // namespace Alabaster::UI
