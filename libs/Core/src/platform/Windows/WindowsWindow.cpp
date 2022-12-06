#include "av_pch.hpp"

#include "core/Window.hpp"

#include "codes/KeyCode.hpp"
#include "core/Application.hpp"
#include "core/Common.hpp"
#include "core/events/ApplicationEvent.hpp"
#include "core/events/KeyEvent.hpp"
#include "core/events/MouseEvent.hpp"
#include "core/exceptions/AlabasterException.hpp"
#include "core/Logger.hpp"
#include "graphics/Swapchain.hpp"

#include <GLFW/glfw3.h>
#include <imgui.h>

namespace Alabaster {
	static inline const void initialize_window_library()
	{
		int success = glfwInit();
		if (!success) {
			throw AlabasterException();
		}
	};
	Window::~Window() = default;

	Window::Window(const ApplicationArguments& arguments)
		: width(arguments.width)
		, height(arguments.height)
	{
		try {
			initialize_window_library();
			Log::info("[Window] GLFW Initialised!");
		} catch (const AlabasterException& exception) {
			Log::error("{}", exception.what());
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, false);
		glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);

		handle = glfwCreateWindow(static_cast<int>(arguments.width), static_cast<int>(arguments.height), arguments.name.data(), nullptr, nullptr);
		int actual_w, actual_h;
		glfwGetWindowSize(handle, &actual_w, &actual_h);

		glfwSetWindowUserPointer(handle, &user_data);

		static constexpr auto set_and_get_window_size = [](GLFWwindow* handle, std::uint32_t w, std::uint32_t h) {
			glfwSetWindowSize(handle, static_cast<int>(w), static_cast<int>(h));

			int actual_w, actual_h;
			glfwGetWindowSize(handle, &actual_w, &actual_h);
			return std::make_tuple(actual_w, actual_h);
		};

		const auto&& [calc_w, calc_h] = set_and_get_window_size(handle, actual_w, actual_h);

		width = calc_w;
		height = calc_h;

		Log::info("[Window] Window size is {} by {}", width, height);

		user_data.width = width;
		user_data.height = height;

		swapchain = std::make_unique<VulkanSwapChain>();
		swapchain->init(handle);

		auto w = std::uint32_t(width);
		auto h = std::uint32_t(height);
		swapchain->create(&w, &h, false);

		setup_events();
	};

	const std::pair<int, int> Window::framebuffer_extent() const
	{
		int tw, th;
		glfwGetFramebufferSize(handle, &tw, &th);
		return { tw, th };
	}

	const std::pair<std::uint32_t, std::uint32_t> Window::size() const
	{
		int tw, th;
		glfwGetWindowSize(handle, &tw, &th);
		return { static_cast<std::uint32_t>(tw), static_cast<std::uint32_t>(th) };
	}

	const std::pair<float, float> Window::framebuffer_scale() const
	{
		float tw, th;
		glfwGetWindowContentScale(handle, &tw, &th);
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
		glfwSetFramebufferSizeCallback(handle, [](GLFWwindow*, int w, int h) { Application::the().resize(w, h); });

		// Set GLFW callbacks
		glfwSetWindowSizeCallback(handle, [](GLFWwindow* window, int width, int height) {
			auto& data = *static_cast<UserData*>(glfwGetWindowUserPointer(window));

			WindowResizeEvent event((std::uint32_t)width, (std::uint32_t)height);
			data.callback(event);
			data.width = width;
			data.height = height;
		});

		glfwSetWindowCloseCallback(handle, [](GLFWwindow* window) {
			auto& data = *static_cast<UserData*>(glfwGetWindowUserPointer(window));

			WindowCloseEvent event;
			data.callback(event);
		});

		glfwSetKeyCallback(handle, [](GLFWwindow* window, int key, int, int action, int) {
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

		glfwSetCharCallback(handle, [](GLFWwindow* window, std::uint32_t codepoint) {
			auto& data = *static_cast<UserData*>(glfwGetWindowUserPointer(window));

			KeyTypedEvent event(static_cast<KeyCode>(codepoint));
			data.callback(event);
		});

		glfwSetMouseButtonCallback(handle, [](GLFWwindow* window, int button, int action, int) {
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
