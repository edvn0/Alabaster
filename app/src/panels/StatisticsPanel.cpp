#include "panels/StatisticsPanel.hpp"

#include <imgui.h>
#include <tuple>

namespace App {

	void StatisticsPanel::on_update(float ts) { }

	void StatisticsPanel::ui(float ts)
	{
		ImGui::Begin("StatisticsPanel", nullptr);
		if (ImGui::BeginTable("StatisticsTable", 1)) {
			const auto& [app_ts, cpu_time, frame_time, last_frametime] = statistics;
			std::array<std::tuple<const char*, float>, 4> descriptives { std::tuple<const char*, float> {
																			 "Application Timestep", static_cast<float>(app_ts) },
				{ "CPU Time", cpu_time }, { "Frametime", frame_time }, { "Last Frametime", last_frametime } };

			ImGui::TableNextColumn();
			for (const auto& [name, value] : descriptives) {
				ImGui::Text("%s: %f", name, value);
				ImGui::TableNextColumn();
			}
			ImGui::EndTable();
		}
		ImGui::End();
	}

} // namespace App