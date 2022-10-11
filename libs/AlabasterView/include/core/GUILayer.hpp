#pragma once

#include "core/Application.hpp"

typedef struct VkDescriptorPool_T* VkDescriptorPool;

namespace Alabaster {

	static constexpr std::string_view gui_layer_name = "ImGuiLayer";

	class GUILayer : public Layer {
	public:
		GUILayer() = default;
		~GUILayer();

		bool initialise() override;
		void update(float timestep) override {};
		void ui(float timestep) override;
		void destroy() override;

		static void begin();
		static void end();

	private:
		std::string_view name() override { return gui_layer_name; }
		VkDescriptorPool imgui_descriptor_pool;
	};

} // namespace Alabaster
