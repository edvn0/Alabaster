#pragma once

#include <string>

#define VMA_DEBUG_LOG(x, ...) std::printf(x, __VA_ARGS__);

#include <vk_mem_alloc.h>

extern "C" {
VkResult vmaMapMemory(VmaAllocator allocator, VmaAllocation allocation, void** p_data);
}

namespace Alabaster {

	class Allocator {
	public:
		enum class Usage : int {
			UNKNOWN = 0,
			GPU_ONLY = 1,
			CPU_ONLY = 2,
			CPU_TO_GPU = 3,
			GPU_TO_CPU = 4,
			CPU_COPY = 5,
			GPU_LAZILY_ALLOCATED = 6,
			AUTO = 7,
			AUTO_PREFER_DEVICE = 8,
			AUTO_PREFER_HOST = 9,
		};
		enum class Creation : std::uint32_t {
			DEDICATED_MEMORY_BIT = 0x00000001,
			NEVER_ALLOCATE_BIT = 0x00000002,
			MAPPED_BIT = 0x00000004,
			USER_DATA_COPY_STRING_BIT = 0x00000020,
			UPPER_ADDRESS_BIT = 0x00000040,
			DONT_BIND_BIT = 0x00000080,
			WITHIN_BUDGET_BIT = 0x00000100,
			CAN_ALIAS_BIT = 0x00000200,
			HOST_ACCESS_SEQUENTIAL_WRITE_BIT = 0x00000400,
			HOST_ACCESS_RANDOM_BIT = 0x00000800,
			HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT = 0x00001000,
			STRATEGY_MIN_MEMORY_BIT = 0x00010000,
			STRATEGY_MIN_TIME_BIT = 0x00020000,
			STRATEGY_MIN_OFFSET_BIT = 0x00040000,
			STRATEGY_BEST_FIT_BIT = STRATEGY_MIN_MEMORY_BIT,
			STRATEGY_FIRST_FIT_BIT = STRATEGY_MIN_TIME_BIT,
			STRATEGY_MASK = STRATEGY_MIN_MEMORY_BIT | STRATEGY_MIN_TIME_BIT | STRATEGY_MIN_OFFSET_BIT,
		};

		Allocator() = default;
		explicit Allocator(const std::string& tag);
		~Allocator();

		VmaAllocation allocate_buffer(VkBufferCreateInfo bci, Usage usage, Creation flags, VkBuffer& out_buffer);
		VmaAllocation allocate_buffer(VkBufferCreateInfo bci, Usage usage, VkBuffer& out_buffer);
		VmaAllocation allocate_image(VkImageCreateInfo ici, Usage usage, Creation flags, VkImage& out_image);
		VmaAllocation allocate_image(VkImageCreateInfo ici, Usage usage, VkImage& out_image);
		void free(VmaAllocation allocation);
		void destroy_image(VkImage image, VmaAllocation allocation);
		void destroy_buffer(VkBuffer buffer, VmaAllocation allocation);

		template <class T> T* map_memory(VmaAllocation allocation);

		void unmap_memory(VmaAllocation allocation);

		void set_tag(std::string name) { tag = std::move(name); }

		static void shutdown();

		static VmaAllocator& get_vma_allocator();

	private:
		std::string tag;
	};

	template <class T> T* Allocator::map_memory(VmaAllocation allocation)
	{
		T* mapped_memory;
		vmaMapMemory(Allocator::get_vma_allocator(), allocation, (void**)&mapped_memory);
		return mapped_memory;
	}

} // namespace Alabaster
