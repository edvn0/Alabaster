#pragma once

#include "core/Layer.hpp"

#include <map>
#include <memory>

namespace Alabaster {

	class Window;

	struct ApplicationArguments {
		uint32_t width;
		uint32_t height;
		const char* name;
	};

	class Application {
		using rev_it = std::map<std::string, Layer*>::reverse_iterator;
		using it = std::map<std::string, Layer*>::iterator;
		using LayerFunction = std::function<void(Layer*)>;

	public:
		void run();
		void stop();

	public:
		Application(const ApplicationArguments& args);
		Application(const Application&) = delete;
		Application(Application&&) = delete;
		void operator=(const Application&) = delete;

		virtual ~Application();

		virtual void on_init() {};

		inline void push_layer(Layer* layer)
		{
			layer->initialise();
			layers.emplace(layer->name(), std::move(layer));
		}

		static Application& the();

		inline const std::unique_ptr<Window>& get_window() { return window; };
		inline const std::unique_ptr<Window>& get_window() const { return window; }

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

	private:
		std::map<std::string, Layer*> layers;
		std::unique_ptr<Window> window;
	};

	Application* create(const ApplicationArguments&);

} // namespace Alabaster
