#include "av_pch.hpp"

#include "ui/ImGui.hpp"

#include "graphics/Image.hpp"
#include "graphics/Texture.hpp"

#include <imgui_impl_vulkan.h>

namespace Alabaster::UI {

	static std::unordered_map<VkImageView, VkDescriptorSet> cached_views;

	void image(const VkDescriptorImageInfo& image_info, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1)
	{
		const auto key = image_info.imageView;
		if (cached_views.contains(key)) {
			const auto set = cached_views[key];
			ImGui::Image(reinterpret_cast<ImU64>(set), size, uv0, uv1, ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1));
			return;
		}

		if (!image_info.imageView)
			return;
		const auto texture_id = ImGui_ImplVulkan_AddTexture(image_info.sampler, image_info.imageView, image_info.imageLayout);
		const auto as_imu64 = reinterpret_cast<ImU64>(texture_id);
		ImGui::Image(as_imu64, size, uv0, uv1, ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1));

		cached_views[image_info.imageView] = texture_id;
	}

	void image(const std::unique_ptr<Image>& img, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1) { image(*img, size, uv0, uv1); }

	void image(const Image& image, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1)
	{
		const auto key = image.get_view();
		if (const auto set = cached_views[key]; cached_views.contains(key)) {
			ImGui::Image(reinterpret_cast<ImU64>(set), size, uv0, uv1, ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1));
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

	void image(const std::shared_ptr<Alabaster::Image>& img, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1)
	{
		image(*img, size, uv0, uv1);
	}
	void image(const std::unique_ptr<Alabaster::Image>& img, const ImVec2& size) { image(*img, size, { 0, 0 }, { 1, 1 }); }

	void image(const std::shared_ptr<Alabaster::Image>& img, const ImVec2& size) { image(*img, size, { 0, 0 }, { 1, 1 }); }

} // namespace Alabaster::UI
