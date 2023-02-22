#include "av_pch.hpp"

#include "graphics/CommandBuffer.hpp"

#include "core/Application.hpp"
#include "graphics/GraphicsContext.hpp"
#include "graphics/Renderer.hpp"

#include <vulkan/vulkan.h>

namespace Alabaster {

	void CommandBuffer::init(std::uint32_t count)
	{
		if (owned_by_swapchain)
			return;

		std::uint32_t frames = count;
		VkCommandPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		pool_info.queueFamilyIndex = GraphicsContext::the().graphics_queue_family();
		pool_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		vk_check(vkCreateCommandPool(GraphicsContext::the().device(), &pool_info, nullptr, &pool));

		VkCommandBufferAllocateInfo command_buffer_allocate_info {};
		command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		command_buffer_allocate_info.commandPool = pool;
		command_buffer_allocate_info.level = is_primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
		if (count == 0)
			frames = Application::the().swapchain().get_image_count();
		command_buffer_allocate_info.commandBufferCount = frames;
		buffers.resize(frames);
		vk_check(vkAllocateCommandBuffers(GraphicsContext::the().device(), &command_buffer_allocate_info, buffers.data()));

		VkFenceCreateInfo fence_create_info {};
		fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		fences.resize(frames);
		for (std::uint32_t i = 0; i < frames; i++) {
			vk_check(vkCreateFence(GraphicsContext::the().device(), &fence_create_info, nullptr, &fences[i]));
		}
	}

	CommandBuffer::CommandBuffer()
		: owned_by_swapchain(true)
	{
		init(3);
	}

	CommandBuffer::CommandBuffer(std::uint32_t count, QueueChoice choice, bool primary)
		: queue_choice(choice)
		, is_primary(primary)
	{
		init(count);
	}

	void CommandBuffer::destroy()
	{
		if (owned_by_swapchain)
			return;

		const auto& device = GraphicsContext::the().device();
		vkDestroyCommandPool(device, pool, nullptr);

		for (auto& fence : fences) {
			vkDestroyFence(device, fence, nullptr);
		}

		destroyed = true;
	}

	CommandBuffer::~CommandBuffer()
	{
		if (!destroyed) {
			destroy();
		}
	}

	void CommandBuffer::begin(VkCommandBufferBeginInfo* begin, bool should_begin)
	{
		std::uint32_t frame_index = get_buffer_index();

		VkCommandBufferBeginInfo begin_info = {};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		begin_info.pNext = nullptr;

		if (owned_by_swapchain) {
			active = Application::the().swapchain().get_drawbuffer(frame_index);
		} else {
			active = buffers[frame_index];
		}

		if (should_begin)
			vk_check(vkBeginCommandBuffer(active, begin ? begin : &begin_info));
	}

	void CommandBuffer::end()
	{
		vk_check(vkEndCommandBuffer(active));
		active = nullptr;
	}

	void CommandBuffer::end_with_no_reset() { vk_check(vkEndCommandBuffer(active)); }

	void CommandBuffer::submit()
	{
		if (owned_by_swapchain)
			return;

		std::uint32_t frame_index = get_buffer_index();

		VkSubmitInfo submit_info {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = 1;
		const VkCommandBuffer& command_buffer = buffers[frame_index];
		submit_info.pCommandBuffers = &command_buffer;

		vk_check(vkWaitForFences(GraphicsContext::the().device(), 1, &fences[frame_index], VK_TRUE, UINT64_MAX));
		vk_check(vkResetFences(GraphicsContext::the().device(), 1, &fences[frame_index]));

		switch (queue_choice) {
		case QueueChoice::Graphics: {
			vk_check(vkQueueSubmit(GraphicsContext::the().graphics_queue(), 1, &submit_info, fences[frame_index]));
			break;
		};
		case QueueChoice::Compute: {
			vk_check(vkQueueSubmit(GraphicsContext::the().compute_queue(), 1, &submit_info, fences[frame_index]));
			break;
		};
		};

		vk_check(vkWaitForFences(GraphicsContext::the().device(), 1, &fences[frame_index], VK_TRUE, UINT64_MAX));

		const auto has_callbacks = !destruction_callbacks.empty();
		if (!has_callbacks)
			return;

		while (!destruction_callbacks.empty()) {
			std::function<void(Allocator&)> cb = destruction_callbacks.front();
			destruction_callbacks.pop();
			cb(*allocator);
		}
	}

	std::uint32_t CommandBuffer::get_buffer_index() { return Renderer::current_frame(); }

	std::uint32_t ImmediateCommandBuffer::get_buffer_index()
	{
		assert_that(buffers.size() == 1, "Immediate mode buffer should NEVER have more than one entry.");
		return 0;
	}

} // namespace Alabaster
