#pragma once

#include "core/Application.hpp"

namespace Alabaster {

	class GUILayer : public Layer {
	public:
		GUILayer() = default;

		bool initialise() override;
		void update(float timestep) override {};
		void destroy() override {};

		static void begin();
		static void end();

	private:
		std::string_view name() override { return "ImGuiLayer"; }
	};

} // namespace Alabaster
