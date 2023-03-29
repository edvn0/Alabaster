//
// Created by Edwin Carlsson on 2022-12-17.
//

#pragma once

#include <memory>

namespace Alabaster {
	class Event;
}

namespace AssetManager {
	class FileWatcher;
}

namespace App {

	class Panel {
	public:
		virtual ~Panel() = default;
		virtual void on_update(float ts) = 0;
		virtual void ui(float ts) = 0;
		virtual void on_event(Alabaster::Event&) = 0;
		virtual void on_init() = 0;
		virtual void on_destroy() = 0;
		virtual void register_file_watcher(AssetManager::FileWatcher&) = 0;
	};

} // namespace App