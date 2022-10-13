#pragma once

#include <string>
#include <vk_mem_alloc.h>

namespace Alabaster {

	class Allocator {
	public:
		Allocator() = default;
		Allocator(const std::string& tag);
		~Allocator();

		VmaAllocation allocate_buffer(VkBufferCreateInfo bci, VmaMemoryUsage usage, VkBuffer& out_buffer);
		VmaAllocation allocate_image(VkImageCreateInfo ici, VmaMemoryUsage usage, VkImage& out_image);
		void free(VmaAllocation allocation);
		void destroy_image(VkImage image, VmaAllocation allocation);
		void destroy_buffer(VkBuffer buffer, VmaAllocation allocation);

		template <typename T> T* map_memory(VmaAllocation allocation)
		{
			T* mapped_memory;
			vmaMapMemory(Allocator::get_vma_allocator(), allocation, (void**)&mapped_memory);
			return mapped_memory;
		}

		void unmap_memory(VmaAllocation allocation);

		static void shutdown();

		static VmaAllocator& get_vma_allocator();

	private:
		std::string tag;
	};

} // namespace Alabaster