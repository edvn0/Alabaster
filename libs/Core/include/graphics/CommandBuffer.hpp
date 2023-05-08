#pragma once

#include "core/exceptions/AlabasterException.hpp"
#include "graphics/Allocator.hpp"

#include <memory>
#include <optional>
#include <queue>
#include <string_view>
#include <vector>

namespace Alabaster {

	enum class QueueChoice { Graphics = 0, Compute = 1 };

	using DeallocationCallback = std::function<void(Allocator&)>;
	class CommandBuffer {
	protected:
	public:
		~CommandBuffer();

		void begin() { begin(nullptr, true); };
		void begin(VkCommandBufferBeginInfo* begin, bool should_start_cb);
		void begin(bool should_start_cb) { begin(nullptr, should_start_cb); };
		void begin(VkCommandBufferBeginInfo* begin_info) { begin(begin_info, true); };

		void end();
		void end_with_no_reset();
		void submit();

		std::uint32_t get_buffer_index();

		const VkCommandBuffer& get_buffer() const;
		VkCommandBuffer& get_buffer();
		const VkCommandPool& get_command_pool() const;
		VkCommandPool& get_command_pool();

		explicit operator VkCommandBuffer();
		VkCommandBuffer operator*();

		void add_destruction_callback(DeallocationCallback&& cb) { destruction_callbacks.push(std::move(cb)); }
		void set_allocator_name(std::string name)
		{
			if (!allocator)
				allocator = std::make_unique<Allocator>(name);
			else {
				allocator->set_tag(name);
			}
		}

		static std::shared_ptr<CommandBuffer> create(std::uint32_t count, QueueChoice choice = QueueChoice::Graphics, bool primary = true)
		{
			return std::shared_ptr<CommandBuffer>(new CommandBuffer(count, choice, primary));
		};
		static std::shared_ptr<CommandBuffer> from_swapchain() { return std::shared_ptr<CommandBuffer>(new CommandBuffer()); };

	private:
		explicit CommandBuffer(std::uint32_t count, QueueChoice choice = QueueChoice::Graphics, bool primary = true);
		CommandBuffer();

		void init(std::uint32_t count = 0);

		VkCommandPool pool { nullptr };
		VkCommandBuffer active { nullptr };
		std::vector<VkCommandBuffer> buffers;
		std::vector<VkFence> fences;

		QueueChoice queue_choice;
		bool is_primary { true };

		std::unique_ptr<Allocator> allocator;
		std::queue<DeallocationCallback> destruction_callbacks {};

		bool owned_by_swapchain { false };
	};

	static constexpr auto default_callback = [](Allocator&) {};

	class ImmediateCommandBuffer {
	public:
		template <typename Str, typename Func = DeallocationCallback>
			requires std::is_convertible_v<Str, std::string>
		explicit ImmediateCommandBuffer(Str&& allocator_tag, Func&& cb = default_callback)
			: buffer(CommandBuffer::create(1))
		{
			buffer->begin();
			buffer->set_allocator_name(std::forward<Str>(allocator_tag));
			buffer->add_destruction_callback(std::forward<Func>(cb));
		}

		~ImmediateCommandBuffer()
		{
			buffer->end();
			buffer->submit();
		}

		void add_destruction_callback(DeallocationCallback&& cb) { buffer->add_destruction_callback(std::forward<DeallocationCallback>(cb)); }

		const VkCommandBuffer& get_buffer() const;
		VkCommandBuffer& get_buffer();

		explicit operator VkCommandBuffer();
		VkCommandBuffer operator*();

		CommandBuffer* operator&() const { return buffer.get(); }
		CommandBuffer* operator&() { return buffer.get(); }

		std::uint32_t get_buffer_index();

	private:
		std::shared_ptr<CommandBuffer> buffer;
	};

} // namespace Alabaster
