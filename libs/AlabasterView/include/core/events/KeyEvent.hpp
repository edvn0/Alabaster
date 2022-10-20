#pragma once

#include "codes/KeyCode.hpp"
#include "Event.hpp"

#include <sstream>

namespace Alabaster {

	class KeyEvent : public Event {
	public:
		inline KeyCode get_key_code() const { return key_code; }

		EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)
	protected:
		KeyEvent(KeyCode keycode)
			: key_code(keycode)
		{
		}

		KeyCode key_code;
	};

	class KeyPressedEvent : public KeyEvent {
	public:
		KeyPressedEvent(KeyCode keycode, int repeat_count)
			: KeyEvent(keycode)
			, repeat_count(repeat_count)
		{
		}

		int get_repeat_count() const { return repeat_count; }

		std::string to_string() const override
		{
			std::stringstream ss;
			ss << "KeyPressedEvent: " << key_code << " (" << repeat_count << " repeats)";
			return ss.str();
		}

		EVENT_STATIC_CLASS_TYPE(KeyPressed)
	private:
		int repeat_count;
	};

	class KeyReleasedEvent : public KeyEvent {
	public:
		KeyReleasedEvent(KeyCode keycode)
			: KeyEvent(keycode)
		{
		}

		std::string to_string() const override
		{
			std::stringstream ss;
			ss << "KeyReleasedEvent: " << key_code;
			return ss.str();
		}

		EVENT_STATIC_CLASS_TYPE(KeyReleased)
	};

	class KeyTypedEvent : public KeyEvent {
	public:
		KeyTypedEvent(KeyCode keycode)
			: KeyEvent(keycode)
		{
		}

		std::string to_string() const override
		{
			std::stringstream ss;
			ss << "KeyTypedEvent: " << key_code;
			return ss.str();
		}

		EVENT_STATIC_CLASS_TYPE(KeyTyped)
	};
} // namespace Alabaster
