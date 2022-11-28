#pragma once

#include "graphics/Allocator.hpp"
#include "vulkan/vulkan_core.h"

#include <memory>
#include <optional>
#include <queue>
#include <string_view>
#include <vector>
#include <vulkan/vulkan.h>

namespace Alabaster {

	enum class QueueChoice { Graphics = 0, Compute = 1 };

	class CommandBuffer {
		using DeallocationCallback = std::function<void(Allocator&)>;

	public:
		explicit CommandBuffer(std::uint32_t count, QueueChoice queue_choice = QueueChoice::Graphics, bool is_primary = true);
		~CommandBuffer();
		void destroy();

		void begin(VkCommandBufferBeginInfo* begin, bool should_start_cb);
		void begin() { begin(nullptr, true); };
		void begin(bool should_start_cb) { begin(nullptr, should_start_cb); };
		void begin(VkCommandBufferBeginInfo* begin_info) { begin(begin_info, true); };

		void end();
		void end_with_no_reset();
		void submit();

		auto& get_buffer() const { return active; }
		auto& get_command_pool() const { return pool; }
		auto& get_buffer() { return active; }
		auto& get_command_pool() { return pool; }

		operator VkCommandBuffer() { return active; }

		void add_destruction_callback(DeallocationCallback&& cb) { destruction_callbacks.push(std::move(cb)); }

	private:
		void init(std::uint32_t count = 0);

	private:
		VkCommandPool pool { nullptr };
		VkCommandBuffer active { nullptr };
		std::vector<VkCommandBuffer> buffers;
		std::vector<VkFence> fences;

		QueueChoice queue_choice;
		bool is_primary { true };

		std::queue<DeallocationCallback> destruction_callbacks {};

		bool owned_by_swapchain { false };
		bool destroyed { false };

		CommandBuffer();

	public:
		static std::unique_ptr<CommandBuffer> from_swapchain() { return std::unique_ptr<CommandBuffer>(new CommandBuffer()); }
	};

} // namespace Alabaster
