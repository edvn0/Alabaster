#include "av_pch.hpp"

#include "core/Application.hpp"
#include "core/Window.hpp"

#include <GLFW/glfw3.h>

namespace Alabaster {

	void error_callback(int error, const char* description) { fprintf(stderr, "Error: %s\n", description); }

	Window::Window(const ApplicationArguments& arguments)
		: width(arguments.width)
		, height(arguments.height)
	{
		UserData user_data;
		int success = glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		// glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_TRUE);

		if (!success) {
			throw std::runtime_error("Initialization of GLFW does not work");
		}

		handle = glfwCreateWindow((int)arguments.width, (int)arguments.height, arguments.name, nullptr, nullptr);

		float pixel_size_x, pixel_size_y;
		glfwGetWindowContentScale(handle, &pixel_size_x, &pixel_size_y);
		glfwSetWindowSize(handle, arguments.width / pixel_size_x, arguments.height / pixel_size_y);

		glfwGetWindowContentScale(handle, &pixel_size_x, &pixel_size_y);
		glfwSetWindowSize(handle, arguments.width / pixel_size_x, arguments.height / pixel_size_y);

		glfwSetWindowUserPointer(handle, &user_data);

		int width, height;
		glfwGetWindowSize(handle, &width, &height);
		user_data.width = width;
		user_data.height = height;

		setup_events();
	};

	void Window::setup_events() { }

	void Window::update() { glfwPollEvents(); }

	bool Window::should_close() { return glfwWindowShouldClose(handle); }

} // namespace Alabaster
