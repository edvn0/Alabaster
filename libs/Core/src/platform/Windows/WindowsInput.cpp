#include "av_pch.hpp"

#include "core/Application.hpp"
#include "core/Input.hpp"
#include "core/Window.hpp"

#include <GLFW/glfw3.h>

namespace Alabaster {

	static auto* get_window() { return Application::the().get_window()->native(); }

	bool Input::key(KeyCode key) { return glfwGetKey(get_window(), static_cast<int>(key)) == GLFW_PRESS; }

	bool Input::mouse(MouseCode key) { return glfwGetMouseButton(get_window(), static_cast<int>(key)) == GLFW_PRESS; }

	glm::vec2 Input::mouse_position()
	{
		double out_x, out_y;
		glfwGetCursorPos(get_window(), &out_x, &out_y);
		return { out_x, out_y };
	}

} // namespace Alabaster