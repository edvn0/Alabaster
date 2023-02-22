#include "av_pch.hpp"

#include "graphics/Mesh.hpp"

#include "core/Clock.hpp"
#include "core/Common.hpp"
#include "graphics/IndexBuffer.hpp"
#include "graphics/Vertex.hpp"
#include "graphics/VertexBuffer.hpp"
#include "utilities/FileInputOutput.hpp"

#include <tiny_obj_loader.h>

#define TINYOBJLOADER_USE_MAPBOX_EARCUT

namespace Alabaster {

	auto handle_vertices(const auto& attrib, const auto& shapes)
	{
		std::unordered_map<Vertex, std::uint32_t> unique_vertices {};
		std::vector<Vertex> vertices;
		vertices.reserve(attrib.vertices.size());

		// Decent metric
		std::vector<Index> indices;
		indices.reserve(shapes.size() * shapes[0].mesh.indices.size());

		for (const auto& shape : shapes) {
			for (const auto& index : shape.mesh.indices) {
				Vertex vertex {};

				vertex.position = { attrib.vertices[3 * index.vertex_index + 0], attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2] };

				const auto u = attrib.texcoords[2 * index.texcoord_index + 0];
				const auto v = 1.0f - attrib.texcoords[2 * index.texcoord_index + 1];
				vertex.uv = { u, v };

				vertex.normal = { attrib.normals[3 * index.normal_index + 0], attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2] };

				vertex.colour = { 1.0f, 1.0f, 1.0f, 1.0f };

				if (!unique_vertices.contains(vertex)) {
					unique_vertices[vertex] = static_cast<std::uint32_t>(vertices.size());
					vertices.push_back(vertex);
				}

				indices.push_back(unique_vertices[vertex]);
			}
		}

		vertices.shrink_to_fit();
		indices.shrink_to_fit();

		return std::make_tuple(vertices, indices);
	}

	Mesh::Mesh(const std::filesystem::path& input_path)
		: path(input_path)
	{
		auto t0 = Clock::get_ms<float>();
		verify(IO::exists(input_path), fmt::format("{} did not exist.", input_path.string()));
		verify(IO::is_file(input_path), fmt::format("{} is not a file.", input_path.string()));

		auto&& [vertices, indices] = load_model();

		index_count = indices.size();

		vertex_buffer = VertexBuffer::create(std::move(vertices));
		index_buffer = IndexBuffer::create(std::move(indices));
		auto t1 = Clock::get_ms<float>() - t0;

		Log::info("[Mesh] Model with name [{}] load took: {}ms", input_path.string(), t1);
	}

	Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<Index>& indices)
	{
		vertex_buffer = VertexBuffer::create(vertices);
		index_buffer = IndexBuffer::create(indices);
	}

	std::tuple<Mesh::Vertices, Mesh::Indices> Mesh::load_model()
	{
		tinyobj::ObjReaderConfig reader_config;
		reader_config.mtl_search_path = IO::textures().string();

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
		// TODO: const auto& materials = reader.GetMaterials();

		return handle_vertices(attrib, shapes);
	}

	Mesh::~Mesh()
	{
		if (!destroyed) {
			destroy();
		}
	}

	void Mesh::destroy()
	{
		if (vertex_buffer)
			vertex_buffer->destroy();

		if (index_buffer)
			index_buffer->destroy();

		destroyed = true;
	}

} // namespace Alabaster
