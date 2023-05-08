#include "panels/StatisticsPanel.hpp"

#include <imgui.h>
#include <tuple>

namespace App {

	static constexpr auto update_interval_ms = 30.0;

	void StatisticsPanel::on_update(float ts)
	{
		should_update_counter += ts;
		if (should_update_counter > update_interval_ms) {
			should_update_counter = 0;
			const auto& [cpu_time, frame_time] = statistics;
			cpu_time_average(cpu_time);
			frame_time_average(frame_time);
		}
	}

	void StatisticsPanel::ui()
	{
		ImGui::Begin("StatisticsPanel", nullptr);
		if (ImGui::BeginTable("StatisticsTable", 1)) {
			ImGui::TableNextColumn();
			ImGui::Text("%s: %fms", "Frametime", double(frame_time_average));
			ImGui::TableNextColumn();
			ImGui::Text("%s: %fms", "CPU Time", double(cpu_time_average));
			ImGui::TableNextColumn();
			ImGui::Text("%s: %fms", "FPS", 1000.0 * frame_time_average.inverse());
			ImGui::TableNextColumn();
			ImGui::EndTable();
		}
		ImGui::End();
	}

} // namespace App
