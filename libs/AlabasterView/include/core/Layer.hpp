#pragma once

#include <string_view>

namespace Alabaster {

	class Application;

	struct Layer {
		virtual auto initialise() -> bool { return true; };
		virtual auto update(float ts) -> void {};
		virtual auto destroy() -> void {};

	private:
		virtual std::string_view name() = 0;
		friend Application;
	};

} // namespace Alabaster