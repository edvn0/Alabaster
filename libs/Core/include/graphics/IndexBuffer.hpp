#pragma once

#include "core/Buffer.hpp"

#include <array>
#include <vector>

using VmaAllocation = struct VmaAllocation_T*;
using VkBuffer = struct VkBuffer_T*;

namespace Alabaster {

	using Index = std::uint32_t;

	class IndexBuffer {
	public:
		~IndexBuffer();

		void set_data(const void* buffer, std::uint32_t size, std::uint32_t offset);

		std::uint32_t count() const { return buffer_count; };

		VkBuffer get_vulkan_buffer() const;

		VkBuffer operator*() const;

	public:
		inline static std::shared_ptr<IndexBuffer> create(std::vector<Index>&& indices)
		{
			return std::shared_ptr<IndexBuffer>(new IndexBuffer { indices.data(), static_cast<std::uint32_t>(indices.size()) });
		}

		inline static std::shared_ptr<IndexBuffer> create(const std::vector<Index>& indices)
		{
			return std::shared_ptr<IndexBuffer>(new IndexBuffer { indices.data(), static_cast<std::uint32_t>(indices.size()) });
		}

		inline static std::shared_ptr<IndexBuffer> create(std::size_t count)
		{
			return std::shared_ptr<IndexBuffer>(new IndexBuffer { static_cast<std::uint32_t>(count) });
		}

	private:
		Buffer index_data;
		VmaAllocation memory_allocation;

		std::uint32_t buffer_size { 0 };
		std::uint32_t buffer_count { 0 };

		VkBuffer vulkan_buffer { nullptr };

		explicit IndexBuffer(std::uint32_t count);
		IndexBuffer(const void* data, std::uint32_t count);

		void offline_set_data(const void* buffer, std::uint32_t size, std::uint32_t offset);
	};

} // namespace Alabaster
