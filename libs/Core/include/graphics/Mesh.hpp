#pragma once

#include "glm/fwd.hpp"
#include "graphics/IndexBuffer.hpp"
#include "graphics/Vertex.hpp"
#include "graphics/VertexBuffer.hpp"
#include "utilities/FileInputOutput.hpp"

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
		~Mesh();

		const VertexBuffer& get_vertex_buffer() const { return *vertex_buffer; }
		const IndexBuffer& get_index_buffer() const { return *index_buffer; }

		std::size_t get_index_count() const { return index_count; }

		void destroy();

		const auto& get_asset_path() const { return path; }

	private:
		std::filesystem::path path;

		std::unique_ptr<VertexBuffer> vertex_buffer;
		std::unique_ptr<IndexBuffer> index_buffer;

		std::optional<glm::mat4> transform { std::nullopt };
		std::optional<glm::vec4> colour { std::nullopt };
		std::optional<glm::mat4> scale { std::nullopt };

		std::size_t index_count { 0 };

		bool destroyed { false };

	private:
		std::tuple<Vertices, Indices> load_model();

	public:
		static std::shared_ptr<Mesh> from_file(const std::filesystem::path& args) { return std::make_shared<Mesh>(IO::model(args)); };
		static std::shared_ptr<Mesh> from_data(const std::vector<Vertex>& vertices, const std::vector<Index>& indices)
		{
			return std::make_shared<Mesh>(vertices, indices);
		};
	};

} // namespace Alabaster
