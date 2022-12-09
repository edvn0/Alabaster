#include "av_pch.hpp"

#include "graphics/VertexBuffer.hpp"

#include "core/Logger.hpp"
#include "core/Utilities.hpp"
#include "graphics/Allocator.hpp"
#include "graphics/CommandBuffer.hpp"
#include "graphics/GraphicsContext.hpp"
#include "graphics/Renderer.hpp"

#include <memory>

namespace Alabaster {

	VertexBuffer::VertexBuffer(std::uint32_t size)
		: buffer_size(size)
	{
		vertex_data.allocate(buffer_size);

		Allocator allocator("VertexBuffer");

		VkBufferCreateInfo buffer_create_info = {};
		buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_create_info.size = buffer_size;
		buffer_create_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

		memory_allocation = allocator.allocate_buffer(buffer_create_info, VMA_MEMORY_USAGE_CPU_TO_GPU, vulkan_buffer);
	}

	VertexBuffer::VertexBuffer(const void* data, std::size_t size)
		: VertexBuffer(data, static_cast<std::uint32_t>(size))
	{
	}

	VertexBuffer::VertexBuffer(const void* data, std::uint32_t size)
		: buffer_size(size)
	{
		vertex_data = Buffer::copy(data, size);
		Allocator allocator("VertexBuffer");

		VkBufferCreateInfo buffer_create_info {};
		buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_create_info.size = buffer_size;
		buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		VkBuffer staging_buffer;
		VmaAllocation staging_buffer_allocation = allocator.allocate_buffer(buffer_create_info, VMA_MEMORY_USAGE_CPU_TO_GPU, staging_buffer);

		auto* dest = allocator.map_memory<uint8_t>(staging_buffer_allocation);
		std::memcpy(dest, vertex_data.data, buffer_size);
		allocator.unmap_memory(staging_buffer_allocation);

		VkBufferCreateInfo vertex_buffer_create_info = {};
		vertex_buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		vertex_buffer_create_info.size = buffer_size;
		vertex_buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		memory_allocation = allocator.allocate_buffer(vertex_buffer_create_info, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, vulkan_buffer);

		ImmediateCommandBuffer immediate_command_buffer;
		immediate_command_buffer.add_destruction_callback([staging_buffer, staging_buffer_allocation](Allocator& allocator) {
			allocator.destroy_buffer(staging_buffer, staging_buffer_allocation);
		});

		VkBufferCopy copy_region = {};
		copy_region.size = buffer_size;
		vkCmdCopyBuffer(immediate_command_buffer, staging_buffer, vulkan_buffer, 1, &copy_region);

		const auto human_readable_size = Utilities::human_readable_size(buffer_size);
		Log::info("[VertexBuffer] Initialised with size: {}", human_readable_size);
	}

	void VertexBuffer::destroy()
	{
		if (!vulkan_buffer)
			return;

		VkBuffer buffer = vulkan_buffer;
		VmaAllocation allocation = memory_allocation;
		Allocator allocator("VertexBuffer");
		allocator.destroy_buffer(buffer, allocation);

		vertex_data.release();
		destroyed = true;
	}

	void VertexBuffer::set_data(const void* buffer, std::uint32_t size, std::uint32_t offset)
	{
		std::memcpy(vertex_data.data, (uint8_t*)buffer + offset, size);
		offline_set_data(vertex_data.data, size, offset);
	}

	void VertexBuffer::offline_set_data(const void* buffer, std::uint32_t size, std::uint32_t offset)
	{
		Allocator allocator("VertexBuffer");
		auto* data_pointer = allocator.map_memory<uint8_t>(memory_allocation);
		memcpy(data_pointer, (uint8_t*)buffer + offset, size);
		allocator.unmap_memory(memory_allocation);
	}

} // namespace Alabaster
