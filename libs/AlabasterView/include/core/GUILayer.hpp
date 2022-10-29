#pragma once

#include "core/Application.hpp"

typedef struct VkDescriptorPool_T* VkDescriptorPool;
typedef struct VkRenderPass_T* VkRenderPass;

namespace Alabaster {

	static constexpr std::string_view gui_layer_name = "ImGuiLayer";

	class GUILayer : public Layer {
	public:
		GUILayer() = default;
		~GUILayer();

		void on_event(Event& e) override {};
		bool initialise() override;
		void update(float timestep) override {};
		void ui(float timestep) override;
		void ui() override;
		void destroy() override;

		void begin();
		void end();

	private:
		std::string_view name() override { return gui_layer_name; }

		void create_render_pass();

		VkDescriptorPool imgui_descriptor_pool;
		VkRenderPass render_pass;
	};

} // namespace Alabaster
