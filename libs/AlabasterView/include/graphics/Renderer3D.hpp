#pragma once

#include "graphics/Camera.hpp"
#include "graphics/CommandBuffer.hpp"
#include "graphics/UniformBuffer.hpp"
#include "SimpleCamera.hpp"

#include <array>
#include <glm/gtx/transform.hpp>
#include <memory>
#include <optional>
#include <vector>
#include <vulkan/vulkan.h>

namespace Alabaster {

	// static constexpr auto default_model =glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	class Mesh;
	class Pipeline;
	class VertexBuffer;
	class IndexBuffer;
	class Image;

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
		static constexpr uint32_t max_vertices = 20000;
		static constexpr uint32_t max_indices = 6 * max_vertices;
		uint32_t draw_calls { 0 };

		uint32_t quad_indices_submitted { 0 };
		uint32_t quad_vertices_submitted { 0 };
		std::unique_ptr<Pipeline> quad_pipeline;
		std::array<QuadVertex, max_vertices> quad_buffer;
		std::unique_ptr<VertexBuffer> quad_vertex_buffer;
		std::unique_ptr<IndexBuffer> quad_index_buffer;

		uint32_t line_indices_submitted { 0 };
		uint32_t line_vertices_submitted { 0 };
		std::unique_ptr<Pipeline> line_pipeline;
		std::array<LineVertex, max_vertices> line_buffer;
		std::unique_ptr<VertexBuffer> line_vertex_buffer;
		std::unique_ptr<IndexBuffer> line_index_buffer;

		std::vector<UniformBuffer> uniforms;

		std::vector<VkDescriptorSet> descriptor_sets;
		VkDescriptorSetLayout descriptor_set_layout;
		VkDescriptorPool descriptor_pool;
		VkRenderPass render_pass;

		uint32_t meshes_submitted { 0 };
		std::array<Mesh*, 50> mesh;
		std::array<Pipeline*, 50> mesh_pipeline_submit;
		std::unique_ptr<Pipeline> mesh_pipeline;

		std::unique_ptr<Image> viking_room_texture;
	};

	class Renderer3D {
	public:
		explicit Renderer3D(Camera& camera) noexcept;

		void begin_scene();
		void quad(const glm::vec3& pos = { 0, 0, 0 }, const glm::vec4& colour = { 1, 1, 1, 1 }, const glm::vec3& scale = { 1, 1, 1 },
			float rotation_degrees = 0.0f);

		void mesh(const std::unique_ptr<Mesh>& mesh, const std::unique_ptr<Pipeline>& pipeline = nullptr, const glm::vec3& pos = { 0, 0, 0 },
			const glm::mat4& rotation_matrix = glm::mat4 { 1.0f }, const glm::vec4& colour = { 1, 1, 1, 1 }, const glm::vec3& scale = { 1, 1, 1 });
		void line(const glm::vec3& from, const glm::vec3& to, const glm::vec4& color);
		void line(float size, const glm::vec3& from, const glm::vec3& to, const glm::vec4& color);
		void end_scene();

	public:
		void destroy();
		void reset_stats();

	public:
		void set_camera(Camera& cam) { camera = cam; }

	public:
		const VkRenderPass& get_render_pass() const;

	private:
		void draw_quads();
		void draw_lines();
		void draw_meshes();

	private:
		void flush();
		void update_uniform_buffers(const std::optional<glm::mat4>& model = {});
		void create_descriptor_set_layout();
		void create_descriptor_pool();
		void create_descriptor_sets();
		void create_renderpass();

	private:
		Camera& camera;
		RendererData data;
		CommandBuffer command_buffer;
	};

} // namespace Alabaster
