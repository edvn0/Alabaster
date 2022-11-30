#include "av_pch.hpp"

#include "graphics/Renderer.hpp"

#include "core/Application.hpp"
#include "core/Common.hpp"
#include "graphics/Camera.hpp"
#include "graphics/CommandBuffer.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/Swapchain.hpp"
#include "utilities/FileInputOutput.hpp"
#include "utilities/FileSystem.hpp"
#include "vulkan/vulkan_core.h"

namespace Alabaster {

	static RenderQueue global_release_queues[3];

	static bool frame_started { false };

	RenderQueue& Renderer::render_queue()
	{
		static RenderQueue render_queue;
		return render_queue;
	}

	RenderQueue& Renderer::resource_release_queue(std::uint32_t index) { return global_release_queues[index]; }

	void Renderer::begin()
	{
		verify(!frame_started);
		frame_started = true;
	}

	void Renderer::begin_render_pass(const std::unique_ptr<CommandBuffer>& buffer, VkRenderPass render_pass, bool explicit_clear)
	{
		const auto& swapchain = Application::the().swapchain();
		const auto extent = swapchain.swapchain_extent();

		VkRenderPassBeginInfo render_pass_info {};
		render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_info.renderPass = render_pass;
		render_pass_info.framebuffer = swapchain.get_current_framebuffer();
		render_pass_info.renderArea.offset = { 0, 0 };
		render_pass_info.renderArea.extent = extent;

		std::array<VkClearValue, 2> clear_values {};
		clear_values[0].color = { { 0, 0, 0, 0 } };
		clear_values[1].depthStencil = { .depth = 1.0f, .stencil = 0 };

		render_pass_info.clearValueCount = static_cast<std::uint32_t>(clear_values.size());
		render_pass_info.pClearValues = clear_values.data();
		verify(*buffer, "[Renderer - Begin Render Pass] Command buffer is not active.");
		vkCmdBeginRenderPass(*buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

		if (explicit_clear) {
			std::array<VkClearAttachment, 2> clears;
			auto& colour = clears[0];
			colour.clearValue = clear_values[0];
			colour.colorAttachment = 0;
			colour.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

			auto& depth = clears[1];
			depth.clearValue = clear_values[1];
			depth.colorAttachment = 1;
			depth.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

			VkClearRect clear_rect;
			clear_rect.rect = render_pass_info.renderArea;
			clear_rect.baseArrayLayer = 0;
			clear_rect.layerCount = 1;

			vkCmdClearAttachments(*buffer, 2, clears.data(), 1, &clear_rect);
		}

		VkViewport viewport {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(extent.width);
		viewport.height = static_cast<float>(extent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(*buffer, 0, 1, &viewport);

		VkRect2D scissor {};
		scissor.offset = { 0, 0 };
		scissor.extent = extent;
		vkCmdSetScissor(*buffer, 0, 1, &scissor);
	}

	void Renderer::end_render_pass(const std::unique_ptr<CommandBuffer>& buffer) { vkCmdEndRenderPass(*buffer); }

	std::uint32_t Renderer::current_frame() { return Application::the().swapchain().frame(); }

	void Renderer::end()
	{
		verify(frame_started);
		frame_started = false;
	}

	void Renderer::init() { Log::info("[Renderer] Initialisation of renderer."); }

	void Renderer::shutdown()
	{
		Log::info("[Renderer] Destruction of renderer.");
		for (std::uint32_t i = 0; i < Application::the().swapchain().get_image_count(); i++) {
			auto& queue = Renderer::resource_release_queue(i);
			queue.execute();
		}
	}

} // namespace Alabaster
