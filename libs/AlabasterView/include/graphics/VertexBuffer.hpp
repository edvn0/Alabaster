//
// Created by Edwin Carlsson on 2022-10-11.
//

#pragma once

#include "graphics/Buffer.hpp"
#include "graphics/Vertex.hpp"

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace Alabaster {

	class VertexBuffer {
	public:
		explicit VertexBuffer(uint32_t size);

		VertexBuffer(const void* data, uint32_t size);
		VertexBuffer(const void* data, size_t size);

		~VertexBuffer()
		{
			if (!destroyed)
				destroy();
		}

		void destroy();

		VkBuffer get_vulkan_buffer() const { return vulkan_buffer; }

	public:
		inline static std::unique_ptr<VertexBuffer> create(std::vector<Vertex>&& vertices)
		{
			return std::make_unique<VertexBuffer>(vertices.data(), vertices.size() * sizeof(Vertex));
		}

		inline static std::unique_ptr<VertexBuffer> create(const std::vector<Vertex>& vertices)
		{
			return std::make_unique<VertexBuffer>(vertices.data(), vertices.size() * sizeof(Vertex));
		}

	private:
		Buffer vertex_data;
		uint32_t buffer_size;
		VmaAllocation memory_allocation;
		VkBuffer vulkan_buffer { nullptr };

		bool destroyed { false };
	};

} // namespace Alabaster
