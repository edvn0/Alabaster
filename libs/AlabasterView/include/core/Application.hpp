#pragma once

#include <memory>

namespace Alabaster {

	class Window;

	struct ApplicationArguments {
		uint32_t width;
		uint32_t height;
		const char* name;
	};

	struct Layer {
		virtual bool initialise() { return true; };
		virtual void update(float ts) {};
		virtual void destroy() {};
	};

	class Application {
		using rev_it = std::vector<Layer*>::reverse_iterator;

	public:
		Application(const ApplicationArguments& args);
		Application(const Application&) = delete;
		Application(Application&&) = delete;

		inline void push_layer(Layer* layer)
		{
			layer->initialise();
			layers.emplace_back(std::move(layer));
		}

		void run();
		void stop();

		static Application& the();

	private:
		std::vector<Layer*> layers;
		std::unique_ptr<Window> window;
	};

} // namespace Alabaster
