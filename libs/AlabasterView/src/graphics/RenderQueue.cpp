#include "av_pch.hpp"

#include "graphics/RenderQueue.hpp"

#include "core/CPUProfiler.hpp"
#include "core/Logger.hpp"

namespace Alabaster {

	static constexpr auto buffer_size = 10 * 1024 * 1024;
	using byte = unsigned char;

	RenderQueue::RenderQueue()
	{
		command_buffer = new uint8_t[buffer_size];
		command_buffer_ptr = command_buffer;
		std::memset(command_buffer, 0, buffer_size);
	}

	RenderQueue::~RenderQueue() { delete[] command_buffer; }

	void* RenderQueue::allocate(RenderFunction&& func, BufferCount size)
	{
		*(RenderFunction*)command_buffer_ptr = func;
		command_buffer_ptr += sizeof(RenderFunction);

		*(uint32_t*)command_buffer_ptr = size;
		command_buffer_ptr += sizeof(uint32_t);

		void* memory = command_buffer_ptr;
		command_buffer_ptr += size;

		command_count++;
		return memory;
	}

	void RenderQueue::execute()
	{
		Log::debug("[RenderQueue] -- [{0} command(s): {1} bytes]", command_count, (command_buffer_ptr - command_buffer));

		CPUProfiler<double> profiler("RenderQueue-execute");
		byte* buffer = command_buffer;

		for (uint32_t i = 0; i < command_count; i++) {
			RenderFunction function = *(RenderFunction*)buffer;
			buffer += sizeof(RenderFunction);

			BufferCount size = *(BufferCount*)buffer;
			buffer += sizeof(BufferCount);
			function(buffer);
			buffer += size;
		}

		command_buffer_ptr = command_buffer;
		command_count = 0;
	}

} // namespace Alabaster
