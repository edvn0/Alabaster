#include "av_pch.hpp"

#include "core/Application.hpp"

#include "core/Clock.hpp"
#include "core/events/ApplicationEvent.hpp"
#include "core/events/Event.hpp"
#include "core/GUILayer.hpp"
#include "core/Input.hpp"
#include "core/Logger.hpp"
#include "core/Timer.hpp"
#include "core/Window.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/Swapchain.hpp"

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

	Application::~Application() { stop(); }

	void Application::stop()
	{
		layer_forward([](auto* l) {
			l->destroy();
			return true;
		});

		Log::info("[Application] Stopping.");

		window->destroy();

		Renderer::shutdown();
	}

	void Application::resize(int w, int h) { window->get_swapchain()->on_resize(w, h); }

	const GUILayer& Application::gui_layer() const { return *static_cast<GUILayer*>(layers.at("ImGuiLayer")); };

	const GUILayer& Application::gui_layer() { return *static_cast<GUILayer*>(layers.at("ImGuiLayer")); };

	void Application::run()
	{
		on_init();

		while (!window->should_close()) {
			window->update();

			Timer<float> on_cpu;

			Renderer::begin();
			{
				Renderer::submit([this, &ts = app_ts] {
					GUILayer::begin();
					layer_forward([&ts](Layer* layer) {
						layer->ui(ts);
						layer->update(ts);
						return true;
					});
					GUILayer::end();
				});
			}
			Renderer::end();

			window->get_swapchain()->begin_frame();
			Renderer::execute();
			window->swap_buffers();

			cpu_time = on_cpu.elapsed();
			float time = Clock::get_ms<float>();
			frame_time = time - last_frametime;
			app_ts = glm::min<float>(frame_time, 0.0333f);
			last_frametime = time;
		}

		on_shutdown();
	}

	double Application::frametime() { return app_ts; }

	void Application::on_shutdown() { window->close(); }

	void Application::on_event(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.dispatch<WindowResizeEvent>([this](WindowResizeEvent& e) { return on_window_change(e); });
		dispatcher.dispatch<WindowMinimizeEvent>([this](WindowMinimizeEvent& e) { return on_window_change(e); });
		dispatcher.dispatch<WindowCloseEvent>([this](WindowCloseEvent& e) { return on_window_change(e); });

		layer_backward([&event](Layer* layer) {
			layer->on_event(event);
			if (event.handled)
				return true;
			return false;
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
