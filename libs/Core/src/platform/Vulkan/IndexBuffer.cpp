#include "av_pch.hpp"

#include "graphics/IndexBuffer.hpp"

#include "core/Logger.hpp"
#include "core/Utilities.hpp"
#include "graphics/Allocator.hpp"
#include "graphics/CommandBuffer.hpp"
#include "graphics/GraphicsContext.hpp"
#include "graphics/Renderer.hpp"

#include <memory>
#include <vulkan/vulkan.h>

namespace Alabaster {

	IndexBuffer::IndexBuffer(std::uint32_t count)
		: buffer_size(count * sizeof(std::uint32_t))
		, buffer_count(count)
	{
		index_data.allocate(buffer_size);

		Allocator allocator("IndexBuffer");

		VkBufferCreateInfo buffer_create_info = {};
		buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_create_info.size = buffer_size;
		buffer_create_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

		memory_allocation = allocator.allocate_buffer(buffer_create_info, Allocator::Usage::CPU_TO_GPU, vulkan_buffer);
	}

	IndexBuffer::IndexBuffer(const void* data, std::uint32_t count)
		: buffer_size(count * sizeof(std::uint32_t))
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
		VmaAllocation staging_buffer_allocation = allocator.allocate_buffer(buffer_create_info, Allocator::Usage::CPU_TO_GPU, staging_buffer);

		auto* dest = allocator.map_memory<std::uint32_t*>(staging_buffer_allocation);
		std::memcpy(dest, index_data.data, index_data.size);
		allocator.unmap_memory(staging_buffer_allocation);

		VkBufferCreateInfo vertex_buffer_create_info = {};
		vertex_buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		vertex_buffer_create_info.size = buffer_size;
		vertex_buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		memory_allocation = allocator.allocate_buffer(vertex_buffer_create_info, Allocator::Usage::AUTO_PREFER_DEVICE, vulkan_buffer);

		ImmediateCommandBuffer immediate_command_buffer { "Index Buffer",
			[staging_buffer, staging_buffer_allocation](Allocator& alloc) { alloc.destroy_buffer(staging_buffer, staging_buffer_allocation); } };

		VkBufferCopy copy_region = {};
		copy_region.size = index_data.size;
		vkCmdCopyBuffer(immediate_command_buffer.get_buffer(), staging_buffer, vulkan_buffer, 1, &copy_region);

		const auto human_readable_size = Utilities::human_readable_size(buffer_size);
		Log::info("[IndexBuffer] Initialised with size: {}", human_readable_size);
	}

	void IndexBuffer::destroy()
	{
		if (!vulkan_buffer)
			return;

		VkBuffer buffer = vulkan_buffer;
		VmaAllocation allocation = memory_allocation;
		Allocator allocator("IndexBuffer");
		allocator.destroy_buffer(buffer, allocation);

		index_data.release();
		destroyed = true;
	}

	void IndexBuffer::set_data(const void* buffer, std::uint32_t size, std::uint32_t offset)
	{
		std::memcpy(index_data.data, (uint8_t*)buffer + offset, size);
		offline_set_data(index_data.data, size, offset);
	}

	VkBuffer IndexBuffer::get_vulkan_buffer() const { return vulkan_buffer; }

	VkBuffer IndexBuffer::operator*() const { return vulkan_buffer; }

	void IndexBuffer::offline_set_data(const void* buffer, std::uint32_t size, std::uint32_t offset)
	{
		Allocator allocator("IndexBuffer");
		auto* data_pointer = allocator.map_memory<uint8_t>(memory_allocation);
		memcpy(data_pointer, (uint8_t*)buffer + offset, size);
		allocator.unmap_memory(memory_allocation);
	}

} // namespace Alabaster
