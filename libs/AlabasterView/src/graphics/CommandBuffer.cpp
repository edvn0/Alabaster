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
		VkCommandPoolCreateInfo cmdPoolInfo = {};
		cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolInfo.queueFamilyIndex = GraphicsContext::the().graphics_queue_family();
		cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		vk_check(vkCreateCommandPool(GraphicsContext::the().device(), &cmdPoolInfo, nullptr, &pool));

		VkCommandBufferAllocateInfo cbai {};
		cbai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cbai.commandPool = pool;
		cbai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		if (count == 0)
			frames = Application::the().swapchain().get_image_count();
		cbai.commandBufferCount = frames;
		buffers.resize(frames);
		vk_check(vkAllocateCommandBuffers(GraphicsContext::the().device(), &cbai, buffers.data()));

		VkFenceCreateInfo fenceCreateInfo {};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		fences.resize(frames);
		for (int i = 0; i < frames; i++) {
			vk_check(vkCreateFence(GraphicsContext::the().device(), &fenceCreateInfo, nullptr, &fences[i]));
		}
	}

	CommandBuffer::CommandBuffer(std::string_view name)
	{
		name = std::string { name };
		init(3);
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
				uint32_t frameIndex = Renderer::current_frame();

				VkCommandBufferBeginInfo cmdBufInfo = {};
				cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				cmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
				cmdBufInfo.pNext = nullptr;

				if (owned_by_swapchain) {
					active = Application::the().swapchain().get_drawbuffer(frameIndex);
				} else {
					active = buffers[frameIndex];
				}

				vk_check(vkBeginCommandBuffer(active, &cmdBufInfo));
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
			uint32_t frameIndex = Renderer::current_frame();

			VkSubmitInfo submitInfo {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			VkCommandBuffer commandBuffer = buffers[frameIndex];
			submitInfo.pCommandBuffers = &commandBuffer;

			vk_check(vkWaitForFences(GraphicsContext::the().device(), 1, &fences[frameIndex], VK_TRUE, UINT64_MAX));
			vk_check(vkResetFences(GraphicsContext::the().device(), 1, &fences[frameIndex]));
			vk_check(vkQueueSubmit(GraphicsContext::the().graphics_queue(), 1, &submitInfo, fences[frameIndex]));
		});
	}

} // namespace Alabaster
