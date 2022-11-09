#include "av_pch.hpp"

#include "graphics/CommandBuffer.hpp"

#include "core/Application.hpp"
#include "graphics/GraphicsContext.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/Swapchain.hpp"

#include <utility>

namespace Alabaster {

	void CommandBuffer::init(uint32_t count)
	{
		uint32_t frames = count;
		VkCommandPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		pool_info.queueFamilyIndex = GraphicsContext::the().graphics_queue_family();
		pool_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		vk_check(vkCreateCommandPool(GraphicsContext::the().device(), &pool_info, nullptr, &pool));

		VkCommandBufferAllocateInfo cbai {};
		cbai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cbai.commandPool = pool;
		cbai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		if (count == 0)
			frames = Application::the().swapchain().get_image_count();
		cbai.commandBufferCount = frames;
		buffers.resize(frames);
		vk_check(vkAllocateCommandBuffers(GraphicsContext::the().device(), &cbai, buffers.data()));

		VkFenceCreateInfo fence_create_info {};
		fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		fences.resize(frames);
		for (int i = 0; i < frames; i++) {
			vk_check(vkCreateFence(GraphicsContext::the().device(), &fence_create_info, nullptr, &fences[i]));
		}
	}

	CommandBuffer::CommandBuffer(std::string_view n)
	{
		std::string name = std::string { n };
		init(4);
		owned_by_swapchain = true;
	}

	CommandBuffer::CommandBuffer(uint32_t count, std::string_view debug_name) { init(count); }

	CommandBuffer::~CommandBuffer()
	{
		if (owned_by_swapchain)
			return;

		const auto& device = GraphicsContext::the().device();
		vkDestroyCommandPool(device, pool, nullptr);
	}

	void CommandBuffer::begin()
	{
		Renderer::submit(
			[this] {
				uint32_t frame_index = Renderer::current_frame();

				VkCommandBufferBeginInfo begin_info = {};
				begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
				begin_info.pNext = nullptr;

				if (owned_by_swapchain) {
					active = Application::the().swapchain().get_drawbuffer(frame_index);
				} else {
					active = buffers[frame_index];
				}

				vk_check(vkBeginCommandBuffer(active, &begin_info));
			},
			"Begin Command Buffer");
	}

	void CommandBuffer::end()
	{
		Renderer::submit(
			[this] {
				vk_check(vkEndCommandBuffer(active));
				active = nullptr;
			},
			"End Command Buffer");
	}

	void CommandBuffer::submit()
	{
		if (owned_by_swapchain)
			return;

		Renderer::submit([this] {
			uint32_t frame_index = Renderer::current_frame();

			VkSubmitInfo submitInfo {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			VkCommandBuffer commandBuffer = buffers[frame_index];
			submitInfo.pCommandBuffers = &commandBuffer;

			vk_check(vkWaitForFences(GraphicsContext::the().device(), 1, &fences[frame_index], VK_TRUE, UINT64_MAX));
			vk_check(vkResetFences(GraphicsContext::the().device(), 1, &fences[frame_index]));
			vk_check(vkQueueSubmit(GraphicsContext::the().graphics_queue(), 1, &submitInfo, fences[frame_index]));
		});
	}

} // namespace Alabaster
