//
// Created by Edwin Carlsson on 2022-10-11.
//

#pragma once

#include "core/Buffer.hpp"
#include "graphics/Vertex.hpp"

using VkBuffer = struct VkBuffer_T*;
using VmaAllocation = struct VmaAllocation_T*;

namespace Alabaster {

	class VertexBuffer {
	public:
		~VertexBuffer();

		void set_data(const void* data, std::uint32_t size, std::uint32_t offset) const;
		void set_data(const void* data, const std::size_t size, const std::size_t offset) const;

		VkBuffer get_vulkan_buffer() const;
		VkBuffer operator*() const;

		inline static std::shared_ptr<VertexBuffer> create(std::vector<Vertex>&& vs)
		{
			const auto&& vertices = std::move(vs);
			const auto size = static_cast<std::uint32_t>(vertices.size() * sizeof(Vertex));
			return std::shared_ptr<VertexBuffer>(new VertexBuffer { vertices.data(), size });
		}

		inline static std::shared_ptr<VertexBuffer> create(const std::vector<Vertex>& vertices)
		{
			return std::shared_ptr<VertexBuffer>(new VertexBuffer { vertices.data(), vertices.size() * sizeof(Vertex) });
		}

		inline static std::shared_ptr<VertexBuffer> create(std::size_t size)
		{
			return std::shared_ptr<VertexBuffer>(new VertexBuffer { static_cast<std::uint32_t>(size) });
		}

	private:
		explicit VertexBuffer(std::uint32_t size);

		VertexBuffer(const void* data, std::uint32_t size);
		VertexBuffer(const void* data, std::size_t size);

		void offline_set_data(const void* buffer, std::uint32_t size, std::uint32_t offset) const;
		void offline_set_data(const void* buffer, const std::size_t size, const std::size_t offset) const;

		Buffer vertex_data;
		std::uint32_t buffer_size;
		VmaAllocation memory_allocation;
		VkBuffer vulkan_buffer { nullptr };
	};

} // namespace Alabaster
