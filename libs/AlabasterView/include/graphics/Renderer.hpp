#pragma once

#include "core/Logger.hpp"
#include "graphics/CommandBuffer.hpp"
#include "graphics/RenderQueue.hpp"

#include <concepts>
#include <glm/glm.hpp>

typedef struct VkPipeline_T* VkPipeline;
typedef struct VkRenderPass_T* VkRenderPass;
typedef struct VkPipelineLayout_T* VkPipelineLayout;

namespace Alabaster {

	template <typename T> using ConstUnique = const std::unique_ptr<T>&;

	template <typename T>
	concept TriviallyDestructible = std::is_trivially_destructible_v<T>;

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
		template <TriviallyDestructible CommandBufferFunction> static void submit(CommandBufferFunction&& func)
		{
			Renderer::submit(std::forward<CommandBufferFunction>(func), {});
		}

		template <TriviallyDestructible CommandBufferFunction> static void submit(CommandBufferFunction&& func, std::string_view message)
		{
			auto render_command = [message](void* function_ptr) {
				const auto& this_function = *static_cast<CommandBufferFunction*>(function_ptr);
				if (!message.empty()) {
					Log::info("[Renderer - Command] {}", message);
				}
				this_function();
			};

			auto storage_buffer = Renderer::render_queue().allocate(render_command, sizeof(func));
			new (storage_buffer) CommandBufferFunction(std::forward<CommandBufferFunction>(func));
		}

		template <TriviallyDestructible ResourceFreeFunction> static void free_resource(ResourceFreeFunction&& func)
		{
			auto command = [](void* function_ptr) {
				const auto& this_function = *static_cast<ResourceFreeFunction*>(function_ptr);
				this_function();
			};

			Renderer::submit([command, func]() {
				const uint32_t index = Renderer::current_frame();
				auto storage_buffer = Renderer::resource_release_queue(index).allocate(command, sizeof(func));
				new (storage_buffer) ResourceFreeFunction(std::forward<ResourceFreeFunction>((ResourceFreeFunction &&) func));
			});
		}

		static void execute();
		static RenderQueue& resource_release_queue(uint32_t index);

	private:
		static RenderQueue& render_queue();
	};

} // namespace Alabaster
