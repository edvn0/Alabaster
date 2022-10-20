#pragma once

#include "codes/KeyCode.hpp"
#include "codes/MouseCode.hpp"

#include <glm/glm.hpp>

namespace Alabaster {

	class Input {
	public:
		static bool key(KeyCode key);
		static bool mouse(MouseCode key);

		static glm::vec2 mouse_position();

		template <typename... KeyCodes> static bool any(KeyCodes&&... codes) { return (key(codes) || ...); }
		template <typename... KeyCodes> static bool all(KeyCodes&&... codes) { return (key(codes) && ...); }
	};

} // namespace Alabaster
