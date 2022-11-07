#include "av_pch.hpp"

#include "graphics/Renderer.hpp"

#include "core/Application.hpp"
#include "core/Common.hpp"
#include "graphics/Camera.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/Swapchain.hpp"
#include "utilities/FileInputOutput.hpp"

namespace Alabaster {

	static std::unordered_map<std::string, Shader> shaders;

	static bool renderer_is_initialized { false };
	static bool scene_renderer_is_initialized { false };
	static bool frame_started { false };

	RenderQueue& Renderer::render_queue()
	{
		static RenderQueue render_queue;
		return render_queue;
	}

	void Renderer::execute()
	{
		verify(renderer_is_initialized, "Renderer should be initialized.");
		render_queue().execute();
	}

	void Renderer::begin()
	{
		verify(!frame_started);
		frame_started = true;

		Renderer::submit([] { Log::info("[Renderer] Begin frame."); });
	}

	void Renderer::begin_render_pass(const CommandBuffer& buffer, VkRenderPass render_pass)
	{
		Renderer::submit(
			[render_pass, &buffer] {
				const auto& swapchain = Application::the().swapchain();
				const auto extent = swapchain.swapchain_extent();

				VkRenderPassBeginInfo render_pass_info {};
				render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				render_pass_info.renderPass = render_pass;
				render_pass_info.framebuffer = swapchain.get_current_framebuffer();
				render_pass_info.renderArea.offset = { 0, 0 };
				render_pass_info.renderArea.extent = extent;

				std::array<VkClearValue, 2> clear_values {};
				clear_values[0].color = { 0, 0, 0, 0 };
				clear_values[1].depthStencil = { .depth = 1.0f, .stencil = 0 };

				render_pass_info.clearValueCount = clear_values.size();
				render_pass_info.pClearValues = clear_values.data();

				vkCmdBeginRenderPass(*buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

				VkViewport viewport {};
				viewport.x = 0.0f;
				viewport.y = static_cast<float>(extent.height);
				viewport.width = static_cast<float>(extent.width);
				viewport.height = -static_cast<float>(extent.height);
				viewport.minDepth = 0.0f;
				viewport.maxDepth = 1.0f;
				vkCmdSetViewport(*buffer, 0, 1, &viewport);

				VkRect2D scissor {};
				scissor.offset = { 0, 0 };
				scissor.extent = extent;
				vkCmdSetScissor(*buffer, 0, 1, &scissor);
			},
			"Begin Renderpass");
	}

	void Renderer::end_render_pass(const CommandBuffer& buffer)
	{
		Renderer::submit([&buffer] { vkCmdEndRenderPass(*buffer); }, "End Renderpass");
	}

	uint32_t Renderer::current_frame() { return Application::the().swapchain().frame(); }

	void Renderer::end()
	{
		verify(frame_started);
		frame_started = false;

		Renderer::submit([] { Log::info("[Renderer] End frame."); });
	}

	void Renderer::init()
	{
		Log::info("[Renderer] Initialisation of renderer.");

		renderer_is_initialized = true;

		const auto all_files_in_shaders = IO::in_directory("app/resources/shaders", { ".spv" });

		for (const auto& shader : all_files_in_shaders) {
			Log::info("[Renderer] Shader found: {}", shader.filename());
		}
	}

	void Renderer::shutdown()
	{
		Log::info("[Renderer] Destruction of renderer.");
		// Destruction code.
	}

} // namespace Alabaster
