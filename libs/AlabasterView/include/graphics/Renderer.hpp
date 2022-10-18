#pragma once

#include "graphics/RenderQueue.hpp"

#include <concepts>

namespace Alabaster {

	template <typename T>
	concept TriviallyDestructible = std::is_trivially_destructible_v<T>;

	class Renderer {
	public:
		static void begin();
		static void end();

	public:
		template <TriviallyDestructible CommandBufferFunction> static void submit(CommandBufferFunction&& func)
		{

			auto render_command = [](void* function_ptr) {
				auto this_function = *static_cast<CommandBufferFunction*>(function_ptr);
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
