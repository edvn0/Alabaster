#pragma once

#include <GLFW/glfw3.h>

namespace Alabaster::Clock {

	template <typename FloatLike> FloatLike get_seconds() { return static_cast<FloatLike>(glfwGetTime()); }

	template <typename FloatLike> FloatLike get_ms() { return static_cast<FloatLike>(glfwGetTime() * 1000); }

} // namespace Alabaster::Clock
