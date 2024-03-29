#pragma once

#include <string_view>

namespace AssetManager {
	class FileWatcher;
}

namespace Alabaster {

	class Application;
	class Event;

	struct Layer {
		virtual ~Layer() = default;
		virtual auto initialise(AssetManager::FileWatcher& watcher) -> bool = 0;
		virtual auto update(float ts) -> void = 0;
		virtual auto render() -> void = 0;
		virtual auto destroy() -> void = 0;
		virtual void ui() = 0;
		virtual void on_event(Event& event) = 0;

		std::string_view get_name() { return name(); }

	private:
		virtual std::string_view name() = 0;
		friend Application;
	};

} // namespace Alabaster