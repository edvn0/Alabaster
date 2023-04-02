#include "panels/StatisticsPanel.hpp"

#include <imgui.h>
#include <tuple>

namespace App {

	static constexpr auto update_interval_ms = 500.0;

	void approximate_rolling_average(double& current, double new_value, auto index) { current = (current * (index - 1) + new_value) / index; }

	void StatisticsPanel::on_update(float ts)
	{
		static std::size_t index = 1;
		should_update_counter += ts;
		if (should_update_counter > update_interval_ms) {
			should_update_counter = 0;
			const auto& [app_ts, cpu_time, frame_time, last_frametime, mean] = statistics;
			approximate_rolling_average(descriptives[0].value, app_ts, index);
			approximate_rolling_average(descriptives[1].value, cpu_time, index);
			approximate_rolling_average(descriptives[2].value, frame_time, index);
			approximate_rolling_average(descriptives[3].value, last_frametime, index);

			index++;
		}
	}

	void StatisticsPanel::ui(float ts)
	{
		ImGui::Begin("StatisticsPanel", nullptr);
		if (ImGui::BeginTable("StatisticsTable", 1)) {

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
