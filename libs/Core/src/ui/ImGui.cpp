#include "av_pch.hpp"

#include "ui/ImGui.hpp"

#include "codes/MouseCode.hpp"
#include "graphics/GraphicsContext.hpp"
#include "graphics/Image.hpp"
#include "graphics/Texture.hpp"

#include <imgui_impl_vulkan.h>
#include <vulkan/vulkan.h>

namespace Alabaster::UI {

	template <typename T> static constexpr auto reinterpret_as(auto in)
	{
#ifndef ALABASTER_MACOS
		return std::bit_cast<T>(in);
#else
		return reinterpret_cast<T>(in);
#endif
	}

	static std::unordered_map<VkImageView, VkDescriptorSet> cached_views;

	void image(const VkDescriptorImageInfo& image_info, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1)
	{
		const auto& [sampler, image_view, layout] = image_info;
		if (cached_views.contains(image_view)) {
			const auto set = cached_views[image_view];
			ImGui::Image(reinterpret_as<ImU64>(set), size, uv0, uv1);
			return;
		}

		if (!image_view)
			return;
		const auto texture_id = ImGui_ImplVulkan_AddTexture(sampler, image_view, layout);
		ImGui::Image(reinterpret_as<ImU64>(texture_id), size, uv0, uv1);

		cached_views[image_view] = texture_id;
	}

	void image(const Image& img, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1)
	{
		const auto& descriptor_info = img.get_descriptor_info();
		image(descriptor_info, size, uv0, uv1);
	}

	void image(const Alabaster::Texture& img, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1)
	{
		const auto& descriptor_info = img.get_descriptor_info();
		image(descriptor_info, size, uv0, uv1);
	}

	void image(const std::shared_ptr<Alabaster::Image>& img, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1)
	{
		image(*img, size, uv0, uv1);
	}

	void image(const std::shared_ptr<Alabaster::Image>& img, const ImVec2& size) { image(*img, size, { 0, 0 }, { 1, 1 }); }

	bool image_button(const VkDescriptorImageInfo& image_info, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1)
	{
		const auto& [sampler, image_view, layout] = image_info;
		if (cached_views.contains(image_view)) {
			const auto set = cached_views[image_view];
			return ImGui::ImageButton(reinterpret_as<ImU64>(set), size, uv0, uv1);
		}

		if (!image_view)
			return false;
		const auto texture_id = ImGui_ImplVulkan_AddTexture(sampler, image_view, layout);
		cached_views[image_view] = texture_id;
		return ImGui::ImageButton(reinterpret_as<ImU64>(texture_id), size, uv0, uv1);
	}

	bool image_button(const std::shared_ptr<Alabaster::Image>& img, float square_size)
	{
		const auto& descriptor_info = img->get_descriptor_info();
		return image_button(descriptor_info, ImVec2(square_size, square_size), { 0, 0 }, { 1, 1 });
	}

	bool image_button(const std::shared_ptr<Alabaster::Image>& img, std::uint32_t square_size)
	{
		return image_button(img, static_cast<float>(square_size));
	};

	bool image_button(const std::shared_ptr<Alabaster::Texture>& tex, float square_size)
	{
		const auto& descriptor_info = tex->get_descriptor_info();
		return image_button(descriptor_info, ImVec2(square_size, square_size), { 0, 0 }, { 1, 1 });
	}

	bool image_button(const std::shared_ptr<Alabaster::Texture>& img, std::uint32_t square_size)
	{
		return image_button(img, static_cast<float>(square_size));
	}

	void empty_cache()
	{
		for (auto it = cached_views.begin(); it != cached_views.end();) {
			auto& entry = *it;
			auto& [key, value] = entry;
			ImGui_ImplVulkan_RemoveTexture(value);
			vkDestroyImageView(GraphicsContext::the().device(), key, nullptr);
			it = cached_views.erase(it);
		}
	}

	bool is_mouse_double_clicked(MouseCode code)
	{
		if (code == Mouse::Left) {
			return ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);
		}
		if (code == Mouse::Right) {
			return ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Right);
		}
		return false;
	}

	bool is_item_hovered() { return ImGui::IsItemHovered(); }

	void remove_image(const VkDescriptorImageInfo& info)
	{
		if (cached_views.contains(info.imageView)) {
			ImGui_ImplVulkan_RemoveTexture(cached_views[info.imageView]);
			cached_views.erase(info.imageView);
		}
	}

} // namespace Alabaster::UI
