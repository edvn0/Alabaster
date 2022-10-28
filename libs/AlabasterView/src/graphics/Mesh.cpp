#include "av_pch.hpp"

#include "graphics/Mesh.hpp"

#include "core/Common.hpp"
#include "graphics/IndexBuffer.hpp"
#include "graphics/Vertex.hpp"
#include "graphics/VertexBuffer.hpp"
#include "utilities/FileInputOutput.hpp"

#include <tiny_obj_loader.h>

#define TINYOBJLOADER_USE_MAPBOX_EARCUT

namespace Alabaster {

	Mesh::Mesh(const std::filesystem::path& path)
		: path(path)
	{
		verify(IO::exists(path));
		verify(IO::is_file(path));

		auto&& [vertices, indices] = load_model();

		vertex_buffer = VertexBuffer::create(std::move(vertices));
		index_buffer = IndexBuffer::create(std::move(indices));
	}

	Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<Index>& indices)
	{
		vertex_buffer = VertexBuffer::create(vertices);
		index_buffer = IndexBuffer::create(indices);
	}

	std::tuple<Mesh::Vertices, Mesh::Indices> Mesh::load_model()
	{
		tinyobj::ObjReaderConfig reader_config;
		reader_config.mtl_search_path = path.parent_path().string().c_str();

		tinyobj::ObjReader reader;

		if (!reader.ParseFromFile(path.string().c_str(), reader_config)) {
			if (!reader.Error().empty()) {
				Log::error("{}", reader.Error());
			}
		}

		if (!reader.Warning().empty()) {
			Log::warn("{}", reader.Warning());
		}

		const auto& attrib = reader.GetAttrib();
		const auto& shapes = reader.GetShapes();
		const auto& materials = reader.GetMaterials();

		std::unordered_map<Vertex, uint32_t> unique_vertices {};

		std::vector<Vertex> vertices;
		vertices.reserve(attrib.vertices.size());

		// Decent metric
		std::vector<Index> indices;
		indices.reserve(shapes.size() * shapes[0].mesh.indices.size());

		for (const auto& shape : shapes) {
			for (const auto& index : shape.mesh.indices) {
				Vertex vertex {};

				vertex.position = { attrib.vertices[3 * index.vertex_index + 0], attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2], 0 };

				const auto u = attrib.texcoords[2 * index.texcoord_index + 0];
				const auto v = 1.0f - attrib.texcoords[2 * index.texcoord_index + 1];
				vertex.uv = { u, v };

				vertex.colour = { 1.0f, 1.0f, 1.0f, 1.0f };

				if (!unique_vertices.contains(vertex)) {
					unique_vertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.push_back(vertex);
				}

				indices.push_back(unique_vertices[vertex]);
			}
		}

		vertices.shrink_to_fit();
		indices.shrink_to_fit();

		return { vertices, indices };
	}

	std::unique_ptr<Mesh> Mesh::from_path(std::string path) { return std::make_unique<Mesh>(IO::slashed_to_fp(path)); }
	std::unique_ptr<Mesh> Mesh::from_data(const std::vector<Vertex>& vertices, const std::vector<Index>& indices)
	{
		return std::make_unique<Mesh>(vertices, indices);
	}

	void Mesh::destroy()
	{
		vertex_buffer->destroy();
		index_buffer->destroy();
	}

} // namespace Alabaster
