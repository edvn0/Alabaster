#include "av_pch.hpp"

#include "core/Application.hpp"

#include "core/Clock.hpp"
#include "core/GUILayer.hpp"
#include "core/Logger.hpp"
#include "core/Window.hpp"
#include "graphics/GraphicsContext.hpp"

namespace Alabaster {

	static Application* global_app;

	Application& Application::the() { return *global_app; }

	Application::Application(const ApplicationArguments& args)
	{
		assert(global_app == nullptr);
		global_app = this;
		Logger::init();
		window = std::make_unique<Window>(args);
		push_layer(new GUILayer());
	}

	Application::~Application() { stop(); }

	void Application::stop() { Logger::shutdown(); }

	void Application::run()
	{
		layer_forward([](Layer* layer) { layer->initialise(); });

		double time = Clock::get_ms<double>();
		static size_t frame_count = 1;
		while (!window->should_close()) {
			window->update();
			double current_time = Clock::get_ms<double>();

			auto ts = current_time - time;
			layer_backward([&ts](Layer* layer) { layer->update(ts); });

			time = current_time;
			frame_count++;
		}
	}
} // namespace Alabaster
