#pragma once

#include "core/events/MouseEvent.hpp"
#include "graphics/Image.hpp"
#include "graphics/Texture.hpp"

#include <imgui.h>
#include <optional>
#include <vulkan/vulkan.h>

namespace Alabaster::UI {

	void image(const VkDescriptorImageInfo&, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1));
	void image(const Alabaster::Image&, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1));
	void image(const Alabaster::Texture&, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1));
	void image(const std::shared_ptr<Alabaster::Image>&, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1));
	void image(const std::shared_ptr<Alabaster::Image>&, const ImVec2& size);

	bool image_button(const VkDescriptorImageInfo& image_info, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1);
	bool image_button(const std::shared_ptr<Alabaster::Image>&, float square_size);
	bool image_button(const std::shared_ptr<Alabaster::Image>& img, std::uint32_t square_size);
    bool image_button(const std::shared_ptr<Alabaster::Texture>&, float square_size);
	bool image_button(const std::shared_ptr<Alabaster::Texture>& img, std::uint32_t square_size);

	bool is_item_hovered();
	bool is_mouse_double_clicked(Alabaster::MouseCode code = Mouse::Left);
	void drag_drop(const std::filesystem::path& path);
	void empty_cache();
	void handle_double_click(auto&& handler)
	{
		if (is_item_hovered() && is_mouse_double_clicked(Mouse::Left)) {
			handler();
		}
	}
	std::optional<std::filesystem::path> accept_drag_drop(const std::string& payload_identifier);

	void remove_image(const VkDescriptorImageInfo& info);

} // namespace Alabaster::UI
