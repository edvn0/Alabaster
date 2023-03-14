#pragma once

#include "core/events/MouseEvent.hpp"
#include "graphics/Image.hpp"
#include "graphics/Texture.hpp"

#include <imgui.h>
#include <vulkan/vulkan.h>

namespace Alabaster::UI {

	void image(const VkDescriptorImageInfo&, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1));
	void image(const Alabaster::Image&, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1));
	void image(const Alabaster::Texture&, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1));
	void image(const std::shared_ptr<Alabaster::Image>&, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1));
	void image(const std::shared_ptr<Alabaster::Image>&, const ImVec2& size);

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

	void remove_image(const VkDescriptorImageInfo& info);

} // namespace Alabaster::UI
