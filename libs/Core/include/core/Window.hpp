#pragma once

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

		void set_event_callback(const EventCallback& cb);

		const std::unique_ptr<Swapchain>& get_swapchain() { return swapchain; }
		const std::unique_ptr<Swapchain>& get_swapchain() const { return swapchain; }

		const std::pair<int, int> framebuffer_extent() const;
		const std::pair<float, float> framebuffer_scale() const;
		const std::pair<std::uint32_t, std::uint32_t> size() const;

		void swap_buffers();

		bool was_resized() const { return resize_status; }
		void reset_resize_status() { resize_status = false; }
		bool resize_status { false };

	private:
		void setup_events();

	private:
		std::uint32_t width;
		std::uint32_t height;

		struct UserData {
			std::uint32_t width;
			std::uint32_t height;

			EventCallback callback;
		} user_data;

		std::unique_ptr<Swapchain> swapchain;
		GLFWwindow* handle;
		GLFWcursor* imgui_mouse_cursors[9];
	};

} // namespace Alabaster
