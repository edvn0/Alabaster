//
// Created by Edwin Carlsson on 2022-12-17.
//

#pragma once

namespace Alabaster {
	class Event;
}

namespace App {

	class Panel {
	public:
		virtual ~Panel() = default;
		virtual void on_update(float ts) = 0;
		virtual void ui(float ts) = 0;
		virtual void on_event(Alabaster::Event& event) = 0;
		virtual void on_init() = 0;
		virtual void on_destroy() = 0;
	};

} // namespace App