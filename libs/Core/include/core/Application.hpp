#pragma once

#include "core/Common.hpp"
#include "core/Layer.hpp"
#include "core/events/ApplicationEvent.hpp"
#include "graphics/GraphicsContext.hpp"
#include "graphics/Swapchain.hpp"
#include "utilities/HashStringView.hpp"

#include <map>
#include <memory>

namespace AssetManager {
	class FileWatcher;
}

namespace Alabaster {

	// clang-format off
	template <typename T>
	concept ConstructibleLayer = requires(T* t, AssetManager::FileWatcher& watcher)
	{
		{
			t->initialise(watcher)
		} -> std::same_as<bool>;
		{
			t->get_name()
		} -> std::same_as<std::string_view>;
		new T();
	};
	// clang-format on

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
		double cpu_time { 0.0f };
		double frame_time { 0.0f };
	};

	class Application {
	public:
		explicit Application(const ApplicationArguments& args);
		void run();
		void exit();

		Application(const Application&) = delete;
		Application(Application&&) = delete;
		void operator=(const Application&) = delete;

		virtual ~Application();

		virtual void on_init() { GraphicsContext::the(); }
		virtual void on_event(Event&);
		virtual void on_shutdown();

		void resize(int w, int h);

		template <ConstructibleLayer L> void push_layer()
		{
			auto layer = std::make_unique<L>();
			layer->initialise(*file_watcher);
			layers.emplace(layer->get_name(), std::move(layer));
		}

		void pop_layer(const std::string& name)
		{
			verify(layers.contains(name), "Layer map did not contain name " + name + ".");

			auto&& layer = std::move(layers.at(name));
			layer->destroy();
		}

		static Application& the();
		inline Window& get_window() { return *window; };
		inline Window& get_window() const { return *window; }
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
		void render_layers();
		void update_layers(float ts);
		void update_layers(double ts);

		void stop();

		std::unordered_map<std::string, std::unique_ptr<Layer>, HashStringView, std::equal_to<>> layers;
		std::unique_ptr<Window> window;

		std::unique_ptr<AssetManager::FileWatcher> file_watcher;

		ApplicationStatistics statistics {};
		double last_frametime_ms { 0 };
		bool is_running { true };
		GUILayer* ui_layer { nullptr };
	};

	Application* create(const ApplicationArguments&);

} // namespace Alabaster
