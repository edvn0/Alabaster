#include "ui/ImGui.hpp"
#include "utilities/FileInputOutput.hpp"

#include <wchar.h>

namespace Alabaster::UI {

	void drag_drop(const std::filesystem::path& path)
	{
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
			const auto relative_path = std::filesystem::relative(path, IO::resources());
			const auto* item_path = relative_path.c_str();
			ImGui::SetDragDropPayload("AlabasterLayer::DragDropPayload", item_path, (wcslen(item_path) + 1) * sizeof(wchar_t));
			ImGui::EndDragDropSource();
		}
	}

	std::optional<std::filesystem::path> accept_drag_drop(const std::string& payload_id)
	{
		std::optional<std::filesystem::path> fp {};
		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payload_id.c_str())) {
				const auto* path = static_cast<const wchar_t*>(payload->Data);
				fp = path;
			}
			ImGui::EndDragDropTarget();
		}
		return fp;
	}

} // namespace Alabaster::UI
