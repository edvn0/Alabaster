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

		void set_transform(glm::mat4&& input) { transform = std::move(input); }
		void reset_transform() { transform.reset(); }
		const auto& get_transform() { return transform; }

		void set_colour(const glm::vec4& input) { colour = input; }
		void reset_colour() { colour.reset(); }
		const auto& get_colour() { return colour; }

		void set_scale(const glm::mat4& input) { scale = input; }
		void reset_scale() { scale.reset(); }
		const auto& get_scale() { return scale; }

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

		std::size_t vertex_count { 0 };
		std::size_t index_count { 0 };

		bool destroyed { false };

	private:
		std::tuple<Vertices, Indices> load_model();

	public:
		static std::unique_ptr<Mesh> from_file(const std::filesystem::path& args) { return std::make_unique<Mesh>(IO::model(std::move(args))); };
		static std::unique_ptr<Mesh> from_data(const std::vector<Vertex>& vertices, const std::vector<Index>& indices)
		{
			return std::make_unique<Mesh>(vertices, indices);
		};
	};

} // namespace Alabaster
