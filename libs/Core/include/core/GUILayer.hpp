#pragma once

#include "core/Application.hpp"

namespace Alabaster {

	static constexpr std::string_view gui_layer_name = "GUILayer";

	class CommandBuffer;

	class GUILayer : public Layer {
	public:
		GUILayer() = default;
		~GUILayer() override = default;

		void on_event(Event&) override;
		bool initialise(AssetManager::FileWatcher&) override;
		void update(float) override {};
		void render() override {};
		void ui() override;
		void destroy() override;

		void block_events(bool set_blocking) { should_block = set_blocking; }

		void begin() const;
		void end() const;

	private:
		std::string_view name() override { return gui_layer_name; }

		bool should_block { false };
		VkDescriptorPool imgui_descriptor_pool { nullptr };
		std::shared_ptr<CommandBuffer> imgui_command_buffer;
	};

} // namespace Alabaster
