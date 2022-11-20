#pragma once

#include "graphics/Allocator.hpp"

#include <memory>
#include <string_view>
#include <vector>
#include <vulkan/vulkan.h>

namespace Alabaster {

	enum class QueueChoice { Graphics = 0, Compute = 1 };

	using DeallocationCallback = std::function<void(Allocator&)>;

	class CommandBuffer {
	public:
		explicit CommandBuffer(uint32_t count, QueueChoice queue_choice = QueueChoice::Graphics);
		void destroy();

		void begin();
		void end();
		void submit();

		auto& get_buffer() const { return active; }
		auto& get_command_pool() const { return pool; }
		auto& get_buffer() { return active; }
		auto& get_command_pool() { return pool; }

		operator VkCommandBuffer() { return active; }

		void add_destruction_callback(DeallocationCallback&& cb) { destruction_callbacks.push_back(std::move(cb)); }

	private:
		void init(uint32_t count = 0);

	private:
		VkCommandPool pool { nullptr };
		VkCommandBuffer active { nullptr };
		std::vector<VkCommandBuffer> buffers;
		std::vector<VkFence> fences;

		QueueChoice queue_choice;

		std::vector<DeallocationCallback> destruction_callbacks {};

		bool owned_by_swapchain { false };

		CommandBuffer();

	public:
		static std::unique_ptr<CommandBuffer> from_swapchain() { return std::unique_ptr<CommandBuffer>(new CommandBuffer()); }
	};

} // namespace Alabaster
