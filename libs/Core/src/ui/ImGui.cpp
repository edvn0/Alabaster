#include "av_pch.hpp"

#include "ui/ImGui.hpp"

#include "graphics/Image.hpp"

#include <imgui_impl_vulkan.h>
#include <unordered_map>

namespace Alabaster::UI {

	static std::unordered_map<VkImageView, VkDescriptorSet> cached_views;

	void image(const std::unique_ptr<Image>& img, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1) { image(*img, size, uv0, uv1); }

	void image(const std::shared_ptr<Image>& img, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1) { image(*img, size, uv0, uv1); }

	void image(const Image& image, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1)
	{
		if (const auto view = image.get_view(); cached_views.contains(view)) {
			ImGui::Image(reinterpret_cast<ImU64>(cached_views[view]), size, uv0, uv1, ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1));
			return;
		}

		const auto layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		if (!image.get_view())
			return;
		const auto texture_id = ImGui_ImplVulkan_AddTexture(image.get_sampler(), image.get_view(), layout);
		const auto as_imu64 = reinterpret_cast<ImU64>(texture_id);
		ImGui::Image(as_imu64, size, uv0, uv1, ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1));

		cached_views[image.get_view()] = texture_id;
	}

} // namespace Alabaster::UI
