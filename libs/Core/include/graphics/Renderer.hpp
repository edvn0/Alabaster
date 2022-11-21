#pragma once

#include "graphics/RenderQueue.hpp"

typedef struct VkPipeline_T* VkPipeline;
typedef struct VkRenderPass_T* VkRenderPass;
typedef struct VkPipelineLayout_T* VkPipelineLayout;

namespace Alabaster {

	class SceneRenderer;
	class Mesh;
	class Camera;
	class EditorCamera;
	class Pipeline;
	class CommandBuffer;

	class Renderer {
	public:
		static void init();
		static void shutdown();

	public:
		static void begin();
		static void begin_render_pass(const std::unique_ptr<CommandBuffer>& buffer, VkRenderPass render_pass, bool explicit_clear = false);
		static void end_render_pass(const std::unique_ptr<CommandBuffer>& buffer);
		static void end();

		static std::uint32_t current_frame();

	public:
		static RenderQueue& resource_release_queue(std::uint32_t index);

	private:
		static RenderQueue& render_queue();
	};

} // namespace Alabaster
