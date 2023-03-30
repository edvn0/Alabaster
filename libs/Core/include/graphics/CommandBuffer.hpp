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
	protected:
		using DeallocationCallback = std::function<void(Allocator&)>;

	public:
		explicit CommandBuffer(std::uint32_t count, QueueChoice choice = QueueChoice::Graphics, bool primary = true);
		virtual ~CommandBuffer();
		void destroy();

		virtual void begin() { begin(nullptr, true); };
		virtual void begin(VkCommandBufferBeginInfo* begin, bool should_start_cb);
		virtual void begin(bool should_start_cb) { begin(nullptr, should_start_cb); };
		virtual void begin(VkCommandBufferBeginInfo* begin_info) { begin(begin_info, true); };

		virtual void end();
		virtual void end_with_no_reset();
		virtual void submit();

		virtual std::uint32_t get_buffer_index();

		auto& get_buffer() const { return active; }
		auto& get_buffer() { return active; }
		auto& get_command_pool() const { return pool; }
		auto& get_command_pool() { return pool; }

		explicit operator VkCommandBuffer() { return active; }
		VkCommandBuffer operator*() { return active; }

		void add_destruction_callback(DeallocationCallback&& cb) { destruction_callbacks.push(std::move(cb)); }

	protected:
		void set_allocator_name(std::string name)
		{
			if (!allocator)
				allocator = std::make_unique<Allocator>(name);
			else {
				allocator->set_tag(name);
			}
		}

		VkCommandPool pool { nullptr };
		VkCommandBuffer active { nullptr };
		std::vector<VkCommandBuffer> buffers;
		std::vector<VkFence> fences;

	private:
		void init(std::uint32_t count = 0);

		QueueChoice queue_choice;
		bool is_primary { true };

		std::unique_ptr<Allocator> allocator;
		std::queue<DeallocationCallback> destruction_callbacks {};

		bool owned_by_swapchain { false };
		bool destroyed { false };

		CommandBuffer();

	public:
		static std::unique_ptr<CommandBuffer> from_swapchain() { return std::unique_ptr<CommandBuffer>(new CommandBuffer()); };
	};

	static constexpr auto default_callback = [](Allocator&) {};

	class ImmediateCommandBuffer final : public CommandBuffer {
	public:
		template <typename Str, typename Func = DeallocationCallback>
		requires std::is_convertible_v<Str, std::string>
		explicit ImmediateCommandBuffer(Str&& allocator_tag, Func&& cb = default_callback)
			: CommandBuffer(1)
		{
			CommandBuffer::begin();
			set_allocator_name(std::forward<Str>(allocator_tag));
			add_destruction_callback(std::forward<Func>(cb));
		}

		~ImmediateCommandBuffer() override
		{
			CommandBuffer::end();
			CommandBuffer::submit();
		}

		std::uint32_t get_buffer_index() override;

		void begin() override { throw AlabasterException("Cannot explicitly begin an immediate command buffer. It is called in the constructor."); }
		void end() override { throw AlabasterException("Cannot explicitly end an immediate command buffer. It is called in the destructor."); }
		void submit() override { throw AlabasterException("Cannot explicitly submit an immediate command buffer. It is called in the destructor."); }
	};

} // namespace Alabaster
