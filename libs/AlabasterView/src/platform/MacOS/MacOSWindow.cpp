#include "av_pch.hpp"

#include "core/Application.hpp"
#include "core/Logger.hpp"
#include "core/Window.hpp"
#include "graphics/Swapchain.hpp"

#include <GLFW/glfw3.h>

namespace Alabaster {

	Window::~Window() = default;

	void Window::destroy()
	{
		swapchain->destroy();

		glfwDestroyWindow(handle);
		glfwTerminate();
	}

	Window::Window(const ApplicationArguments& arguments)
		: width(arguments.width)
		, height(arguments.height)
	{
		int success = glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_TRUE);

		if (!success) {
			throw std::runtime_error("Initialization of GLFW does not work");
		}

		Log::info("GLFW Initialised! Code: {}", success);

		handle = glfwCreateWindow(static_cast<int>(arguments.width), static_cast<int>(arguments.height), arguments.name, nullptr, nullptr);

		float pixel_size_x, pixel_size_y;
		glfwGetWindowContentScale(handle, &pixel_size_x, &pixel_size_y);
		glfwSetWindowSize(handle, arguments.width / pixel_size_x, arguments.height / pixel_size_y);

		glfwSetWindowUserPointer(handle, &user_data);

		int w, h;
		glfwGetWindowSize(handle, &w, &h);
		user_data.width = w;
		user_data.height = h;

		setup_events();

		swapchain = std::make_unique<Swapchain>();
		swapchain->construct(handle, width, height);
	};

	const std::pair<int, int> Window::framebuffer_extent() const
	{
		int tw, th;
		glfwGetFramebufferSize(handle, &tw, &th);
		return { tw, th };
	}

	void Window::swap_buffers() { swapchain->present(); }

	void Window::setup_events() { }

	void Window::update() { glfwPollEvents(); }

	bool Window::should_close() { return glfwWindowShouldClose(handle); }

} // namespace Alabaster
