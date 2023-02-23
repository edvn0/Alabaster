#include "av_pch.hpp"

#include "graphics/UniformBuffer.hpp"

#include "graphics/Allocator.hpp"
#include "graphics/GraphicsContext.hpp"
#include "graphics/Renderer.hpp"

namespace Alabaster {

	UniformBuffer::UniformBuffer(std::uint32_t in_size, std::uint32_t in_binding)
		: size(in_size)
		, binding(in_binding)
	{
		invalidate();
	}

	void UniformBuffer::destroy()
	{
		destroyed = true;
		release();
	}

	void UniformBuffer::release()
	{
		if (!allocation)
			return;

		Allocator allocator("UniformBuffer");
		allocator.destroy_buffer(buffer, allocation);

		buffer = nullptr;
		allocation = nullptr;
	}

	void UniformBuffer::invalidate()
	{
		if (allocation)
			release();

		VkBufferCreateInfo buffer_info = {};
		buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		buffer_info.size = size;

		Allocator allocator("UniformBuffer");
		allocation = allocator.allocate_buffer(buffer_info, VMA_MEMORY_USAGE_AUTO, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT, buffer);
	}

	void UniformBuffer::set_data(const void* data, std::uint32_t input_size, std::uint32_t offset)
	{
		Allocator allocator("UniformBuffer");
		uint8_t* mapped = allocator.map_memory<uint8_t>(allocation);
		std::memcpy(mapped, (const uint8_t*)data + offset, input_size);
		allocator.unmap_memory(allocation);
	}

} // namespace Alabaster