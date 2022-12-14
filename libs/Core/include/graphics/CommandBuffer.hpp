#pragma once

#include "core/exceptions/AlabasterException.hpp"
#include "graphics/Allocator.hpp"

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
		virtual ~CommandBuffer();
		void destroy();

		virtual void begin() { begin(nullptr, true); };
		void begin(VkCommandBufferBeginInfo* begin, bool should_start_cb);
		void begin(bool should_start_cb) { begin(nullptr, should_start_cb); };
		void begin(VkCommandBufferBeginInfo* begin_info) { begin(begin_info, true); };

		virtual void end();
		virtual void end_with_no_reset();
		virtual void submit();

		virtual std::uint32_t get_buffer_index();

		auto& get_buffer() const { return active; }
		auto& get_command_pool() const { return pool; }
		auto& get_buffer() { return active; }
		auto& get_command_pool() { return pool; }

		operator VkCommandBuffer() { return active; }
		VkCommandBuffer operator*() { return active; }

		void add_destruction_callback(DeallocationCallback&& cb) { destruction_callbacks.push(std::move(cb)); }

	private:
		void init(std::uint32_t count = 0);

	protected:
		void set_allocator_name(std::string name)
		{
			if (!allocator)
				allocator = std::make_unique<Allocator>(name);
			else {
				allocator->set_tag(name);
			}
		}

	protected:
		VkCommandPool pool { nullptr };
		VkCommandBuffer active { nullptr };
		std::vector<VkCommandBuffer> buffers;
		std::vector<VkFence> fences;

	private:
		QueueChoice queue_choice;
		bool is_primary { true };

		std::unique_ptr<Allocator> allocator;
		std::queue<DeallocationCallback> destruction_callbacks {};

		bool owned_by_swapchain { false };
		bool destroyed { false };

		CommandBuffer();

	public:
		static std::unique_ptr<CommandBuffer> from_swapchain() { return std::unique_ptr<CommandBuffer>(new CommandBuffer()); }
	};

	class ImmediateCommandBuffer final : public CommandBuffer {
	public:
		ImmediateCommandBuffer(std::string allocator_tag)
			: CommandBuffer(1)
		{
			begin();
			set_allocator_name(std::move(allocator_tag));
		}

		~ImmediateCommandBuffer() final
		{
			CommandBuffer::end();
			CommandBuffer::submit();
		}

		std::uint32_t get_buffer_index() final;

		void end() final { throw AlabasterException("Cannot explicitly end an immediate command buffer. It is called in the destructor"); }
		void submit() final { throw AlabasterException("Cannot explicitly submit an immediate command buffer. It is called in the destructor"); }
	};

} // namespace Alabaster
