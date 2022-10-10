#include "av_pch.hpp"

#include "core/Application.hpp"

#include "core/Clock.hpp"
#include "core/GUILayer.hpp"
#include "core/Input.hpp"
#include "core/Logger.hpp"
#include "core/Window.hpp"
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
		auto shader = Shader(std::filesystem::path { "app/resources/shaders/main" });
		push_layer(new GUILayer());
	}

	Application::~Application() { stop(); }

	void Application::stop()
	{
		layer_forward([](auto* l) { l->destroy(); });

		window->destroy();
		GraphicsContext::the().destroy();
	}

	void Application::run()
	{
		on_init();

		layer_forward([](Layer* layer) {
			if (layer->name() != "ImGuiLayer")
				layer->initialise();
		});

		double time = Clock::get_ms<double>();
		static size_t frame_count = 1;
		while (!window->should_close()) {
			window->update();
			double current_time = Clock::get_ms<double>();

			auto ts = current_time - time;
			layer_backward([&ts](Layer* layer) { layer->update(ts); });

			window->get_swapchain()->begin_frame();
			GUILayer::begin();
			layer_backward([&ts](Layer* layer) { layer->ui(ts); });
			GUILayer::end();

			time = current_time;
			frame_count++;

			if (Input::key(Key::Escape)) {
				Log::info("Exiting");
				break;
			}
			window->swap_buffers();
		}
	}

} // namespace Alabaster
