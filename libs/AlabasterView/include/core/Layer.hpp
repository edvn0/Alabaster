#pragma once

#include <string_view>

namespace Alabaster {

	class Application;
	class Event;

	struct Layer {
		virtual ~Layer() = default;
		virtual auto initialise() -> bool { return true; };
		virtual auto update(float ts) -> void {};
		virtual auto destroy() -> void {};
		virtual void ui(float ts) {};
		virtual bool on_event(Event& event) { return true; };

		std::string_view get_name() { return name(); }

	private:
		virtual std::string_view name() = 0;
		friend Application;
	};

} // namespace Alabaster