#pragma once

#include "graphics/Buffer.hpp"

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace Alabaster {

	class IndexBuffer {
	public:
		explicit IndexBuffer(uint32_t size);

		IndexBuffer(const void* data, uint32_t size);

		~IndexBuffer()
		{
			if (!destroyed)
				destroy();
		}

		void destroy();

		VkBuffer get_vulkan_buffer() const { return vulkan_buffer; }

	private:
		Buffer index_data;
		uint32_t buffer_size;
		VmaAllocation memory_allocation;
		VkBuffer vulkan_buffer { nullptr };

		bool destroyed { false };
	};

} // namespace Alabaster
