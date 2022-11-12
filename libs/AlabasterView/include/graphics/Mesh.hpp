#pragma once

#include "glm/fwd.hpp"
#include "graphics/IndexBuffer.hpp"
#include "graphics/Vertex.hpp"
#include "graphics/VertexBuffer.hpp"

#include <filesystem>
#include <memory>
#include <unordered_map>

namespace Alabaster {

	class VertexBuffer;
	class IndexBuffer;

	class Mesh {
		using Indices = std::vector<Index>;
		using Vertices = std::vector<Vertex>;

	public:
		explicit Mesh(const std::filesystem::path& path);
		Mesh(const std::vector<Vertex>& vertices, const std::vector<Index>& indices);

		const VertexBuffer& get_vertex_buffer() const { return *vertex_buffer; }
		const IndexBuffer& get_index_buffer() const { return *index_buffer; }

		void set_transform(glm::mat4&& input) { transform = input; }
		void reset_transform() { transform.reset(); }
		const auto& get_transform() { return transform; }

		size_t get_index_count() const { return index_count; }

		void destroy();

	private:
		std::filesystem::path path;

		std::unique_ptr<VertexBuffer> vertex_buffer;
		std::unique_ptr<IndexBuffer> index_buffer;

		std::optional<glm::mat4> transform { std::nullopt };

		size_t vertex_count { 0 };
		size_t index_count { 0 };

		std::tuple<Vertices, Indices> load_model();

	public:
		static std::unique_ptr<Mesh> from_file(std::string args);
		static std::unique_ptr<Mesh> from_data(const std::vector<Vertex>& vertices, const std::vector<Index>& indices);
	};

} // namespace Alabaster
