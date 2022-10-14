#include "av_pch.hpp"

#include "core/Window.hpp"

#include "core/Application.hpp"
#include "core/Logger.hpp"
#include "GLFW/glfw3.h"
#include "graphics/Swapchain.hpp"

#include <GLFW/glfw3.h>

namespace Alabaster {

	Window::~Window() = default;

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

		Log::info("[Window] GLFW Initialised! Code: {}", success);

		handle = glfwCreateWindow(static_cast<int>(arguments.width), static_cast<int>(arguments.height), arguments.name, nullptr, nullptr);

		glfwSetWindowUserPointer(handle, &user_data);

		int w, h;
		glfwGetWindowSize(handle, &w, &h);

		Log::info("[Window] Window size to {} by {}", w, h);

		uint32_t use_width = arguments.width;
		uint32_t use_height = arguments.height;
		if (w < arguments.width)
			use_width = w;

		if (h < arguments.height)
			use_height = h;

		float pixel_size_x, pixel_size_y;
		glfwGetWindowContentScale(handle, &pixel_size_x, &pixel_size_y);

		Log::info("[Window] Set window pixel size to {} by {}", pixel_size_x, pixel_size_y);

		glfwSetWindowSize(handle, use_width / pixel_size_x, use_height / pixel_size_y);

		Log::info("[Window] Set window size to {} by {}", use_width / pixel_size_x, use_height / pixel_size_y);

		glfwGetWindowSize(handle, &w, &h);
		user_data.width = w;
		user_data.height = h;

		swapchain = std::make_unique<Swapchain>();
		swapchain->construct(handle, width, height);

		setup_events();
	};

	const std::pair<int, int> Window::framebuffer_extent() const
	{
		int tw, th;
		glfwGetFramebufferSize(handle, &tw, &th);
		return { tw, th };
	}

	void Window::destroy()
	{
		swapchain->destroy();

		glfwDestroyWindow(handle);
		glfwTerminate();
	}

	void Window::swap_buffers() { swapchain->present(); }

	void Window::setup_events()
	{
		glfwSetFramebufferSizeCallback(handle, [](GLFWwindow* window, int w, int h) { Application::the().resize(w, h); });
		glfwSetKeyCallback(handle, [](GLFWwindow* window, auto a, auto b, auto c, auto d) { Log::info("{},{},{},{}", a, b, c, d); });
	}

	void Window::update() { glfwPollEvents(); }

	bool Window::should_close() { return glfwWindowShouldClose(handle); }

	void Window::close() { glfwSetWindowShouldClose(handle, 1); }

} // namespace Alabaster
