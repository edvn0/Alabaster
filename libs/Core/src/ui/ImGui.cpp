#include "av_pch.hpp"

#include "ui/ImGui.hpp"

#include "graphics/Image.hpp"
#include "graphics/Texture.hpp"

#include <imgui_impl_vulkan.h>

namespace Alabaster::UI {

	static std::unordered_map<VkImageView, VkDescriptorSet> cached_views;

	void image(const Image& image, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1)
	{
		const auto& descriptor_info = image.get_descriptor_info();
		const auto key = descriptor_info.imageView;
		// if (cached_views.contains(key)) {
		//	ImGui::Image(reinterpret_cast<ImU64>(cached_views[key]), size, uv0, uv1, ImVec4(), ImVec4());
		//	return;
		// }

		if (!descriptor_info.imageView)
			return;
		const auto texture_id = ImGui_ImplVulkan_AddTexture(descriptor_info.sampler, descriptor_info.imageView, descriptor_info.imageLayout);
		const auto as_imu64 = reinterpret_cast<ImU64>(texture_id);
		ImGui::Image(as_imu64, size, uv0, uv1);

		// cached_views[descriptor_info.imageView] = texture_id;
	}

	void image(const Alabaster::Texture& image, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1)
	{
		const auto& descriptor_info = image.get_descriptor_info();
		const auto key = descriptor_info.imageView;
		if (cached_views.contains(key)) {
			ImGui::Image(reinterpret_cast<ImU64>(cached_views[key]), size, uv0, uv1, ImVec4(), ImVec4());
			return;
		}

		const auto layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		if (!descriptor_info.imageView)
			return;
		const auto texture_id = ImGui_ImplVulkan_AddTexture(descriptor_info.sampler, descriptor_info.imageView, layout);
		const auto as_imu64 = reinterpret_cast<ImU64>(texture_id);
		ImGui::Image(as_imu64, size, uv0, uv1, ImVec4(1, 1, 1, 1));

		cached_views[descriptor_info.imageView] = texture_id;
	}

	void image(const VkDescriptorImageInfo& image_info, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1)
	{
		const auto key = image_info.imageView;
		if (cached_views.contains(key)) {
			const auto set = cached_views[key];
			ImGui::Image(reinterpret_cast<ImU64>(set), size, uv0, uv1, ImVec4(1, 1, 1, 1));
			return;
		}

		if (!image_info.imageView)
			return;
		const auto texture_id = ImGui_ImplVulkan_AddTexture(image_info.sampler, image_info.imageView, image_info.imageLayout);
		const auto as_imu64 = reinterpret_cast<ImU64>(texture_id);
		ImGui::Image(as_imu64, size, uv0, uv1, ImVec4(1, 1, 1, 1));

		cached_views[image_info.imageView] = texture_id;
	}

	void image(const std::unique_ptr<Image>& img, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1) { image(*img, size, uv0, uv1); }

	void image(const std::shared_ptr<Alabaster::Image>& img, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1)
	{
		image(*img, size, uv0, uv1);
	}
	void image(const std::unique_ptr<Alabaster::Image>& img, const ImVec2& size) { image(*img, size, { 0, 0 }, { 1, 1 }); }

	void image(const std::shared_ptr<Alabaster::Image>& img, const ImVec2& size) { image(*img, size, { 0, 0 }, { 1, 1 }); }

} // namespace Alabaster::UI
