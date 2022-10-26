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
		uint32_t width;
		uint32_t height;
		const char* name;
	};

	class Application {
		using rev_it = std::map<std::string, Layer*>::reverse_iterator;
		using it = std::map<std::string, Layer*>::iterator;
		using LayerFunction = std::function<void(Layer*)>;
		using LayerTimestepFunction = void(Layer*, float);

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

			(*found_it).second->destroy();
			layers.erase(found_it);
		}

		static Application& the();
		inline const std::unique_ptr<Window>& get_window() { return window; };
		inline const std::unique_ptr<Window>& get_window() const { return window; }
		inline GUILayer& gui_layer();

	private:
		bool on_window_change(WindowResizeEvent& event);
		bool on_window_change(WindowMinimizeEvent& event);
		bool on_window_change(WindowCloseEvent& event);
		void render_imgui(float ts);
		void update_layers(float ts);

	private:
		void stop();

		inline void layer_forward(LayerFunction&& func)
		{
			for (it layer = layers.begin(); layer != layers.end(); ++layer) {
				auto&& [k, l] = *layer;
				func(l);
			}
		}

		inline void layer_backward(LayerFunction&& func)
		{
			for (rev_it layer = layers.rbegin(); layer != layers.rend(); ++layer) {
				auto&& [k, l] = *layer;
				func(l);
			}
		}

		inline void layer_forward(float ts, LayerTimestepFunction&& func)
		{
			for (it layer = layers.begin(); layer != layers.end(); ++layer) {
				auto&& [k, l] = *layer;
				func(l, ts);
			}
		}

		inline void layer_backward(float ts, LayerTimestepFunction&& func)
		{
			for (rev_it layer = layers.rbegin(); layer != layers.rend(); ++layer) {
				auto&& [k, l] = *layer;
				func(l, ts);
			}
		}

		Swapchain& swapchain();
		Swapchain& swapchain() const;

	private:
		std::map<std::string, Layer*> layers;
		std::unique_ptr<Window> window;

		double app_ts { 7.5 };
		float cpu_time;
		float frame_time;
		float last_frametime;

		bool is_running { true };
		GUILayer* ui_layer { nullptr };
	};

	Application* create(const ApplicationArguments&);

} // namespace Alabaster
