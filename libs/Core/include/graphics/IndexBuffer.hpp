#pragma once

#include "graphics/Buffer.hpp"

#include <array>
#include <vector>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace Alabaster {

	using Index = uint32_t;

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

		void set_data(const void* buffer, uint32_t size, uint32_t offset);

		uint32_t count() const { return buffer_count; };

		VkBuffer get_vulkan_buffer() const { return vulkan_buffer; }

		VkBuffer operator*() const { return vulkan_buffer; }

	public:
		inline static std::unique_ptr<IndexBuffer> create(std::vector<Index>&& indices)
		{
			return std::make_unique<IndexBuffer>(indices.data(), static_cast<uint32_t>(indices.size()));
		}

		inline static std::unique_ptr<IndexBuffer> create(const std::vector<Index>& indices)
		{
			return std::make_unique<IndexBuffer>(indices.data(), static_cast<uint32_t>(indices.size()));
		}

		inline static std::unique_ptr<IndexBuffer> create(std::size_t count) { return std::make_unique<IndexBuffer>(static_cast<uint32_t>(count)); }

	private:
		Buffer index_data;
		VmaAllocation memory_allocation;

		uint32_t buffer_size { 0 };
		uint32_t buffer_count { 0 };

		VkBuffer vulkan_buffer { nullptr };

		bool destroyed { false };

	private:
		void offline_set_data(const void* buffer, uint32_t size, uint32_t offset);
	};

} // namespace Alabaster
