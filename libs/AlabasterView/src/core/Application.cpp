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

#define ALABASTER_USE_IMGUI

namespace Alabaster {

	static Application* global_app;

	Application& Application::the() { return *global_app; }

	Application::Application(const ApplicationArguments& args)
	{
		assert(global_app == nullptr);

		global_app = this;
		window = std::make_unique<Window>(args);
		push_layer(new GUILayer());

		window->set_event_callback([this](Event& e) { on_event(e); });

		Renderer::init();
	}

	Application::~Application()
	{
		if (is_running)
			stop();
	}

	void Application::exit() { is_running = false; }

	void Application::stop()
	{
		layer_forward([](Layer* l) { l->destroy(); });

		for (const auto& [key, layer] : layers) {
			pop_layer(key);
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

		while (!window->should_close() && is_running) {
			Timer<ClockGranularity::MILLIS, float> on_cpu;

			window->update();

			Renderer::begin();
			update_layers(app_ts);

#ifdef ALABASTER_USE_IMGUI
			Renderer::submit([this] { gui_layer().begin(); }, "Begin ImGui");
			Renderer::submit([this] { render_imgui(); }, "Update Imgui");
			Renderer::submit([this] { gui_layer().end(); }, "End Scene ImGui");
#endif
			Renderer::end();

			swapchain().begin_frame();
			{
				Renderer::execute();
				cpu_time = on_cpu.elapsed();
				float time = Clock::get_ms<float>();
				frame_time = time - last_frametime;
				app_ts = frame_time;
				last_frametime = time;
			}
			swapchain().end_frame();

			Log::info("[Application] CPU time: \t{}ms", cpu_time);
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
		layer_forward([&ts](Layer* layer) { layer->update(ts); });
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

		layer_backward([&event](Layer* layer) {
			layer->on_event(event);
			if (event.handled)
				return;
		});

		if (event.handled)
			return;
	}

	bool Application::on_window_change(WindowResizeEvent& e)
	{
		const uint32_t width = e.width(), height = e.height();
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

	bool Application::on_window_change(WindowCloseEvent& e) { return false; }

} // namespace Alabaster
