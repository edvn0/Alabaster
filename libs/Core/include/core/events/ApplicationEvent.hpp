#pragma once

#include "core/events/Event.hpp"

#include <sstream>

namespace Alabaster {
	class WindowResizeEvent : public Event {
	public:
		WindowResizeEvent(unsigned int width, unsigned int height)
			: w(width)
			, h(height)
		{
		}

		inline unsigned int width() const { return w; }
		inline unsigned int height() const { return h; }

		std::string to_string() const override
		{
			std::stringstream ss;
			ss << "WindowResizeEvent: " << width() << ", " << height();
			return ss.str();
		}

		EVENT_STATIC_CLASS_TYPE(WindowResize)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	private:
		unsigned int w, h;
	};

	class WindowMinimizeEvent : public Event {
	public:
		WindowMinimizeEvent(bool is_minimized)
			: minimized(is_minimized)
		{
		}

		bool is_minimized() const { return minimized; }

		EVENT_STATIC_CLASS_TYPE(WindowMinimize)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	private:
		bool minimized = false;
	};

	class WindowCloseEvent : public Event {
	public:
		WindowCloseEvent() { }

		EVENT_STATIC_CLASS_TYPE(WindowClose)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class WindowTitleBarHitTestEvent : public Event {
	public:
		WindowTitleBarHitTestEvent(int x_pos, int y_pos)
			: x(x_pos)
			, y(y_pos)
		{
		}

		inline int get_x() const { return x; }
		inline int get_y() const { return y; }

		EVENT_STATIC_CLASS_TYPE(WindowTitleBarHitTest)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	private:
		int x;
		int y;
	};

	class AppTickEvent : public Event {
	public:
		AppTickEvent() { }

		EVENT_STATIC_CLASS_TYPE(AppTick)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class AppUpdateEvent : public Event {
	public:
		AppUpdateEvent() { }

		EVENT_STATIC_CLASS_TYPE(AppUpdate)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class AppRenderEvent : public Event {
	public:
		AppRenderEvent() { }

		EVENT_STATIC_CLASS_TYPE(AppRender)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};
} // namespace Alabaster
