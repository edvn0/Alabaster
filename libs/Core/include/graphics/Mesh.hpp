#pragma once

#include "filesystem/FileSystem.hpp"
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
		~Mesh();

		const VertexBuffer& get_vertex_buffer() const { return *vertex_buffer; }
		const IndexBuffer& get_index_buffer() const { return *index_buffer; }

		std::size_t get_index_count() const { return index_count; }

		const auto& get_asset_path() const { return path; }

	private:
		explicit Mesh(const std::filesystem::path& input_path);
		Mesh(const std::vector<Vertex>& vertices, const std::vector<Index>& indices);

		std::filesystem::path path;

		std::shared_ptr<VertexBuffer> vertex_buffer;
		std::shared_ptr<IndexBuffer> index_buffer;

		std::optional<glm::mat4> transform { std::nullopt };
		std::optional<glm::vec4> colour { std::nullopt };
		std::optional<glm::mat4> scale { std::nullopt };

		std::size_t index_count { 0 };

		std::tuple<Vertices, Indices> load_model();

	public:
		static std::shared_ptr<Mesh> from_file(const std::filesystem::path& args)
		{
			return std::shared_ptr<Mesh>(new Mesh { FileSystem::model(args) });
		};
		static std::shared_ptr<Mesh> from_data(const std::vector<Vertex>& vertices, const std::vector<Index>& indices)
		{
			return std::shared_ptr<Mesh>(new Mesh { vertices, indices });
		};
	};

} // namespace Alabaster
