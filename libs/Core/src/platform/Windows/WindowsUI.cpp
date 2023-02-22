#include "ui/ImGui.hpp"
#include "utilities/FileInputOutput.hpp"

#include <wchar.h>

namespace Alabaster::UI {

	void drag_drop(const std::filesystem::path& path)
	{
        #ifdef ALABASTER_WINDOWS
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
			auto relative_path = std::filesystem::relative(path, IO::resources());
			const auto* item_path = relative_path.c_str();
			ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", item_path, (wcslen(item_path) + 1) * sizeof(wchar_t));
			ImGui::EndDragDropSource();
		}
        #else
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
			auto relative_path = std::filesystem::relative(path, Alabaster::IO::resources());
			const char* item_path = relative_path.c_str();
			const auto size = (std::strlen(item_path) + 1) * sizeof(char);
			ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", item_path, size);
			ImGui::EndDragDropSource();
		}
        #endif
	}

} // namespace Alabaster::UI
