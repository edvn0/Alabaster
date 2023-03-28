
#pragma once

#include "core/Application.hpp"
#include "panels/Panel.hpp"

namespace App {

	class StatisticsPanel : public Panel {
	public:
		explicit StatisticsPanel(const auto& application_stats)
			: statistics(application_stats) {};

		void on_init() override {};
		void on_destroy() override {};
		void on_event(Alabaster::Event&) override {};
		void on_update(float ts) override;
		void ui(float ts) override;
		void register_file_watcher(AssetManager::FileWatcher&) { }

	private:
		const Alabaster::ApplicationStatistics& statistics;

		std::size_t should_update { 0 };
		static constexpr auto update_interval_frames = 15;
	};

} // namespace App
