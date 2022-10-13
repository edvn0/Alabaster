#include "av_pch.hpp"

#include "core/Application.hpp"

#include "core/Clock.hpp"
#include "core/GUILayer.hpp"
#include "core/Input.hpp"
#include "core/Logger.hpp"
#include "core/Window.hpp"
#include "graphics/Allocator.hpp"
#include "graphics/GraphicsContext.hpp"
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

		auto shader = Shader(std::filesystem::path { "app/resources/shaders/main" });
		shader.destroy();
	}

	Application::~Application() { stop(); }

	void Application::stop()
	{
		layer_forward([](auto* l) { l->destroy(); });

		Log::info("[Application] Calling the destructor of the Application.");

		window->destroy();
	}

	void Application::resize(int w, int h) { window->get_swapchain()->on_resize(w, h); }

	const GUILayer& Application::gui_layer() const { return *static_cast<GUILayer*>(layers.at("ImGuiLayer")); };

	const GUILayer& Application::gui_layer() { return *static_cast<GUILayer*>(layers.at("ImGuiLayer")); };

	void Application::run()
	{
		on_init();

		auto time = Clock::get_ms<double>();
		static size_t frame_count = 1;
		static double frametime_counter { 0.0 };
		while (!window->should_close()) {
			if (Input::key(Key::Escape) || Input::key(Key::Q)) {
				window->close();
				continue;
			}

			window->update();
			double current_time = Clock::get_ms<double>();

			auto ts = current_time - time;
			frametime_counter += ts;

			window->get_swapchain()->begin_frame();

			GUILayer::begin();
			layer_backward([ts](Layer* layer) { layer->ui(ts); });
			GUILayer::end();

			layer_backward([ts](Layer* layer) { layer->update(ts); });

			time = current_time;
			frame_count++;

			if (frame_count % 50 == 0) {
				app_ts = frametime_counter / 50;
				frametime_counter = 0;
			}

			window->swap_buffers();
		}

		vkDeviceWaitIdle(GraphicsContext::the().device());
	}

	double Application::frametime() { return app_ts; }

} // namespace Alabaster
