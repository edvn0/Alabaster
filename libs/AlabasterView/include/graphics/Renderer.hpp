#pragma once

#include "core/Logger.hpp"
#include "core/Utilities.hpp"
#include "graphics/CommandBuffer.hpp"
#include "graphics/RenderQueue.hpp"

#include <concepts>
#include <glm/glm.hpp>

typedef struct VkPipeline_T* VkPipeline;
typedef struct VkRenderPass_T* VkRenderPass;
typedef struct VkPipelineLayout_T* VkPipelineLayout;

namespace Alabaster {

	template <typename T> using ConstUnique = const std::unique_ptr<T>&;

	class SceneRenderer;
	class Mesh;
	class Camera;
	class EditorCamera;
	class Pipeline;

	class Renderer {
	public:
		static void init();
		static void shutdown();

	public:
		static void begin();
		static void begin_render_pass(const CommandBuffer& buffer, VkRenderPass render_pass);
		static void end_render_pass(const CommandBuffer& buffer);
		static void end();

		static uint32_t current_frame();

	public:
		static RenderQueue& resource_release_queue(uint32_t index);

	private:
		static RenderQueue& render_queue();
	};

} // namespace Alabaster
