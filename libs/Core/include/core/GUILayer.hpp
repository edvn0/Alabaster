#pragma once

#include "core/Application.hpp"
#include "graphics/CommandBuffer.hpp"

typedef struct VkDescriptorPool_T* VkDescriptorPool;
typedef struct VkRenderPass_T* VkRenderPass;

namespace Alabaster {

	static constexpr std::string_view gui_layer_name = "GUILayer";

	class GUILayer : public Layer {
	public:
		GUILayer()
			: imgui_command_buffer(new CommandBuffer(3, QueueChoice::Graphics, false))
		{
		}
		~GUILayer();

		void on_event(Event&) override;
		bool initialise() override;
		void update(float) override {};
		void ui(float) override;
		void ui() override;
		void destroy() override;

		void block_events() { should_block = true; }

		void begin();
		void end();

	private:
		std::string_view name() override { return gui_layer_name; }

	private:
		VkDescriptorPool imgui_descriptor_pool { nullptr };
		std::unique_ptr<CommandBuffer> imgui_command_buffer;

		bool should_block { false };
	};

} // namespace Alabaster
