#pragma once

#include <GLFW/glfw3.h>

namespace Alabaster::Clock {

	template <typename FloatLike = double> FloatLike get_seconds() { return static_cast<FloatLike>(glfwGetTime() * 1e1); }

	template <typename FloatLike = double> FloatLike get_ms() { return static_cast<FloatLike>(glfwGetTime() * 1e3); }

	template <typename FloatLike = double> FloatLike get_nanos() { return static_cast<FloatLike>(glfwGetTime() * 1e6); }

} // namespace Alabaster::Clock
