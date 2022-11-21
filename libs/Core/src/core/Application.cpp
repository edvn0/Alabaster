#include "av_pch.hpp"

#include "core/Application.hpp"

#include "core/Clock.hpp"
#include "core/CPUProfiler.hpp"
#include "core/events/ApplicationEvent.hpp"
#include "core/events/Event.hpp"
#include "core/GUILayer.hpp"
#include "core/Input.hpp"
#include "core/Logger.hpp"
#include "core/Timer.hpp"
#include "core/Window.hpp"
#include "graphics/GraphicsContext.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/Swapchain.hpp"

#include <cstddef>

#define ALABASTER_USE_IMGUI

namespace Alabaster {

	static constexpr auto last_fifty = [](const auto& frametime_queue) -> double {
		auto start = frametime_queue.size() - 1;
		auto end = start - 100;

		double average = 0;
		for (std::size_t i = frametime_queue.size() - 1; i >= end; i--) {
			average += frametime_queue[i];
		}

		return average / (start - end);
	};

	static Application* global_app;

	Application& Application::the() { return *global_app; }

	Application::Application(const ApplicationArguments& args)
	{
		assert(global_app == nullptr);

		global_app = this;
		window = std::make_unique<Window>(args);
		window->set_event_callback([this](Event& e) { on_event(e); });

		push_layer(new GUILayer());

		Renderer::init();

		for (auto i = 0; i < 144; i++) {
			frametime_queue[i] = -1;
		}
	}

	Application::~Application()
	{
		if (is_running)
			stop();
	}

	void Application::exit() { is_running = false; }

	void Application::stop()
	{
		using Map = std::map<std::string, Layer*>;
		for (Map::iterator itr = layers.begin(); itr != layers.end();) {
			itr->second->destroy();
			itr->second->~Layer();
			itr = layers.erase(itr);
		}
		Log::info("[Application] Stopping.");

		window->destroy();
	}

	void Application::resize(int w, int h) { window->get_swapchain()->on_resize(w, h); }

	GUILayer& Application::gui_layer()
	{
		if (!ui_layer) {
			ui_layer = static_cast<GUILayer*>(layers.at("GUILayer"));
		}
		return *ui_layer;
	};

	void Application::run()
	{
		on_init();

		static int frametime_index = 0;
		while (!window->should_close() && is_running) {
			Timer<ClockGranularity::MILLIS, float> on_cpu;

			window->update();

			swapchain().begin_frame();
			{
				Renderer::begin();

				update_layers(app_ts);
				gui_layer().begin();
				render_imgui();
				gui_layer().end();

				Renderer::end();
			}
			swapchain().end_frame();
			cpu_time = on_cpu.elapsed();
			frametime_queue[frametime_index] = cpu_time;
			float time = Clock::get_ms<float>();
			frame_time = time - last_frametime;
			app_ts = frame_time;
			last_frametime = time;

			Log::info("[Application] CPU time: \t{}ms", last_fifty(frametime_queue));

			frametime_index = (frametime_index + 1) % frametime_queue.size();
		}

		on_shutdown();
	}

	Swapchain& Application::swapchain() { return *window->get_swapchain(); }

	Swapchain& Application::swapchain() const { return *window->get_swapchain(); }

	void Application::render_imgui()
	{

		for (const auto& [key, layer] : layers) {
			layer->ui(app_ts);
		}
	}

	void Application::update_layers(float ts)
	{
		for (const auto& [key, layer] : layers) {
			layer->update(ts);
		}
	}

	double Application::frametime() { return app_ts; }

	void Application::on_shutdown()
	{
		vkDeviceWaitIdle(GraphicsContext::the().device());
		stop();
	}

	void Application::on_event(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.dispatch<WindowResizeEvent>([this](WindowResizeEvent& e) { return on_window_change(e); });
		dispatcher.dispatch<WindowMinimizeEvent>([this](WindowMinimizeEvent& e) { return on_window_change(e); });
		dispatcher.dispatch<WindowCloseEvent>([this](WindowCloseEvent& e) { return on_window_change(e); });

		for (const auto& [key, layer] : layers) {
			layer->on_event(event);
			if (event.handled)
				return;
		};

		if (event.handled)
			return;
	}

	bool Application::on_window_change(WindowResizeEvent& e)
	{
		const std::uint32_t width = e.width(), height = e.height();
		if (width == 0 || height == 0) {
			return false;
		}

		window->get_swapchain()->on_resize(width, height);

		return false;
	}

	bool Application::on_window_change(WindowMinimizeEvent& event)
	{
		// minimized = e.is_minimized();
		return false;
	}

	bool Application::on_window_change(WindowCloseEvent& e)
	{
		Log::info("[Application] Window close event registered. {}", e.get_name());

		is_running = false;

		return true;
	}

} // namespace Alabaster
