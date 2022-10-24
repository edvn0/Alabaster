#pragma once

#include "graphics/Buffer.hpp"

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace Alabaster {

	class IndexBuffer {
	public:
		explicit IndexBuffer(uint32_t count);

		IndexBuffer(const void* data, uint32_t count);

		~IndexBuffer()
		{
			if (!destroyed)
				destroy();
		}

		void destroy();

		uint32_t count() const { return buffer_count; };

		VkBuffer get_vulkan_buffer() const { return vulkan_buffer; }

	private:
		Buffer index_data;
		VmaAllocation memory_allocation;

		uint32_t buffer_size { 0 };
		uint32_t buffer_count { 0 };

		VkBuffer vulkan_buffer { nullptr };

		bool destroyed { false };
	};

} // namespace Alabaster
