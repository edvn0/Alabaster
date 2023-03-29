#pragma once

#include "core/Common.hpp"
#include "core/Layer.hpp"
#include "core/events/ApplicationEvent.hpp"
#include "graphics/GraphicsContext.hpp"
#include "graphics/Swapchain.hpp"
#include "utilities/StringHash.hpp"

#include <map>
#include <memory>

namespace AssetManager {
	class FileWatcher;
}

namespace Alabaster {

	class Window;
	class GUILayer;

	enum class SyncMode {
		VSync,
		Immediate,
		Mailbox,
	};

	struct ApplicationArguments {
		std::uint32_t width;
		std::uint32_t height;
		std::string name;
		SyncMode sync_mode;
	};

	struct ApplicationStatistics {
		double app_ts { 7.5 };
		float cpu_time;
		float frame_time;
		float last_frametime;
	};

	class Application {
	public:
		void run();
		void exit();

		double frametime();

	public:
		explicit Application(const ApplicationArguments& args);
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

		void pop_layer(const std::string& name)
		{
			verify(layers.contains(name), "Layer map did not contain name " + name + ".");

			auto found_it = layers.find(name);
			verify(found_it != layers.end());

			const auto& [key, layer] = *found_it;
			layer->destroy();
			layers.erase(key);
		}

		static Application& the();
		inline const std::unique_ptr<Window>& get_window() { return window; };
		inline const std::unique_ptr<Window>& get_window() const { return window; }
		Swapchain& swapchain();
		Swapchain& swapchain() const;
		GUILayer& gui_layer();

		const auto& get_file_watcher() const { return *file_watcher; }
		auto& get_file_watcher() { return *file_watcher; }

		const auto& get_statistics() const { return statistics; }

	private:
		bool on_window_change(WindowResizeEvent& event);
		bool on_window_change(WindowMinimizeEvent& event);
		bool on_window_change(WindowCloseEvent& event);
		void render_imgui();
		void update_layers(float ts);
		void update_layers(double ts);

		void stop();

		std::unordered_map<std::string, Layer*, AssetManager::StringHash, std::equal_to<>> layers;
		std::unique_ptr<Window> window;

		std::unique_ptr<AssetManager::FileWatcher> file_watcher;

		ApplicationStatistics statistics {};

		std::array<double, 500> frametime_queue;

		bool is_running { true };
		GUILayer* ui_layer { nullptr };
	};

	Application* create(const ApplicationArguments&);

} // namespace Alabaster
