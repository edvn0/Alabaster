#include "ui/ImGui.hpp"
#include "utilities/FileInputOutput.hpp"

#include <wchar.h>

namespace Alabaster::UI {

	void drag_drop(const std::filesystem::path& path)
	{
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
			auto relative_path = std::filesystem::relative(path, IO::resources());
			const wchar_t* item_path = relative_path.c_str();
			ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", item_path, (wcslen(item_path) + 1) * sizeof(wchar_t));
			ImGui::EndDragDropSource();
		}
	}

} // namespace Alabaster::UI
