#pragma once

#include "vulkan/vulkan_core.h"

#include <string_view>
#include <vector>
#include <vulkan/vulkan.h>

namespace Alabaster {

	class CommandBuffer {
	public:
		CommandBuffer(std::string_view name);
		explicit CommandBuffer(uint32_t count, std::string_view debug_name = "");
		~CommandBuffer();

		void begin();
		void end();
		void submit();

		auto& get_buffer() const { return active; }
		auto& get_command_pool() const { return pool; }
		auto& get_buffer() { return active; }
		auto& get_command_pool() { return pool; }

		operator VkCommandBuffer() { return active; }

	private:
		void init(uint32_t count = 0);

	private:
		VkCommandPool pool = nullptr;
		std::vector<VkCommandBuffer> buffers;
		VkCommandBuffer active = nullptr;
		std::vector<VkFence> fences;

		bool owned_by_swapchain = false;
	};

} // namespace Alabaster
