#pragma once

#include "graphics/Camera.hpp"
#include "graphics/CommandBuffer.hpp"
#include "SimpleCamera.hpp"

#include <array>
#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

namespace Alabaster {

	class Mesh;
	class Pipeline;
	class VertexBuffer;
	class IndexBuffer;

	struct RenderProps {
		glm::vec3 pos;
		glm::vec4 colour;
		glm::vec3 scale;
	};

	struct QuadVertex {
		glm::vec4 position;
		glm::vec4 colour;
		glm::vec2 uvs;
	};

	struct LineVertex {
		glm::vec4 position;
		glm::vec4 colour;
	};

	struct UBO {
		alignas(16) glm::mat4 model;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 projection;
		alignas(16) glm::mat4 view_projection;
	};

	struct RendererData {
		static constexpr size_t max_vertices = 20000;
		static constexpr size_t max_indices = 6 * max_vertices;

		size_t indices_submitted { 0 };
		size_t vertices_submitted { 0 };
		size_t draw_calls { 0 };

		std::unique_ptr<Pipeline> quad_pipeline;
		std::array<QuadVertex, max_vertices> quad_buffer;
		QuadVertex* quad_buffer_ptr;
		std::array<glm::vec4, 4> quad_positions;
		std::unique_ptr<VertexBuffer> quad_vertex_buffer;
		std::unique_ptr<IndexBuffer> quad_index_buffer;

		std::unique_ptr<Pipeline> line_pipeline;
		std::array<LineVertex, max_vertices> line_buffer;
		LineVertex* line_buffer_ptr;
		std::unique_ptr<VertexBuffer> line_vertex_buffer;
		std::unique_ptr<IndexBuffer> line_index_buffer;
		size_t line_indices_submitted { 0 };
		size_t line_vertices_submitted { 0 };

		std::vector<VkBuffer> uniform_buffers;
		std::vector<VkDeviceMemory> uniform_buffers_memory;

		std::vector<VkDescriptorSet> descriptor_sets;
		VkDescriptorSetLayout descriptor_set_layout;
		VkDescriptorPool descriptor_pool;

		VkRenderPass render_pass;

		Mesh* mesh;
		Pipeline* mesh_pipeline_submit;
		std::unique_ptr<Pipeline> mesh_pipeline;
	};

	class Renderer3D {
	public:
		explicit Renderer3D(Camera& camera) noexcept;

		void begin_scene();
		void quad(const glm::vec4& pos = { 0, 0, 0, 0 }, const glm::vec4& colour = { 1, 1, 1, 1 }, const glm::vec3& scale = { 1, 1, 1 },
			float rotation_degrees = 0.0f);
		void mesh(const std::unique_ptr<Mesh>& mesh, const std::unique_ptr<Pipeline>& pipeline = nullptr, const glm::vec4& pos = { 0, 0, 0, 0 },
			const glm::vec4& colour = { 1, 1, 1, 1 }, const glm::vec3& scale = { 1, 1, 1 });
		void line(const glm::vec3& from, const glm::vec3& to, const glm::vec4& color);
		void end_scene();

		void reset_stats();

		void set_camera(Camera& cam) { camera = cam; }

	private:
		void draw_quads();
		void draw_lines();

	private:
		void flush();
		void update_uniform_buffers();
		void create_descriptor_set_layout();
		void create_descriptor_pool();
		void create_descriptor_sets();
		void create_renderpass();

	private:
		Camera& camera;
		RendererData data;
		CommandBuffer command_buffer;
		void draw_meshes();
	};

} // namespace Alabaster
