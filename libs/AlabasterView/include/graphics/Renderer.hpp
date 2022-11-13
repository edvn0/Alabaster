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
		/*template <typename CommandBufferFunction> static void submit(CommandBufferFunction&& func) { Renderer::submit(std::move(func), {}); }

		template <typename CommandBufferFunction> static void submit(CommandBufferFunction&& func, std::string_view message)
		{
			auto render_command = [message](void* function_ptr) {
				auto* this_function = (CommandBufferFunction*)function_ptr;
				if (!message.empty()) {
					Log::info("[Renderer - Command] {}", message);
				}
				(*this_function)();
				this_function->~CommandBufferFunction();
			};

			auto storage_buffer = Renderer::render_queue().allocate(render_command, sizeof(func));
			new (storage_buffer) CommandBufferFunction(std::forward<CommandBufferFunction>(std::move(func)));
		}

		template <typename ResourceFreeFunction> static void free_resource(ResourceFreeFunction&& func, std::string_view message = "")
		{
			auto command = [message](void* function_ptr) {
				auto this_function = (ResourceFreeFunction*)function_ptr;
				(*this_function)();
				if (!message.empty()) {
					Log::info("[Renderer - Free - Command] {}", message);
				}
				this_function->~ResourceFreeFunction();
			};

			submit(
				[command, func] {
					Log::info("Sizeof render free submission {}", Utilities::human_readable_size<Utilities::OutputSize::B>(sizeof(func)));
					const uint32_t index = Renderer::current_frame();
					auto* storage_buffer = Renderer::resource_release_queue(index).allocate(command, sizeof(func));
					new (storage_buffer) ResourceFreeFunction(std::forward<ResourceFreeFunction>((ResourceFreeFunction &&) func));
				},
				"Destroying entity");
		}*/

		static void execute();
		static RenderQueue& resource_release_queue(uint32_t index);

	private:
		static RenderQueue& render_queue();
	};

} // namespace Alabaster
