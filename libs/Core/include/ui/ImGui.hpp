#pragma once

#include <imgui.h>
#include <memory>
#include <vulkan/vulkan.h>

namespace Alabaster {
	class Image;
}

namespace Alabaster::UI {

	void image(const std::unique_ptr<Alabaster::Image>&, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1));
	void image(const std::shared_ptr<Alabaster::Image>&, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1));
	void image(const Alabaster::Image&, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1));

} // namespace Alabaster::UI
