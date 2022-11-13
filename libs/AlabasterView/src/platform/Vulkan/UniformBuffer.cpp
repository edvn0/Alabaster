#include "av_pch.hpp"

#include "graphics/UniformBuffer.hpp"

#include "graphics/Allocator.hpp"
#include "graphics/GraphicsContext.hpp"
#include "graphics/Renderer.hpp"

namespace Alabaster {

	UniformBuffer::UniformBuffer(uint32_t size, uint32_t binding)
		: size(size)
		, binding(binding)
	{
		invalidate();
	}

	UniformBuffer::~UniformBuffer() { release(); }

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
		release();

		const auto& device = GraphicsContext::the().device();

		VkBufferCreateInfo buffer_info = {};
		buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		buffer_info.size = size;

		Allocator allocator("UniformBuffer");
		allocation = allocator.allocate_buffer(buffer_info, VMA_MEMORY_USAGE_CPU_TO_GPU, buffer);
	}

	void UniformBuffer::set_data(const void* data, uint32_t input_size, uint32_t offset)
	{
		Allocator allocator("UniformBuffer");
		uint8_t* mapped = allocator.map_memory<uint8_t>(allocation);
		std::memcpy(mapped, (const uint8_t*)data + offset, size);
		allocator.unmap_memory(allocation);
	}

} // namespace Alabaster