#pragma once

#include "graphics/Swapchain.hpp"

#include <functional>
#include <memory>

struct GLFWwindow;
struct GLFWcursor;

namespace Alabaster {

	struct ApplicationArguments;
	class Swapchain;
	class Event;

	class Window {
	public:
		using EventCallback = std::function<void(Event&)>;

		explicit Window(const ApplicationArguments&);
		~Window();

		void destroy();

		bool should_close();
		void close();

		void update();

		GLFWwindow* native() { return handle; }
		GLFWwindow* native() const { return handle; }

		void set_event_callback(const EventCallback& cb) { user_data.callback = cb; }

		const std::unique_ptr<Swapchain>& get_swapchain() { return swapchain; }
		const std::unique_ptr<Swapchain>& get_swapchain() const { return swapchain; }

		std::pair<int, int> framebuffer_extent() const;
		std::pair<float, float> framebuffer_scale() const;
		std::pair<std::uint32_t, std::uint32_t> size() const;

		void swap_buffers();

		bool was_resized() const { return resize_status; }
		void reset_resize_status() { resize_status = false; }

	private:
		void setup_events();

		bool resize_status { false };
		std::uint32_t width;
		std::uint32_t height;

		struct UserData {
			std::uint32_t width;
			std::uint32_t height;

			EventCallback callback;
		};

		UserData user_data;

		std::unique_ptr<Swapchain> swapchain;
		GLFWwindow* handle;
		std::array<GLFWcursor*, 9> imgui_mouse_cursors;
	};

} // namespace Alabaster
