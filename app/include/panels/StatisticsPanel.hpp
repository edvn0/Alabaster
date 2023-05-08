
#pragma once

#include "core/Application.hpp"
#include "panels/Panel.hpp"

namespace App {

	template <typename T>
	concept IsNumber = std::is_floating_point_v<T> || std::is_integral_v<T>;

	template <IsNumber T, IsNumber Total, std::size_t N> class MovingAverage {
	public:
		MovingAverage& operator()(T sample)
		{
			total += sample;
			if (num_samples < N)
				samples[num_samples++] = sample;
			else {
				T& oldest = samples[num_samples++ % N];
				total -= oldest;
				oldest = sample;
			}
			return *this;
		}

		operator double() const { return total / std::min(num_samples, N); }
		operator float() const { return static_cast<float>(total) / std::min(num_samples, N); }

		double inverse() const { return std::min(num_samples, N) / total; }

	private:
		T samples[N];
		size_t num_samples { 0 };
		Total total { 0 };
	};

	struct Descriptive {
		const char* name;
		double value;
	};

	class StatisticsPanel : public Panel {
	public:
		explicit StatisticsPanel(const Alabaster::ApplicationStatistics& application_stats)
			: statistics(application_stats)
		{
			descriptives = { Descriptive { "CPU Time", 1.8 }, Descriptive { "FT", 9.3 } };
		};

		void initialise(AssetManager::FileWatcher&) override {};
		void on_destroy() override {};
		void on_event(Alabaster::Event&) override {};
		void on_update(float ts) override;
		void ui() override;
		void register_file_watcher(AssetManager::FileWatcher&) { }

	private:
		const Alabaster::ApplicationStatistics& statistics;
		std::array<Descriptive, 2> descriptives {};

		// 144fps, keep for 6 frames, update every 30th frame.
		MovingAverage<double, double, (144 * 6) / 30> cpu_time_average;
		MovingAverage<double, double, (144 * 6) / 30> frame_time_average;

		double should_update_counter { 0.0 };
	};

} // namespace App
