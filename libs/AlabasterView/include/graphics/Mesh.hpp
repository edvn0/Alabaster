#pragma once

#include "graphics/Vertex.hpp"

#include <filesystem>
#include <memory>
#include <unordered_map>

namespace Alabaster {

	class VertexBuffer;
	class IndexBuffer;

	class Mesh {
		using VertexMap = std::unordered_map<Vertex, uint32_t>;
		using Indices = std::vector<Index>;
		using Vertices = std::vector<Vertex>;

	public:
		explicit Mesh(const std::filesystem::path& path);

		const VertexBuffer& get_vertex_buffer() const { return *vertex_buffer; }
		const IndexBuffer& get_index_buffer() const { return *index_buffer; }

		void destroy();

	private:
		std::filesystem::path path;

		std::unique_ptr<VertexBuffer> vertex_buffer;
		std::unique_ptr<IndexBuffer> index_buffer;

		size_t vertex_count { 0 };
		size_t index_count { 0 };

		std::tuple<Vertices, Indices> load_model();

	public:
		static std::unique_ptr<Mesh> from_path(std::string args);
	};

} // namespace Alabaster
