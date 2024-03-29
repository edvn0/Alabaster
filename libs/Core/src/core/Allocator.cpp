#include "av_pch.hpp"

#include "graphics/Allocator.hpp"

#include "core/Common.hpp"
#include "graphics/GraphicsContext.hpp"

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

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

	Allocator::~Allocator() = default;

	VmaAllocation Allocator::allocate_buffer(
		VkBufferCreateInfo buffer_create_info, Usage usage, Creation flags, VkBuffer& out_buffer, std::string_view name)
	{
		VmaAllocationCreateInfo allocation_create_info = {};
		allocation_create_info.usage = static_cast<VmaMemoryUsage>(usage);
		allocation_create_info.flags = static_cast<VmaAllocationCreateFlags>(flags);

		VmaAllocation allocation;
		vk_check(vmaCreateBuffer(vma_data().allocator, &buffer_create_info, &allocation_create_info, &out_buffer, &allocation, nullptr));
		vmaSetAllocationName(vma_data().allocator, allocation, name.data());

		VmaAllocationInfo allocation_info {};
		vmaGetAllocationInfo(vma_data().allocator, allocation, &allocation_info);
		vma_data().total_allocated_bytes += allocation_info.size;

		return allocation;
	}

	VmaAllocation Allocator::allocate_buffer(VkBufferCreateInfo buffer_create_info, Usage usage, VkBuffer& out_buffer, std::string_view name)
	{
		VmaAllocationCreateInfo allocation_create_info = {};
		allocation_create_info.usage = static_cast<VmaMemoryUsage>(usage);

		VmaAllocation allocation;
		vk_check(vmaCreateBuffer(vma_data().allocator, &buffer_create_info, &allocation_create_info, &out_buffer, &allocation, nullptr));
		vmaSetAllocationName(vma_data().allocator, allocation, name.data());

		VmaAllocationInfo allocation_info {};
		vmaGetAllocationInfo(vma_data().allocator, allocation, &allocation_info);
		vma_data().total_allocated_bytes += allocation_info.size;
		return allocation;
	}

	VmaAllocation Allocator::allocate_image(
		VkImageCreateInfo image_create_info, Usage usage, Creation flags, VkImage& out_image, std::string_view name)
	{
		VmaAllocationCreateInfo allocation_create_info = {};
		allocation_create_info.usage = static_cast<VmaMemoryUsage>(usage);
		allocation_create_info.flags = static_cast<VmaAllocationCreateFlags>(flags);

		VmaAllocation allocation;
		vk_check(vmaCreateImage(vma_data().allocator, &image_create_info, &allocation_create_info, &out_image, &allocation, nullptr));
		vmaSetAllocationName(vma_data().allocator, allocation, name.data());

		VmaAllocationInfo allocation_info;
		vmaGetAllocationInfo(vma_data().allocator, allocation, &allocation_info);
		vma_data().total_allocated_bytes += allocation_info.size;
		return allocation;
	}

	VmaAllocation Allocator::allocate_image(VkImageCreateInfo image_create_info, Usage usage, VkImage& out_image, std::string_view name)
	{
		VmaAllocationCreateInfo allocation_create_info = {};
		allocation_create_info.usage = static_cast<VmaMemoryUsage>(usage);

		VmaAllocation allocation;
		vk_check(vmaCreateImage(vma_data().allocator, &image_create_info, &allocation_create_info, &out_image, &allocation, nullptr));
		vmaSetAllocationName(vma_data().allocator, allocation, name.data());

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
