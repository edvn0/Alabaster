#include "av_pch.hpp"

#include "core/Application.hpp"

#include "core/Clock.hpp"
#include "core/GUILayer.hpp"
#include "core/Input.hpp"
#include "core/Logger.hpp"
#include "core/Timer.hpp"
#include "core/Window.hpp"
#include "graphics/Allocator.hpp"
#include "graphics/GraphicsContext.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/Shader.hpp"
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
	}

	Application::~Application() { stop(); }

	void Application::stop()
	{
		layer_forward([](auto* l) { l->destroy(); });

		Log::info("[Application] Stopping.");

		window->destroy();
	}

	void Application::resize(int w, int h) { window->get_swapchain()->on_resize(w, h); }

	const GUILayer& Application::gui_layer() const { return *static_cast<GUILayer*>(layers.at("ImGuiLayer")); };

	const GUILayer& Application::gui_layer() { return *static_cast<GUILayer*>(layers.at("ImGuiLayer")); };

	void Application::run()
	{
		on_init();

		static size_t frame_count = 1;
		static double frametime_counter { 0.0 };
		while (!window->should_close()) {
			window->update();
			if (Input::key(Key::Escape) || Input::key(Key::Q)) {
				window->close();
				continue;
			}

			Timer<float> on_cpu;

			Renderer::begin();
			{
				Renderer::submit(&GUILayer::begin);
				Renderer::submit([this, &ts = app_ts] { layer_backward([&ts](Layer* layer) { layer->ui(ts); }); });
				Renderer::submit(&GUILayer::end);
				layer_backward([ts = app_ts](Layer* layer) { layer->update(ts); });
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
	}

	double Application::frametime() { return app_ts; }

} // namespace Alabaster
