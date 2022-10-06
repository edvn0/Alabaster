#pragma once

struct GLFWwindow;

namespace Alabaster {

	struct ApplicationArguments;

	class Window {
	public:
		Window(const ApplicationArguments&);

		bool should_close();

		void update();

	private:
		void setup_events();

	private:
		uint32_t width;
		uint32_t height;

		struct UserData {
			uint32_t width;
			uint32_t height;
		};

		GLFWwindow* handle;
	};

} // namespace Alabaster
