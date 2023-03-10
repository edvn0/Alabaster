#pragma once

#include "core/Application.hpp"
#include "graphics/CommandBuffer.hpp"

namespace Alabaster {

	static constexpr std::string_view gui_layer_name = "GUILayer";

	class GUILayer : public Layer {
	public:
		GUILayer() = default;
		~GUILayer() override = default;

		void on_event(Event&) override;
		bool initialise() override;
		void update(float) override {};
		void ui(float) override;
		void ui() override {};
		void destroy() override;

		void block_events() { should_block = true; }

		void begin();
		void end();

	private:
		std::string_view name() override { return gui_layer_name; }

		VkDescriptorPool imgui_descriptor_pool { nullptr };
		std::unique_ptr<CommandBuffer> imgui_command_buffer { std::make_unique<CommandBuffer>(3, QueueChoice::Graphics, false) };

		bool should_block { false };
	};

} // namespace Alabaster
