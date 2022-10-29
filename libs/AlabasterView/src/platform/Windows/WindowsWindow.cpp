#include "av_pch.hpp"

#include "core/Window.hpp"

#include "codes/KeyCode.hpp"
#include "core/Application.hpp"
#include "core/Common.hpp"
#include "core/events/ApplicationEvent.hpp"
#include "core/events/KeyEvent.hpp"
#include "core/events/MouseEvent.hpp"
#include "core/Logger.hpp"
#include "graphics/Swapchain.hpp"

#include <GLFW/glfw3.h>
#include <imgui.h>

namespace Alabaster {

	Window::~Window() = default;

	Window::Window(const ApplicationArguments& arguments)
		: width(arguments.width)
		, height(arguments.height)
	{
		int success = glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		// glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
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
		swapchain->init(handle);
		swapchain->construct(width, height);

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

		// Set GLFW callbacks
		glfwSetWindowSizeCallback(handle, [](GLFWwindow* window, int width, int height) {
			auto& data = *static_cast<UserData*>(glfwGetWindowUserPointer(window));

			WindowResizeEvent event((uint32_t)width, (uint32_t)height);
			data.callback(event);
			data.width = width;
			data.height = height;
		});

		glfwSetWindowCloseCallback(handle, [](GLFWwindow* window) {
			auto& data = *static_cast<UserData*>(glfwGetWindowUserPointer(window));

			WindowCloseEvent event;
			data.callback(event);
		});

		glfwSetKeyCallback(handle, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
			auto& data = *static_cast<UserData*>(glfwGetWindowUserPointer(window));

			switch (action) {
			case GLFW_PRESS: {
				KeyPressedEvent event(static_cast<KeyCode>(key), 0);
				data.callback(event);
				break;
			}
			case GLFW_RELEASE: {
				KeyReleasedEvent event(static_cast<KeyCode>(key));
				data.callback(event);
				break;
			}
			case GLFW_REPEAT: {
				KeyPressedEvent event(static_cast<KeyCode>(key), 1);
				data.callback(event);
				break;
			}
			}
		});

		glfwSetCharCallback(handle, [](GLFWwindow* window, uint32_t codepoint) {
			auto& data = *static_cast<UserData*>(glfwGetWindowUserPointer(window));

			KeyTypedEvent event(static_cast<KeyCode>(codepoint));
			data.callback(event);
		});

		glfwSetMouseButtonCallback(handle, [](GLFWwindow* window, int button, int action, int mods) {
			auto& data = *static_cast<UserData*>(glfwGetWindowUserPointer(window));

			switch (action) {
			case GLFW_PRESS: {
				MouseButtonPressedEvent event(static_cast<MouseCode>(button));
				data.callback(event);
				break;
			}
			case GLFW_RELEASE: {
				MouseButtonReleasedEvent event(static_cast<MouseCode>(button));
				data.callback(event);
				break;
			}
			}
		});

		glfwSetScrollCallback(handle, [](GLFWwindow* window, double x_offset, double y_offset) {
			auto& data = *static_cast<UserData*>(glfwGetWindowUserPointer(window));

			MouseScrolledEvent event(static_cast<float>(x_offset), static_cast<float>(y_offset));
			data.callback(event);
		});

		glfwSetCursorPosCallback(handle, [](GLFWwindow* window, double x, double y) {
			auto& data = *static_cast<UserData*>(glfwGetWindowUserPointer(window));
			MouseMovedEvent event((float)x, (float)y);
			data.callback(event);
		});

		glfwSetWindowIconifyCallback(handle, [](GLFWwindow* window, int iconified) {
			auto& data = *static_cast<UserData*>(glfwGetWindowUserPointer(window));
			WindowMinimizeEvent event((bool)iconified);
			data.callback(event);
		});

		imgui_mouse_cursors[ImGuiMouseCursor_Arrow] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
		imgui_mouse_cursors[ImGuiMouseCursor_TextInput] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
		imgui_mouse_cursors[ImGuiMouseCursor_ResizeAll] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR); // FIXME: GLFW doesn't have this.
		imgui_mouse_cursors[ImGuiMouseCursor_ResizeNS] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
		imgui_mouse_cursors[ImGuiMouseCursor_ResizeEW] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
		imgui_mouse_cursors[ImGuiMouseCursor_ResizeNESW] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR); // FIXME: GLFW doesn't have this.
		imgui_mouse_cursors[ImGuiMouseCursor_ResizeNWSE] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR); // FIXME: GLFW doesn't have this.
		imgui_mouse_cursors[ImGuiMouseCursor_Hand] = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
	}

	void Window::update() { glfwPollEvents(); }

	void Window::set_event_callback(const EventCallback& cb) { user_data.callback = cb; }

	bool Window::should_close() { return glfwWindowShouldClose(handle); }

	void Window::close() { glfwSetWindowShouldClose(handle, 1); }

} // namespace Alabaster
