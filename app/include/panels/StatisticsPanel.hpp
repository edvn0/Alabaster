
#pragma once

#include "core/Application.hpp"
#include "panels/Panel.hpp"

namespace App {

	struct Descriptive {
		const char* name;
		double value;
	};

	class StatisticsPanel : public Panel {
	public:
		explicit StatisticsPanel(const auto& application_stats)
			: statistics(application_stats)
		{
			descriptives = { Descriptive { "App Timestep", 0 }, { "CPU Time", 0 }, { "FT", 0 }, { "Last FT", 0 } };
		};

		void initialise(AssetManager::FileWatcher&) override {};
		void on_destroy() override {};
		void on_event(Alabaster::Event&) override {};
		void on_update(float ts) override;
		void ui(float ts) override;
		void register_file_watcher(AssetManager::FileWatcher&) { }

	private:
		const Alabaster::ApplicationStatistics& statistics;
		std::array<Descriptive, 4> descriptives {};
		double should_update_counter { 0.0 };
	};

} // namespace App
