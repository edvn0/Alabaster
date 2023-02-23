#include "ui/ImGui.hpp"

namespace Alabaster::UI {

	void drag_drop(const std::filesystem::path& path)
	{
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
			auto relative_path = std::filesystem::relative(path, Alabaster::IO::resources());
			const char* item_path = relative_path.c_str();
			const auto size = (std::strlen(item_path) + 1) * sizeof(char);
			ImGui::SetDragDropPayload("AlabasterLayer::DragDropPayload", item_path, size);
			ImGui::EndDragDropSource();
		}
	}

} // namespace Alabaster::UI
