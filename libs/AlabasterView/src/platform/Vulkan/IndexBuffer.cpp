#include "av_pch.hpp"

#include "graphics/IndexBuffer.hpp"

#include "core/Common.hpp"
#include "core/Logger.hpp"
#include "core/Utilities.hpp"
#include "graphics/Allocator.hpp"
#include "graphics/GraphicsContext.hpp"

#include <vulkan/vulkan.h>

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

		memory_allocation = allocator.allocate_buffer(buffer_create_info, VMA_MEMORY_USAGE_GPU_ONLY, vulkan_buffer);
	}

	IndexBuffer::IndexBuffer(const void* data, uint32_t count)
		: buffer_size(count * sizeof(uint32_t))
		, buffer_count(count)
	{
		index_data = Buffer::copy(data, buffer_size);

		Allocator allocator("IndexBuffer");

		VkBufferCreateInfo staging_buffer_create_info {};
		staging_buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		staging_buffer_create_info.size = buffer_size;
		staging_buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		staging_buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkBuffer staging_buffer;
		VmaAllocation staging_buffer_allocation = allocator.allocate_buffer(staging_buffer_create_info, VMA_MEMORY_USAGE_CPU_TO_GPU, staging_buffer);

		auto* dest = allocator.map_memory<uint32_t*>(staging_buffer_allocation);
		std::memcpy(dest, index_data.data, index_data.size);
		allocator.unmap_memory(staging_buffer_allocation);

		VkBufferCreateInfo index_buffer_create_info = {};
		index_buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		index_buffer_create_info.size = buffer_size;
		index_buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		memory_allocation = allocator.allocate_buffer(index_buffer_create_info, VMA_MEMORY_USAGE_GPU_ONLY, vulkan_buffer);

		auto copy_command = GraphicsContext::the().get_command_buffer();

		VkBufferCopy copy_region = {};
		copy_region.size = index_data.size;
		vkCmdCopyBuffer(copy_command, staging_buffer, vulkan_buffer, 1, &copy_region);

		GraphicsContext::the().flush_command_buffer(copy_command);

		allocator.destroy_buffer(staging_buffer, staging_buffer_allocation);
		index_data.release();

		auto human_readable_size = Utilities::human_readable_size(buffer_size);
		Log::info("[IndexBuffer] Initialised with size: {}", human_readable_size);
	}

	void IndexBuffer::destroy()
	{
		Allocator allocator("IndexBuffer");
		allocator.destroy_buffer(vulkan_buffer, memory_allocation);

		index_data.release();
		Log::info("[IndexBuffer] Destroying via VMA.");
		destroyed = true;
	};

} // namespace Alabaster
