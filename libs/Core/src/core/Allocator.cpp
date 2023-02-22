#include "av_pch.hpp"

#include "graphics/Allocator.hpp"

#define VMA_IMPLEMENTATION
#include "core/Common.hpp"
#include "graphics/GraphicsContext.hpp"

namespace Alabaster {

	struct AllocatorData {
		VmaAllocator allocator;
		uint64_t total_allocated_bytes = 0;
	};

	static AllocatorData& vma_data()
	{
		static AllocatorData* data_impl = nullptr;

		if (!data_impl) {
			data_impl = new AllocatorData();
			VmaAllocatorCreateInfo allocator_info = {};
			allocator_info.vulkanApiVersion = VK_API_VERSION_1_1;
			allocator_info.physicalDevice = GraphicsContext::the().physical_device();
			allocator_info.device = GraphicsContext::the().device();
			allocator_info.instance = GraphicsContext::the().instance();

			vk_check(vmaCreateAllocator(&allocator_info, &data_impl->allocator));
		}

		return *data_impl;
	}

	Allocator::Allocator(const std::string& allocator_tag)
		: tag(allocator_tag)
	{
	}

	Allocator::~Allocator() { }

	VmaAllocation Allocator::allocate_buffer(
		VkBufferCreateInfo buffer_create_info, VmaMemoryUsage usage, VmaAllocationCreateFlags flags, VkBuffer& out_buffer)
	{
		VmaAllocationCreateInfo allocation_create_info = {};
		allocation_create_info.usage = usage;
		allocation_create_info.flags = flags;

		VmaAllocation allocation;
		vk_check(vmaCreateBuffer(vma_data().allocator, &buffer_create_info, &allocation_create_info, &out_buffer, &allocation, nullptr));

		VmaAllocationInfo allocation_info {};
		vmaGetAllocationInfo(vma_data().allocator, allocation, &allocation_info);
		vma_data().total_allocated_bytes += allocation_info.size;

		return allocation;
	}

	VmaAllocation Allocator::allocate_buffer(VkBufferCreateInfo buffer_create_info, VmaMemoryUsage usage, VkBuffer& out_buffer)
	{
		VmaAllocationCreateInfo allocation_create_info = {};
		allocation_create_info.usage = usage;

		VmaAllocation allocation;
		vk_check(vmaCreateBuffer(vma_data().allocator, &buffer_create_info, &allocation_create_info, &out_buffer, &allocation, nullptr));

		VmaAllocationInfo allocation_info {};
		vmaGetAllocationInfo(vma_data().allocator, allocation, &allocation_info);
		vma_data().total_allocated_bytes += allocation_info.size;

		return allocation;
	}

	VmaAllocation Allocator::allocate_image(
		VkImageCreateInfo image_create_info, VmaMemoryUsage usage, VmaAllocationCreateFlags flags, VkImage& out_image)
	{
		VmaAllocationCreateInfo allocation_create_info = {};
		allocation_create_info.usage = usage;
		allocation_create_info.flags = flags;

		VmaAllocation allocation;
		vk_check(vmaCreateImage(vma_data().allocator, &image_create_info, &allocation_create_info, &out_image, &allocation, nullptr));

		VmaAllocationInfo allocation_info;
		vmaGetAllocationInfo(vma_data().allocator, allocation, &allocation_info);
		vma_data().total_allocated_bytes += allocation_info.size;
		return allocation;
	}

	VmaAllocation Allocator::allocate_image(VkImageCreateInfo image_create_info, VmaMemoryUsage usage, VkImage& out_image)
	{
		VmaAllocationCreateInfo allocation_create_info = {};
		allocation_create_info.usage = usage;

		VmaAllocation allocation;
		vk_check(vmaCreateImage(vma_data().allocator, &image_create_info, &allocation_create_info, &out_image, &allocation, nullptr));

		VmaAllocationInfo allocation_info;
		vmaGetAllocationInfo(vma_data().allocator, allocation, &allocation_info);
		vma_data().total_allocated_bytes += allocation_info.size;
		return allocation;
	}

	void Allocator::free(VmaAllocation allocation) { vmaFreeMemory(vma_data().allocator, allocation); }

	void Allocator::destroy_image(VkImage image, VmaAllocation allocation)
	{
		verify(image, "Image is not allocated");
		verify(allocation, "Allocation does not exist");
		vmaDestroyImage(vma_data().allocator, image, allocation);
		Log::info("[Allocator] Destroying image for {}", this->tag);
	}

	void Allocator::destroy_buffer(VkBuffer buffer, VmaAllocation allocation)
	{
		verify(buffer, "Buffer is not allocated");
		verify(allocation, "Allocation does not exist");
		vmaDestroyBuffer(vma_data().allocator, buffer, allocation);
		Log::info("[Allocator] Destroying buffer for {}", this->tag);
	}

	void Allocator::unmap_memory(VmaAllocation allocation) { vmaUnmapMemory(vma_data().allocator, allocation); }

	void Allocator::shutdown() { vmaDestroyAllocator(vma_data().allocator); }

	VmaAllocator& Allocator::get_vma_allocator() { return vma_data().allocator; }

} // namespace Alabaster
