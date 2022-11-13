#include "av_pch.hpp"

#include "graphics/IndexBuffer.hpp"

#include "core/Logger.hpp"
#include "core/Utilities.hpp"
#include "graphics/Allocator.hpp"
#include "graphics/GraphicsContext.hpp"
#include "graphics/Renderer.hpp"

#include <memory>
#include <vulkan/vulkan_core.h>

namespace Alabaster {

	IndexBuffer::IndexBuffer(uint32_t count)
		: buffer_size(count * sizeof(uint32_t))
		, buffer_count(count)
	{
		index_data.allocate(buffer_size);

		Allocator allocator("IndexBuffer");

		VkBufferCreateInfo buffer_create_info = {};
		buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_create_info.size = buffer_size;
		buffer_create_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

		memory_allocation = allocator.allocate_buffer(buffer_create_info, VMA_MEMORY_USAGE_CPU_TO_GPU, vulkan_buffer);
	}

	IndexBuffer::IndexBuffer(const void* data, uint32_t count)
		: buffer_size(count * sizeof(uint32_t))
		, buffer_count(count)
	{
		index_data = Buffer::copy(data, buffer_size);
		Allocator allocator("IndexBuffer");

		VkBufferCreateInfo buffer_create_info {};
		buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_create_info.size = buffer_size;
		buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		VkBuffer staging_buffer;
		VmaAllocation staging_buffer_allocation = allocator.allocate_buffer(buffer_create_info, VMA_MEMORY_USAGE_CPU_TO_GPU, staging_buffer);

		auto* dest = allocator.map_memory<uint32_t*>(staging_buffer_allocation);
		std::memcpy(dest, index_data.data, index_data.size);
		allocator.unmap_memory(staging_buffer_allocation);

		VkBufferCreateInfo vertex_buffer_create_info = {};
		vertex_buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		vertex_buffer_create_info.size = buffer_size;
		vertex_buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		memory_allocation = allocator.allocate_buffer(vertex_buffer_create_info, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, vulkan_buffer);

		auto copy_command = GraphicsContext::the().get_command_buffer();

		VkBufferCopy copy_region = {};
		copy_region.size = index_data.size;
		vkCmdCopyBuffer(copy_command, staging_buffer, vulkan_buffer, 1, &copy_region);

		GraphicsContext::the().flush_command_buffer(copy_command);

		allocator.destroy_buffer(staging_buffer, staging_buffer_allocation);

		const auto human_readable_size = Utilities::human_readable_size(buffer_size);
		Log::info("[IndexBuffer] Initialised with size: {}", human_readable_size);
	}

	void IndexBuffer::destroy()
	{
		VkBuffer buffer = vulkan_buffer;
		VmaAllocation allocation = memory_allocation;
		Allocator allocator("IndexBuffer");
		allocator.destroy_buffer(buffer, allocation);

		index_data.release();
		destroyed = true;
	}

	void IndexBuffer::set_data(const void* buffer, uint32_t size, uint32_t offset)
	{
		std::memcpy(index_data.data, (uint8_t*)buffer + offset, size);
		offline_set_data(index_data.data, size, offset);
	}

	void IndexBuffer::offline_set_data(const void* buffer, uint32_t size, uint32_t offset)
	{
		Allocator allocator("IndexBuffer");
		auto* data_pointer = allocator.map_memory<uint8_t>(memory_allocation);
		memcpy(data_pointer, (uint8_t*)buffer + offset, size);
		allocator.unmap_memory(memory_allocation);
	}

} // namespace Alabaster
