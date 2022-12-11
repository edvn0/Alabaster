//
// Created by Edwin Carlsson on 2022-10-11.
//

#pragma once

#include "core/Buffer.hpp"
#include "graphics/Vertex.hpp"

#include <vulkan/vulkan.h>

typedef struct VmaAllocation_T* VmaAllocation;

namespace Alabaster {

	class VertexBuffer {
	public:
		explicit VertexBuffer(std::uint32_t size);

		VertexBuffer(const void* data, std::uint32_t size);
		VertexBuffer(const void* data, std::size_t size);

		~VertexBuffer()
		{
			if (!destroyed)
				destroy();
		}

		void destroy();

		void set_data(const void* data, std::uint32_t size, std::uint32_t offset);

		VkBuffer get_vulkan_buffer() const { return vulkan_buffer; }

		VkBuffer operator*() const { return vulkan_buffer; }

	private:
		void offline_set_data(const void* buffer, std::uint32_t size, std::uint32_t offset);

	public:
		inline static std::unique_ptr<VertexBuffer> create(std::vector<Vertex>&& vertices)
		{
			const std::uint32_t size = static_cast<std::uint32_t>(vertices.size() * sizeof(Vertex));
			return std::make_unique<VertexBuffer>(vertices.data(), size);
		}

		inline static std::unique_ptr<VertexBuffer> create(const std::vector<Vertex>& vertices)
		{
			return std::make_unique<VertexBuffer>(vertices.data(), vertices.size() * sizeof(Vertex));
		}

		inline static std::unique_ptr<VertexBuffer> create(std::size_t size)
		{
			return std::make_unique<VertexBuffer>(static_cast<std::uint32_t>(size));
		}

	private:
		Buffer vertex_data;
		std::uint32_t buffer_size;
		VmaAllocation memory_allocation;
		VkBuffer vulkan_buffer { nullptr };

		bool destroyed { false };
	};

} // namespace Alabaster
