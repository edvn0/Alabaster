#include "av_pch.hpp"

#include "core/Application.hpp"

#include "core/Clock.hpp"
#include "core/GUILayer.hpp"
#include "core/Input.hpp"
#include "core/Logger.hpp"
#include "core/Timer.hpp"
#include "core/Window.hpp"
#include "core/events/ApplicationEvent.hpp"
#include "core/events/Event.hpp"
#include "filesystem/FileSystem.hpp"
#include "graphics/GraphicsContext.hpp"
#include "graphics/Renderer.hpp"

#include <AssetManager.hpp>

namespace Alabaster {

	static Application* global_app;

	Application& Application::the() { return *global_app; }

	Application::Application(const ApplicationArguments& args)
	{
		assert(global_app == nullptr);

		global_app = this;
		window = std::make_unique<Window>(args);
		window->set_event_callback([this](Event& event) { on_event(event); });

		file_watcher = std::make_unique<AssetManager::FileWatcher>(FileSystem::executable());

		push_layer<GUILayer>();

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
		for (auto itr = layers.begin(); itr != layers.end();) {
			const auto& [name, layer] = *itr;
			Log::warn("[Application] Destroying layer: {}", layer->get_name());

			layer->destroy();
			itr = layers.erase(itr);
		}
		Log::info("[Application] Stopping.");

		window->destroy();
	}

	void Application::resize(int w, int h) { window->get_swapchain().on_resize(w, h); }

	GUILayer& Application::gui_layer()
	{
		if (!ui_layer) {
			ui_layer = static_cast<GUILayer*>(layers.at("GUILayer").get());
		}
		return *ui_layer;
	}

	void Application::run()
	{
		static constexpr double dt = 0.01;
		double accumulator = 0.0;

		on_init();

		last_frametime_ms = Clock::get_ms();
		while (!window->should_close() && is_running) {
			double new_time = Clock::get_ms();
			double frame_time = new_time - last_frametime_ms;
			last_frametime_ms = new_time;
			statistics.frame_time = frame_time;

			accumulator += frame_time;

			Timer<double> layer_update;
			while (accumulator >= dt) {
				update_layers(dt);
				accumulator -= dt;
			}

			window->update();

			const auto updated_timer = layer_update.elapsed();
			statistics.cpu_time = updated_timer;

			swapchain().begin_frame();
			Renderer::begin();
			{
				render_layers();
				render_imgui();
			}
			Renderer::end();
			swapchain().end_frame();
		}

		on_shutdown();
	}

	Swapchain& Application::swapchain() { return window->get_swapchain(); }

	Swapchain& Application::swapchain() const { return window->get_swapchain(); }

	void Application::render_imgui()
	{
		gui_layer().begin();
		for (const auto& [key, layer] : layers) {
			layer->ui();
		}
		gui_layer().end();
	}

	void Application::update_layers(float ts)
	{
		for (const auto& [key, layer] : layers) {
			layer->update(ts);
		}
	}

	void Application::update_layers(double ts)
	{
		auto time_step = static_cast<float>(ts);
		for (const auto& [key, layer] : layers) {
			layer->update(time_step);
		}
	}

	void Application::render_layers()
	{
		for (const auto& [key, layer] : layers) {
			layer->render();
		}
	}

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
			if (event.handled)
				return;

			layer->on_event(event);
		}
	}

	bool Application::on_window_change(WindowResizeEvent& e)
	{
		const auto width = e.width();
		const auto height = e.height();
		if (width == 0 || height == 0) {
			return false;
		}

		auto before_resize = Clock::get_ms();
		window->get_swapchain().on_resize(width, height);
		last_frametime_ms -= Clock::get_ms() - before_resize;

		return false;
	}

	bool Application::on_window_change(WindowMinimizeEvent&)
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
