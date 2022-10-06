#include "av_pch.hpp"

#include "core/Application.hpp"

#include "core/Clock.hpp"
#include "core/Window.hpp"

namespace Alabaster {

	Application::Application(const ApplicationArguments& args) { window = std::make_unique<Window>(args); }

	void Application::run()
	{
		for (auto* layer : layers) {
			layer->initialise();
		}

		double time = Clock::get_ms<double>();
		static size_t frame_count = 1;
		double mean_frametime = 144.0;
		while (!window->should_close()) {
			window->update();
			double current_time = Clock::get_ms<double>();

			auto ts = current_time - time;
			for (rev_it layer = layers.rbegin(); layer != layers.rend(); ++layer) {
				(*layer)->update(ts);
			}

			time = current_time;
			frame_count++;
			mean_frametime += ts;
		}
	}

} // namespace Alabaster
