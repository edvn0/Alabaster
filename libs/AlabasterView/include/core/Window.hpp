#pragma once

struct GLFWwindow;

namespace Alabaster {

	struct ApplicationArguments;
	class Swapchain;

	class Window {
	public:
		explicit Window(const ApplicationArguments&);
		~Window();

		void destroy();

		bool should_close();
		void close();

		void update();

		GLFWwindow* native() { return handle; }
		GLFWwindow* native() const { return handle; }

		const std::unique_ptr<Swapchain>& get_swapchain() { return swapchain; }
		const std::unique_ptr<Swapchain>& get_swapchain() const { return swapchain; }

		const std::pair<int, int> framebuffer_extent() const;

		void swap_buffers();

	private:
		void setup_events();

	private:
		uint32_t width;
		uint32_t height;

		struct {
			uint32_t width;
			uint32_t height;
		} user_data;

		std::unique_ptr<Swapchain> swapchain;
		GLFWwindow* handle;
	};

} // namespace Alabaster
