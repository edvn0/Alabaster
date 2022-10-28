#pragma once

#include "core/Logger.hpp"
#include "graphics/RenderQueue.hpp"

#include <concepts>
#include <glm/glm.hpp>

typedef struct VkPipeline_T* VkPipeline;
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
		static void end();

	public:
		template <TriviallyDestructible CommandBufferFunction> static void submit(CommandBufferFunction&& func)
		{
			Renderer::submit(std::move(func), {});
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

		static void execute();

	private:
		static RenderQueue& render_queue();
	};

} // namespace Alabaster
