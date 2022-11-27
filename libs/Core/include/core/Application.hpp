#pragma once

#include "core/Common.hpp"
#include "core/events/ApplicationEvent.hpp"
#include "core/Layer.hpp"
#include "graphics/GraphicsContext.hpp"
#include "graphics/Swapchain.hpp"

#include <map>
#include <memory>

namespace Alabaster {

	class Window;
	class GUILayer;

	struct ApplicationArguments {
		std::uint32_t width;
		std::uint32_t height;
		std::string name;
	};

	class Application {
	public:
		void run();
		void exit();

		double frametime();

	public:
		Application(const ApplicationArguments& args);
		Application(const Application&) = delete;
		Application(Application&&) = delete;
		void operator=(const Application&) = delete;

		virtual ~Application();

		virtual void on_init() { GraphicsContext::the(); }
		virtual void on_event(Event&);
		virtual void on_shutdown();

		void resize(int w, int h);

		void push_layer(Layer* layer)
		{
			layer->initialise();
			layers.emplace(layer->name(), std::move(layer));
		}

		void pop_layer(std::string name)
		{
			verify(layers.contains(name), "Layer map did not contain name " + name + ".");

			auto found_it = layers.find(name);
			verify(found_it != layers.end());

			auto& [key, layer] = *found_it;
			layer->destroy();
			layers.erase(key);
		}

		static Application& the();
		inline const std::unique_ptr<Window>& get_window() { return window; };
		inline const std::unique_ptr<Window>& get_window() const { return window; }
		Swapchain& swapchain();
		Swapchain& swapchain() const;
		inline GUILayer& gui_layer();

	private:
		bool on_window_change(WindowResizeEvent& event);
		bool on_window_change(WindowMinimizeEvent& event);
		bool on_window_change(WindowCloseEvent& event);
		void render_imgui();
		void update_layers(float ts);
		void update_layers(double ts);

	private:
		void stop();

	private:
		std::map<std::string, Layer*> layers;
		std::unique_ptr<Window> window;

		double app_ts { 7.5 };
		float cpu_time;
		float frame_time;
		float last_frametime;

		std::array<double, 500> frametime_queue;

		bool is_running { true };
		GUILayer* ui_layer { nullptr };
	};

	Application* create(const ApplicationArguments&);

} // namespace Alabaster
