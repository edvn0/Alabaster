#pragma once

#include <functional>

namespace Alabaster {

	class RenderQueue {
		using Buffer = uint8_t*;
		using BufferPtr = uint8_t*;
		using BufferCount = uint32_t;

	public:
		using RenderFunction = std::function<void(void*)>;

		RenderQueue();
		~RenderQueue();

		void* allocate(RenderFunction&& func, uint32_t size);
		void execute();

		BufferCount count() const { return command_count; }

	private:
		Buffer command_buffer;
		BufferPtr command_buffer_ptr;
		BufferCount command_count = 0;
	};

} // namespace Alabaster
